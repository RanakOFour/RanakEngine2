// RanakEngine demo: a small 3D scene with a controllable quad and camera.
//
// Self-contained: writes its Lua rule scripts and GLSL shaders to a temp dir at
// startup, then drives the engine through RE::Initialise / Start / Shutdown.
//   Entity:  W/A/S/D  move the quad (XY plane)
//   Camera:  I/J/K/L  pan, U/O roll, Q/E yaw
//   Quit:    close the window

#include "RanakEngine/RanakEngine.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace
{
    void WriteFile(const std::filesystem::path& _path, const std::string& _contents)
    {
        std::filesystem::create_directories(_path.parent_path());
        std::ofstream l_out(_path, std::ios::trunc);
        l_out << _contents;
    }

    const char* k_vertShader =
        "#version 430 core\n"
        "in vec3 a_Position;\n"
        "in vec2 a_PixelColor;\n"
        "uniform mat4 u_Projection;\n"
        "uniform mat4 u_View;\n"
        "uniform mat4 u_Model;\n"
        "out vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);\n"
        "    v_texCoord = a_PixelColor;\n"
        "}\n";

    const char* k_fragShader =
        "#version 430 core\n"
        "in vec2 v_texCoord;\n"
        "out vec4 o_fragColor;\n"
        "void main()\n"
        "{\n"
        "    o_fragColor = vec4(0.95, 0.45, 0.20, 1.0);\n"
        "}\n";

    const char* k_movementRule =
        "return Rule {\n"
        "    categories = { \"Transform\" },\n"
        "    Update = function(self, data)\n"
        "        local dt = Core.DeltaTime()\n"
        "        local speed = 15.0\n"
        "        local p = data.Transform.Position\n"
        "        local dx, dy = 0.0, 0.0\n"
        "        if IO.GetKeyDown('w') then dy = dy + 1.0 end\n"
        "        if IO.GetKeyDown('s') then dy = dy - 1.0 end\n"
        "        if IO.GetKeyDown('a') then dx = dx - 1.0 end\n"
        "        if IO.GetKeyDown('d') then dx = dx + 1.0 end\n"
        "        data.Transform.Position = Vector3(p.x + dx * speed * dt, p.y + dy * speed * dt, p.z)\n"
        "    end\n"
        "}\n";

    // Camera control: pan (IJKL), dolly along z (U/O), yaw (Q/E)
    const char* k_cameraRule =
        "return Rule {\n"
        "    categories = { \"Transform\" },\n"
        "    fields = { yaw = 0.0 },\n"
        "    Update = function(self, data)\n"
        "        local cam = Core.Camera\n"
        "        local dt = Core.DeltaTime()\n"
        "        local panSpeed = 10.0\n"
        "        local p = cam:getPosition()\n"
        "        local dx, dy, dz = 0.0, 0.0, 0.0\n"
        "        if IO.GetKeyDown('l') then dx = dx + 1.0 end\n"
        "        if IO.GetKeyDown('j') then dx = dx - 1.0 end\n"
        "        if IO.GetKeyDown('i') then dy = dy + 1.0 end\n"
        "        if IO.GetKeyDown('k') then dy = dy - 1.0 end\n"
        "        if IO.GetKeyDown('o') then dz = dz + 1.0 end\n"
        "        if IO.GetKeyDown('u') then dz = dz - 1.0 end\n"
        "        cam:setPosition(Vector3(p.x + dx * panSpeed * dt, p.y + dy * panSpeed * dt, p.z + dz * panSpeed * dt))\n"
        "        local yawSpeed = 60.0\n"
        "        if IO.GetKeyDown('q') then self.fields.yaw = self.fields.yaw - yawSpeed * dt end\n"
        "        if IO.GetKeyDown('e') then self.fields.yaw = self.fields.yaw + yawSpeed * dt end\n"
        "        cam:setRotation(Vector3(0.0, self.fields.yaw, 0.0))\n"
        "    end\n"
        "}\n";

    const char* k_renderingRule =
        "return Rule {\n"
        "    categories = { \"Transform\" },\n"
        "    Draw = function(self, data)\n"
        "        Core.Camera:Draw(data)\n"
        "    end\n"
        "}\n";
}

namespace fs = std::filesystem;

int main()
{
    fs::path l_assetDir = fs::temp_directory_path() / "demo_assets";
    fs::path l_vertPath = l_assetDir / "REDemo.vert";
    fs::path l_fragPath = l_assetDir / "REDemo.frag";
    fs::path l_movePath = l_assetDir / "Move.lua";
    fs::path l_cameraPath = l_assetDir / "Camera.lua";
    fs::path l_renderPath = l_assetDir / "Render.lua";

    WriteFile(l_vertPath, k_vertShader);
    WriteFile(l_fragPath, k_fragShader);
    WriteFile(l_movePath, k_movementRule);
    WriteFile(l_cameraPath, k_cameraRule);
    WriteFile(l_renderPath, k_renderingRule);

    RE::EngineContents* l_engine = RE::Initialise(true, Vector2(1280.0f, 720.0f), "RanakEngine Demo");

    std::shared_ptr<RE::Core::Manager> l_core = l_engine->core;
    std::shared_ptr<RE::Core::Scene> l_scene = l_core->GetScene().lock();
    std::shared_ptr<RE::Core::ScriptRegistry> l_registry = l_core->GetScriptRegistry();

    int l_entity = l_scene->AddEntity();          // gets Transform automatically
    l_scene->AddToCategory(l_entity, "Shader");
    l_scene->AddToCategory(l_entity, "Model");

    RE::Core::Rule l_moveRule = l_registry->CreateRule(
        RE::Asset::Load<RE::Asset::LuaScript>(l_movePath.string()));
    l_scene->AddRule(l_moveRule);

    RE::Core::Rule l_cameraRule = l_registry->CreateRule(
        RE::Asset::Load<RE::Asset::LuaScript>(l_cameraPath.string()));
    l_scene->AddRule(l_cameraRule);

    RE::Core::Rule l_renderRule = l_registry->CreateRule(
        RE::Asset::Load<RE::Asset::LuaScript>(l_renderPath.string()));
    l_scene->AddRule(l_renderRule);

    l_core->Start(); // blocks until the window is closed

    RE::Shutdown(l_engine);
    return 0;
}
