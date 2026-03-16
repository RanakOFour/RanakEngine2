#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#include "sol/sol.hpp"
#include "GLM/ext.hpp"
#include "RanakEngine/Math.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Asset/Shader.h"

namespace RanakEngine::Core
{
    class Camera
    {
        friend LuaContext;
        public:
        enum ProjectionType
        {
            Perspective,
            Orthographic
        };

        private:
        Vector3 m_position;
        float m_rotation;

        float m_fov;
        Vector2 m_cameraSize;

        bool m_viewDirty;

        ProjectionType m_projectionType;
        glm::mat4 m_projection;
        glm::mat4 m_view;

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Camera>("Camera",
                                        "Draw", &Camera::Draw,
                                        "setPosition", &Camera::SetPosition,
                                        "getPosition", &Camera::GetPosition,
                                        "setRotation", &Camera::SetRotation,
                                        "getRotation", &Camera::GetRotation,
                                        "setFOV", &Camera::SetFOV,
                                        "getFOV", &Camera::GetFOV,
                                        "setCameraSize", &Camera::SetCameraSize,
                                        "getCameraSize", &Camera::GetCameraSize,
                                        "isPerspective", &Camera::IsPerspective,
                                        "setPerspective", &Camera::SetPerspective,
                                        "setOrthographic", &Camera::SetOrthographic);
        }

        public:
        Camera();
        ~Camera();

        void Use(std::shared_ptr<Asset::Shader> _shader);

        void Draw(sol::table _transform, sol::table _drawable);

        void SetPosition(Vector3 _pos);
        Vector3 GetPosition();

        void SetRotation(float _rot);
        float GetRotation();

        void SetFOV(float _fov);
        float GetFOV();

        void SetCameraSize(Vector2 _size);
        Vector2 GetCameraSize();

        bool IsPerspective() const
        {
            return m_projectionType == ProjectionType::Perspective;
        }

        void SetPerspective();
        void SetOrthographic();
    };
}

#endif