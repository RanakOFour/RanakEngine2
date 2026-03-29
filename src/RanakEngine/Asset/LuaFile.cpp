#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/CoreManager.h"
#include "RanakEngine/Log.h"

#include <fstream> 
#include <sstream>
#include <memory>

namespace RanakEngine::Asset
{
    LuaFile::LuaFile(std::string _filePath)
    : AssetFile(_filePath, AssetType::LUA)
    , m_toBeReloaded(false)
    , m_name()
    {
        int l_nameStart = _filePath.find_last_of('/');
        int l_dotPos = _filePath.find_last_of('.');
        m_name = _filePath.substr(l_nameStart + 1, l_dotPos - l_nameStart - 1);

        // Open file
        std::ifstream l_stream;
        l_stream.open(_filePath);

        // Get size of file
        l_stream.seekg(0,std::ios::end);
        std::streampos l_fileLength = l_stream.tellg();
        l_stream.seekg(0,std::ios::beg);

        // Read file into content vector
        m_contents.resize(l_fileLength);
        l_stream.read(&m_contents[0], l_fileLength);

        printf("LuaFile %s created!\n", m_name.c_str());
    }

    LuaFile::~LuaFile()
    {

    }

    void LuaFile::Reload()
    {
        m_toBeReloaded = false;
        // Resolve the associated Category at call-time by name rather than
        // storing a forward pointer on the file itself.
        auto l_context  = Core::LuaContext::Instance().lock();
        auto l_category = l_context->GetCategory(m_name);
        l_context->ReloadCategory(l_category);
    }

    void LuaFile::Save()
    {
        std::ofstream l_stream(m_path, std::ios::out | std::ios::trunc);
        if (!l_stream.is_open())
        {
            Log::Warning("LuaFile::Save() - Stream could not be opened for writing at " + m_path);
            return;
        }

        l_stream.write(m_contents.data(), m_contents.size());
        l_stream.close();
    }

    void LuaFile::SetCode(std::string _code)
    {
        m_contents.clear();
        m_contents.resize(_code.size());
        m_contents.assign(_code.begin(), _code.end());
    }

    void LuaFile::FlagReloaded()
    {
        m_toBeReloaded = true;
    }

    bool LuaFile::GetReloaded()
    {
        return m_toBeReloaded;
    }

    std::string LuaFile::GetName()
    {
        return m_name;
    }

    std::string LuaFile::GetCode()
    {
        std::string l_toReturn = std::string(m_contents.data(), m_contents.size());
        return l_toReturn;
    }
}