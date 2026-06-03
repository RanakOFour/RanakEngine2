#include "RanakEngine/UI.h"
#include "RanakEngine/UI/DefaultFont.h"

#include "RanakEngine/LuaEngine.h"

#include <memory>
#include <sol/sol.hpp>

namespace RanakEngine::UI
{
    // Module-private state (internal linkage): reachable only from this file.
    namespace
    {
        sol::table g_UITable;
        std::shared_ptr<UIRenderer> g_Renderer = nullptr;
    }

    const unsigned char* DefaultFontData()
    {
        return k_defaultFontData;
    }

    unsigned int DefaultFontDataSize()
    {
        return k_defaultFontDataSize;
    }

    void Init(std::weak_ptr<IO::Manager> _io, const unsigned char* _fontData,
              unsigned int _fontDataSize, float _fontSize)
    {
        g_Renderer = std::make_shared<UIRenderer>();
        if (auto io = _io.lock())
        {
            g_Renderer->Init(io, _fontData, _fontDataSize, _fontSize);
        }
    }

    void DefineLuaLib(LuaEngine& _engine)
    {
        g_UITable = _engine.AddTable();

        g_UITable.set_function("DrawRect",
            [](Vector2 _pos, Vector2 _size, Vector4 _color)
            { if (g_Renderer) g_Renderer->DrawRect(_pos, _size, _color); });

        g_UITable.set_function("DrawRectOutline",
            [](Vector2 _pos, Vector2 _size, Vector4 _color, float _thickness)
            { if (g_Renderer) g_Renderer->DrawRectOutline(_pos, _size, _color, _thickness); });

        g_UITable.set_function("DrawText",
            [](Vector2 _pos, Vector4 _color,
               const std::string& text, float fontSize, bool centered)
            { if (g_Renderer) g_Renderer->DrawText(_pos, _color, text, fontSize, centered); });

        g_UITable.set_function("DrawImage",
            [](unsigned int texId, Vector2 _pos, Vector2 _size, Vector4 _color)
            { if (g_Renderer) g_Renderer->DrawImage(texId, _pos, _size, _color); });

        g_UITable.set_function("DrawCircle",
            [](Vector2 _pos, float radius, Vector4 _color)
            { if (g_Renderer) g_Renderer->DrawCircle(_pos, radius, _color); });

        g_UITable.set_function("DrawCircleOutline",
            [](Vector2 _pos, float radius, Vector4 _color, float _thickness)
            { if (g_Renderer) g_Renderer->DrawCircleOutline(_pos, radius, _color, _thickness); });

        g_UITable.set_function("DrawCapsule",
            [](Vector2 _pos, Vector2 _size, Vector4 _color)
            { if (g_Renderer) g_Renderer->DrawCapsule(_pos, _size, _color); });

        g_UITable.set_function("DrawCapsuleOutline",
            [](Vector2 _pos, Vector2 _size, Vector4 _color, float _thickness)
            { if (g_Renderer) g_Renderer->DrawCapsuleOutline(_pos, _size, _color, _thickness); });

        g_UITable.set_function("IsHovered",
            [](Vector2 _pos, Vector2 _size) -> bool
            { return g_Renderer ? g_Renderer->IsHovered(_pos, _size) : false; });

        g_UITable.set_function("IsClicked",
            [](Vector2 _pos, Vector2 _size) -> bool
            { return g_Renderer ? g_Renderer->IsClicked(_pos, _size) : false; });

        g_UITable.set_function("GetScreenWidth",
            []() -> float { return g_Renderer ? g_Renderer->GetScreenWidth() : 0.0f; });

        g_UITable.set_function("GetScreenHeight",
            []() -> float { return g_Renderer ? g_Renderer->GetScreenHeight() : 0.0f; });

        _engine.SetGlobal<sol::table>("UI", g_UITable);
    }

    void Stop()
    {
        g_UITable.abandon();
        g_Renderer.reset();
    }

    std::weak_ptr<UIRenderer> GetRenderer()
    {
        return g_Renderer;
    }
}
