#pragma once

#include <RTUtil/Camera.hpp>
#include <RTUtil/CameraController.hpp>
#include <RTUtil/Sky.hpp>
#include <GLWrap/Mesh.hpp>
#include <GLWrap/Program.hpp>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <map>
#include <RTUtil/sceneinfo.hpp>
#include <RTUtil/microfacet.hpp>
#include <assimp/scene.h>

#include "PositionalLight.hpp"
#include "AreaLight.hpp"
#include "AmbientLight.hpp"
#include "Node.hpp"

/* 
Camera 
Lists of materials and lights (same) 
Pointer to the root node 
*/

class Scene
{

public:
    Scene(int w, int h, const aiScene *importedSceneOriginal, RTUtil::SceneInfo sceneInfo, std::shared_ptr<RTUtil::PerspectiveCamera> cam);
    // RTUtil::PerspectiveCamera camera() const;

    void draw(std::unique_ptr<GLWrap::Program> &prog, bool useUniform = true);
    void drawWithCamera(std::unique_ptr<GLWrap::Program> &prog, const RTUtil::PerspectiveCamera &camera, bool useUniform);

    /**
     * Get the number of point lights in the scene
     * 
     * @return The number of point lights
     */
    unsigned int numPointLights();

    /**
     * Get the point light at a particular index from the scene
     * 
     * Requires: idx must be less than numPointLights()
     * 
     * @param idx The index of the point light
     * 
     * @return A PositionalLight representing the info about the light
     */
    PositionalLight getLight(unsigned int idx);

    /**
     * To check if an ambient light exists in the scene
     * 
     * @return Whether the scene has an ambient light
     */
    bool hasAmbientLight() const;

    /**
     * Get the ambient light list from the scene, return a light with 0 power otherwise
     * 
     * @return A AmbientLight representing the info about the ambient light 
     */
    AmbientLight getAmbientLight() const;

    RTUtil::Sky getSky() const;

private:
    static PositionalLight defaultLight;

    RTUtil::Sky sky;

    std::shared_ptr<RTUtil::PerspectiveCamera>
        defaultCamera;

    std::vector<Node> nodes;
    GLWrap::Program prog;

    std::map<std::shared_ptr<GLWrap::Mesh>, std::shared_ptr<nori::BSDF>> meshToMaterialMap;

    std::vector<PositionalLight> pointLights;
    std::vector<AreaLight> areaLights;
    std::vector<AmbientLight> ambientLights;

    std::shared_ptr<Node> rootNode;

    std::shared_ptr<Node> createHierarchy(const aiScene *importedScene, aiNode *node, std::shared_ptr<Node> parent, const RTUtil::SceneInfo &sceneInfo);
    Node createNode(const aiScene *importedScene, aiNode *sceneNode, std::shared_ptr<Node> parent, const RTUtil::SceneInfo &sceneInfo);

    void drawFromNode(std::shared_ptr<Node> node, Eigen::Affine3f transformation, std::unique_ptr<GLWrap::Program> &prog, const RTUtil::PerspectiveCamera &cam, bool useUniforms = true);
};
