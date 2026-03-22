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

        std::shared_ptr<Core::Category> m_category;

        void SetCategory(std::shared_ptr<Core::Category> _category);

        public:
        LuaFile(std::string _filePath);
        ~LuaFile();

        void Reload();

        void FlagReloaded();
        bool GetReloaded();

        std::weak_ptr<Core::Category> GetCategory();

        std::string GetName();
        std::string GetCode();
    };
}

#endif