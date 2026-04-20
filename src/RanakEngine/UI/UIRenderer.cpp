#include "RanakEngine/UI/UIRenderer.h"
#include "RanakEngine/UI/DefaultFont.h"

#include "RanakEngine/Log.h"

#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <vector>
#include <cstdio>
#include <cstring>

#define STB_TRUETYPE_IMPLEMENTATION
#include "box2d/samples/stb_truetype.h"

namespace RanakEngine::UI
{

// ── Embedded shader sources ──────────────────────────────────────────────────

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
    if (m_quadVAO)        glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO)        glDeleteBuffers(1, &m_quadVBO);
    if (m_shaderProgram)  glDeleteProgram(m_shaderProgram);
    if (m_fontAtlasTexId) glDeleteTextures(1, &m_fontAtlasTexId);
}

void UIRenderer::Init(IO::Manager& _io,
                      const unsigned char* _fontData,
                      unsigned int _fontDataSize,
                      float _fontSize)
{
    m_io = &_io;
    m_bakedFontSize = _fontSize;

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
        Log::Message(std::string("UIRenderer: shader link error:\n") + l_log);
    }

    glDeleteShader(l_vert);
    glDeleteShader(l_frag);

    m_locProjection = glGetUniformLocation(m_shaderProgram, "u_Projection");
    m_locModel      = glGetUniformLocation(m_shaderProgram, "u_Model");
    m_locColor      = glGetUniformLocation(m_shaderProgram, "u_Color");
    m_locUseTexture = glGetUniformLocation(m_shaderProgram, "u_UseTexture");
    m_locTexture    = glGetUniformLocation(m_shaderProgram, "u_Texture");

    // Use provided font data, falling back to the embedded default.
    const unsigned char* l_fontDataPtr  = _fontData  ? _fontData  : k_defaultFontData;
    unsigned int         l_fontDataSize = _fontData   ? _fontDataSize : k_defaultFontDataSize;

    if (!l_fontDataPtr || l_fontDataSize == 0)
    {
        Log::Message("UIRenderer: no font data available.");
        return;
    }

    std::vector<unsigned char> l_bitmap(k_atlasW * k_atlasH, 0);
    stbtt_bakedchar l_bakedChars[k_charCount];

    int l_result = stbtt_BakeFontBitmap(l_fontDataPtr, 0, _fontSize,
                                        l_bitmap.data(), k_atlasW, k_atlasH,
                                        k_firstChar, k_charCount, l_bakedChars);
    if (l_result <= 0)
    {
        Log::Message("UIRenderer: stbtt_BakeFontBitmap failed (result=" +
                     std::to_string(l_result) + "). Atlas may be too small.");
    }

    for (int i = 0; i < k_charCount; ++i)
    {
        const stbtt_bakedchar& bc = l_bakedChars[i];
        m_glyphs[i].x0 = bc.xoff;
        m_glyphs[i].y0 = bc.yoff;
        m_glyphs[i].x1 = bc.xoff + (bc.x1 - bc.x0);
        m_glyphs[i].y1 = bc.yoff + (bc.y1 - bc.y0);
        m_glyphs[i].s0 = bc.x0 / static_cast<float>(k_atlasW);
        m_glyphs[i].t0 = bc.y0 / static_cast<float>(k_atlasH);
        m_glyphs[i].s1 = bc.x1 / static_cast<float>(k_atlasW);
        m_glyphs[i].t1 = bc.y1 / static_cast<float>(k_atlasH);
        m_glyphs[i].xAdvance = bc.xadvance;
    }

    glGenTextures(1, &m_fontAtlasTexId);
    glBindTexture(GL_TEXTURE_2D, m_fontAtlasTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, k_atlasW, k_atlasH, 0,
                 GL_RED, GL_UNSIGNED_BYTE, l_bitmap.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLint l_swizzle[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, l_swizzle);
    glBindTexture(GL_TEXTURE_2D, 0);

    printf("UIRenderer: initialised (fontSize: %.0f)\n", _fontSize);
}

void UIRenderer::BeginFrame(float _screenW, float _screenH)
{
    if (_screenW <= 0.0f || _screenH <= 0.0f)
        return;

    m_screenW = _screenW;
    m_screenH = _screenH;

    glm::mat4 l_proj = glm::ortho(0.0f, _screenW, _screenH, 0.0f, -1.0f, 1.0f);
    std::memcpy(m_projMatrix, glm::value_ptr(l_proj), sizeof(m_projMatrix));

    if (m_io)
    {
        const auto& l_mouse = m_io->GetMouseInfo();
        m_mouseX        = l_mouse.position.x;
        m_mouseY        = l_mouse.position.y;
        m_mouseClicked  = l_mouse.LMBDown && !m_mousePrevDown;
        m_mousePrevDown = l_mouse.LMBDown;
        m_mouseDown     = l_mouse.LMBDown;
    }

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
    DrawQuad(_x, _y, _w, _h, _r, _g, _b, _a, 0, false);
}

void UIRenderer::DrawRectOutline(float _x, float _y, float _w, float _h,
                                 float _r, float _g, float _b, float _a,
                                 float _thickness)
{
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
    if (m_fontAtlasTexId == 0) return;

    float l_scale = _fontSize / m_bakedFontSize;

    float l_totalWidth = 0.0f;
    float l_maxHeight  = _fontSize;
    if (_centered)
    {
        for (char c : _text)
        {
            int l_idx = static_cast<int>(c) - k_firstChar;
            if (l_idx < 0 || l_idx >= k_charCount) l_idx = 0;
            l_totalWidth += m_glyphs[l_idx].xAdvance * l_scale;
        }
    }

    float l_cursorX  = _centered ? _x - l_totalWidth * 0.5f : _x;
    float l_cursorY  = _centered ? _y - l_maxHeight  * 0.5f : _y;
    float l_baseline = _fontSize;

    for (char c : _text)
    {
        int l_idx = static_cast<int>(c) - k_firstChar;
        if (l_idx < 0 || l_idx >= k_charCount)
        {
            l_cursorX += m_glyphs[0].xAdvance * l_scale;
            continue;
        }

        const GlyphInfo& g = m_glyphs[l_idx];

        float l_glyphW = (g.x1 - g.x0) * l_scale;
        float l_glyphH = (g.y1 - g.y0) * l_scale;
        float l_glyphX = l_cursorX + g.x0 * l_scale;
        float l_glyphY = l_cursorY + l_baseline + g.y0 * l_scale;

        float l_verts[] = {
            l_glyphX,            l_glyphY,            0.0f, g.s0, g.t0,
            l_glyphX + l_glyphW, l_glyphY,            0.0f, g.s1, g.t0,
            l_glyphX + l_glyphW, l_glyphY + l_glyphH, 0.0f, g.s1, g.t1,
            l_glyphX,            l_glyphY,            0.0f, g.s0, g.t0,
            l_glyphX + l_glyphW, l_glyphY + l_glyphH, 0.0f, g.s1, g.t1,
            l_glyphX,            l_glyphY + l_glyphH, 0.0f, g.s0, g.t1,
        };

        glm::mat4 l_identity(1.0f);

        glUseProgram(m_shaderProgram);
        glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, m_projMatrix);
        glUniformMatrix4fv(m_locModel,      1, GL_FALSE, glm::value_ptr(l_identity));
        glUniform4f(m_locColor, _r, _g, _b, _a);
        glUniform1i(m_locUseTexture, 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fontAtlasTexId);
        glUniform1i(m_locTexture, 0);

        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(l_verts), l_verts);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        l_cursorX += g.xAdvance * l_scale;
    }

    // Restore the unit quad VBO.
    float l_unitQuad[] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(l_unitQuad), l_unitQuad);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UIRenderer::DrawImage(unsigned int _texId,
                           float _x, float _y, float _w, float _h,
                           float _tR, float _tG, float _tB, float _tA)
{
    DrawQuad(_x, _y, _w, _h, _tR, _tG, _tB, _tA, _texId, true);
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