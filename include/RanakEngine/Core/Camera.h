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
        Vector3 m_position = Vector3(0.0f, 0.0f, 3.0f);
        float m_rotation = 0.0f;

        float m_fov = 45.0f;
        float m_cameraWidth = 10.0f;

        bool m_viewDirty = true;

        ProjectionType m_projectionType = ProjectionType::Orthographic;
        glm::mat4 m_projection;
        glm::mat4 m_view;

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Camera>("Camera",
                                        "Draw", &Camera::Draw,
                                        "ScreenToWorldPoint", &Camera::ScreenToWorldPoint,
                                        "setPosition", &Camera::SetPosition,
                                        "getPosition", &Camera::GetPosition,
                                        "setRotation", &Camera::SetRotation,
                                        "getRotation", &Camera::GetRotation,
                                        "setFOV", &Camera::SetFOV,
                                        "getFOV", &Camera::GetFOV,
                                        "setCameraWidth", &Camera::SetCameraWidth,
                                        "getCameraWidth", &Camera::GetCameraWidth,
                                        "isPerspective", &Camera::IsPerspective,
                                        "setPerspective", &Camera::SetPerspective,
                                        "setOrthographic", &Camera::SetOrthographic);
        }

        public:
        Camera();
        ~Camera();

        void Draw(sol::table _transform, sol::table _drawable);

        Vector3 ScreenToWorldPoint(Vector2 _screenPos);

        void SetPosition(Vector3 _pos);
        Vector3 GetPosition();

        void SetRotation(float _rot);
        float GetRotation();

        void SetFOV(float _fov);
        float GetFOV();

        void SetCameraWidth(float _width);
        float GetCameraWidth();

        bool IsPerspective() const
        {
            return m_projectionType == ProjectionType::Perspective;
        }

        void SetPerspective();
        void SetOrthographic();

        glm::mat4 GetProjection();
        glm::mat4 GetView();
    };
}

#endif