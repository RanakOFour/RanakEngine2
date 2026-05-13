#include "RanakEngine/UI/UIRenderer.h"
#include "RanakEngine/Math/Vector2.h"
#include "RanakEngine/Math/Vector4.h"
#include "RanakEngine/UI/DefaultFont.h"

#include "RanakEngine/Log.h"
#include "RanakEngine/Assets.h"
#include "RanakEngine/Asset/Model.h"

#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>

#if _WIN32
#define M_PI 3.14159265358979323846
#endif

namespace RanakEngine::UI
{

static const char* k_vertexShaderSrc = R"glsl(#version 430

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

static const char* k_fragmentShaderSrc = R"glsl(#version 430

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

static const char* k_textVertexShaderSrc = R"glsl(#version 430

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)glsl";

static const char* k_textFragmentShaderSrc = R"glsl(#version 430

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
    if (m_circleVAO)          glDeleteVertexArrays(1, &m_circleVAO);
    if (m_circleVBO)          glDeleteBuffers(1, &m_circleVBO);
    if (m_circleLineVAO)      glDeleteVertexArrays(1, &m_circleLineVAO);
    if (m_circleLineVBO)      glDeleteBuffers(1, &m_circleLineVBO);
    if (m_rectLineVAO)        glDeleteVertexArrays(1, &m_rectLineVAO);
    if (m_rectLineVBO)        glDeleteBuffers(1, &m_rectLineVBO);
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

    m_quadModel = Asset::GetDefaultModel();

    // Circle VAO/VBO — triangle fan for filled circle (radius=1, centred at origin).
    {
        const int l_segments = 64;
        std::vector<float> l_circleVerts;
        l_circleVerts.reserve((l_segments + 2) * 5);
        // centre vertex
        l_circleVerts.insert(l_circleVerts.end(), {0.0f, 0.0f, 0.0f, 0.5f, 0.5f});
        for (int i = l_segments; i >= 0; --i)
        {
            float l_angle = (float)i / (float)l_segments * 2.0f * M_PI;
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
    auto genLineLoop = [&](const char* _name, float _angle0, float _angle1,
                           GLuint& _vao, GLuint& _vbo, GLuint& _count)
    {
        const int l_segments = 64;
        std::vector<float> l_verts;
        l_verts.reserve((l_segments + 1) * 5);
        for (int i = 0; i <= l_segments; ++i)
        {
            float l_a = _angle0 + (float)i / (float)l_segments * (_angle1 - _angle0);
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

    // Rect line-loop VAO — unit-rect corners (0,0)-(1,1) for GL_LINE_LOOP outlines.
    // Layout matches genLineLoop output: 5 floats per vert (x,y,z,u,v).
    {
        const float l_rectVerts[] = {
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        };
        glGenVertexArrays(1, &m_rectLineVAO);
        glGenBuffers(1, &m_rectLineVBO);
        glBindVertexArray(m_rectLineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_rectLineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(l_rectVerts), l_rectVerts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
    }

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

void UIRenderer::BeginFrame(Vector2 _screenSize)
{
    if (_screenSize.x <= 0.0f || _screenSize.y <= 0.0f)
        return;

    m_screenW = _screenSize.x;
    m_screenH = _screenSize.y;

    // Pixel-space, Y-down for both shapes and text: (0,0) top-left, (screenW,screenH) bottom-right.
    m_projMatrix     = glm::ortho(0.0f, _screenSize.x, _screenSize.y, 0.0f, -1.0f, 1.0f);

    if (auto io = m_IOwptr.lock())
    {
        const auto& l_mouse = io->GetMouseInfo();
        // Convert pixel coords (Y=0 bottom, OpenGL window space) to NDC.
        m_mouseX        = (l_mouse.position.x / _screenSize.x) * 2.0f - 1.0f;
        m_mouseY        = (l_mouse.position.y / _screenSize.y) * 2.0f - 1.0f;
        m_mouseClicked  = l_mouse.LMBDown && !m_mousePrevDown;
        m_mousePrevDown = l_mouse.LMBDown;
        m_mouseDown     = l_mouse.LMBDown;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void UIRenderer::DrawQuad(Vector2 _pos, Vector2 _size, Vector4 _color,
                          unsigned int _texId, bool _useTexture)
{
    // Ensure correct state regardless of what 3D rendering rules may have changed.
    // GL_CULL_FACE must be disabled: the quad model's triangles are wound CCW in
    // model space (Y-up OBJ convention), and the Y-flipping ortho projection
    // inverts that to CW in window space — back-face culling would discard them.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Default quad model spans (-1,-1)..(1,1), so translate to the rect's
    // centre and scale by half-size so the geometry lands at _pos..(_pos+_size).
    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_pos.x + _size.x * 0.5f,
                                                _pos.y + _size.y * 0.5f,
                                                0.0f));
    l_model = glm::scale(l_model, glm::vec3(_size.x * 0.5f, _size.y * 0.5f, 1.0f));

    glUseProgram(m_shaderProgram);

    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _color.x, _color.y, _color.z, _color.w);
    glUniform1i(m_locUseTexture, _useTexture ? 1 : 0);

    if (_useTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _texId);
        glUniform1i(m_locTexture, 0);
    }

    if(auto l_quadModelPtr = m_quadModel.lock())
    {
        glBindVertexArray(l_quadModelPtr->GetVAO());
        glDrawArrays(GL_TRIANGLES, 0, l_quadModelPtr->GetVertexCount());
        glBindVertexArray(0);
    }

    if (_useTexture)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glUseProgram(0);
}

void UIRenderer::DrawRect(Vector2 _pos, Vector2 _size, Vector4 _colour)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Rect;
        cmd.pos = _pos; cmd.size = _size; cmd.colour = _colour;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    
    // _pos is NDC centre (-1..1, Y-up). Convert to pixel-space top-left for DrawQuad.
    float l_cx = (_pos.x + 1.0f) * 0.5f * m_screenW;
    float l_cy = (1.0f - _pos.y) * 0.5f * m_screenH;
    Vector2 l_topLeft(l_cx - _size.x * 0.5f, l_cy - _size.y * 0.5f);
    DrawQuad(l_topLeft, _size, _colour, 0, false);
}

void UIRenderer::DrawRectOutline(Vector2 _pos, Vector2 _size, Vector4 _colour, float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::RectOutline;
        cmd.pos = _pos; cmd.size = _size; cmd.colour = _colour;
        cmd.thickness = _thickness;
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
    l_model = glm::translate(l_model, glm::vec3(_pos.x, _pos.y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_size.x, _size.y, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _colour.x, _colour.y, _colour.z, _colour.w);
    glUniform1i(m_locUseTexture, 0);

    glBindVertexArray(m_rectLineVAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);

    glLineWidth(1.0f);
}

void UIRenderer::DrawText(Vector2 _pos, Vector4 _color,
                          const std::string& _text, float _fontSize,
                          bool _centered)
{
    // Log::Message("UIRenderer::DrawText: text='" + _text + "'"
    //              + " pos=(" + std::to_string(_pos.x) + "," + std::to_string(_pos.y) + ")"
    //              + " color=(" + std::to_string(_color.x) + "," + std::to_string(_color.y)
    //              + "," + std::to_string(_color.z) + "," + std::to_string(_color.w) + ")"
    //              + " fontSize=" + std::to_string(_fontSize)
    //              + " centered=" + (_centered ? "true" : "false")
    //              + " buffering=" + (m_buffering ? "true" : "false")
    //              + " screenH=" + std::to_string(m_screenH)
    //              + " glyphs=" + std::to_string(m_characters.size()));

    if (_text.empty())
    {
        Log::Message("UIRenderer::DrawText: ABORT - empty text");
        return;
    }
    if (m_characters.empty())
    {
        Log::Message("UIRenderer::DrawText: ABORT - no glyphs loaded (FreeType init failed?)");
        return;
    }

    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Text;
        cmd.pos = _pos;
        cmd.colour = _color;
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

    // Measure width and ascender/descender extents of the actual glyphs in the
    // string so vertical centring lines up with the visible ink, not just the
    // nominal font size (which over-allocates space above the cap-height).
    float l_totalWidth  = 0.0f;
    float l_maxBearingY = 0.0f;  // tallest ascender extent above baseline
    float l_maxDescent  = 0.0f;  // deepest descender extent below baseline
    if (_centered)
    {
        for (char c : _text)
        {
            auto l_it = m_characters.find(c);
            if (l_it == m_characters.end()) l_it = m_characters.find(' ');
            if (l_it != m_characters.end())
            {
                const Character& l_ch = l_it->second;
                l_totalWidth += (l_ch.advance >> 6) * l_scale;
                float l_bY = l_ch.bearingY * l_scale;
                float l_dY = (l_ch.sizeY - l_ch.bearingY) * l_scale;
                if (l_bY > l_maxBearingY) l_maxBearingY = l_bY;
                if (l_dY > l_maxDescent)  l_maxDescent  = l_dY;
            }
        }
    }

    // _pos is NDC: (-1,-1) bottom-left, (1,1) top-right. Convert to pixel-space
    float l_xPix = (_pos.x + 1.0f) * 0.5f * m_screenW;
    float l_yPix = (1.0f - _pos.y) * 0.5f * m_screenH;

    float l_cursorX = _centered ? l_xPix - l_totalWidth * 0.5f : l_xPix;
    // In Y-down pixel space, place the baseline below the requested centre by
    // half the asymmetry between ascender and descender extents. That makes
    // the ink span (visual top → visual bottom) straddle l_yPix evenly.
    float l_startY  = _centered
                      ? l_yPix + (l_maxBearingY - l_maxDescent) * 0.5f
                      : l_yPix - (_fontSize * 0.5f);

    // Activate text shader (uses pixel-space projection).
    glUseProgram(m_textShaderProgram);
    glUniformMatrix4fv(m_locTextProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniform4f(m_locTextColor, _color.x, _color.y, _color.z, _color.w);
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

        float l_xpos = l_cursorX + ch.bearingX * l_scale;
        float l_ypos = l_startY + (ch.sizeY - ch.bearingY) * l_scale;
        float l_w    = ch.sizeX * l_scale;
        float l_h    = ch.sizeY * l_scale;

        // 6 vertices, 4 floats each (x, y, u, v) — two triangles.
        // Y-down projection: l_ypos is visual bottom (descender baseline + h),
        // l_ypos - l_h is visual top. v=0 = glyph top (FT row 0), v=1 = glyph bottom.
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

        l_cursorX += (ch.advance >> 6) * l_scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void UIRenderer::DrawImage(unsigned int _texId,
                           Vector2 _pos, Vector2 _size,
                           Vector4 _color)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Image;
        cmd.texId = _texId;
        cmd.pos = _pos; cmd.size = _size; cmd.colour = _color;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    DrawQuad(_pos, _size, _color, _texId, true);
}

void UIRenderer::DrawCircle(Vector2 _pos, float _radius, Vector4 _color)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Circle;
        cmd.pos = _pos;
        cmd.size = {_radius, _radius};  // size.x = size.y = radius for circles
        cmd.colour = _color;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_pos.x, _pos.y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_radius, _radius, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _color.x, _color.y, _color.z, _color.w);
    glUniform1i(m_locUseTexture, 0);

    glBindVertexArray(m_circleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_circleVertCount);
    glBindVertexArray(0);
    glUseProgram(0);
}

void UIRenderer::DrawCircleOutline(Vector2 _pos, float _radius, Vector4 _color, float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::CircleOutline;
        cmd.pos = _pos;
        cmd.size = {_radius, _radius};
        cmd.colour = _color;
        cmd.thickness = _thickness;
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
    l_model = glm::translate(l_model, glm::vec3(_pos.x, _pos.y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_radius, _radius, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _color.x, _color.y, _color.z, _color.w);
    glUniform1i(m_locUseTexture, 0);

    glBindVertexArray(m_circleLineVAO);
    glDrawArrays(GL_LINE_LOOP, 0, m_circleLineVertCount);
    glBindVertexArray(0);
    glUseProgram(0);

    glLineWidth(1.0f);
}

void UIRenderer::DrawCapsule(Vector2 _pos, Vector2 _size, Vector4 _color)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Capsule;
        cmd.pos = _pos; cmd.size = _size; cmd.colour = _color;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    // Capsule = centre rectangle + two half-circle endcaps
    float l_r = _size.x * 0.5f;  // radius = half-width
    float l_bodyH = _size.y - _size.x; // height minus endcaps
    if (l_bodyH < 0.0f) l_bodyH = 0.0f;

    DrawRect(_pos, {_size.x, l_bodyH}, _color);
    DrawCircle(_pos + Vector2(l_r, l_r), l_r, _color);
    DrawCircle(_pos + Vector2(l_r, l_r + l_bodyH), l_r, _color);
}

void UIRenderer::DrawCapsuleOutline(Vector2 _pos, Vector2 _size, Vector4 _color, float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::CapsuleOutline;
        cmd.pos = _pos; cmd.size = _size; cmd.colour = _color;
        cmd.thickness = _thickness;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    float l_r = _size.x * 0.5f;
    float l_bodyH = _size.y - _size.x;
    if (l_bodyH < 0.0f) l_bodyH = 0.0f;

    // Rectangular body outline — spans vertically between the two endcap centres
    // so its top/bottom edges line up with the endcap diameter chords.
    DrawRectOutline({_pos.x, _pos.y + l_r}, {_size.x, l_bodyH}, _color, _thickness);
    // Half-circle endcaps. The Y-down orthographic projection flips the
    // circle geometry: the "top" half-ring (angles 0→π, Y-up) renders as
    // the bottom half on screen, and vice versa. So we swap them here so
    // the visible arcs sit on the correct ends of the capsule.
    DrawSemiCircleBotOutline(_pos + Vector2(l_r, l_r), l_r, _color, _thickness);
    DrawSemiCircleTopOutline(_pos + Vector2(l_r, l_r + l_bodyH), l_r, _color, _thickness);
}

void UIRenderer::DrawSemiCircleTop(Vector2 _pos, float _radius, Vector4 _color)
{
    // Filled semi-circle uses same triangle fan but only top half.
    // For simplicity, draw a full filled circle — the capsule's centre rect
    // will cover the bottom half.
    DrawCircle(_pos, _radius, _color);
}

void UIRenderer::DrawSemiCircleTopOutline(Vector2 _pos, float _radius, Vector4 _color, float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::SemiCircleTopOutline;
        cmd.pos = _pos; cmd.size = {_radius, _radius}; cmd.colour = _color;
        cmd.thickness = _thickness;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(_thickness < 0.5f ? 0.5f : _thickness);

    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_pos.x, _pos.y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_radius, _radius, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _color.x, _color.y, _color.z, _color.w);
    glUniform1i(m_locUseTexture, 0);

    glBindVertexArray(m_semiCircleTopLineVAO);
    glDrawArrays(GL_LINE_LOOP, 0, m_semiCircleTopLineVertCount);
    glBindVertexArray(0);
    glUseProgram(0);

    glLineWidth(1.0f);
}

void UIRenderer::DrawSemiCircleBot(Vector2 _pos, float _radius, Vector4 _color)
{
    DrawCircle(_pos, _radius, _color);
}

void UIRenderer::DrawSemiCircleBotOutline(Vector2 _pos, float _radius, Vector4 _color, float _thickness)
{
    if (m_buffering)
    {
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::SemiCircleBotOutline;
        cmd.pos = _pos; cmd.size = {_radius, _radius}; cmd.colour = _color;
        cmd.thickness = _thickness;
        m_commandBuffer.push_back(std::move(cmd));
        return;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(_thickness < 0.5f ? 0.5f : _thickness);

    glm::mat4 l_model(1.0f);
    l_model = glm::translate(l_model, glm::vec3(_pos.x, _pos.y, 0.0f));
    l_model = glm::scale(l_model, glm::vec3(_radius, _radius, 1.0f));

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projMatrix));
    glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_model));
    glUniform4f(m_locColor, _color.x, _color.y, _color.z, _color.w);
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
                DrawRect(l_cmd.pos, l_cmd.size, l_cmd.colour);
                break;
            case DrawCommand::Type::RectOutline:
                DrawRectOutline(l_cmd.pos, l_cmd.size, l_cmd.colour, l_cmd.thickness);
                break;
            case DrawCommand::Type::Text:
                DrawText(l_cmd.pos, l_cmd.colour,
                         l_cmd.text, l_cmd.fontSize, l_cmd.centered);
                break;
            case DrawCommand::Type::Image:
                DrawImage(l_cmd.texId, l_cmd.pos, l_cmd.size, l_cmd.colour);
                break;
            case DrawCommand::Type::Circle:
                DrawCircle(l_cmd.pos, l_cmd.size.x, l_cmd.colour);
                break;
            case DrawCommand::Type::CircleOutline:
                DrawCircleOutline(l_cmd.pos, l_cmd.size.x, l_cmd.colour, l_cmd.thickness);
                break;
            case DrawCommand::Type::Capsule:
                DrawCapsule(l_cmd.pos, l_cmd.size, l_cmd.colour);
                break;
            case DrawCommand::Type::CapsuleOutline:
                DrawCapsuleOutline(l_cmd.pos, l_cmd.size, l_cmd.colour, l_cmd.thickness);
                break;
            case DrawCommand::Type::SemiCircleTopOutline:
                DrawSemiCircleTopOutline(l_cmd.pos, l_cmd.size.x, l_cmd.colour, l_cmd.thickness);
                break;
            case DrawCommand::Type::SemiCircleBotOutline:
                DrawSemiCircleBotOutline(l_cmd.pos, l_cmd.size.x, l_cmd.colour, l_cmd.thickness);
                break;
            case DrawCommand::Type::SemiCircleTop:
                DrawSemiCircleTop(l_cmd.pos, l_cmd.size.x, l_cmd.colour);
                break;
            case DrawCommand::Type::SemiCircleBot:
                DrawSemiCircleBot(l_cmd.pos, l_cmd.size.x, l_cmd.colour);
                break;
        }
    }
    m_commandBuffer.clear();
    m_buffering = true;

    glEnable(GL_DEPTH_TEST);
}

bool UIRenderer::IsHovered(Vector2 _pos, Vector2 _size) const
{
    // _pos is NDC centre (-1..1, Y-up), _size is pixel size — same convention
    // as DrawRect. Build the rect in pixel space and compare against the mouse
    // in pixel space (m_mouseX/Y are NDC Y-up, set in BeginFrame).
    float l_cx     = (_pos.x + 1.0f) * 0.5f * m_screenW;
    float l_cy     = (1.0f - _pos.y) * 0.5f * m_screenH;
    float l_left   = l_cx - _size.x * 0.5f;
    float l_right  = l_cx + _size.x * 0.5f;
    float l_top    = l_cy - _size.y * 0.5f;
    float l_bottom = l_cy + _size.y * 0.5f;

    float l_mx = (m_mouseX + 1.0f) * 0.5f * m_screenW;
    float l_my = (1.0f - m_mouseY) * 0.5f * m_screenH;

    return l_mx >= l_left && l_mx <= l_right &&
           l_my >= l_top  && l_my <= l_bottom;
}

bool UIRenderer::IsClicked(Vector2 _pos, Vector2 _size) const
{
    return IsHovered(_pos, _size) && m_mouseClicked;
}

}