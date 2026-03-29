#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#include "sol/sol.hpp"
#include "GLM/ext.hpp"
#include "RanakEngine/Math.h"
#include "RanakEngine/Core/LuaContext.h"

namespace RanakEngine::Asset
{
    class Model;
    class Texture;
    class Shader;
}

namespace RanakEngine::Core
{
    /**
     * @class Camera
     * @brief Scene camera supporting orthographic and perspective projection.
     *
     * Owns the view and projection matrices used by the renderer.  The view
     * matrix is rebuilt lazily whenever m_viewDirty is true (i.e. after a
     * position or rotation change).
     *
     * Provides world<->screen conversion utilities (ScreenToWorldPoint,
     * WorldToScreenPoint) used by the editor, raycasting, and Lua scripts.
     *
     * Camera is registered as a Lua usertype so Lua rules can reposition or
     * query it at runtime via the global camera handle.
     */
    class Camera
    {
        friend LuaContext;
        public:
        /**
         * @enum ProjectionType
         * @brief Selects the mathematical model used to build the projection matrix.
         */
        enum ProjectionType
        {
            Perspective,  ///< 3-D perspective division — objects shrink with distance.
            Orthographic  ///< Parallel projection — no depth shrinkage (2-D games).
        };

        private:
        Vector3 m_position  = Vector3(0.0f, 0.0f, 10.0f); ///< Camera position in world space.
        float   m_rotation  = 0.0f;                        ///< 2-D rotation around the Z axis (degrees).

        float m_fov         = 45.0f;  ///< Vertical field of view in degrees (perspective only).
        float m_cameraWidth = 30.0f;  ///< Half-width of the orthographic view frustum.

        bool m_viewDirty = true; ///< True when the view matrix needs rebuilding.

        ProjectionType m_projectionType = ProjectionType::Orthographic; ///< Active projection mode.
        glm::mat4 m_projection; ///< Cached projection matrix.
        glm::mat4 m_view;       ///< Cached view matrix.

        inline static std::string DefaultModelPath = "./resources/Models/FlatTexture.obj";
        inline static std::string DefaultShaderPath = "./resources/Shaders/default/frag.fs;./resources/Shaders/default/vert.vs";

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Camera>("Camera",
                                        "Draw", sol::overload(static_cast<void (Camera::*)(sol::table)>(&Camera::Draw),
                                                              static_cast<void (Camera::*)(sol::table, 
                                                                                           std::weak_ptr<Asset::Model>,
                                                                                           std::weak_ptr<Asset::Texture>,
                                                                                           std::weak_ptr<Asset::Shader>
                                                                                          )>(&Camera::Draw)),

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

        /**
         * @brief Renders a single entity's drawable component via the camera.
         * @param _transform Lua table with position/rotation/scale fields.
         * @param _drawable  Lua table describing the renderable (e.g. texture, model).
         */
        void Draw(sol::table _entityData);

        void Draw(
                  sol::table _transform,
                  std::weak_ptr<Asset::Model> _model,
                  std::weak_ptr<Asset::Texture> _texture,
                  std::weak_ptr<Asset::Shader> _shader
                 );

        /**
         * @brief Converts a screen-space pixel coordinate to a world-space point.
         * @param _screenPos Pixel coordinate (origin top-left).
         * @return Corresponding world-space position.
         */
        Vector3 ScreenToWorldPoint(Vector2 _screenPos);
        /**
         * @brief Converts a world-space position to a screen-space pixel coordinate.
         * @param _worldPos World-space XY position.
         * @return Screen-space pixel coordinate.
         */
        Vector2 WorldToScreenPoint(Vector2 _worldPos);

        /** @brief Sets the camera's world-space position. @param _pos New position. */
        void SetPosition(Vector3 _pos);
        /** @brief Returns the camera's world-space position. */
        Vector3 GetPosition();

        /** @brief Sets the camera's 2-D rotation in degrees. @param _rot Rotation angle. */
        void SetRotation(float _rot);
        /** @brief Returns the camera's current 2-D rotation in degrees. */
        float GetRotation();

        /** @brief Sets the vertical field of view (perspective only). @param _fov FOV in degrees. */
        void SetFOV(float _fov);
        /** @brief Returns the vertical field of view in degrees. */
        float GetFOV();

        /** @brief Sets the half-width of the orthographic view frustum. @param _width Width value. */
        void SetCameraWidth(float _width);
        /** @brief Returns the orthographic half-width. */
        float GetCameraWidth();

        /** @brief Returns true when the camera is in perspective mode. */
        bool IsPerspective() const
        {
            return m_projectionType == ProjectionType::Perspective;
        }

        /** @brief Switches to perspective projection. */
        void SetPerspective();
        /** @brief Switches to orthographic projection. */
        void SetOrthographic();

        /** @brief Returns the current projection matrix (rebuilt if dirty). */
        glm::mat4 GetProjection();
        /** @brief Returns the current view matrix (rebuilt if dirty). */
        glm::mat4 GetView();
    };
}

#endif