#include "RanakEngine/Core/Camera.h"
#include "RanakEngine/Assets.h"
#include "RanakEngine/Math.h"

#include "RanakEngine/IO/IOManager.h"
#include "RanakEngine/IO/Window.h"

#include "sol/sol.hpp"
#include "GL/glew.h"
#include "GLM/ext.hpp"

namespace RanakEngine::Core
{
    Camera::Camera()
    : m_position(0.0f, 0.0f, 3.0f)
    , m_rotation(0.0f)
    , m_fov(45.0f)
    , m_projectionType(ProjectionType::Orthographic)
    , m_cameraWidth(10.0f)
    , m_viewDirty(true)
    {
        SetOrthographic();
    }

    Camera::~Camera()
    {

    }

    glm::mat4 CalculateModelMatrix(Vector3 _pos, Vector3 _euler, Vector3 _scale)
    {
        glm::vec3 l_glPos = glm::vec3(_pos.x, _pos.y, _pos.z);
        glm::quat l_glQuat = (glm::quat)Quaternion(glm::radians(_euler.x), glm::radians(_euler.y), glm::radians(_euler.z));

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), l_glPos);
        glm::mat4 rotation = glm::mat4(l_glQuat);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(_scale.x, _scale.y, _scale.z));
        
        return translation * rotation * scale;
    }

    void Camera::Draw(sol::table _transform, sol::table _drawable)
    {
        sol::optional<std::weak_ptr<Asset::Model>> l_modelPtr = _drawable.raw_get<sol::optional<std::weak_ptr<Asset::Model>>>("model");
        
        std::string l_modelPath = _drawable.raw_get<std::string>("modelPath");
        std::shared_ptr<Asset::Model> l_model;

        if(!l_modelPtr.has_value())
        {
            // Instantiate value for ptr
            if(l_modelPath == "")
            {
                l_modelPath = "./resources/Models/FlatTexture.obj";
            }

            auto l_modelWPtr = Asset::Load<Asset::Model>(l_modelPath);

            if(l_modelWPtr.lock() == nullptr)
            {
                Log::Warning("Object cannot be drawn: invalid model path" + l_modelPath);
                return;
            }

            _drawable.raw_set("model", l_modelWPtr);
            l_modelPtr = _drawable.raw_get<std::weak_ptr<Asset::Model>>("model");

            l_model = l_modelPtr.value().lock();
        }
        else
        {
            // Check if path has changed
            l_model = l_modelPtr.value().lock();
            if(l_modelPath != "" && l_model->GetPath() != l_modelPath)
            {
                auto l_modelWPtr = Asset::Load<Asset::Model>(l_modelPath);

                if(l_modelWPtr.lock() == nullptr)
                {
                    Log::Warning("Object cannot be drawn: invalid model path " + l_modelPath);
                    return;
                }

                _drawable.raw_set("model", l_modelWPtr);
                l_modelPtr = _drawable.raw_get<std::weak_ptr<Asset::Model>>("model");
            }
        }

        sol::optional<std::weak_ptr<Asset::Texture>> l_texturePtr = _drawable.raw_get<sol::optional<std::weak_ptr<Asset::Texture>>>("texture");
        
        std::string l_texturePath = _drawable.raw_get<std::string>("texturePath");
        std::shared_ptr<Asset::Texture> l_texture;
        
        if(!l_texturePtr.has_value())
        {
            if(l_texturePath != "")
            {
                auto l_textureWPtr = Asset::Load<Asset::Texture>(l_texturePath);

                if(l_textureWPtr.lock() == nullptr)
                {
                    Log::Warning("Object cannot be drawn: invalid texture path");
                    return;
                }

                _drawable.raw_set("texture", l_textureWPtr);
                l_texturePtr = _drawable.raw_get<std::weak_ptr<Asset::Texture>>("texture");

                l_texture = l_texturePtr.value().lock();
            }
        }
        else
        {
            // Check if path has changed
            l_texture = l_texturePtr.value().lock();
            if(l_texturePath != "" && l_texture->GetPath() != l_texturePath)
            {
                auto l_textureWPtr = Asset::Load<Asset::Texture>(l_texturePath);

                if(l_textureWPtr.lock() == nullptr)
                {
                    Log::Warning("Object cannot be drawn: invalid model path");
                    return;
                }

                _drawable.raw_set("texture", l_textureWPtr);
                l_texturePtr = _drawable.raw_get<std::weak_ptr<Asset::Texture>>("texture");
            }
        }

        sol::optional<std::weak_ptr<Asset::Shader>> l_shaderPtr = _drawable.raw_get<sol::optional<std::weak_ptr<Asset::Shader>>>("shader");

        std::string l_shaderPath = _drawable.raw_get<std::string>("shaderPath");
        std::shared_ptr<Asset::Shader> l_shader;
        
        if(!l_shaderPtr.has_value())
        {
            if(l_shaderPath == "")
            {
                l_shaderPath = "./resources/Shaders/default/frag.fs;./resources/Shaders/default/vert.vs";
            }

            auto l_shaderWPtr = Asset::Load<Asset::Shader>(l_shaderPath);

            if(l_shaderWPtr.lock() == nullptr)
            {
                Log::Warning("Object cannot be drawn: invalid shader path");
                return;
            }

            _drawable.raw_set("shader", l_shaderWPtr);
            l_shaderPtr = _drawable.raw_get<std::weak_ptr<Asset::Shader>>("shader");
            l_shader = l_shaderPtr.value().lock();
        }
        else
        {
            // Check if path has changed
            l_shader = l_shaderPtr.value().lock();
            if(l_shaderPath != "" && l_shader->GetPath() != l_shaderPath)
            {
                auto l_shaderWPtr = Asset::Load<Asset::Shader>(l_shaderPath);

                if(l_shaderWPtr.lock() == nullptr)
                {
                    Log::Warning("Object cannot be drawn: invalid model path");
                    return;
                }

                _drawable.raw_set("shader", l_shaderWPtr);
                l_shaderPtr = _drawable.raw_get<std::weak_ptr<Asset::Shader>>("shader");
            }
        }
        
        l_shader->Use();
        
        glBindVertexArray(l_model->GetVAO());

        if(l_texture != nullptr)
        {
            glBindTexture(GL_TEXTURE_2D, l_texture->GetID());
        }

        glm::mat4 l_modelMat = CalculateModelMatrix(_transform.raw_get<Vector3>("Position"),
                                                    _transform.raw_get<Vector3>("Rotation"),
                                                    _transform.raw_get<Vector3>("Scale"));

        l_shader->SetUniform("u_Model", l_modelMat);
        
        if(m_viewDirty)
        {
            m_view = CalculateModelMatrix(m_position, Vector3(0.0f, m_rotation, 0.0f), Vector3(1.0f));
            m_view = glm::inverse(m_view);
        }

        l_shader->SetUniform("u_View", m_view);
        l_shader->SetUniform("u_Projection", m_projection);

        glDrawArrays(GL_TRIANGLES, 0, l_model->GetVertexCount());
        
        glUseProgram(0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Vector3 Camera::ScreenToWorldPoint(Vector2 _screenPoint)
    {
        // Get window dimensions from ImGui
        auto l_window = IO::Manager::Instance().lock()->GetWindow().lock();
        Vector2 l_screenSize = l_window->GetScreenSize();

        Vector2 l_pointNDC(2 * (_screenPoint.x / l_screenSize.x) - 1,
                           2 * (_screenPoint.y / l_screenSize.y) - 1);

        if(m_viewDirty)
        {
            m_view = CalculateModelMatrix(m_position, Vector3(0.0f, m_rotation, 0.0f), Vector3(1.0f));
            m_view = glm::inverse(m_view);
            m_viewDirty = false;
        }

        glm::mat4 l_modelView = glm::inverse(m_view * m_projection);

        glm::vec3 l_worldPos = glm::unProject(
        glm::vec3(_screenPoint.x, _screenPoint.y, 0),
        m_view,
        m_projection,
        glm::vec4(0.0f, 0.0f, l_screenSize.x, l_screenSize.y));

        return (Vector3)l_worldPos;
    }

    void Camera::SetPosition(Vector3 _p)
    {
        m_position = _p;
        m_viewDirty = true;
    }

    Vector3 Camera::GetPosition()
    {
        return m_position;
    }

    void Camera::SetRotation(float _rot)
    {
        m_rotation = _rot;
        m_viewDirty = true;
    }

    float Camera::GetRotation()
    {
        return m_rotation;
    }

    void Camera::SetFOV(float _fov)
    {
        m_fov = _fov;
        if(m_projectionType == ProjectionType::Perspective)
        {
            SetPerspective();
        }
    }

    float Camera::GetFOV()
    {
        return m_fov;
    }

    void Camera::SetCameraWidth(float _width)
    {
        m_cameraWidth = _width;
        if(m_projectionType == ProjectionType::Perspective)
        {
            SetPerspective();
        }
        else
        {
            SetOrthographic();
        }
    }

    float Camera::GetCameraWidth()
    {
        return m_cameraWidth;
    }

    void Camera::SetPerspective()
    {
        
        m_projectionType = ProjectionType::Perspective;
        
        auto l_window = IO::Manager::Instance().lock()->GetWindow().lock();
        Vector2 l_screenSize = l_window->GetScreenSize();

        m_projection = glm::perspective(glm::radians(m_fov), l_screenSize.x / l_screenSize.y, 0.1f, 100.0f);
    }

    void Camera::SetOrthographic()
    {
        m_projectionType = ProjectionType::Orthographic;
        
        // Get the viewport size to maintain correct aspect ratio
        auto l_window = IO::Manager::Instance().lock()->GetWindow().lock();
        Vector2 l_viewportSize = l_window->GetScreenSize();
        float l_aspectRatio = l_viewportSize.x / l_viewportSize.y;
        
        // Calculate the height based on the width and viewport aspect ratio to prevent stretching
        float l_orthoHeight = m_cameraWidth / l_aspectRatio;
        
        m_projection = glm::ortho(-m_cameraWidth/2.0f, m_cameraWidth/2.0f, -l_orthoHeight/2.0f, l_orthoHeight/2.0f, 0.1f, 100.0f);
    }

    glm::mat4 Camera::GetProjection()
    {
        return m_projection;
    }

    glm::mat4 Camera::GetView()
    {
        if(m_viewDirty)
        {
            m_view = CalculateModelMatrix(m_position, glm::vec3(0.0f, m_rotation, 0.0f), glm::vec3(1.0f));
            m_view = glm::inverse(m_view);
            m_viewDirty = false;
        }

        return m_view;
    }
}