#include "Scene.hpp"
#include <cpplocate/cpplocate.h>
#include <RTUtil/Camera.hpp>
#include "Utils.hpp"

#include <cfloat>

/*
https://monkeyproofsolutions.nl/wordpress/how-to-calculate-the-shortest-distance-between-a-point-and-a-line/
*/

const std::string resourcePath =
    cpplocate::locatePath("resources/Common", "", nullptr) + "resources/";

Scene::Scene(int w, int h, const aiScene *importedSceneOriginal, RTUtil::SceneInfo sceneInfo, std::shared_ptr<RTUtil::PerspectiveCamera> camera) : sky(80 * M_PI / 180, 3), defaultCamera(camera),
                                                                                                                                                   prog("program", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/min.vs"},
                                                                                                                                                                    {GL_GEOMETRY_SHADER, resourcePath + "Common/shaders/flat.gs"},
                                                                                                                                                                    {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/lambert.fs"}})
{
    // Create light objects
    for (std::shared_ptr<RTUtil::LightInfo> lightInfo : sceneInfo.lights)
    {

        if (lightInfo->type == RTUtil::LightType::Ambient)
        {
            ambientLights.push_back(AmbientLight(lightInfo));
        }
        else
        {
            if (lightInfo->type == RTUtil::LightType::Area)
            {
                areaLights.push_back(AreaLight(importedSceneOriginal, lightInfo));
            }
            else
            {
                pointLights.push_back(PositionalLight(importedSceneOriginal, lightInfo));
            }
        }
    }

    rootNode = createHierarchy(importedSceneOriginal, importedSceneOriginal->mRootNode, nullptr, sceneInfo);
}

void Scene::draw(std::unique_ptr<GLWrap::Program> &prog, bool useUniform)
{
    drawWithCamera(prog, *defaultCamera, useUniform);
}

void Scene::drawWithCamera(std::unique_ptr<GLWrap::Program> &prog, const RTUtil::PerspectiveCamera &camera, bool useUniform)
{
    prog->use();

    drawFromNode(rootNode, Eigen::Affine3f::Identity(), prog, camera, useUniform);
    prog->unuse();
}

unsigned int Scene::numPointLights()
{
    return pointLights.size();
}

PositionalLight Scene::getLight(unsigned int idx)
{
    return pointLights[idx];
}

bool Scene::hasAmbientLight() const
{
    return ambientLights.size() > 0;
}

AmbientLight Scene::getAmbientLight() const
{
    if (ambientLights.size() > 0)
    {
        return ambientLights[0];
    }
    else
    {
        return AmbientLight(Eigen::Vector3f(0, 0, 0), 1);
    }
}

RTUtil::Sky Scene::getSky() const
{
    return sky;
}

// Private functions

void Scene::drawFromNode(std::shared_ptr<Node> node, Eigen::Affine3f transformation, std::unique_ptr<GLWrap::Program> &prog, const RTUtil::PerspectiveCamera &cam, bool useUniforms)
{
    Eigen::Affine3f objTransformation = transformation * node->transformation;

    if (useUniforms)
    {
        prog->uniform("mM", objTransformation.matrix());
        prog->uniform("mV", cam.getViewMatrix().matrix());
        prog->uniform("mP", cam.getProjectionMatrix().matrix());
    }

    if (pointLights.size() > 0)
    {
        PositionalLight light = pointLights[0];

        if (useUniforms)
        {
            prog->uniform("mL", light.getNodeToWorldTransform().matrix());
            prog->uniform("lightPosition", light.getPos());
            prog->uniform("lightPower", light.getPower());
        }
    }
    else
    {
        //DEFAULT POINT LIGHT

        if (useUniforms)
        {
            prog->uniform("mL", Eigen::Affine3f::Identity().matrix());
            prog->uniform("lightPosition", Eigen::Vector3f(1.0, 2.0, 0.0));
            prog->uniform("lightPower", Eigen::Vector3f(300, 300, 300));
        }
    }

    if (useUniforms)
    {
        prog->uniform("k_a", Eigen::Vector3f(0.1, 0.1, 0.1));
        prog->uniform("k_d", Eigen::Vector3f(0.9, 0.9, 0.9));
        prog->uniform("lightDir", Eigen::Vector3f(1.0, 1.0, 1.0).normalized());
    }

    for (std::shared_ptr<GLWrap::Mesh> mesh : node->meshes)
    {

        auto bsdfIt = meshToMaterialMap.find(mesh);

        if (bsdfIt != meshToMaterialMap.end())
        {
            // This scares me. I hope this doesn't ever throw exceptions
            auto microfacet = std::dynamic_pointer_cast<nori::Microfacet>(bsdfIt->second);

            if (microfacet == nullptr)
            {
                continue;
            }

            if (useUniforms)
            {
                prog->uniform("refractiveIndex", microfacet->eta());
                prog->uniform("roughness", microfacet->alpha());
                prog->uniform("diffuseReflectance", microfacet->diffuseReflectance());
            }
        }

        mesh->drawElements();
    }

    for (std::shared_ptr<Node> child : node->children)
    {
        drawFromNode(child, objTransformation, prog, cam, useUniforms);
    }
}

std::shared_ptr<Node> Scene::createHierarchy(const aiScene *importedScene, aiNode *node, std::shared_ptr<Node> parent, const RTUtil::SceneInfo &sceneInfo)
{
    // Eigen::Affine3f nodeToWorld = transformation * RTUtil::a2e(node->mTransformation);
    // std::cout << "a\n";

    nodes.push_back(createNode(importedScene, node, parent, sceneInfo));

    auto newNode = std::make_shared<Node>(nodes.back());

    for (int childIdx = 0; childIdx < node->mNumChildren; childIdx++)
    {
        std::shared_ptr<Node> child = createHierarchy(importedScene, node->mChildren[childIdx], newNode, sceneInfo);
        newNode->children.push_back(child);
    }

    return newNode;
}

Node Scene::createNode(const aiScene *importedScene, aiNode *sceneNode, std::shared_ptr<Node> parent, const RTUtil::SceneInfo &sceneInfo)
{
    std::vector<std::shared_ptr<GLWrap::Mesh>> meshes;
    Eigen::Affine3f transformation = RTUtil::a2e(sceneNode->mTransformation);

    for (int meshIdx = 0; meshIdx < sceneNode->mNumMeshes; meshIdx++)
    {
        aiMesh *meshData = importedScene->mMeshes[sceneNode->mMeshes[meshIdx]];

        std::shared_ptr<GLWrap::Mesh> mesh = std::make_shared<GLWrap::Mesh>();

        Eigen::Matrix<float, 3, Eigen::Dynamic> positions(3, meshData->mNumVertices);
        Eigen::Matrix<float, 3, Eigen::Dynamic> normals(3, meshData->mNumVertices);
        for (int i = 0; i < meshData->mNumVertices; i++)
        {
            positions.col(i) << meshData->mVertices[i].x, meshData->mVertices[i].y, meshData->mVertices[i].z;
            // std::cout << "M NORMALS:  " << meshData->mNormals[i].x << meshData->mNormals[i].y << meshData->mNormals[i].z << std::endl;
            normals.col(i) << meshData->mNormals[i].x, meshData->mNormals[i].y, meshData->mNormals[i].z;
        }

        mesh->setAttribute(0, positions);
        mesh->setAttribute(1, normals);

        // Add indices to the mesh
        Eigen::VectorXi indices(3 * (int)meshData->mNumFaces);
        for (int i = 0; i < meshData->mNumFaces; i++)
        {
            indices[3 * i + 0] = (int)meshData->mFaces[i].mIndices[0];
            indices[3 * i + 1] = (int)meshData->mFaces[i].mIndices[1];
            indices[3 * i + 2] = (int)meshData->mFaces[i].mIndices[2];
        }

        mesh->setIndices(indices, GL_TRIANGLES);

        meshes.push_back(mesh);

        // rtcCommitGeometry(meshGeometry);
        // unsigned int geomID = rtcAttachGeometry(scene, meshGeometry);

        aiMaterial *aiMat = importedScene->mMaterials[meshData->mMaterialIndex];
        aiString meshMaterialName = aiMat->GetName();

        // std::cout << "material name" << std::string(meshMaterialName.C_Str()) << std::endl;
        // std::string matName = meshMaterialName.C_Str();
        // std::transform(matName.begin(), matName.end(), matName.begin(),
        //                [](unsigned char c) { return std::tolower(c); });
        // std::cout << "material name " << sceneNode->mName.C_Str() << std::endl;
        // std::cout << " named material keys " << sceneInfo.namedMaterials.

        auto materialFromMatName = sceneInfo.namedMaterials.find(meshMaterialName.C_Str());
        auto materialFromNodeName = sceneInfo.nodeMaterials.find(sceneNode->mName.C_Str());
        if (materialFromMatName != sceneInfo.namedMaterials.end())
        {
            meshToMaterialMap.insert(std::make_pair(mesh, materialFromMatName->second));
        }
        else if (materialFromNodeName != sceneInfo.nodeMaterials.end())
        {
            meshToMaterialMap.insert(std::make_pair(mesh, materialFromNodeName->second));
        }
        else
        {
            auto parentMaterial = sceneInfo.nodeMaterials.find(sceneNode->mParent->mName.C_Str());
            if (parentMaterial != sceneInfo.nodeMaterials.end())
            {
                meshToMaterialMap.insert(std::make_pair(mesh, parentMaterial->second));
            }
            else
            {
                meshToMaterialMap.insert(std::make_pair(mesh, sceneInfo.defaultMaterial));
            }
        }
        // meshToMaterialMap.insert(std::make_pair(geomID, sceneNode->));
        // rtcReleaseGeometry(meshGeometry);
    }

    // std::cout << "III\n";
    return Node(meshes, transformation, parent);
}