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
