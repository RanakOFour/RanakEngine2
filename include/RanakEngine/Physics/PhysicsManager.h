#ifndef RANAK_PHYSICS_MANAGER_H
#define RANAK_PHYSICS_MANAGER_H

#include "box2d/box2d.h"

#include "RanakEngine/Math.h"

#include <memory>
#include <vector>

namespace RanakEngine::Physics
{
    class Manager
    {
        private:
        inline static std::shared_ptr<Physics::Manager> m_self;
        b2WorldId m_world;

        Manager();
        public:
        ~Manager();

        static std::shared_ptr<Physics::Manager> Init();
        static void Stop();

        b2BodyId CreateBody(const b2BodyDef* _def);
        void DestroyBody(b2BodyId _body);

        void Step(float _deltaTime);
        void Reset();

        void SetGravity(const Vector2 &_gravity);
        Vector2 GetGravity() const;

        static std::weak_ptr<Physics::Manager> Get();
    };
}

#endif