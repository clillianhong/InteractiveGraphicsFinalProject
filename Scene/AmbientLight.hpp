#pragma once

#include <string>
#include <iostream>
#include <cmath>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <RTUtil/sceneinfo.hpp>
#include <assimp/scene.h>

class AmbientLight
{
public:
    AmbientLight(std::shared_ptr<RTUtil::LightInfo> info);
    AmbientLight(Eigen::Vector3f ambientRadiance, float ambientRange);

    Eigen::Vector3f radiance() const;
    float range() const;

private:
    Eigen::Vector3f ambientRadiance;
    float ambientRange;
};