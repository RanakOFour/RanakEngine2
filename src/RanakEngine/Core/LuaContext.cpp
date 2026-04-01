#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/Camera.h"
#include "RanakEngine/Core/Scene.h"

#include "RanakEngine/Physics/PhysicsBody.h"

#include "RanakEngine/Assets.h"

#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    LuaContext::LuaContext()
    : m_state()
    , m_loadedScripts()
    {
        // Opens all basic libraries
        m_state.open_libraries(
                               sol::lib::base,
                               sol::lib::table,
                               sol::lib::string
                              );

        m_categoryFactory = std::make_shared<CategoryFactory>();
        Category::DefineUsertype(m_state);
        Rule::DefineUsertype(m_state);
        Camera::DefineUsertype(m_state);
        Scene::DefineUsertype(m_state);
        Physics::Body::DefineUserType(m_state);

        // Field(default, opts) — wraps a default value with optional editor metadata.
        // CloneTable extracts only the default when creating per-entity data.
        m_state.set_function("Field", [this](sol::object _default, sol::optional<sol::table> _opts) -> sol::table {
            sol::table l_field = m_state.create_table();
            l_field["__isField"] = true;
            l_field["default"]   = _default;
            if (_opts.has_value())
            {
                l_field["opts"] = *_opts;
            }
            return l_field;
        });
    }

    LuaContext::~LuaContext()
    {
        m_categoryFactory.reset();
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

    std::weak_ptr<Category> LuaContext::CreateCategory(const std::string _code)
    {
        sol::protected_function_result l_result = m_state.script(_code);
        Category l_category = l_result.get<Category>();
        
        return m_categoryFactory->RegisterCategory(l_category);
    }


    std::weak_ptr<Category> LuaContext::CreateCategory(std::weak_ptr<Asset::LuaFile> _file)
    {
        Log::Message("Creating category from file " + _file.lock()->GetPath() + "...\n");
        Category l_categoryTable = RunScript<Category>(_file);
        // Derive the category name from the filename rather than relying on the Lua script to supply it.
        l_categoryTable.m_name = _file.lock()->GetName();
        l_categoryTable.SetOriginFile(_file);
        
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

    Rule LuaContext::CreateRule(std::weak_ptr<Asset::LuaFile> _file)
    {
        Log::Message("Creating rule from file " + _file.lock()->GetPath() + "...\n");
        Rule l_rule = RunScript<Rule>(_file);
        // Derive the rule name from the filename, matching the same convention as categories.
        l_rule.m_name = _file.lock()->GetName();
        l_rule.SetOriginFile(_file);
        return l_rule;
    }

    void LuaContext::ReloadCategory(std::weak_ptr<Core::Category> _category)
    {
        auto l_oldCategory = _category.lock();
        if (!l_oldCategory) return;

        auto l_file = l_oldCategory->GetOriginFile().lock();
        if (!l_file) return;

        // Reload the Lua file to get the new definition
        LoadScript(l_file);
        Category l_newCategory = RunScript<Category>(l_file);
        l_newCategory.m_name = l_file->GetName();

        // Attempt to reload through the factory (will abort if category has entities)
        auto l_newCategoryPtr = m_categoryFactory->ReloadCategory(l_oldCategory, l_newCategory).lock();
        if (l_newCategoryPtr)
        {
            l_newCategoryPtr->SetOriginFile(l_file);
        }
        else
        {
            Log::Error("Reload of category '" + l_oldCategory->GetName() + "' failed because it still has entities.");
        }
    }

    void LuaContext::SetGlobal(std::string _name, sol::object &_obj)
    {
        m_state[_name] = _obj;
    }

    sol::table LuaContext::GetGlobalTable()
    {
        return m_state.globals();
    }

    std::string LuaContext::GetCategoryNames()
    {
        return m_categoryFactory->GetCategoryNames();
    }
}