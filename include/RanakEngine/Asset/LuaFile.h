#ifndef LUAFILE_H
#define LUAFILE_H

#include <string>
#include <memory>

#include "RanakEngine/Asset/AssetFile.h"

namespace RanakEngine::Core
{
    class Category;
    class LuaContext;
}

namespace RanakEngine::Asset
{
    class LuaFile : public AssetFile
    {
        friend Core::LuaContext;

        private:
        bool m_toBeReloaded;
        std::string m_name;

        std::weak_ptr<Core::Category> m_category;
        // Need to do same/similar for rule

        void SetCategory(std::shared_ptr<Core::Category> _category);

        public:
        LuaFile(std::string _filePath);
        ~LuaFile();

        void Reload();

        void Save();

        void FlagReloaded();
        bool GetReloaded();

        void SetCode(std::string _code);
        std::string GetCode();

        std::weak_ptr<Core::Category> GetCategory();

        std::string GetName();
    };
}

#endif