#include "RanakEngine/Core/ScriptRegistry.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/Camera.h"
#include "RanakEngine/Core/Scene.h"

#include "RanakEngine/Asset/LuaScript.h"
#include "RanakEngine/LuaEngine.h"
#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    ScriptRegistry::ScriptRegistry(LuaEngine& _engine)
    : m_engine(&_engine)
    {
        m_categoryFactory = std::make_shared<CategoryFactory>();

        // Register all Core usertypes with the shared Lua runtime.
        Category::DefineUsertype(_engine);
        Rule::DefineUsertype(_engine);
        Camera::DefineUsertype(_engine);
        Scene::DefineUsertype(_engine);
    }

    ScriptRegistry::~ScriptRegistry()
    {
        m_categoryFactory.reset();
    }

    std::shared_ptr<ScriptRegistry> ScriptRegistry::Init(LuaEngine& _engine)
    {
        if (auto l_existing = m_self.lock())
        {
            return l_existing;
        }

        std::shared_ptr<ScriptRegistry> l_toReturn;
        l_toReturn.reset(new ScriptRegistry(_engine));
        l_toReturn->m_self = l_toReturn;

        return l_toReturn;
    }

    std::weak_ptr<ScriptRegistry> ScriptRegistry::Instance()
    {
        return m_self;
    }

    sol::table ScriptRegistry::CreateTable()
    {
        return m_engine->AddTable();
    }

    LuaEngine& ScriptRegistry::GetEngine()
    {
        return *m_engine;
    }

    std::weak_ptr<Category> ScriptRegistry::CreateCategory(const std::string _code)
    {
        if (_code.empty())
        {
            Log::Warning("CreateCategory: received empty code string.");
            return std::weak_ptr<Category>();
        }

        sol::optional<Category> l_opt = m_engine->RunScript<Category>(_code);
        if (!l_opt.has_value())
        {
            Log::Error("CreateCategory: script did not return a Category.");
            return std::weak_ptr<Category>();
        }

        return m_categoryFactory->RegisterCategory(l_opt.value());
    }

    std::weak_ptr<Category> ScriptRegistry::CreateCategory(std::weak_ptr<Asset::LuaScript> _file)
    {
        auto l_file = _file.lock();
        if (!l_file)
        {
            Log::Warning("CreateCategory: received expired file pointer.");
            return std::weak_ptr<Category>();
        }

        Log::Message("Creating category from file " + l_file->GetPath() + "...");

        if (l_file->GetContents().empty())
        {
            Log::Warning("CreateCategory: file is empty: " + l_file->GetPath());
            return std::weak_ptr<Category>();
        }

        Category l_categoryTable = RunScript<Category>(_file);
        if (l_categoryTable.GetName().empty() && l_file->GetScriptName().empty())
        {
            Log::Warning("CreateCategory: script produced no usable category from: " + l_file->GetPath());
            return std::weak_ptr<Category>();
        }

        l_categoryTable.m_name = l_file->GetScriptName();
        l_categoryTable.SetOriginFile(_file);

        return m_categoryFactory->RegisterCategory(l_categoryTable);
    }

    std::weak_ptr<Category> ScriptRegistry::GetCategory(std::bitset<1024> _signature)
    {
        return m_categoryFactory->GetBySignature(_signature);
    }

    std::weak_ptr<Category> ScriptRegistry::GetCategory(std::string _name)
    {
        return m_categoryFactory->GetByName(_name);
    }

    Rule ScriptRegistry::CreateRule(std::weak_ptr<Asset::LuaScript> _file)
    {
        auto l_file = _file.lock();
        if (!l_file)
        {
            Log::Warning("CreateRule: received expired file pointer.");
            return Rule{};
        }

        if (l_file->GetContents().empty())
        {
            Log::Warning("CreateRule: file is empty: " + l_file->GetPath());
            return Rule{};
        }

        Rule l_rule = RunScript<Rule>(_file);
        l_rule.m_name = l_file->GetScriptName();
        l_rule.SetOriginFile(_file);
        return l_rule;
    }

    void ScriptRegistry::ReloadCategory(std::weak_ptr<Core::Category> _category)
    {
        auto l_oldCategory = _category.lock();
        if (!l_oldCategory) return;

        auto l_file = l_oldCategory->GetOriginFile().lock();
        if (!l_file) return;

        Category l_newCategory = RunScript<Category>(l_file);
        l_newCategory.m_name = l_file->GetScriptName();

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

    std::string ScriptRegistry::GetCategoryNames()
    {
        return m_categoryFactory->GetCategoryNames();
    }
}
