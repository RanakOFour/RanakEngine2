#ifndef RANAKMATH_H
#define RANAKMATH_H

#include "RanakEngine/Math/Quaternion.h"
#include "RanakEngine/Math/Vector2.h"
#include "RanakEngine/Math/Vector3.h"
#include "RanakEngine/Math/Vector4.h"

namespace RanakEngine::Math
{
    static float PI() { return 3.14159265358979f; };

    /** @brief Registers math Lua bindings (Vector2, Vector3, Vector4, Quaternion) with the LuaContext. */
    void DefineLuaLib();

    /** @brief Releases any math-subsystem resources (currently a no-op). */
    void Stop();
}

#endif