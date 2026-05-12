#ifndef RANAKENGINE_UI_UIRENDERER_H
#define RANAKENGINE_UI_UIRENDERER_H

/**
 * @file UIRenderer.h
 * @brief Pure-OpenGL UI renderer with text, rect, circle, and image
 *        drawing — independent of ImGui.
 *
 * Owns a quad VAO/VBO, circle VAOs, UI shaders, and per-glyph textures
 * loaded via FreeType.  Exposes a Lua "UI" table so UI rules can draw
 * UIText, UIButton, UIPanel, and UIImage entities.
 *
 * **Coordinate systems:**
 * - Shape functions (DrawRect, DrawCircle, DrawCapsule, etc.) use
 *   **normalised device coordinates** (NDC): X right, Y up, origin at
 *   screen centre, (-1,-1) bottom-left, (1,1) top-right.
 * - DrawText uses **pixel coordinates** (Y-up, origin at bottom-left)
 *   with its own pixel-space orthographic projection.
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

#include "RanakEngine/Asset/Model.h"
#include "RanakEngine/IO.h"
#include "RanakEngine/Math.h"
#include "RanakEngine/Math/Vector2.h"

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
    void DrawQuad(Vector2 _pos, Vector2 _size,
                  Vector4 _color,
                  unsigned int _texId, bool _useTexture);

    // TODO: Make a wrapper for these things, maybe utilising Model or Texture?
    // Either way, this is incredibly ugly
    std::weak_ptr<Asset::Model> m_quadModel;
    GLuint m_circleVAO = 0;
    GLuint m_circleVBO = 0;
    GLuint m_circleVertCount = 0;
    GLuint m_circleLineVAO = 0;
    GLuint m_circleLineVBO = 0;
    GLuint m_circleLineVertCount = 0;
    GLuint m_rectLineVAO = 0;
    GLuint m_rectLineVBO = 0;
    GLuint m_semiCircleTopLineVAO = 0;
    GLuint m_semiCircleTopLineVBO = 0;
    GLuint m_semiCircleTopLineVertCount = 0;
    GLuint m_semiCircleBotLineVAO = 0;
    GLuint m_semiCircleBotLineVBO = 0;
    GLuint m_semiCircleBotLineVertCount = 0;
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
        Vector2      pos = {0, 0};
        Vector2      size = {0, 0};
        Vector4      color = {1, 1, 1, 1};
        float        thickness = 1.0f;
        std::string  text;
        float        fontSize = 32.0f;
        bool         centered = false;
        unsigned int texId = 0;
    };
    std::vector<DrawCommand> m_commandBuffer;
    bool m_buffering = true; ///< When true, draw calls are buffered; set false during Flush.

    glm::mat4 m_projMatrix{};     ///< Identity — shapes use NDC.
    glm::mat4 m_textProjMatrix{}; ///< Pixel-space ortho for text rendering.
    float m_screenW = 0.0f;
    float m_screenH = 0.0f;

    float m_mouseX        = 0.0f; ///< Mouse position in NDC.
    float m_mouseY        = 0.0f; ///< Mouse position in NDC.
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
    void BeginFrame(Vector2 _screenSize);

    /**
     * @brief Execute all buffered draw commands.
     *
     * Call this AFTER ImGui_ImplOpenGL3_RenderDrawData() so game UI renders
     * on top of editor panels. Clears the command buffer when done.
     */
    void Flush();

    void DrawText(Vector2 _pos, Vector4 _color,
                  const std::string& _text, float _fontSize,
                  bool _centered);

    void DrawImage(unsigned int _texId, Vector2 _pos, Vector2 _size,
                   Vector4 _color);

    void DrawRect(Vector2 _pos, Vector2 _size, Vector4 _color);
    void DrawRectOutline(Vector2 _pos, Vector2 _size, Vector4 _color, float _thickness = 1.0f);

    void DrawCircle(Vector2 _pos, float _radius, Vector4 _color);
    void DrawCircleOutline(Vector2 _pos, float _radius, Vector4 _color, float _thickness = 1.0f);

    void DrawCapsule(Vector2 _pos, Vector2 _size, Vector4 _color);
    void DrawCapsuleOutline(Vector2 _pos, Vector2 _size, Vector4 _color, float _thickness = 1.0f);

    void DrawSemiCircleTop(Vector2 _pos, float _radius, Vector4 _color);
    void DrawSemiCircleTopOutline(Vector2 _pos, float _radius, Vector4 _color, float _thickness = 1.0f);

    void DrawSemiCircleBot(Vector2 _pos, float _radius, Vector4 _color);
    void DrawSemiCircleBotOutline(Vector2 _pos, float _radius, Vector4 _color, float _thickness = 1.0f);

    bool IsHovered(Vector2 _pos, Vector2 _size) const;
    bool IsClicked(Vector2 _pos, Vector2 _size) const;

    float GetScreenWidth()  const { return m_screenW; }
    float GetScreenHeight() const { return m_screenH; }
};

} // namespace RanakEngine::UI

#endif
