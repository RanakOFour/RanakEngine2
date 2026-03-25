#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/CoreManager.h"
#include "RanakEngine/Core/Category.h"
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
        Core::LuaContext::Instance().lock()
        ->ReloadCategory(m_category);
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
        m_contents.assign(_code.begin(), _code.end());
    }

    void LuaFile::SetCategory(std::shared_ptr<Core::Category> _category)
    {
        m_category = _category;
    }

    void LuaFile::FlagReloaded()
    {
        m_toBeReloaded = true;
    }

    bool LuaFile::GetReloaded()
    {
        return m_toBeReloaded;
    }

    std::weak_ptr<Core::Category> LuaFile::GetCategory()
    {
        return m_category;
    }

    std::string LuaFile::GetName()
    {
        return m_name;
    }

    std::string LuaFile::GetCode()
    {
        std::string l_toReturn = std::string(m_contents.data());
        return l_toReturn;
    }
}