#include "RanakEngine/Physics.h"

#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Log.h"

#include "box2d/box2d.h"
#include "box2d/collision.h"

#include "sol/sol.hpp"

namespace RanakEngine::Physics
{
    namespace
    {
        static std::shared_ptr<Physics::Manager> PhysicsManagerInstance;
        static sol::table PhysicsTable;
    }

    void DefineLuaLib()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        PhysicsTable = l_context->CreateTable();

        // Physics.CreateBody(position: Vector2, bodyType: string) -> Physics.Body
        PhysicsTable.set_function("CreateBody", [](Vector2 _position, std::string _typeStr) -> Physics::Body
        {
            auto l_manager = Physics::Manager::Get().lock();
            if (!l_manager)
            {
                throw std::runtime_error("PhysicsManager not initialized");
            }

            b2BodyDef l_def = b2DefaultBodyDef();
            l_def.position = {_position.x, _position.y};

            if (_typeStr == "static")
                l_def.type = b2_staticBody;
            else if (_typeStr == "kinematic")
                l_def.type = b2_kinematicBody;
            else
                l_def.type = b2_dynamicBody;

            return Physics::Body(l_manager->CreateBody(&l_def));
        });

        PhysicsTable.set_function("CreateBody", [](Vector2 _position, float _rotation, std::string _typeStr) -> Physics::Body
        {
            auto l_manager = Physics::Manager::Get().lock();
            if (!l_manager)
            {
                throw std::runtime_error("PhysicsManager not initialized");
            }

            b2BodyDef l_def = b2DefaultBodyDef();
            l_def.position = {_position.x, _position.y};
            l_def.rotation = b2MakeRot(Math::DegToRad(_rotation));

            if (_typeStr == "static")
                l_def.type = b2_staticBody;
            else if (_typeStr == "kinematic")
                l_def.type = b2_kinematicBody;
            else
                l_def.type = b2_dynamicBody;

            return Physics::Body(l_manager->CreateBody(&l_def));
        });

        // Overload with extended parameters: gravityScale, linearDamping, angularDamping,
        // fixedRotation, linearVelocity, angularVelocity.
        PhysicsTable.set_function("CreateBody", [](Vector2 _position, float _rotation, std::string _typeStr,
                                                     sol::object _extras) -> Physics::Body
        {
            auto l_manager = Physics::Manager::Get().lock();
            if (!l_manager)
            {
                throw std::runtime_error("PhysicsManager not initialized");
            }

            b2BodyDef l_def = b2DefaultBodyDef();
            l_def.position = {_position.x, _position.y};
            l_def.rotation = b2MakeRot(Math::DegToRad(_rotation));

            if (_typeStr == "static")
                l_def.type = b2_staticBody;
            else if (_typeStr == "kinematic")
                l_def.type = b2_kinematicBody;
            else
                l_def.type = b2_dynamicBody;

            if (_extras.valid() && _extras.get_type() == sol::type::table)
            {
                sol::table l_t = _extras.as<sol::table>();
                l_def.gravityScale         = l_t.get_or("gravityScale",     l_def.gravityScale);
                l_def.linearDamping        = l_t.get_or("linearDamping",    l_def.linearDamping);
                l_def.angularDamping       = l_t.get_or("angularDamping",   l_def.angularDamping);
                l_def.motionLocks.angularZ = l_t.get_or("fixedRotation",   l_def.motionLocks.angularZ);
                l_def.enableSleep          = l_t.get_or("enableSleep",      l_def.enableSleep);
                l_def.isAwake              = l_t.get_or("isAwake",          l_def.isAwake);
                l_def.isBullet             = l_t.get_or("isBullet",         l_def.isBullet);

                Vector2 l_linVel(0.0f);
                l_linVel = l_t.get_or("linearVelocity", l_linVel);

                l_def.linearVelocity = {l_linVel.x, l_linVel.y};
                l_def.angularVelocity = l_t.get_or("angularVelocity", 0.0f);
            }

            return Physics::Body(l_manager->CreateBody(&l_def));
        });

        // Physics.AddBoxShape(body: Body, halfW, halfH, density, friction, restitution)
        PhysicsTable.set_function("AddBoxShape", [](Physics::Body& _body, float _halfW, float _halfH,
                                                     float _density, float _friction, float _restitution)
        {
            b2ShapeDef l_shapeDef = b2DefaultShapeDef();
            l_shapeDef.density              = _density;
            l_shapeDef.material.friction    = _friction;
            l_shapeDef.material.restitution = _restitution;

            b2Polygon l_box = b2MakeBox(_halfW, _halfH);
            b2CreatePolygonShape(_body.Get(), &l_shapeDef, &l_box);
        });

        // Physics.AddCircleShape(body: Body, radius, density, friction, restitution)
        PhysicsTable.set_function("AddCircleShape", [](Physics::Body& _body, float _radius,
                                                        float _density, float _friction, float _restitution)
        {
            b2ShapeDef l_shapeDef = b2DefaultShapeDef();
            l_shapeDef.density              = _density;
            l_shapeDef.material.friction    = _friction;
            l_shapeDef.material.restitution = _restitution;

            b2Circle l_circle = {{0.0f, 0.0f}, _radius};
            b2CreateCircleShape(_body.Get(), &l_shapeDef, &l_circle);
        });

        // Physics.SetGravity(gravity: Vector2)
        PhysicsTable.set_function("SetGravity", [](Vector2 _gravity)
        {
            auto l_manager = Physics::Manager::Get().lock();
            if (l_manager) l_manager->SetGravity(_gravity);
        });

        // Physics.GetGravity() -> Vector2
        PhysicsTable.set_function("GetGravity", []() -> Vector2
        {
            auto l_manager = Physics::Manager::Get().lock();
            if (l_manager) return l_manager->GetGravity();
            return Vector2(0.0f, 0.0f);
        });

        l_context->SetGlobal("Physics", PhysicsTable);
    }

    std::shared_ptr<Physics::Manager> Init()
    {
        PhysicsManagerInstance = Physics::Manager::Init();
        return PhysicsManagerInstance;
    }

    void Stop()
    {
        PhysicsTable.abandon();
        PhysicsManagerInstance.reset();
        Physics::Manager::Stop();
    }
}
