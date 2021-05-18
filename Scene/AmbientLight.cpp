#include "AmbientLight.hpp"

#include <cfloat>

// Constructor

AmbientLight::AmbientLight(std::shared_ptr<RTUtil::LightInfo> info)
{
    ambientRadiance = info->radiance;
    if (info->range >= FLT_MAX)
    {
        ambientRange = 1.0;
    }
    else
    {

        ambientRange = info->range;
    }
}

AmbientLight::AmbientLight(Eigen::Vector3f ambientRadiance, float ambientRange)
{
    this->ambientRadiance = ambientRadiance;
    this->ambientRange = ambientRange;
}

// Public Functions

Eigen::Vector3f AmbientLight::radiance() const
{
    return ambientRadiance;
}

float AmbientLight::range() const
{
    return ambientRange;
}