#include "RanakEngine/Core/Raycast.h"

namespace RanakEngine::Core
{
    bool AABBIntersection(Ray& _ray, Vector3 _min, Vector3 _max, HitInfo& _out)
    {
        Vector3 tMin = (_min - _ray.origin) / _ray.direction;
        Vector3 tMax = (_max - _ray.origin) / _ray.direction;

        // Find the nearest and farthest intersections
        Vector3 tNear = Vector3(
                                glm::min(tMin.x, tMax.x),
                                glm::min(tMin.y, tMax.y),
                                glm::min(tMin.z, tMax.z)
        );

        Vector3 tFar = Vector3(
                                glm::max(tMin.x, tMax.x),
                                glm::max(tMin.y, tMax.y),
                                glm::max(tMin.z, tMax.z)
        );

        // Find the maximum of the nearest intersections
        float tNearMax = glm::max(glm::max(tNear.x, tNear.y), tNear.z);
        // Find the minimum of the farthest intersections
        float tFarMin = glm::min(glm::min(tFar.x, tFar.y), tFar.z);

        // Check if the _ray intersects the bounding box
        if (tNearMax <= tFarMin && _out.distance > tNearMax)
        {
            // Ray intersects the bounding box
            // Calculate the intersection point
            Vector3 intersectionPoint = _ray.origin + _ray.direction * tNearMax;
            _out.hitPoint = intersectionPoint;
            _out.distance = tNearMax;
            _out.hit = true;

            return true;
        }

        // Ray does not intersect the bounding box
        return false;
    };
}