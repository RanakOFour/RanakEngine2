#ifndef RANAKPHYSICS_H
#define RANAKPHYSICS_H

#include "RanakEngine/Physics/PhysicsManager.h"
#include "RanakEngine/Physics/PhysicsBody.h"

#include <memory>

namespace RanakEngine::Physics
{
    /** @brief Registers Physics Lua bindings (Body usertype, CreateBody, AddBoxShape, AddCircleShape). */
    void DefineLuaLib();

    /** @brief Creates and returns the Physics::Manager singleton. */
    std::shared_ptr<Physics::Manager> Init();

    /** @brief Destroys the Physics::Manager singleton and clears Lua globals. */
    void Stop();
}

#endif
