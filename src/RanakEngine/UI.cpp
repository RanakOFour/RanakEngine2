#include "RanakEngine/UI.h"
#include "RanakEngine/UI/DefaultFont.h"

#include "RanakEngine/Core/LuaContext.h"

#include <memory>
#include <sol/sol.hpp>

namespace RanakEngine::UI
{
    namespace
    {
        static sol::table UITable;
        static std::shared_ptr<UIRenderer> s_renderer = nullptr;
    };

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
        s_renderer = std::make_shared<UIRenderer>();
        if (auto io = _io.lock())
        {
            s_renderer->Init(io, _fontData, _fontDataSize, _fontSize);
        }
    }

    void DefineLuaLib()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        UITable = l_context->CreateTable();

        UITable.set_function("DrawRect",
            [](Vector2 _pos, Vector2 _size, Vector4 _color)
            { if (s_renderer) s_renderer->DrawRect(_pos, _size, _color); });

        UITable.set_function("DrawRectOutline",
            [](Vector2 _pos, Vector2 _size, Vector4 _color, float _thickness)
            { if (s_renderer) s_renderer->DrawRectOutline(_pos, _size, _color, _thickness); });

        UITable.set_function("DrawText",
            [](Vector2 _pos, Vector4 _color,
               const std::string& text, float fontSize, bool centered)
            { if (s_renderer) s_renderer->DrawText(_pos, _color, text, fontSize, centered); });

        UITable.set_function("DrawImage",
            [](unsigned int texId, Vector2 _pos, Vector2 _size, Vector4 _color)
            { if (s_renderer) s_renderer->DrawImage(texId, _pos, _size, _color); });

        UITable.set_function("DrawCircle",
            [](Vector2 _pos, float radius, Vector4 _color)
            { if (s_renderer) s_renderer->DrawCircle(_pos, radius, _color); });

        UITable.set_function("DrawCircleOutline",
            [](Vector2 _pos, float radius, Vector4 _color, float _thickness)
            { if (s_renderer) s_renderer->DrawCircleOutline(_pos, radius, _color, _thickness); });

        UITable.set_function("DrawCapsule",
            [](Vector2 _pos, Vector2 _size, Vector4 _color)
            { if (s_renderer) s_renderer->DrawCapsule(_pos, _size, _color); });

        UITable.set_function("DrawCapsuleOutline",
            [](Vector2 _pos, Vector2 _size, Vector4 _color, float _thickness)
            { if (s_renderer) s_renderer->DrawCapsuleOutline(_pos, _size, _color, _thickness); });

        UITable.set_function("IsHovered",
            [](Vector2 _pos, Vector2 _size) -> bool
            { return s_renderer ? s_renderer->IsHovered(_pos, _size) : false; });

        UITable.set_function("IsClicked",
            [](Vector2 _pos, Vector2 _size) -> bool
            { return s_renderer ? s_renderer->IsClicked(_pos, _size) : false; });

        UITable.set_function("GetScreenWidth",
            []() -> float { return s_renderer ? s_renderer->GetScreenWidth() : 0.0f; });

        UITable.set_function("GetScreenHeight",
            []() -> float { return s_renderer ? s_renderer->GetScreenHeight() : 0.0f; });

        l_context->SetGlobal("UI", UITable);
    }

    void Stop()
    {
        UITable.abandon();
        s_renderer.reset();
    }

    std::weak_ptr<UIRenderer> GetRenderer()
    {
        return s_renderer;
    }
}
