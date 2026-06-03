#ifndef SHADER_H
#define SHADER_H

#include "RanakEngine/Asset/AssetBase.h"

#include "RanakEngine/Math/Vector2.h"
#include "RanakEngine/Math/Vector3.h"
#include "RanakEngine/Math/Vector4.h"

#include "GL/glew.h"

#include "GLM/ext/matrix_float4x4.hpp"
#include "GLM/ext/matrix_float3x3.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace RanakEngine::Asset
{
    /**
    * @struct ShaderProperty
    * @brief Represents a single shader uniform property
    * 
    * Stores information about a shader uniform including its name, type,
    * value, and location.
    */
    struct ShaderProperty
    {
        /** @brief Name of the uniform */
        std::string name;
        
        /**
        * @enum PropertyType
        * @brief Supported uniform types
        */
        enum PropertyType
        {
            INT,
            FLOAT,
            BOOL,
            VEC2,
            VEC3,
            VEC4,
            SAMPLER2D,
            TEXTURE2D
        } type;

        /**
        * @union Value
        * @brief Union storing the property value based on type
        */
        union Value
        {
            int i;            ///< Integer value
            float f;          ///< Float value
            bool b;           ///< Boolean value
            glm::vec2 vec2;   ///< 2D vector value
            glm::vec3 vec3;   ///< 3D vector value
            glm::vec4 vec4;   ///< 4D vector value
            GLint textureID; ///< Texture/Sampler ID
        } value;

        /** @brief OpenGL uniform location */
        GLuint location;

        /**
        * @brief Constructor without initial value
        * @param _name Uniform name
        * @param _type Property type
        * @param _loc OpenGL uniform location
        */
        ShaderProperty(std::string _name, PropertyType _type, GLuint _loc);

        /**
        * @brief Constructor with initial value
        * @param _name Uniform name
        * @param _type Property type
        * @param _value Initial value
        * @param _loc OpenGL uniform location
        */
        ShaderProperty(std::string _name, PropertyType _type, Value _value, GLuint _loc);

        /**
        * @brief Re-evaluate visibility based on current hidden prefixes
        */
        void RefreshVisibility();

        /**
        * @brief Re-evaluate read-only status based on current read-only prefixes
        */
        void RefreshReadOnly();

        /**
        * @brief Manually set visibility (overrides prefix-based visibility)
        * @param _visible New visibility state
        */
        void SetVisible(bool _visible);

        /**
        * @brief Manually set read-only status
        * @param _readOnly New read-only state
        */
        void SetReadOnly(bool _readOnly);

        /**
        * @brief Get string representation of the property type
        * @return Type name as string
        */
        std::string Type() const;

        /**
        * @brief Get string representation of the property value
        * @return Value as formatted string
        */
        std::string AsString() const;
    };


    /**
     * @enum ShaderType
     * @brief Enumeration of shader program types.
     */
    enum ShaderType
    {
        Compute = 0,    ///< Compute shader
        FragVert = 1    ///< Fragment and vertex shader pair
    };

    /**
     * @class Shader
     * @brief Asset class for loading and managing OpenGL shader programs.
     * 
     * Compiles and links shader source files into OpenGL shader programs.
     * Supports both compute shaders and traditional vertex/fragment pairs.
     * 
     * @see Asset
     */
    class Shader : public AssetBase
    {
        private:
        GLuint m_ID;                ///< OpenGL shader program ID
        ShaderType m_shaderType;    ///< The type of shader program
        
        std::string m_name;
        std::vector<ShaderProperty> m_properties;
        bool m_enabled;

        public:
        Shader() : AssetBase("", AssetType::SHADER), m_ID(-1), m_shaderType(FragVert) {};

        /**
         * @brief Constructs a shader from a file.
         * 
         * @param _computePath Path to the compute shader source file.
         */
        Shader(std::string _paths);

        /**
         * @brief Destructs the Shader and releases OpenGL resources.
         */
        ~Shader();

        bool LoadFromString(std::string _compute);
        bool LoadFromString(std::string _vert, std::string _frag);

        /**
         * @brief Activates this shader for rendering.
         */
        void Use();

        /**
        * @brief Update shader uniforms with stored property values
        * @note Skips properties marked as read-only since they are managed externally
        */
        void UpdateShader();

        /**
        * @brief Set the shader's display name
        * @param _n New name
        */
        void Name(std::string _name);

        /**
        * @brief Get the shader's display name
        * @return The shader name
        */
        std::string Name();

        /**
        * @brief Check if the shader is enabled
        * @return True if enabled
        */
        bool Enabled();
        
        /**
        * @brief Set the enabled state
        * @param _e New enabled state
        */
        void Enabled(bool _e);

        /**
         * @brief Gets the OpenGL shader program ID.
         * 
         * @return GLuint& Reference to the program ID.
         */
        GLuint& GetID();

        /**
         * @brief Gets the shader type.
         * 
         * @return ShaderType& Reference to the shader type.
         */
        ShaderType& GetShaderType();

        /**
        * @brief Get all properties
        * @return Reference to the properties vector
        */
        std::vector<ShaderProperty>& GetProperties();

        /**
         * @brief Sets a mat4 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The matrix value to set.
         */
        void SetUniform(const std::string _uniformName, glm::mat4 _value);

        /**
         * @brief Sets a mat3 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The matrix value to set.
         */
        void SetUniform(const std::string _uniformName, glm::mat3 _value);

        /**
         * @brief Sets a vec4 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The vector value to set.
         */
        void SetUniform(const std::string _uniformName, Vector4 _value);

        /**
         * @brief Sets a vec3 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The vector value to set.
         */
        void SetUniform(const std::string _uniformName, Vector3 _value);

        /**
         * @brief Sets a vec2 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The vector value to set.
         */
        void SetUniform(const std::string _uniformName, Vector2 _value);

        /**
         * @brief Sets a float uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The float value to set.
         */
        void SetUniform(const std::string _uniformName, float _value);

        /**
        * @brief Sets integer value
        *
        * @param _uniformName Name of the uniform variable.
        * @param _value The integer value to set.
        */
        void SetUniform(const std::string _uniformName, int _value);
    };
};

#endif