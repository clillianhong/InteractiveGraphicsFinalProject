#pragma once

#include <string>
#include <iostream>
#include <cmath>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <RTUtil/sceneinfo.hpp>
#include <assimp/scene.h>

#include "Utils.hpp"

class PositionalLight
{

public:
    PositionalLight(const aiScene *importedSceneOriginal, std::shared_ptr<RTUtil::LightInfo> info);
    Eigen::Affine3f getNodeToWorldTransform() const;
    Eigen::Vector3f getPower() const;
    Eigen::Vector3f getPos() const;

private:
    Eigen::Affine3f nodeToWorldTransform;
    std::shared_ptr<RTUtil::LightInfo> lightInfo;
    Eigen::Vector3f localPosition;
    Eigen::Vector3f power;
};
