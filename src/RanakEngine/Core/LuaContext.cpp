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
        if (!l_file)
        {
            Log::Warning("LoadScript: received expired file pointer.");
            return;
        }

        std::string l_path = l_file->GetPath();
        if (l_path.empty())
        {
            Log::Warning("LoadScript: file has an empty path.");
            return;
        }

        // Reject empty files — load_file would succeed but execution would
        // return nil, which then causes a sol2 conversion error for typed
        // RunScript<T>() calls.
        if (l_file->GetCode().empty())
        {
            Log::Warning("LoadScript: file is empty, skipping: " + l_path);
            return;
        }

        sol::load_result l_loaded = m_state.load_file(l_path);
        if (!l_loaded.valid())
        {
            sol::error l_err = l_loaded;
            Log::Error("Could not load script " + l_path + "!\n" + std::string(l_err.what()));
            return;
        }
        
        m_loadedScripts[l_path] = l_loaded.get<sol::protected_function>();
    }

    sol::table LuaContext::CreateTable()
    {
        return m_state.create_table();
    }

    std::weak_ptr<Category> LuaContext::CreateCategory(const std::string _code)
    {
        if (_code.empty())
        {
            Log::Warning("CreateCategory: received empty code string.");
            return std::weak_ptr<Category>();
        }

        sol::protected_function_result l_result = m_state.safe_script(_code, sol::script_pass_on_error);
        if (!l_result.valid())
        {
            sol::error l_err = l_result;
            Log::Error("CreateCategory: script error:\n" + std::string(l_err.what()));
            return std::weak_ptr<Category>();
        }

        sol::optional<Category> l_opt = l_result.get<sol::optional<Category>>();
        if (!l_opt.has_value())
        {
            Log::Error("CreateCategory: script did not return a Category.");
            return std::weak_ptr<Category>();
        }

        return m_categoryFactory->RegisterCategory(l_opt.value());
    }


    std::weak_ptr<Category> LuaContext::CreateCategory(std::weak_ptr<Asset::LuaFile> _file)
    {
        auto l_file = _file.lock();
        if (!l_file)
        {
            Log::Warning("CreateCategory: received expired file pointer.");
            return std::weak_ptr<Category>();
        }

        Log::Message("Creating category from file " + l_file->GetPath() + "...\n");

        if (l_file->GetCode().empty())
        {
            Log::Warning("CreateCategory: file is empty: " + l_file->GetPath());
            return std::weak_ptr<Category>();
        }

        Category l_categoryTable = RunScript<Category>(_file);
        if (l_categoryTable.GetName().empty() && l_file->GetName().empty())
        {
            Log::Warning("CreateCategory: script produced no usable category from: " + l_file->GetPath());
            return std::weak_ptr<Category>();
        }

        l_categoryTable.m_name = l_file->GetName();
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
        auto l_file = _file.lock();
        if (!l_file)
        {
            Log::Warning("CreateRule: received expired file pointer.");
            return Rule{};
        }

        Log::Message("Creating rule from file " + l_file->GetPath() + "...\n");

        if (l_file->GetCode().empty())
        {
            Log::Warning("CreateRule: file is empty: " + l_file->GetPath());
            return Rule{};
        }

        Rule l_rule = RunScript<Rule>(_file);
        l_rule.m_name = l_file->GetName();
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