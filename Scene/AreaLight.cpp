#include "AreaLight.hpp"

// Constructor

AreaLight::AreaLight(const aiScene *importedSceneOriginal, std::shared_ptr<RTUtil::LightInfo> info) : PositionalLight(importedSceneOriginal, info),
                                                                                                      normalVec(info->normal), upVec(info->up),
                                                                                                      // Right is positive and up is positive
                                                                                                      uVec(upVec.cross(normalVec).normalized()), vVec(normalVec.cross(uVec).normalized()),
                                                                                                      lightWidth(info->size[0]), lightHeight(info->size[1]),
                                                                                                      lightArea(lightWidth * lightHeight)
{
}

// Public Functions
Eigen::Vector3f AreaLight::normal() const
{
    return normalVec;
}

Eigen::Vector3f AreaLight::up() const
{
    return upVec;
}

Eigen::Vector3f AreaLight::u() const
{
    return uVec;
}

Eigen::Vector3f AreaLight::v() const
{
    return vVec;
}

float AreaLight::width() const
{
    return lightWidth;
}

float AreaLight::height() const
{
    return lightHeight;
}

float AreaLight::area() const
{
    return lightArea;
}