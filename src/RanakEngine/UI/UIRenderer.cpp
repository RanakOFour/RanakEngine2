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

#if WIN32
#define M_PI 3.14159265358979323846
#endif

namespace RanakEngine::UI
{

static constexpr const char* k_vertexShaderSrc = R"glsl(#version 430

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

static constexpr const char* k_fragmentShaderSrc = R"glsl(#version 430

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

static constexpr const char* k_textVertexShaderSrc = R"glsl(#version 430

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)glsl";

static constexpr const char* k_textFragmentShaderSrc = R"glsl(#version 430

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

UIRenderer::UIRenderer()
{
    m_IOwptr = IO::Manager::Instance();
}

UIRenderer::~UIRenderer()
{
    if (m_quadVAO)            glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO)            glDeleteBuffers(1, &m_quadVBO);
    if (m_circleVAO)          glDeleteVertexArrays(1, &m_circleVAO);
    if (m_circleVBO)          glDeleteBuffers(1, &m_circleVBO);
    if (m_circleLineVAO)      glDeleteVertexArrays(1, &m_circleLineVAO);
    if (m_circleLineVBO)      glDeleteBuffers(1, &m_circleLineVBO);
    if (m_semiCircleTopLineVAO) glDeleteVertexArrays(1, &m_semiCircleTopLineVAO);
    if (m_semiCircleTopLineVBO) glDeleteBuffers(1, &m_semiCircleTopLineVBO);
    if (m_semiCircleBotLineVAO) glDeleteVertexArrays(1, &m_semiCircleBotLineVAO);
    if (m_semiCircleBotLineVBO) glDeleteBuffers(1, &m_semiCircleBotLineVBO);
    if (m_shaderProgram)      glDeleteProgram(m_shaderProgram);
    if (m_textVAO)            glDeleteVertexArrays(1, &m_textVAO);
    if (m_textVBO)            glDeleteBuffers(1, &m_textVBO);
    if (m_textShaderProgram)  glDeleteProgram(m_textShaderProgram);

    for (auto& [ch, info] : m_characters)
    {
        if (info.textureID)
        {
            glDeleteTextures(1, &info.textureID);
        }
    }
}

void UIRenderer::Init(std::weak_ptr<IO::Manager> _io,
                      const unsigned char* _fontData,
                      unsigned int _fontDataSize,
                      float _fontSize)
{
    m_IOwptr = _io;
    m_bakedFontSize = _fontSize;

    // Quad VAO/VBO

    // Quad: two triangles, wound CW in local (Y-up) coords so that
    // they become CCW after the Y-down ortho projection flips Y.
    // (glFrontFace defaults to CCW.)
    float l_verts[] = {
        // x,    y,    z,    u,    v
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
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

    // Circle VAO/VBO — triangle fan for filled circle (radius=1, centred at origin).
    // Perimeter wound CW locally so it becomes CCW after the Y-down ortho flip.
    {
        constexpr int k_segments = 64;
        std::vector<float> l_circleVerts;
        l_circleVerts.reserve((k_segments + 2) * 5);
        // centre vertex
        l_circleVerts.insert(l_circleVerts.end(), {0.0f, 0.0f, 0.0f, 0.5f, 0.5f});
        for (int i = k_segments; i >= 0; --i)
        {
            float l_angle = (float)i / (float)k_segments * 2.0f * M_PI;
            float l_x = std::cos(l_angle);
            float l_y = std::sin(l_angle);
            float l_u = l_x * 0.5f + 0.5f;
            float l_v = l_y * 0.5f + 0.5f;
            l_circleVerts.insert(l_circleVerts.end(), {l_x, l_y, 0.0f, l_u, l_v});
        }
        m_circleVertCount = (unsigned int)l_circleVerts.size() / 5;

        glGenVertexArrays(1, &m_circleVAO);
        glGenBuffers(1, &m_circleVBO);

        glBindVertexArray(m_circleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_circleVBO);
        glBufferData(GL_ARRAY_BUFFER, l_circleVerts.size() * sizeof(float),
                     l_circleVerts.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
    }

    // Circle line-loop VAOs — perimeter-only for glLineWidth-thick outlines.
    // Uses GL_LINE_LOOP so _thickness → glLineWidth gives the caller direct
    // control over the visual width of the outline.
    auto genLineLoop = [&](const char* _name, float _angle0, float _angle1,
                           GLuint& _vao, GLuint& _vbo, GLuint& _count)
    {
        constexpr int k_segs = 64;
        std::vector<float> l_verts;
        l_verts.reserve((k_segs + 1) * 5);
        for (int i = 0; i <= k_segs; ++i)
        {
            float l_a = _angle0 + (float)i / (float)k_segs * (_angle1 - _angle0);
            l_verts.insert(l_verts.end(), {std::cos(l_a), std::sin(l_a), 0.0f, 0.5f, 0.5f});
        }
        _count = (GLuint)l_verts.size() / 5;

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, l_verts.size() * sizeof(float),
                     l_verts.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
        (void)_name;
    };

    genLineLoop("circle",  0.0f, 2.0f * M_PI,  m_circleLineVAO,  m_circleLineVBO,  m_circleLineVertCount);
    genLineLoop("semiTop", 0.0f, M_PI,         m_semiCircleTopLineVAO, m_semiCircleTopLineVBO, m_semiCircleTopLineVertCount);
    genLineLoop("semiBot", M_PI, 2.0f * M_PI,  m_semiCircleBotLineVAO, m_semiCircleBotLineVBO, m_semiCircleBotLineVertCount);

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

    m_locColor      = glGetUniformLocation(m_shaderProgram, "u_Color");
    m_locModel      = glGetUniformLocation(m_shaderProgram, "u_Model");
    m_locProjection = glGetUniformLocation(m_shaderProgram, "u_Projection");
    m_locTexture    = glGetUniformLocation(m_shaderProgram, "u_Texture");
    m_locUseTexture = glGetUniformLocation(m_shaderProgram, "u_UseTexture");

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
    m_locTextSampler    = glGetUniformLocation(m_textShaderProgram, "text");
    m_locTextColor      = glGetUniformLocation(m_textShaderProgram, "textColor");

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
                     l_face->glyph->bitmap.buffer
                    );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character l_ch = {
            l_texId,
            (int)l_face->glyph->bitmap.width,
            (int)l_face->glyph->bitmap.rows,
            (int)l_face->glyph->bitmap_left,
            (int)l_face->glyph->bitmap_top,
            (unsigned int)l_face->glyph->advance.x
        };

        m_characters.insert(std::pair<char, Character>(c, l_ch));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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

    m_projMatrix = glm::ortho(0.0f, m_screenW, m_screenH, 0.0f);

    if (auto io = m_IOwptr.lock())
    {
        const auto& l_mouse = io->GetMouseInfo();
        m_mouseX        = l_mouse.position.x;
        m_mouseY        = l_mouse.position.y;
        m_mouseClicked  = l_mouse.LMBDown && !m_mousePrevDown;
        m_mousePrevDown = l_mouse.LMBDown;
        m_mouseDown     = l_mouse.LMBDown;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
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

    float l_cursorX = _centered ? 
                                  _x - l_totalWidth * 0.5f 
                                : _x;

    float l_startY  = _centered ? 
                                  _y - _fontSize * 0.5f
                                : _y;

    // Activate text shader
    glUseProgram(m_textShaderProgram);
    glUniformMatrix4fv(m_locTextProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniform4f(m_locTextColor, _r, _g, _b, _a);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_textVAO);

    // Render each glyph

    for (char c : _text)
    {
        auto l_it = m_characters.find(c);
        if (l_it == m_characters.end())
        {
            // Skip unknown characters; advance by space width.
            auto l_sp = m_characters.find(' ');
            if (l_sp != m_characters.end())
            {
                l_cursorX += (l_sp->second.advance >> 6) * l_scale;
            }
            continue;
        }

        const Character& ch = l_it->second;

        float l_xpos = _x + ch.bearingX * l_scale;
        float l_ypos = _y + (ch.sizeY - ch.bearingY) * l_scale;
        float l_w    = ch.sizeX * l_scale;
        float l_h    = ch.sizeY * l_scale;

        // 6 vertices, 4 floats each (x, y, u, v) — two triangles.
        // Y-up projection: l_ypos is visual top, l_ypos - l_h is visual bottom.
        // v=0 maps to top of glyph bitmap, v=1 to bottom.
        float l_verts[6][4] = {
            { l_xpos,       l_ypos - l_h, 0.0f, 0.0f },
            { l_xpos,       l_ypos,       0.0f, 1.0f },
            { l_xpos + l_w, l_ypos,       1.0f, 1.0f },

            { l_xpos,       l_ypos - l_h, 0.0f, 0.0f },
            { l_xpos + l_w, l_ypos,       1.0f, 1.0f },
            { l_xpos + l_w, l_ypos - l_h,       1.0f, 0.0f },
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(l_verts), l_verts);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        _x += (ch.advance >> 6) * l_scale;
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

void UIRenderer::DrawCircle(float _x, float _y, float _radius,
                            float _r, float _g, float _b, float _a)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Circle;
        cmd.x = _x; cmd.y = _y;
        cmd.w = _radius;  // w = radius for circles
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_x, _y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_radius, _radius, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _r, _g, _b, _a);
    glUniform1i(m_locUseTexture, 0);

    glBindVertexArray(m_circleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_circleVertCount);
    glBindVertexArray(0);
    glUseProgram(0);
}

void UIRenderer::DrawCircleOutline(float _x, float _y, float _radius,
                                   float _r, float _g, float _b, float _a,
                                   float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::CircleOutline;
        cmd.x = _x; cmd.y = _y;
        cmd.w = _radius; cmd.thickness = _thickness;
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float l_t = _thickness;
    if (l_t < 0.5f) l_t = 0.5f;
    glLineWidth(l_t);

    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_x, _y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_radius, _radius, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _r, _g, _b, _a);
    glUniform1i(m_locUseTexture, 0);

    glBindVertexArray(m_circleLineVAO);
    glDrawArrays(GL_LINE_LOOP, 0, m_circleLineVertCount);
    glBindVertexArray(0);
    glUseProgram(0);

    glLineWidth(1.0f);
}

void UIRenderer::DrawCapsule(float _x, float _y, float _w, float _h,
                             float _r, float _g, float _b, float _a)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Capsule;
        cmd.x = _x; cmd.y = _y; cmd.w = _w; cmd.h = _h;
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    // Capsule = centre rectangle + two half-circle endcaps
    float l_r = _w * 0.5f;  // radius = half-width
    float l_bodyH = _h - _w; // height minus endcaps
    if (l_bodyH < 0.0f) l_bodyH = 0.0f;

    DrawRect(_x, _y + l_r, _w, l_bodyH, _r, _g, _b, _a);
    DrawCircle(_x + l_r, _y + l_r, l_r, _r, _g, _b, _a);
    DrawCircle(_x + l_r, _y + l_r + l_bodyH, l_r, _r, _g, _b, _a);
}

void UIRenderer::DrawCapsuleOutline(float _x, float _y, float _w, float _h,
                                    float _r, float _g, float _b, float _a,
                                    float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::CapsuleOutline;
        cmd.x = _x; cmd.y = _y; cmd.w = _w; cmd.h = _h;
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        cmd.thickness = _thickness;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    float l_r = _w * 0.5f;
    float l_bodyH = _h - _w;
    if (l_bodyH < 0.0f) l_bodyH = 0.0f;

    float l_topY    = _y + l_r;
    float l_bottomY = _y + l_r + l_bodyH;

    // Rectangular body outline
    DrawRectOutline(_x, l_topY, _w, l_bodyH, _r, _g, _b, _a, _thickness);
    // Half-circle endcaps. The Y-down orthographic projection flips the
    // circle geometry: the "top" half-ring (angles 0→π, Y-up) renders as
    // the bottom half on screen, and vice versa. So we swap them here so
    // the visible arcs sit on the correct ends of the capsule.
    DrawSemiCircleBotOutline(_x + l_r, l_topY,    l_r, _r, _g, _b, _a, _thickness);
    DrawSemiCircleTopOutline(_x + l_r, l_bottomY, l_r, _r, _g, _b, _a, _thickness);
}

void UIRenderer::DrawSemiCircleTop(float _x, float _y, float _radius,
                                   float _r, float _g, float _b, float _a)
{
    // Filled semi-circle uses same triangle fan but only top half.
    // For simplicity, draw a full filled circle — the capsule's centre rect
    // will cover the bottom half.
    DrawCircle(_x, _y, _radius, _r, _g, _b, _a);
}

void UIRenderer::DrawSemiCircleTopOutline(float _x, float _y, float _radius,
                                          float _r, float _g, float _b, float _a,
                                          float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::SemiCircleTopOutline;
        cmd.x = _x; cmd.y = _y; cmd.w = _radius;
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        cmd.thickness = _thickness;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(_thickness < 0.5f ? 0.5f : _thickness);

    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_x, _y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_radius, _radius, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _r, _g, _b, _a);
    glUniform1i(m_locUseTexture, 0);

    glBindVertexArray(m_semiCircleTopLineVAO);
    glDrawArrays(GL_LINE_LOOP, 0, m_semiCircleTopLineVertCount);
    glBindVertexArray(0);
    glUseProgram(0);

    glLineWidth(1.0f);
}

void UIRenderer::DrawSemiCircleBot(float _x, float _y, float _radius,
                                   float _r, float _g, float _b, float _a)
{
    DrawCircle(_x, _y, _radius, _r, _g, _b, _a);
}

void UIRenderer::DrawSemiCircleBotOutline(float _x, float _y, float _radius,
                                          float _r, float _g, float _b, float _a,
                                          float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::SemiCircleBotOutline;
        cmd.x = _x; cmd.y = _y; cmd.w = _radius;
        cmd.r = _r; cmd.g = _g; cmd.b = _b; cmd.a = _a;
        cmd.thickness = _thickness;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(_thickness < 0.5f ? 0.5f : _thickness);

    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_x, _y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_radius, _radius, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _r, _g, _b, _a);
    glUniform1i(m_locUseTexture, 0);

    glBindVertexArray(m_semiCircleBotLineVAO);
    glDrawArrays(GL_LINE_LOOP, 0, m_semiCircleBotLineVertCount);
    glBindVertexArray(0);
    glUseProgram(0);

    glLineWidth(1.0f);
}

void UIRenderer::Flush()
{
    if (m_commandBuffer.empty()) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_buffering = false;
    for (const auto& l_cmd : m_commandBuffer)
    {
        switch (l_cmd.type)
        {
            case DrawCommand::Type::Rect:
                DrawRect(l_cmd.x, l_cmd.y, l_cmd.w, l_cmd.h,
                         l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a);
                break;
            case DrawCommand::Type::RectOutline:
                DrawRectOutline(l_cmd.x, l_cmd.y, l_cmd.w, l_cmd.h,
                                l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a, l_cmd.thickness);
                break;
            case DrawCommand::Type::Text:
                DrawText(l_cmd.x, l_cmd.y,
                         l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a,
                         l_cmd.text, l_cmd.fontSize, l_cmd.centered);
                break;
            case DrawCommand::Type::Image:
                DrawImage(l_cmd.texId, l_cmd.x, l_cmd.y, l_cmd.w, l_cmd.h,
                          l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a);
                break;
            case DrawCommand::Type::Circle:
                DrawCircle(l_cmd.x, l_cmd.y, l_cmd.w,
                           l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a);
                break;
            case DrawCommand::Type::CircleOutline:
                DrawCircleOutline(l_cmd.x, l_cmd.y, l_cmd.w,
                                  l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a, l_cmd.thickness);
                break;
            case DrawCommand::Type::Capsule:
                DrawCapsule(l_cmd.x, l_cmd.y, l_cmd.w, l_cmd.h,
                            l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a);
                break;
            case DrawCommand::Type::CapsuleOutline:
                DrawCapsuleOutline(l_cmd.x, l_cmd.y, l_cmd.w, l_cmd.h,
                                   l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a, l_cmd.thickness);
                break;
            case DrawCommand::Type::SemiCircleTopOutline:
                DrawSemiCircleTopOutline(l_cmd.x, l_cmd.y, l_cmd.w,
                                         l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a, l_cmd.thickness);
                break;
            case DrawCommand::Type::SemiCircleBotOutline:
                DrawSemiCircleBotOutline(l_cmd.x, l_cmd.y, l_cmd.w,
                                         l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a, l_cmd.thickness);
                break;
            case DrawCommand::Type::SemiCircleTop:
                DrawSemiCircleTop(l_cmd.x, l_cmd.y, l_cmd.w,
                                  l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a);
                break;
            case DrawCommand::Type::SemiCircleBot:
                DrawSemiCircleBot(l_cmd.x, l_cmd.y, l_cmd.w,
                                  l_cmd.r, l_cmd.g, l_cmd.b, l_cmd.a);
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