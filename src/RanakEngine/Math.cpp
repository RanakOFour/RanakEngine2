#include "RanakEngine/Math.h"

#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Math
{
    // Unnamed namespace for 'internal' Lua bindings
    namespace
    {
        static sol::table MathTable;
    };

    void DefineLuaLib()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        MathTable = l_context->CreateTable();

#pragma region MATHSETUP

        // Add usertypes for Vector2, Vector3, and Vector4
        l_context->AddUserType<Vector2>("Vector2",
                                        sol::call_constructor,
                                        sol::factories([](float _x, float _y) { return Vector2(_x, _y); },
                                                       [](float _value) { return Vector2(_value); },
                                                       []() { return Vector2(); }),
                                        "x", &Vector2::x,
                                        "y", &Vector2::y,
                                        "Length", &Vector2::Length,
                                        "Normalise", &Vector2::Normalise,
                                        "Normalised", &Vector2::Normalised,
                                        // Americanised spellings just because
                                        "Normalize", &Vector2::Normalise,
                                        "Normalized", &Vector2::Normalised,
                                        "ToString", &Vector2::ToString);

        l_context->AddUserType<Vector3>("Vector3",
                                        sol::call_constructor,
                                        sol::factories([](float _x, float _y, float _z) { return Vector3(_x, _y, _z); },
                                                       [](float _value) { return Vector3(_value); },
                                                       []() { return Vector3(); }),
                                        "x", &Vector3::x,
                                        "y", &Vector3::y,
                                        "z", &Vector3::z,
                                        "Length", &Vector3::Length,
                                        "Normalise", &Vector3::Normalise,
                                        "Normalised", &Vector3::Normalised,
                                        // Americanised spellings just because
                                        "Normalize", &Vector3::Normalise,
                                        "Normalized", &Vector3::Normalised,
                                        "ToString", &Vector3::ToString);

        l_context->AddUserType<Vector4>("Vector4",
                                        sol::call_constructor,
                                        sol::factories([](float _x, float _y, float _z, float _w) { return Vector4(_x, _y, _z, _w); },
                                                       [](float _value) { return Vector4(_value); },
                                                       []() { return Vector4(); }),
                                        "x", &Vector4::x,
                                        "y", &Vector4::y,
                                        "z", &Vector4::z,
                                        "w", &Vector4::w,
                                        "Length", &Vector4::Length,
                                        "Normalise", &Vector4::Normalise,
                                        "Normalised", &Vector4::Normalised,
                                        // Americanised spellings just because
                                        "Normalize", &Vector4::Normalise,
                                        "Normalized", &Vector4::Normalised,
                                        "ToString", &Vector4::ToString);

        // Functions for vector operations
        MathTable.set_function("DotProduct", [](Vector2 &_a, Vector2 &_b)
                               { return _a.x * _b.x + _a.y * _b.y; });
        MathTable.set_function("DotProduct", [](Vector3 &_a, Vector3 &_b)
                               { return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z; });
        MathTable.set_function("DotProduct", [](Vector4 &_a, Vector4 &_b)
                               { return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z + _a.w * _b.w; });

        MathTable.set_function("CrossProduct", [](Vector3 &_a, Vector3 &_b)
                               { return Vector3(_a.y * _b.z - _a.z * _b.y, _a.z * _b.x - _a.x * _b.z, _a.x * _b.y - _a.y * _b.x); });

#pragma endregion

        l_context->SetGlobal("Math", MathTable);
    };

    void Stop()
    {
        MathTable.abandon();
    }
}