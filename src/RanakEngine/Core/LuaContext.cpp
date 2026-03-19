#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/Camera.h"

#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    LuaContext::LuaContext()
    : m_state()
    , m_loadedScripts()
    {
        // Opens all basic libraries
        m_state.open_libraries();

        m_categoryFactory = std::make_shared<CategoryFactory>();
        Category::DefineUsertype(m_state);
        Rule::DefineUsertype(m_state);
        Camera::DefineUsertype(m_state);

        const std::string l_transformScript = "local categoryName = 'Transform'\nlocal baseAttributes = {\nPosition = Vector3.new(0, 0, 0),\nRotation = Vector3.new(0),\nScale = Vector3.new(1)\n}\nreturn Category.new(categoryName, baseAttributes)";
        sol::protected_function_result l_transformRes = m_state.script(l_transformScript);
        Category l_transformCategory = l_transformRes.get<Category>();
        m_categoryFactory->RegisterCategory(l_transformCategory);
    }

    LuaContext::~LuaContext()
    {
    }

    std::shared_ptr<LuaContext> LuaContext::Init()
    {
        std::shared_ptr<LuaContext> l_toReturn = std::shared_ptr<LuaContext>();
        LuaContext *l_wrapper = new LuaContext();
        l_toReturn.reset(l_wrapper);
        l_toReturn->m_self = l_toReturn;

        return l_toReturn;
    };

    std::weak_ptr<LuaContext> LuaContext::Instance()
    {
        if (!m_self.lock())
        {
            Log::Message("LuaContext instance was requested but it doesn't exist! Creating new instance...\n");
            return Init();
        }

        return m_self;
    }

    void LuaContext::LoadScript(std::weak_ptr<Asset::LuaFile> _file)
    {
        auto l_file = _file.lock();
        std::string l_path = l_file->GetPath();
        std::string l_code = l_file->GetCode();

        m_loadedScripts[l_path] = m_state.load_file(l_path);
        if (!m_loadedScripts[l_path].valid())
        {
            sol::error l_err = m_loadedScripts[l_path];
            Log::Message("Could not load script " + l_path + "!\n" + std::string(l_err.what()));
            m_loadedScripts.erase(l_path);
        }
    }

    sol::table LuaContext::CreateTable()
    {
        return m_state.create_table();
    }

    std::weak_ptr<Category> LuaContext::CreateCategory(std::weak_ptr<Asset::LuaFile> _file)
    {
        Log::Message("Creating category from file " + _file.lock()->GetPath() + "...\n");
        Category l_categoryTable = RunScript<Category>(_file);
        
        return m_categoryFactory->RegisterCategory(l_categoryTable);
    }

    std::weak_ptr<Category> LuaContext::GetCategory(std::bitset<1024> _signature)
    {
        return m_categoryFactory->GetBySignature(_signature);
    }

    std::weak_ptr<Category> LuaContext::GetCategory(std::string _name)
    {
        return m_categoryFactory->GetByName(_name);
    }

    void LuaContext::SetGlobal(std::string _name, sol::object &_obj)
    {
        m_state[_name] = _obj;
    }

    std::string LuaContext::GetCategoryNames()
    {
        return m_categoryFactory->GetCategoryNames();
    }
}