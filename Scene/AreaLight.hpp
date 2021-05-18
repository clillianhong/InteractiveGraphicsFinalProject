#pragma once

#include <string>
#include <iostream>
#include <cmath>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <RTUtil/sceneinfo.hpp>
#include <assimp/scene.h>

#include "Utils.hpp"
#include "PositionalLight.hpp"

class AreaLight : public PositionalLight
{
public:
    AreaLight(const aiScene *importedSceneOriginal, std::shared_ptr<RTUtil::LightInfo> info);

    Eigen::Vector3f normal() const;
    Eigen::Vector3f up() const;
    Eigen::Vector3f u() const;
    Eigen::Vector3f v() const;
    float width() const;
    float height() const;
    float area() const;

private:
    const Eigen::Vector3f normalVec, upVec;
    const Eigen::Vector3f uVec, vVec;
    const float lightWidth, lightHeight;
    const float lightArea;
};