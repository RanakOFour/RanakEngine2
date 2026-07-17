#include "RanakEngine/RanakEngine.h"

namespace RanakEngine
{
    namespace
    {
        const char* k_TransformCategory = "return Category{name = \"Transform\", Position = Vector3(0.0), Rotation = Quaternion(), Scale = Vector3(1.0)}";
        const char* k_ModelCategory = "return Category{name = \"Model\", modelPath = \"\", asset = Field(nil, {hidden = true})}";
        const char* k_TextureCategory = "return Category{name = \"Texture\", texturePath = \"\", asset = Field(nil, {hidden = true}) }";
        const char* k_ShaderCategory = "return Category{name = \"Shader\", vertexPath = \"\", fragmentPath = \"\", asset = Field(nil, {hidden = true}) }";
    }

    EngineContents* Initialise(bool _debug, Vector2 _screenSize, std::string _appName)
    {
        // This also creates a LuaEngine used to initialise everything else
        EngineContents* l_toReturn = new EngineContents();

        Log::Init();

        l_toReturn->io = IO::Init(_screenSize);
        l_toReturn->assets = Asset::Init(l_toReturn->luaEngine);
        l_toReturn->core = Core::Init(_debug, _appName, l_toReturn->luaEngine);
        l_toReturn->physics = Physics::Init();

        UI::Init(l_toReturn->io);

        // Define libraries for use in engine
        Math::DefineLuaLib(l_toReturn->luaEngine);
        Core::DefineLuaLib(l_toReturn->luaEngine);
        IO::DefineLuaLib(l_toReturn->luaEngine);
        Log::DefineLuaLib(l_toReturn->luaEngine);
        Physics::DefineLuaLib(l_toReturn->luaEngine);
        UI::DefineLuaLib(l_toReturn->luaEngine);

        auto l_registry = l_toReturn->core->GetScriptRegistry();
        // Create default Transform category
        l_registry->CreateCategory(k_TransformCategory);
        l_registry->CreateCategory(k_ModelCategory);
        l_registry->CreateCategory(k_TextureCategory);
        l_registry->CreateCategory(k_ShaderCategory);

        return l_toReturn;
    };

    void Shutdown(EngineContents* _contents)
    {
        UI::Stop();
        Physics::Stop();

        Log::Message("Stopping IO");
        IO::Stop();

        Log::Message("Stopping Asset");
        Asset::Stop();

        Math::Stop();

        Log::Message("Stopping Log and Core");

        // Core shuts down Lua, which should be the last thing to stop since other subsystems
        // have Lua tables that need to be cleaned up.

        // Core also shuts down Log
        Core::Stop();
    }
}
