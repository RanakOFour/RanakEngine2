#include "RanakEngine/Physics/PhysicsBody.h"

#include "box2d/math_functions.h"

namespace RanakEngine::Physics
{
    Body::Body()
    : m_body()
    {
    }

    Body::Body(b2BodyId _body)
    : m_body(_body)
    {
    }

    Body::~Body()
    {
    }

    b2BodyId Body::Get() const
    {
        return m_body;
    }

    Vector2 Body::GetPosition() const
    {
        b2Vec2 l_pos = b2Body_GetPosition(m_body);
        return Vector2(l_pos.x, l_pos.y);
    }

    float Body::GetAngle() const
    {
        b2Rot l_rot = b2Body_GetRotation(m_body);
        return b2Rot_GetAngle(l_rot);
    }

    void Body::SetTransform(const Vector2& _position, float _angleRadians)
    {
        b2Body_SetTransform(m_body, {_position.x, _position.y}, b2MakeRot(_angleRadians));
    }

    Vector2 Body::GetLinearVelocity() const
    {
        b2Vec2 l_vel = b2Body_GetLinearVelocity(m_body);
        return Vector2(l_vel.x, l_vel.y);
    }

    void Body::SetLinearVelocity(const Vector2& _velocity)
    {
        b2Body_SetLinearVelocity(m_body, {_velocity.x, _velocity.y});
    }

    void Body::ApplyForce(const Vector2& _force, const Vector2& _point)
    {
        b2Body_ApplyForce(m_body, {_force.x, _force.y}, {_point.x, _point.y}, true);
    }

    void Body::ApplyForceToCenter(const Vector2& _force)
    {
        b2Body_ApplyForceToCenter(m_body, {_force.x, _force.y}, true);
    }

    bool Body::IsValid() const
    {
        return b2Body_IsValid(m_body);
    }
}
