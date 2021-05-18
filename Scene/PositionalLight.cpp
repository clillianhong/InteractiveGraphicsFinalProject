#include "PositionalLight.hpp"

// Constructor

PositionalLight::PositionalLight(const aiScene *importedSceneOriginal, std::shared_ptr<RTUtil::LightInfo> info)
{
    nodeToWorldTransform = Utils::collectNodeToWorldTransform(importedSceneOriginal, aiString(info->nodeName));
    power = info->power;
    localPosition = info->position;
}

// Public Functions

Eigen::Affine3f PositionalLight::getNodeToWorldTransform() const
{
    return nodeToWorldTransform;
}

Eigen::Vector3f PositionalLight::getPower() const
{
    return power;
}

Eigen::Vector3f PositionalLight::getPos() const
{
    return localPosition;
}
