#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include "RanakEngine/Core.h"
#include "RanakEngine/Math.h"

#include "box2d/box2d.h"

namespace RanakEngine::Physics
{
    class Body
    {
        private:
        b2BodyId m_body;

        static void DefineUserType(sol::state& _state)
        {
            _state.new_usertype<Body>("Body",
                                            sol::constructors<Body()>(),
                                            "Get", &Body::Get
                                         );
        };

        public:
        Body();
        Body(b2BodyId _body);
        ~Body();

        b2BodyId Get() const;
    };
}

#endif