#include "RanakEngine/Asset/LuaScript.h"
#include "RanakEngine/Asset/AssetBase.h"

#include <cassert>
#include <filesystem>
#include <fstream>

namespace RanakEngine::Asset
{
    LuaScript::LuaScript(std::string _filePath)
    : AssetBase(_filePath, AssetType::LUASCRIPT)
    , m_name()
    {
        int l_nameStart = _filePath.find_last_of('/');
        if (l_nameStart == -1)
        {
            l_nameStart = _filePath.find_last_of('\\');
        }
        int l_dotPos = _filePath.find_last_of('.');
        m_name = _filePath.substr(l_nameStart + 1, l_dotPos - l_nameStart - 1);

        if (!std::filesystem::is_regular_file(_filePath))
        {
            printf("[LuaScript] Path is not a regular file: %s\n", _filePath.c_str());
        }

        std::ifstream l_stream;
        l_stream.open(_filePath);

        if (!l_stream.is_open())
        {
            printf("[LuaScript] could not open file for reading: %s\n", _filePath.c_str());
        }

        l_stream.seekg(0, std::ios::end);
        std::streampos l_fileLength = l_stream.tellg();
        l_stream.seekg(0, std::ios::beg);

        if (l_fileLength <= 0)
        {
            printf("[LuaScript] Loaded script object is empty!\n");
        }

        m_rawContents.resize(l_fileLength);
        l_stream.read(&m_rawContents[0], l_fileLength);
        m_rawContents.resize(static_cast<size_t>(l_stream.gcount()));

        printf("LuaScript %s created!\n", m_name.c_str());
    }

    LuaScript::~LuaScript()
    {

    }

    void LuaScript::Reload()
    {
        // To be implemented
        assert(false && "You have not implemented this yet!");
    }

    std::string LuaScript::GetScriptName()
    {
        return m_name;
    }
}