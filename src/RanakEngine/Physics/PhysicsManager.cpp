#include "RanakEngine/Physics/PhysicsManager.h"

namespace RanakEngine::Physics
{
    std::shared_ptr<Physics::Manager> Manager::Init()
    {
        if (!m_self)
        {
            m_self = std::make_shared<Manager>();
        }

        return m_self;
    };

    Manager::Manager()
    : m_world()
    {
        b2WorldDef l_worldDef = b2DefaultWorldDef();
        
        l_worldDef.gravity = (b2Vec2){0.0f, -9.81f};

        m_world = b2CreateWorld(&l_worldDef);
    }

    Manager::~Manager()
    {
        b2DestroyWorld(m_world);
    }

    void Manager::Init()
    {
        if (!m_self)
        {
            m_self = std::make_shared<Manager>();
        }
    }

    void Manager::Stop()
    {
        m_self.reset();
    }

    void Manager::Step(float _deltaTime)
    {
        m_world->Step(_deltaTime, 8, 3);
    }

    void Manager::SetGravity(const Vector2 &_gravity)
    {
        m_world->SetGravity(b2Vec2(_gravity.x, _gravity.y));
    }

    Vector2 Manager::GetGravity() const
    {
        b2Vec2 l_gravity = m_world->GetGravity();
        return Vector2(l_gravity.x, l_gravity.y);
    }

    std::weak_ptr<Physics::Manager> Manager::Get()
    {
        return m_self;
    }
}