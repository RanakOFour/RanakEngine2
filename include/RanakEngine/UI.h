#ifndef RANAKENGINE_UI_H
#define RANAKENGINE_UI_H

#include "RanakEngine/UI/UIRenderer.h"
#include <memory>

namespace RanakEngine::UI
{
    /**
     * @brief Creates and initialises the UI subsystem (renderer, shaders, font atlas).
     *
     * When _fontData is nullptr the built-in embedded font is used.
     *
     * @param _io           The IO manager (for mouse input).
     * @param _fontData     Raw TTF data, or nullptr for the embedded default.
     * @param _fontDataSize Byte count of _fontData (ignored when nullptr).
     * @param _fontSize     Pixel height used to bake the font atlas.
     */
    void Init(IO::Manager& _io,
              const unsigned char* _fontData = nullptr,
              unsigned int _fontDataSize = 0,
              float _fontSize = 32.0f);

    /** @brief Registers the "UI" Lua table with drawing/hit-testing functions. */
    void DefineLuaLib();

    /** @brief Releases the Lua table and destroys the UIRenderer. */
    void Stop();

    /** @brief Returns a pointer to the active UIRenderer, or nullptr if not initialised. */
    std::weak_ptr<UIRenderer> GetRenderer();
}

#endif
