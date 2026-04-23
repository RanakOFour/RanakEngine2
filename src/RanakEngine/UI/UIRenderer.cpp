#include "RanakEngine/UI/UIRenderer.h"
#include "RanakEngine/UI/DefaultFont.h"

#include "RanakEngine/Log.h"

#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <cstdio>
#include <cstring>

namespace RanakEngine::UI
{

// ── Embedded shader sources (rects / images) ────────────────────────────────

static constexpr const char* k_vertexShaderSrc = R"glsl(
#version 430

in vec3 a_Position;
in vec2 a_PixelColor;

uniform mat4 u_Projection;
uniform mat4 u_Model;

out vec2 v_texCoord;

void main()
{
    gl_Position = u_Projection * u_Model * vec4(a_Position, 1.0);
    v_texCoord  = a_PixelColor;
}
)glsl";

static constexpr const char* k_fragmentShaderSrc = R"glsl(
#version 430

in vec2 v_texCoord;

uniform sampler2D u_Texture;
uniform vec4      u_Color;
uniform int       u_UseTexture;

out vec4 o_fragColor;

void main()
{
    if (u_UseTexture == 1)
    {
        vec4 tex = texture(u_Texture, v_texCoord);
        o_fragColor = tex * u_Color;
    }
    else
    {
        o_fragColor = u_Color;
    }
}
)glsl";

// Dedicated text shaders (LearnOpenGL style)

static constexpr const char* k_textVertexShaderSrc = R"glsl(
#version 430

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)glsl";

static constexpr const char* k_textFragmentShaderSrc = R"glsl(
#version 430

in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = textColor * sampled;
}
)glsl";

static GLuint CompileShader(GLenum _type, const std::string& _src)
{
    GLuint l_id = glCreateShader(_type);
    const char* l_code = _src.c_str();
    glShaderSource(l_id, 1, &l_code, nullptr);
    glCompileShader(l_id);

    GLint l_ok = 0;
    glGetShaderiv(l_id, GL_COMPILE_STATUS, &l_ok);
    if (!l_ok)
    {
        char l_log[512];
        glGetShaderInfoLog(l_id, 512, nullptr, l_log);
        Log::Message(std::string("UIRenderer: shader compile error:\n") + l_log);
    }
    return l_id;
}

UIRenderer::UIRenderer() = default;

UIRenderer::~UIRenderer()
{
    if (m_quadVAO)            glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO)            glDeleteBuffers(1, &m_quadVBO);
    if (m_shaderProgram)      glDeleteProgram(m_shaderProgram);
    if (m_textVAO)            glDeleteVertexArrays(1, &m_textVAO);
    if (m_textVBO)            glDeleteBuffers(1, &m_textVBO);
    if (m_textShaderProgram)  glDeleteProgram(m_textShaderProgram);

    for (auto& [ch, info] : m_characters)
    {
        if (info.textureID) glDeleteTextures(1, &info.textureID);
    }
}

void UIRenderer::Init(IO::Manager& _io,
                      const unsigned char* _fontData,
                      unsigned int _fontDataSize,
                      float _fontSize)
{
    m_io = &_io;
    m_bakedFontSize = _fontSize;

    // Quad VAO/VBO

    float l_verts[] = {
        // x,    y,    z,    u,    v
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(l_verts), l_verts, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    // Compile and link shaders

    GLuint l_vert = CompileShader(GL_VERTEX_SHADER,   k_vertexShaderSrc);
    GLuint l_frag = CompileShader(GL_FRAGMENT_SHADER, k_fragmentShaderSrc);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, l_vert);
    glAttachShader(m_shaderProgram, l_frag);
    glBindAttribLocation(m_shaderProgram, 0, "a_Position");
    glBindAttribLocation(m_shaderProgram, 1, "a_PixelColor");
    glLinkProgram(m_shaderProgram);

    GLint l_ok = 0;
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &l_ok);
    if (!l_ok)
    {
        char l_log[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, l_log);
        Log::Message(std::string("UIRenderer: rect shader link error:\n") + l_log);
    }

    glDeleteShader(l_vert);
    glDeleteShader(l_frag);

    m_locProjection = glGetUniformLocation(m_shaderProgram, "u_Projection");
    m_locModel      = glGetUniformLocation(m_shaderProgram, "u_Model");
    m_locColor      = glGetUniformLocation(m_shaderProgram, "u_Color");
    m_locUseTexture = glGetUniformLocation(m_shaderProgram, "u_UseTexture");
    m_locTexture    = glGetUniformLocation(m_shaderProgram, "u_Texture");

    // Text shader

    GLuint l_textVert = CompileShader(GL_VERTEX_SHADER,   k_textVertexShaderSrc);
    GLuint l_textFrag = CompileShader(GL_FRAGMENT_SHADER, k_textFragmentShaderSrc);

    m_textShaderProgram = glCreateProgram();
    glAttachShader(m_textShaderProgram, l_textVert);
    glAttachShader(m_textShaderProgram, l_textFrag);
    glLinkProgram(m_textShaderProgram);

    l_ok = 0;
    glGetProgramiv(m_textShaderProgram, GL_LINK_STATUS, &l_ok);
    if (!l_ok)
    {
        char l_log[512];
        glGetProgramInfoLog(m_textShaderProgram, 512, nullptr, l_log);
        Log::Message(std::string("UIRenderer: text shader link error:\n") + l_log);
    }

    glDeleteShader(l_textVert);
    glDeleteShader(l_textFrag);

    m_locTextProjection = glGetUniformLocation(m_textShaderProgram, "projection");
    m_locTextColor      = glGetUniformLocation(m_textShaderProgram, "textColor");
    m_locTextSampler    = glGetUniformLocation(m_textShaderProgram, "text");

    // Text VAO/VBO (6 verts × 4 floats each, updated per glyph)

    glGenVertexArrays(1, &m_textVAO);
    glGenBuffers(1, &m_textVBO);

    glBindVertexArray(m_textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Load glyphs with FreeType
    const unsigned char* l_fontDataPtr  = _fontData  ? _fontData  : k_defaultFontData;
    unsigned int         l_fontDataSize = _fontData  ? _fontDataSize : k_defaultFontDataSize;

    if (!l_fontDataPtr || l_fontDataSize == 0)
    {
        Log::Message("UIRenderer: no font data available.");
        return;
    }

    FT_Library l_ft;
    if (FT_Init_FreeType(&l_ft))
    {
        Log::Error("UIRenderer: could not init FreeType library.");
        return;
    }

    FT_Face l_face;
    if (FT_New_Memory_Face(l_ft, l_fontDataPtr, l_fontDataSize, 0, &l_face))
    {
        Log::Error("UIRenderer: failed to load font face from memory.");
        FT_Done_FreeType(l_ft);
        return;
    }

    FT_Set_Pixel_Sizes(l_face, 0, static_cast<FT_UInt>(_fontSize));

    // Disable byte-alignment restriction (glyph bitmaps are not 4-byte aligned).
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; ++c)
    {
        if (FT_Load_Char(l_face, c, FT_LOAD_RENDER))
        {
            Log::Message("UIRenderer: failed to load glyph '" +
                         std::string(1, static_cast<char>(c)) + "'");
            continue;
        }

        GLuint l_texId = 0;
        glGenTextures(1, &l_texId);
        glBindTexture(GL_TEXTURE_2D, l_texId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     l_face->glyph->bitmap.width,
                     l_face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     l_face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character l_ch = {
            l_texId,
            static_cast<int>(l_face->glyph->bitmap.width),
            static_cast<int>(l_face->glyph->bitmap.rows),
            l_face->glyph->bitmap_left,
            l_face->glyph->bitmap_top,
            static_cast<unsigned int>(l_face->glyph->advance.x)
        };
        m_characters.insert(std::pair<char, Character>(static_cast<char>(c), l_ch));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(l_face);
    FT_Done_FreeType(l_ft);

    Log::Message("UIRenderer: initialised with FreeType (fontSize: " +
                 std::to_string(static_cast<int>(_fontSize)) + "px, " +
                 std::to_string(m_characters.size()) + " glyphs loaded)");
}

void UIRenderer::BeginFrame(float _screenW, float _screenH)
{
    if (_screenW <= 0.0f || _screenH <= 0.0f)
        return;

    m_screenW = _screenW;
    m_screenH = _screenH;

    glm::mat4 l_proj = glm::ortho(0.0f, _screenW, _screenH, 0.0f, -1.0f, 1.0f);
    std::memcpy(m_projMatrix, glm::value_ptr(l_proj), sizeof(m_projMatrix));

    const auto& l_mouse = m_io->GetMouseInfo();
    m_mouseX        = l_mouse.position.x;
    m_mouseY        = l_mouse.position.y;
    m_mouseClicked  = l_mouse.LMBDown && !m_mousePrevDown;
    m_mousePrevDown = l_mouse.LMBDown;
    m_mouseDown     = l_mouse.LMBDown;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
}

void UIRenderer::EndFrame()
{
    glEnable(GL_DEPTH_TEST);
}

void UIRenderer::DrawQuad(float _x, float _y, float _w, float _h,
                          float _r, float _g, float _b, float _a,
                          unsigned int _texId, bool _useTexture)
{
    // Ensure correct state regardless of what 3D rendering rules may have changed.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_x, _y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_w, _h, 1.0f));

    glUseProgram(m_shaderProgram);

    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, m_projMatrix);
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _r, _g, _b, _a);
    glUniform1i(m_locUseTexture, _useTexture ? 1 : 0);

    if (_useTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _texId);
        glUniform1i(m_locTexture, 0);
    }

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    if (_useTexture)
        glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
}

void UIRenderer::DrawRect(float _x, float _y, float _w, float _h,
                          float _r, float _g, float _b, float _a)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Rect;
        cmd.x = _x; cmd.y = _y; cmd.w = _w; cmd.h = _h;
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    DrawQuad(_x, _y, _w, _h, _r, _g, _b, _a, 0, false);
}

void UIRenderer::DrawRectOutline(float _x, float _y, float _w, float _h,
                                 float _r, float _g, float _b, float _a,
                                 float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::RectOutline;
        cmd.x = _x; cmd.y = _y; cmd.w = _w; cmd.h = _h;
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        cmd.thickness = _thickness;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    DrawRect(_x,                    _y,                    _w,          _thickness, _r, _g, _b, _a);
    DrawRect(_x,                    _y + _h - _thickness,  _w,          _thickness, _r, _g, _b, _a);
    DrawRect(_x,                    _y + _thickness,        _thickness,  _h - 2.0f * _thickness, _r, _g, _b, _a);
    DrawRect(_x + _w - _thickness,  _y + _thickness,        _thickness,  _h - 2.0f * _thickness, _r, _g, _b, _a);
}

void UIRenderer::DrawText(float _x, float _y,
                          float _r, float _g, float _b, float _a,
                          const std::string& _text, float _fontSize,
                          bool _centered)
{
    if (_text.empty()) return;
    if (m_characters.empty()) return;

    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Text;
        cmd.x = _x; cmd.y = _y;
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        cmd.text = _text;
        cmd.fontSize = _fontSize;
        cmd.centered = _centered;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float l_scale = _fontSize / m_bakedFontSize;

    // Measure advance for width calculation (only if centered)
    float l_totalWidth = 0.0f;
    if (_centered)
    {
        for (char c : _text)
        {
            auto l_it = m_characters.find(c);
            if (l_it == m_characters.end()) l_it = m_characters.find(' ');
            if (l_it != m_characters.end())
                l_totalWidth += (l_it->second.advance >> 6) * l_scale;
        }
    }

    float l_cursorX = _centered ? _x - l_totalWidth * 0.5f : _x;
    float l_startY  = _centered ? _y - _fontSize * 0.5f    : _y;

    // Activate text shader
    glUseProgram(m_textShaderProgram);
    glUniformMatrix4fv(m_locTextProjection, 1, GL_FALSE, m_projMatrix);
    glUniform4f(m_locTextColor, _r, _g, _b, _a);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_locTextSampler, 0);
    glBindVertexArray(m_textVAO);

    // Render each glyph
    // Projection is Y-down (top-left = 0,0).
    // bearingY is the distance from the baseline to the TOP of the glyph.
    // In a Y-down system, ypos = baseline - bearingY.
    // We treat l_startY as the top of the text line, so baseline = l_startY + fontSize.

    float l_baseline = l_startY + _fontSize;

    for (char c : _text)
    {
        auto l_it = m_characters.find(c);
        if (l_it == m_characters.end())
        {
            // Skip unknown characters; advance by space width.
            auto l_sp = m_characters.find(' ');
            if (l_sp != m_characters.end())
                l_cursorX += (l_sp->second.advance >> 6) * l_scale;
            continue;
        }

        const Character& ch = l_it->second;

        float l_xpos = l_cursorX + ch.bearingX * l_scale;
        float l_ypos = l_baseline - ch.bearingY * l_scale; // top of glyph
        float l_w    = ch.sizeX * l_scale;
        float l_h    = ch.sizeY * l_scale;

        // 6 vertices, 4 floats each (x, y, u, v) — two triangles.
        float l_verts[6][4] = {
            { l_xpos,       l_ypos,       0.0f, 0.0f },
            { l_xpos + l_w, l_ypos,       1.0f, 0.0f },
            { l_xpos + l_w, l_ypos + l_h, 1.0f, 1.0f },

            { l_xpos,       l_ypos,       0.0f, 0.0f },
            { l_xpos + l_w, l_ypos + l_h, 1.0f, 1.0f },
            { l_xpos,       l_ypos + l_h, 0.0f, 1.0f },
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(l_verts), l_verts);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        l_cursorX += (ch.advance >> 6) * l_scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void UIRenderer::DrawImage(unsigned int _texId,
                           float _x, float _y, float _w, float _h,
                           float _tR, float _tG, float _tB, float _tA)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Image;
        cmd.texId = _texId;
        cmd.x = _x; cmd.y = _y; cmd.w = _w; cmd.h = _h;
        cmd.r = _tR; cmd.g = _tG; cmd.b = _tB; cmd.a = _tA;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    DrawQuad(_x, _y, _w, _h, _tR, _tG, _tB, _tA, _texId, true);
}

void UIRenderer::Flush()
{
    if (m_commandBuffer.empty()) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_buffering = false;
    for (const auto& cmd : m_commandBuffer)
    {
        switch (cmd.type)
        {
            case DrawCommand::Type::Rect:
                DrawRect(cmd.x, cmd.y, cmd.w, cmd.h,
                         cmd.r, cmd.g, cmd.b, cmd.a);
                break;
            case DrawCommand::Type::RectOutline:
                DrawRectOutline(cmd.x, cmd.y, cmd.w, cmd.h,
                                cmd.r, cmd.g, cmd.b, cmd.a, cmd.thickness);
                break;
            case DrawCommand::Type::Text:
                DrawText(cmd.x, cmd.y,
                         cmd.r, cmd.g, cmd.b, cmd.a,
                         cmd.text, cmd.fontSize, cmd.centered);
                break;
            case DrawCommand::Type::Image:
                DrawImage(cmd.texId, cmd.x, cmd.y, cmd.w, cmd.h,
                          cmd.r, cmd.g, cmd.b, cmd.a);
                break;
        }
    }
    m_commandBuffer.clear();
    m_buffering = true;

    glEnable(GL_DEPTH_TEST);
}

bool UIRenderer::IsHovered(float _x, float _y, float _w, float _h) const
{
    return m_mouseX >= _x && m_mouseX <= _x + _w &&
           m_mouseY >= _y && m_mouseY <= _y + _h;
}

bool UIRenderer::IsClicked(float _x, float _y, float _w, float _h) const
{
    return IsHovered(_x, _y, _w, _h) && m_mouseClicked;
}

}