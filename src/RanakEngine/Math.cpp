#include "RanakEngine/Math.h"

#include "RanakEngine/LuaEngine.h"
#include "RanakEngine/Math/Quaternion.h"
#include "RanakEngine/Math/Vector4.h"
#include "sol/sol.hpp"

#include <math.h>
#include <stdio.h>

namespace RanakEngine::Math
{
    // Unnamed namespace for 'internal' Lua bindings
    namespace
    {
        static sol::table MathTable;
    };

    void DefineLuaLib(LuaEngine& _engine)
    {
        MathTable = _engine.AddTable();

#pragma region MATHSETUP

        MathTable.set_function("PI", &PI);
        MathTable.set_function("DegToRad", &DegToRad);
        MathTable.set_function("RadToDeg", &RadToDeg);
        MathTable.set_function("Max",  [](float a, float b) { return std::max(a, b); });
        MathTable.set_function("Min",  [](float a, float b) { return std::min(a, b); });
        MathTable.set_function("Abs",  [](float a)          { return std::abs(a); });
        MathTable.set_function("Cos", [](float r) { return cos(r); });
        MathTable.set_function("Sin", [](float r) { return sin(r); });
        MathTable.set_function("Random", []() { return (float)rand() / (float)RAND_MAX; });
        MathTable.set_function("RandomRange", [](float _min, float _max) { return _min + (((float)rand() / (float)RAND_MAX) * _max); });

        // Add usertypes for Vector2, Vector3, and Vector4
        _engine.AddUserType<Vector2>("Vector2",
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
                                        "ToString", &Vector2::ToString,
                                        sol::meta_function::addition,       [](const Vector2& a, const Vector2& b) { return a + b; },
                                        sol::meta_function::subtraction,    [](const Vector2& a, const Vector2& b) { return a - b; },
                                        sol::meta_function::multiplication, sol::overload(
                                            [](const Vector2& v, float s) { return v * s; },
                                            [](float s, const Vector2& v) { return v * s; }
                                        ),
                                        sol::meta_function::division,       [](const Vector2& v, float s) { return v / s; },
                                        sol::meta_function::unary_minus,    [](const Vector2& v) { return Vector2(-v.x, -v.y); },
                                        sol::meta_function::equal_to,       [](const Vector2& a, const Vector2& b) { return a == b; },
                                        sol::meta_function::to_string, &Vector2::ToString
                                        );

        _engine.AddUserType<Vector3>("Vector3",
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
                                        "ToString", &Vector3::ToString,
                                        sol::meta_function::addition,       [](const Vector3& a, const Vector3& b) { return a + b; },
                                        sol::meta_function::subtraction,    [](const Vector3& a, const Vector3& b) { return a - b; },
                                        sol::meta_function::multiplication, sol::overload(
                                            [](const Vector3& v, float s) { return v * s; },
                                            [](float s, const Vector3& v) { return v * s; }
                                        ),
                                        sol::meta_function::division,       [](const Vector3& v, float s) { return v / s; },
                                        sol::meta_function::unary_minus,    [](const Vector3& v) { return Vector3(-v.x, -v.y, -v.z); },
                                        sol::meta_function::equal_to,       [](const Vector3& a, const Vector3& b) { return a == b; },
                                        sol::meta_function::to_string, &Vector3::ToString
                                        );

        _engine.AddUserType<Vector4>("Vector4",
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
                                        "ToString", &Vector4::ToString,
                                        sol::meta_function::addition,       [](const Vector4& a, const Vector4& b) { return a + b; },
                                        sol::meta_function::subtraction,    [](const Vector4& a, const Vector4& b) { return a - b; },
                                        sol::meta_function::multiplication, sol::overload(
                                            [](const Vector4& v, float s) { return v * s; },
                                            [](float s, const Vector4& v) { return v * s; }
                                        ),
                                        sol::meta_function::division,       [](const Vector4& v, float s) { return v / s; },
                                        sol::meta_function::unary_minus,    [](const Vector4& v) { return Vector4(-v.x, -v.y, -v.z, -v.w); },
                                        sol::meta_function::equal_to,       [](const Vector4& a, const Vector4& b) { return a == b; },
                                        sol::meta_function::to_string, &Vector4::ToString
                                        );

        _engine.AddUserType<Quaternion>("Quaternion",
            sol::call_constructor,
            sol::factories([]() { return Quaternion(); },
                           [](float _x){ return Quaternion(_x); },
                           [](float _x, float _y, float _z){ return Quaternion(_x, _y, _z); },
                           [](float _angle, Vector3 _axis){ return Quaternion(_angle, _axis); },
                           [](Vector3 _eulers){ return Quaternion(_eulers); },
                           [](float _x, float _y, float _z, float _w){ return Quaternion(_x, _y, _z, _w); }
                          ),
            "x", &Quaternion::x,
            "y", &Quaternion::y,
            "z", &Quaternion::z,
            "w", &Quaternion::w,
            "Normalise", &Quaternion::Normalise,
            "Normalised", &Quaternion::Normalised,
            "Normalize", &Quaternion::Normalise,
            "Normalized", &Quaternion::Normalised,
            "ToString", &Quaternion::ToString,
            "EulerAngles", &Quaternion::EulerAngles,
            "Dot", &Quaternion::Dot,
            "ToGlm", &Quaternion::ToGlm,
            sol::meta_function::multiplication, sol::overload(
                // quat * quat -> composed rotation
                [](Quaternion _a, Quaternion _b) { return _a * _b; },
                // quat * vector -> rotated vector
                [](Quaternion _q, Vector3 _v)
                {
                    glm::vec3 l_r = _q.ToGlm() * glm::vec3(_v.x, _v.y, _v.z);
                    return Vector3(l_r.x, l_r.y, l_r.z);
                }
            ),
            sol::meta_function::equal_to, [](const Quaternion& _a, const Quaternion& _b)
            {
                return _a.x == _b.x && _a.y == _b.y && _a.z == _b.z && _a.w == _b.w;
            },
            sol::meta_function::to_string, &Quaternion::ToString
        );

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

        _engine.SetGlobal<sol::table>("Math", MathTable);
    };

    void Stop()
    {
        MathTable.abandon();
    }
}