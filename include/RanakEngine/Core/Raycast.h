#ifndef RAYCAST_H
#define RAYCAST_H

#include "RanakEngine/Math.h"
#include <cmath>

namespace RanakEngine::Core
{
    struct Ray
    {
        Vector3 origin = Vector3(0.0f);
        Vector3 direction = Vector3(0.0f);
    };

    struct HitInfo
    {
        bool hit = false;
        Vector3 hitPoint = Vector3(0.0f);
        float distance = std::numeric_limits<float>::max();
    };

    struct RaycastHit
    {
        HitInfo hitInfo;
        int entityId;
    };

    bool AABBIntersection(Ray& _ray, Vector3 _min, Vector3 _max, HitInfo& _out);
}

#endif