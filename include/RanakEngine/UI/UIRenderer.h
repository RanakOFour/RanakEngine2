#ifndef RANAKENGINE_UI_UIRENDERER_H
#define RANAKENGINE_UI_UIRENDERER_H

/**
 * @file UIRenderer.h
 * @brief Pure-OpenGL screen-space UI renderer with text, rect, and image
 *        drawing — independent of ImGui.
 *
 * Owns a quad VAO/VBO, a UI shader, and per-glyph textures loaded via
 * FreeType (following the LearnOpenGL text-rendering approach).  Exposes a
 * Lua "UI" table so UI rules can draw UIText, UIButton, UIPanel, and
 * UIImage entities.
 *
 * Usage:
 *   UIRenderer renderer;
 *   renderer.Init(ioManager);              // after GL context ready
 *   renderer.RegisterLuaBindings(state);   // before rules are loaded
 *   // … each frame, between glClear and swap:
 *   renderer.BeginFrame(screenW, screenH);
 *   // UI.Draw<Element> calls will buffer commands
 *   renderer.Flush(); <- Draw everything in buffer at end of frame
 */

#include "RanakEngine/IO.h"
#include "RanakEngine/Math.h"

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace RanakEngine::UI
{

/**
 * @class UIRenderer
 * @brief Screen-space UI drawing with OpenGL — no ImGui dependency.
 */
class UIRenderer
{
    private:
    /** @brief Draws a quad positioned/scaled by the given parameters. */
    void DrawQuad(float _x, float _y, float _w, float _h,
                  float _r, float _g, float _b, float _a,
                  unsigned int _texId, bool _useTexture);

    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;
    GLuint m_circleVAO = 0;
    GLuint m_circleVBO = 0;
    GLuint m_circleVertCount = 0;
    GLuint m_circleOutlineVAO = 0;
    GLuint m_circleOutlineVBO = 0;
    GLuint m_circleOutlineVertCount = 0;
    GLuint m_semiCircleTopVAO = 0;
    GLuint m_semiCircleTopVBO = 0;
    GLuint m_semiCircleTopVertCount = 0;
    GLuint m_semiCircleBotVAO = 0;
    GLuint m_semiCircleBotVBO = 0;
    GLuint m_semiCircleBotVertCount = 0;
    GLuint m_shaderProgram = 0;

    // Text-specific VAO/VBO (4-component vertices: x,y,u,v)
    GLuint m_textVAO = 0;
    GLuint m_textVBO = 0;
    GLuint m_textShaderProgram = 0;

    int m_locProjection = -1;
    int m_locModel      = -1;
    int m_locColor      = -1;
    int m_locUseTexture = -1;
    int m_locTexture    = -1;

    // Text shader uniform locations
    int m_locTextProjection = -1;
    int m_locTextColor      = -1;
    int m_locTextSampler    = -1;

    struct Character
    {
        unsigned int textureID; ///< GL texture handle for this glyph
        int sizeX, sizeY;      ///< Size of glyph in pixels
        int bearingX, bearingY;   ///< Offset from baseline to left/top of glyph
        unsigned int advance;   ///< Horizontal advance (in 1/64 pixels)
    };
    std::map<char, Character> m_characters;
    float m_bakedFontSize = 32.0f;

    // Deferred command buffer
    // Draw calls issued by Lua rules are recorded here and executed in Flush()
    // which is called AFTER ImGui renders so game UI appears on top of editor.
    struct DrawCommand
    {
        enum class Type { 
                          Rect, RectOutline, 
                          Text, Image,
                          Circle, CircleOutline,
                          Capsule, CapsuleOutline,
                          SemiCircleTop, SemiCircleTopOutline,
                          SemiCircleBot, SemiCircleBotOutline
                        };

        Type         type;
        float        x = 0, y = 0, w = 0, h = 0;
        float        r = 1, g = 1, b = 1, a = 1;
        float        thickness = 1.0f;
        std::string  text;
        float        fontSize = 32.0f;
        bool         centered = false;
        unsigned int texId = 0;
    };
    std::vector<DrawCommand> m_commandBuffer;
    bool m_buffering = true; ///< When true, draw calls are buffered; set false during Flush.

    glm::mat4 m_projMatrix{};
    float m_screenW = 0.0f;
    float m_screenH = 0.0f;

    float m_mouseX        = 0.0f;
    float m_mouseY        = 0.0f;
    bool  m_mouseDown     = false;
    bool  m_mouseClicked  = false;
    bool  m_mousePrevDown = false;

    std::weak_ptr<IO::Manager> m_IOwptr;

    public:
    UIRenderer();
    ~UIRenderer();

    /**
     * @brief Initialises GL resources (quad mesh, UI shader, FreeType glyphs).
     *
     * Must be called after the OpenGL context is current.  When _fontData is
     * nullptr the built-in embedded font (MapleMono) is used.
     *
     * @param _io           The IO manager (used each frame to read mouse state).
     * @param _fontData     Raw TTF data, or nullptr for the embedded default.
     * @param _fontDataSize Byte count of _fontData (ignored when nullptr).
     * @param _fontSize     Pixel height used to rasterise glyphs.
     */
    void Init(std::weak_ptr<IO::Manager> _io,
              const unsigned char* _fontData = nullptr,
              unsigned int _fontDataSize = 0,
              float _fontSize = 32.0f);

    /** @brief Call at the start of each frame to set up screen-space projection. */
    void BeginFrame(float _screenW, float _screenH);

    /**
     * @brief Execute all buffered draw commands.
     *
     * Call this AFTER ImGui_ImplOpenGL3_RenderDrawData() so game UI renders
     * on top of editor panels. Clears the command buffer when done.
     */
    void Flush();

    void DrawRect(float _x, float _y, float _w, float _h,
                  float _r, float _g, float _b, float _a);

    void DrawRectOutline(float _x, float _y, float _w, float _h,
                         float _r, float _g, float _b, float _a,
                         float _thickness = 1.0f);

    void DrawText(float _x, float _y,
                  float _r, float _g, float _b, float _a,
                  const std::string& _text, float _fontSize,
                  bool _centered);

    void DrawImage(unsigned int _texId, float _x, float _y, float _w, float _h,
                   float _tR, float _tG, float _tB, float _tA);

    void DrawCircle(float _x, float _y, float _radius,
                    float _r, float _g, float _b, float _a);

    void DrawCircleOutline(float _x, float _y, float _radius,
                           float _r, float _g, float _b, float _a,
                           float _thickness = 1.0f);

    void DrawCapsule(float _x, float _y, float _w, float _h,
                     float _r, float _g, float _b, float _a);

    void DrawCapsuleOutline(float _x, float _y, float _w, float _h,
                            float _r, float _g, float _b, float _a,
                            float _thickness = 1.0f);

    void DrawSemiCircleTop(float _x, float _y, float _radius,
                           float _r, float _g, float _b, float _a);

    void DrawSemiCircleTopOutline(float _x, float _y, float _radius,
                                  float _r, float _g, float _b, float _a,
                                  float _thickness = 1.0f);

    void DrawSemiCircleBot(float _x, float _y, float _radius,
                           float _r, float _g, float _b, float _a);

    void DrawSemiCircleBotOutline(float _x, float _y, float _radius,
                                  float _r, float _g, float _b, float _a,
                                  float _thickness = 1.0f);

    bool IsHovered(float _x, float _y, float _w, float _h) const;
    bool IsClicked(float _x, float _y, float _w, float _h) const;

    float GetScreenWidth()  const { return m_screenW; }
    float GetScreenHeight() const { return m_screenH; }
};

} // namespace RanakEngine::UI

#endif
