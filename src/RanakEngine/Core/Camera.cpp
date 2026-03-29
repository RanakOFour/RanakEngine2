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
    : m_position(0.0f, 0.0f, 10.0f)
    , m_rotation(0.0f)
    , m_fov(45.0f)
    , m_projectionType(ProjectionType::Orthographic)
    , m_cameraWidth(30.0f)
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

    void Camera::Draw(sol::table _entityData)
    {
        sol::optional<sol::table> l_modelCatOpt = _entityData.traverse_raw_get<sol::optional<sol::table>>("Model");
        
        std::shared_ptr<Asset::Model> l_model;

        if(l_modelCatOpt.has_value())
        {
            sol::optional<std::weak_ptr<Asset::Model>> l_modelPtr = _entityData.traverse_raw_get<sol::optional<std::weak_ptr<Asset::Model>>>("Model", "asset");
            std::string l_modelPath = _entityData.traverse_raw_get<std::string>("Model", "modelPath");

            if(!l_modelPtr.has_value())
            {
                // Instantiate value for ptr
                if(l_modelPath == "")
                {
                    l_modelPath = DefaultModelPath;
                }

                auto l_modelWPtr = Asset::Load<Asset::Model>(l_modelPath);

                if(l_modelWPtr.lock() == nullptr)
                {
                    Log::Warning("Object cannot be drawn: invalid model path" + l_modelPath);
                    return;
                }

                _entityData["Model"]["asset"] = l_modelWPtr;
                l_modelPtr = _entityData.traverse_raw_get<std::weak_ptr<Asset::Model>>("Model", "asset");
            }
            else
            {
                // Check if path has changed
                if(l_modelPath != "")
                {
                    auto l_modelWPtr = Asset::Load<Asset::Model>(l_modelPath);

                    if(l_modelWPtr.lock() == nullptr)
                    {
                        Log::Warning("Object cannot be drawn: invalid model path" + l_modelPath);
                        return;
                    }

                    _entityData["Model"]["asset"] = l_modelWPtr;
                    l_modelPtr = _entityData.traverse_raw_get<std::weak_ptr<Asset::Model>>("Model", "asset");
                }
            }

            l_model = (*l_modelPtr).lock();
        }
        else
        {
            // Check if path has changed
            l_model = Asset::Load<Asset::Model>(DefaultModelPath).lock();
        }

        sol::optional<sol::table> l_textureCatOpt = _entityData.traverse_raw_get<sol::optional<sol::table>>("Texture");

        std::shared_ptr<Asset::Texture> l_texture;
        
        if(l_textureCatOpt.has_value())
        {
            sol::optional<std::weak_ptr<Asset::Texture>> l_texturePtr = _entityData.traverse_raw_get<sol::optional<std::weak_ptr<Asset::Texture>>>("Texture", "asset");
            std::string l_texturePath = _entityData.traverse_raw_get<std::string>("Texture", "texturePath");
                
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

                    _entityData["Texture"]["asset"] = l_textureWPtr;
                    l_texturePtr = _entityData.traverse_raw_get<std::weak_ptr<Asset::Texture>>("Texture", "asset");
                }
            }
            else
            {
                // Check if path has changed
                if(l_texturePath != "")
                {
                    auto l_textureWPtr = Asset::Load<Asset::Texture>(l_texturePath);

                    if(l_textureWPtr.lock() == nullptr)
                    {
                        Log::Warning("Object cannot be drawn: invalid texture path");
                        return;
                    }

                    _entityData["Texture"]["asset"] = l_textureWPtr;
                    l_texturePtr = _entityData.traverse_raw_get<std::weak_ptr<Asset::Texture>>("Texture", "asset");
                }
            }
            
            l_texture = l_texturePtr.value().lock(); 
        }

        sol::optional<sol::table> l_shaderCatOpt = _entityData.traverse_raw_get<sol::optional<sol::table>>("Shader");

        std::shared_ptr<Asset::Shader> l_shader;
        
        if(l_shaderCatOpt.has_value())
        {
            sol::optional<std::weak_ptr<Asset::Shader>> l_shaderPtr = _entityData.traverse_raw_get<sol::optional<std::weak_ptr<Asset::Shader>>>("Shader", "asset");
            std::string l_shaderPath = _entityData.traverse_raw_get<std::string>("Shader", "shaderPath");
                
            if(!l_shaderPtr.has_value())
            {
                if(l_shaderPath == "")
                {
                    l_shaderPath = DefaultShaderPath;
                }

                auto l_shaderWPtr = Asset::Load<Asset::Shader>(l_shaderPath);

                if(l_shaderWPtr.lock() == nullptr)
                {
                    Log::Warning("Object cannot be drawn: invalid shader path");
                    return;
                }

                _entityData["Shader"]["asset"] = l_shaderWPtr;
                l_shaderPtr = _entityData.traverse_raw_get<std::weak_ptr<Asset::Shader>>("Shader", "asset");
            }
            else
            {
                // Check if path has changed
                if(l_shaderPath != "")
                {
                    auto l_shaderWPtr = Asset::Load<Asset::Shader>(l_shaderPath);

                    if(l_shaderWPtr.lock() == nullptr)
                    {
                        Log::Warning("Object cannot be drawn: invalid shader path");
                        return;
                    }

                    _entityData["Shader"]["asset"] = l_shaderWPtr;
                    l_shaderPtr = _entityData.traverse_raw_get<std::weak_ptr<Asset::Shader>>("Shader", "asset");
                }
            }

            l_shader = l_shaderPtr.value().lock();
        }
        else
        {
            // Check if path has changed
            l_shader = Asset::Load<Asset::Shader>(DefaultShaderPath).lock();
        }
        
        l_shader->Use();
        
        glBindVertexArray(l_model->GetVAO());

        if(l_texture != nullptr)
        {
            glBindTexture(GL_TEXTURE_2D, l_texture->GetID());
        }

        sol::table l_transform = _entityData["Transform"];

        Vector3 l_modelPos(l_transform.raw_get<Vector2>("Position"), l_transform.raw_get<float>("Layer"));
        Vector3 l_modelRotation(0.0f, l_transform.raw_get<float>("Rotation"), 0.0f);
        Vector3 l_modelScale(l_transform.raw_get<Vector2>("Scale"), 0.5f);

        glm::mat4 l_modelMat = CalculateModelMatrix(l_modelPos,
                                                    l_modelRotation,
                                                    l_modelScale);

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

    void Camera::Draw(
                        sol::table _transform,
                        std::weak_ptr<Asset::Model> _model,
                        std::weak_ptr<Asset::Texture> _texture,
                        std::weak_ptr<Asset::Shader> _shader
                     )
    {
        auto l_shader = _shader.lock();
        if(l_shader == nullptr)
        {
            Log::Warning("Object cannot be drawn: invalid shader");
            return;
        }

        auto l_model = _model.lock();
        if(l_model == nullptr)
        {
            Log::Warning("Object cannot be drawn: invalid model");
            return;
        }

        auto l_texture = _texture.lock();
        if(l_texture == nullptr)
        {
            Log::Warning("Object cannot be drawn: invalid texture");
            return;
        }

        l_shader->Use();
        
        glBindVertexArray(l_model->GetVAO());

        if(l_texture != nullptr)
        {
            glBindTexture(GL_TEXTURE_2D, l_texture->GetID());
        }

        Vector3 l_modelPos(_transform.raw_get<Vector2>("Position"), _transform.raw_get<float>("Layer"));
        Vector3 l_modelRotation(0.0f, _transform.raw_get<float>("Rotation"), 0.0f);
        Vector3 l_modelScale(_transform.raw_get<Vector2>("Scale"), 0.5f);

        glm::mat4 l_modelMat = CalculateModelMatrix(l_modelPos,
                                                    l_modelRotation,
                                                    l_modelScale);

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
    };

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

    Vector2 Camera::WorldToScreenPoint(Vector2 _worldPoint)
    {
        auto l_window = IO::Manager::Instance().lock()->GetWindow().lock();
        Vector2 l_screenSize = l_window->GetScreenSize();

        if(m_viewDirty)
        {
            m_view = CalculateModelMatrix(m_position, Vector3(0.0f, m_rotation, 0.0f), Vector3(1.0f));
            m_view = glm::inverse(m_view);
            m_viewDirty = false;
        }

        glm::vec3 l_world3(_worldPoint.x, _worldPoint.y, 0.0f);

        glm::vec4 l_clipSpacePos = m_projection * (m_view * glm::vec4(l_world3, 1.0f));

        if(l_clipSpacePos.w != 0.0f)
        {
            glm::vec2 l_ndcSpacePos = glm::vec2(l_clipSpacePos.x, l_clipSpacePos.y) / l_clipSpacePos.w;
            glm::vec2 l_windowSpacePos = (l_ndcSpacePos + 1.0f) * 0.5f * glm::vec2(l_screenSize);

            return Vector2(l_windowSpacePos);
        }

        return Vector2(0.0f, 0.0f);
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