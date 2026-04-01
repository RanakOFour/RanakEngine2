#include "RanakEngine/Physics/PhysicsManager.h"

namespace RanakEngine::Physics
{
    Manager::Manager()
    : m_world()
    {
        b2WorldDef l_worldDef = b2DefaultWorldDef();
        l_worldDef.gravity = {0.0f, -9.81f};
        m_world = b2CreateWorld(&l_worldDef);
    }

    Manager::~Manager()
    {
        b2DestroyWorld(m_world);
    }

    std::shared_ptr<Physics::Manager> Manager::Init()
    {
        if (!m_self)
        {
            m_self.reset(new Manager());
        }
        return m_self;
    }

    void Manager::Stop()
    {
        m_self.reset();
    }

    b2BodyId Manager::CreateBody(const b2BodyDef* _def)
    {
        return b2CreateBody(m_world, _def);
    }

    void Manager::DestroyBody(b2BodyId _body)
    {
        b2DestroyBody(_body);
    }

    void Manager::Step(float _deltaTime)
    {
        b2World_Step(m_world, _deltaTime, 4);
    }

    void Manager::Reset()
    {
        b2DestroyWorld(m_world);
        b2WorldDef l_worldDef = b2DefaultWorldDef();
        l_worldDef.gravity = {0.0f, -9.81f};
        m_world = b2CreateWorld(&l_worldDef);
    }

    void Manager::SetGravity(const Vector2& _gravity)
    {
        b2World_SetGravity(m_world, {_gravity.x, _gravity.y});
    }

    Vector2 Manager::GetGravity() const
    {
        b2Vec2 l_gravity = b2World_GetGravity(m_world);
        return Vector2(l_gravity.x, l_gravity.y);
    }

    std::weak_ptr<Physics::Manager> Manager::Get()
    {
        return m_self;
    }
}