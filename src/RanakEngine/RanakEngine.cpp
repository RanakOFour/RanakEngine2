#include "RanakEngine/RanakEngine.h"

#include <filesystem>

namespace RanakEngine
{
    EngineContents Initialise(bool _debug, Vector2 _screenSize)
    {
        EngineContents l_toReturn;

        Log::Init();

        l_toReturn.resources = Asset::Init();
        l_toReturn.io = IO::Init(_screenSize);
        l_toReturn.core = Core::Init(_debug);
        // l_toReturn.physics = Physics::Init();

        Math::DefineLuaLib();
        Asset::DefineLuaLib();
        Core::DefineLuaLib();
        IO::DefineLuaLib();
        Log::DefineLuaLib();

        // Load transform category (Required by registry to exist)
        std::weak_ptr<Asset::LuaFile> l_categoryFile = l_toReturn.resources->Load<Asset::LuaFile>("./resources/Categories/Transform.lua");
        std::ofstream l_fileStream;
        
        if (!std::filesystem::exists("./resources/Categories/"))
        {
            std::filesystem::create_directories("./resources/Categories/");
        }

        if(!std::filesystem::exists("./resources/Categories/Transform.lua"))
        {
            printf("Could not find transform category. Creating...\n");
            l_fileStream.open("./resources/Categories/Transform.lua", std::ios::out);

            l_fileStream << "return Category {\n"
                            "        Position = Vector2(0.0),\n"
                            "        Layer = 0,\n"
                            "        Rotation = 0.0,\n"
                            "        Scale = Vector2(1.0)\n"
                            "}";

            l_fileStream.close();
        }

        l_categoryFile = l_toReturn.resources->Load<Asset::LuaFile>("./resources/Categories/Transform.lua");
        l_toReturn.core->GetLuaContext()->CreateCategory(l_categoryFile);

        // Load model category
        if (!std::filesystem::exists("./resources/Categories/Model.lua"))
        {
            printf("Could not find model category. Creating...\n");
            l_fileStream.open("./resources/Categories/Model.lua", std::ios::out);

            l_fileStream << "return Category {\n"
                            "       modelPath = \"\",\n"
                            "       asset = Field(nil, { hidden = true })\n"
                            "}";

            l_fileStream.close();
        }

        l_categoryFile = l_toReturn.resources->Load<Asset::LuaFile>("./resources/Categories/Model.lua");
        l_toReturn.core->GetLuaContext()->CreateCategory(l_categoryFile);

        if(!std::filesystem::exists("./resources/Categories/Texture.lua"))
        {
            printf("Could not find texture category. Creating...\n");
            l_fileStream.open("./resources/Categories/Texture.lua", std::ios::out);

            l_fileStream << "return Category {\n"
                            "       texturePath = \"\",\n"
                            "       asset = Field(nil, { hidden = true })\n"
                            "}";

            l_fileStream.close();
        }
        
        l_categoryFile = l_toReturn.resources->Load<Asset::LuaFile>("./resources/Categories/Texture.lua");
        l_toReturn.core->GetLuaContext()->CreateCategory(l_categoryFile);

        if(!std::filesystem::exists("./resources/Categories/Shader.lua"))
        {
            printf("Could not find shader category. Creating...\n");
            l_fileStream.open("./resources/Categories/Shader.lua", std::ios::out);

            l_fileStream << "return Category {\n"
                            "       vertexshaderPath = \"\",\n"
                            "       fragmentshaderPath = \"\",\n"
                            "       asset = Field(nil, { hidden = true })\n"
                            "}";

            l_fileStream.close();
        }

        l_categoryFile = l_toReturn.resources->Load<Asset::LuaFile>("./resources/Categories/Shader.lua");
        l_toReturn.core->GetLuaContext()->CreateCategory(l_categoryFile);

        return l_toReturn;
    };

    void Shutdown(EngineContents &_contents)
    {
        // Physics::Stop();

        Log::Message("Stopping IO");
        IO::Stop();

        Log::Message("Stopping Asset");
        Asset::Stop();

        Log::Message("Stopping Log");
        Log::Stop();

        Math::Stop();

        Log::Message("Stopping Core");
        Core::Stop();
    }
}