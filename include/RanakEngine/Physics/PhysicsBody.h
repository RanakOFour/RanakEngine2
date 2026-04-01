#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include "RanakEngine/Math.h"

#include "box2d/box2d.h"

#include "sol/sol.hpp"

namespace RanakEngine::Physics
{
    class Body
    {
        private:
        b2BodyId m_body;

        public:
        Body();
        Body(b2BodyId _body);
        ~Body();

        b2BodyId Get() const;

        Vector2 GetPosition() const;
        float   GetAngle() const;
        void    SetTransform(const Vector2& _position, float _angleRadians);

        Vector2 GetLinearVelocity() const;
        void    SetLinearVelocity(const Vector2& _velocity);

        void    ApplyForce(const Vector2& _force, const Vector2& _point);
        void    ApplyForceToCenter(const Vector2& _force);

        bool    IsValid() const;

        static void DefineUserType(sol::state& _state)
        {
            _state.new_usertype<Body>("Body",
                sol::constructors<Body()>(),
                "GetPosition",        &Body::GetPosition,
                "GetAngle",           &Body::GetAngle,
                "SetTransform",       &Body::SetTransform,
                "GetLinearVelocity",  &Body::GetLinearVelocity,
                "SetLinearVelocity",  &Body::SetLinearVelocity,
                "ApplyForce",         &Body::ApplyForce,
                "ApplyForceToCenter", &Body::ApplyForceToCenter,
                "IsValid",            &Body::IsValid
            );
        }
    };
}

#endif