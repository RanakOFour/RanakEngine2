#ifndef RAYCAST_H
#define RAYCAST_H

#include "RanakEngine/Math.h"
#include <cmath>

namespace RanakEngine::Core
{
    /**
     * @struct Ray
     * @brief Defines an infinite ray from an origin along a direction.
     */
    struct Ray
    {
        Vector3 origin    = Vector3(0.0f); ///< World-space origin point of the ray.
        Vector3 direction = Vector3(0.0f); ///< Normalised world-space direction of the ray.
    };

    /**
     * @struct HitInfo
     * @brief Stores intersection details from a raycast test.
     */
    struct HitInfo
    {
        bool    hit      = false;                                ///< True when the ray intersected the tested shape.
        Vector3 hitPoint = Vector3(0.0f);                        ///< World-space point of intersection.
        float   distance = std::numeric_limits<float>::max();    ///< Distance from the ray origin to the hit point.
    };

    /**
     * @struct RaycastHit
     * @brief Combines HitInfo with the entity ID of the object that was hit.
     */
    struct RaycastHit
    {
        HitInfo hitInfo;   ///< Intersection details.
        int     entityId;  ///< ID of the entity that was intersected, or -1 if none.
    };

    /**
     * @brief Tests a ray against an axis-aligned bounding box (AABB).
     *
     * Uses the slab method for efficient ray-AABB intersection testing.
     *
     * @param _ray Ray to test.
     * @param _min Minimum corner of the AABB.
     * @param _max Maximum corner of the AABB.
     * @param _out Populated with hit information if the ray intersects.
     * @return True when the ray intersects the AABB.
     */
    bool AABBIntersection(Ray& _ray, Vector3 _min, Vector3 _max, HitInfo& _out);
}

#endif