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
    /**
     * @class LuaFile
     * @brief Asset that represents a Lua source file used by the ECR system.
     *
     * Extends AssetFile with the ability to be reloaded at runtime (hot-reload),
     * written back to disk, and associated with the Category it defines.  The
     * LuaContext uses LuaFile to compile and run scripts; the TextEditTab uses
     * it to display and edit source code in the editor.
     *
     * The "reloaded" flag is set by FlagReloaded() and cleared after the
     * TextEditTab saves, allowing the UI to show an unsaved-changes indicator.
     */
    class LuaFile : public AssetFile
    {
        friend Core::LuaContext;

        private:
        bool m_toBeReloaded; ///< True when the file has been modified but not yet re-executed.
        std::string m_name;  ///< Human-readable name derived from the filename (no extension).

        std::weak_ptr<Core::Category> m_category; ///< The Category this file defines (may be expired for rule files).

        /** @brief Associates this file with the given Category after a successful compile. */
        void SetCategory(std::shared_ptr<Core::Category> _category);

        public:
        /**
         * @brief Constructs the LuaFile by reading the file at _filePath into m_contents.
         * @param _filePath Filesystem path to the .lua source file.
         */
        LuaFile(std::string _filePath);
        ~LuaFile();

        /**
         * @brief Re-reads the file from disk, replacing m_contents.
         *
         * Called during hot-reload to pick up external edits.  The LuaContext
         * must subsequently re-compile and re-run the script.
         */
        void Reload();

        /** @brief Writes the current m_contents back to disk. */
        void Save();

        /** @brief Sets the m_toBeReloaded flag (the editor shows a dirty indicator). */
        void FlagReloaded();
        /** @brief Returns true if the file has been modified since the last save/reload. */
        bool GetReloaded();

        /**
         * @brief Replaces the in-memory source code (does not write to disk).
         * @param _code New Lua source to store in m_contents.
         */
        void SetCode(std::string _code);
        /** @brief Returns the current in-memory Lua source as a std::string. */
        std::string GetCode();

        /** @brief Returns a weak pointer to the Category this file defines. */
        std::weak_ptr<Core::Category> GetCategory();

        /** @brief Returns the filename without extension used as the category/rule name. */
        std::string GetName();
    };
}

#endif