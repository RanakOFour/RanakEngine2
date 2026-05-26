#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#include <string>

#include "RanakEngine/Asset/AssetBase.h"

namespace RanakEngine::Core
{
    class LuaContext;
}

namespace RanakEngine::Asset
{
    /**
     * @class LuaScript
     * @brief Asset that represents a Lua source file used by the ECR system.
     *
     * Extends AssetBase with the ability to be reloaded at runtime and written
     * back to disk.  The LuaContext uses LuaScript to compile and run
     * scripts; the TextEditTab uses it to display and edit source code.
     *
     * The "reloaded" flag is set by FlagReloaded() and cleared after the
     * TextEditTab saves, allowing the UI to show an unsaved-changes indicator.
     */
    class LuaScript : public AssetBase
    {
        private:
        std::string m_name;  ///< File name (Without extension).

        public:
        /**
         * @brief Constructs the LuaFile by reading the file at _filePath into m_contents.
         * @param _filePath Filesystem path to the .lua source file.
         */
        LuaScript(std::string _filePath);
        ~LuaScript();

        /**
         * @brief Re-reads the file from disk, replacing m_contents.
         *
         * Resolves the associated Category at call-time via
         * LuaContext::GetCategory(GetName()) and triggers a hot-reload through
         * the scripting context.
         */
        void Reload();

        /** @brief Returns the filename without extension used as the category/rule name. */
        std::string GetScriptName();
    };
}

#endif