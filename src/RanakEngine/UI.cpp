#include "RanakEngine/UI.h"

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

    void Init(IO::Manager& _io, const unsigned char* _fontData,
              unsigned int _fontDataSize, float _fontSize)
    {
        s_renderer = std::make_shared<UIRenderer>();
        s_renderer->Init(_io, _fontData, _fontDataSize, _fontSize);
    }

    void DefineLuaLib()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        UITable = l_context->CreateTable();

        UITable.set_function("DrawRect",
            [](float x, float y, float w, float h,
               float r, float g, float b, float a)
            { if (s_renderer) s_renderer->DrawRect(x, y, w, h, r, g, b, a); });

        UITable.set_function("DrawRectOutline",
            [](float x, float y, float w, float h,
               float r, float g, float b, float a, float t)
            { if (s_renderer) s_renderer->DrawRectOutline(x, y, w, h, r, g, b, a, t); });

        UITable.set_function("DrawText",
            [](float x, float y,
               float r, float g, float b, float a,
               const std::string& text, float fontSize, bool centered)
            { if (s_renderer) s_renderer->DrawText(x, y, r, g, b, a, text, fontSize, centered); });

        UITable.set_function("DrawImage",
            [](unsigned int texId, float x, float y, float w, float h,
               float tR, float tG, float tB, float tA)
            { if (s_renderer) s_renderer->DrawImage(texId, x, y, w, h, tR, tG, tB, tA); });

        UITable.set_function("IsHovered",
            [](float x, float y, float w, float h) -> bool
            { return s_renderer ? s_renderer->IsHovered(x, y, w, h) : false; });

        UITable.set_function("IsClicked",
            [](float x, float y, float w, float h) -> bool
            { return s_renderer ? s_renderer->IsClicked(x, y, w, h) : false; });

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
