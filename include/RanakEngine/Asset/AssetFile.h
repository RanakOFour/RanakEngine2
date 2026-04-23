#ifndef ASSET_H
#define ASSET_H

#include <string>
#include <vector>

namespace RanakEngine::Asset
{
    /**
     * @enum AssetType
     * @brief Classification tag stored on every AssetFile.
     */
    enum AssetType
    {
        TEXTURE, ///< OpenGL texture
        MODEL,   ///< 3-D mesh
        SHADER,  ///< GLSL shader program
        AUDIO,   ///< Audio clip
        LUA      ///< Lua source script
    };

    /**
     * @class AssetFile
     * @brief Base class for all engine assets loaded from disk.
     *
     * Stores the file path, raw byte contents, and an AssetType tag.
     * Concrete asset types (Texture, Shader, LuaFile, …) inherit from this
     * and perform type-specific parsing in their constructors.
     * The AssetManager keeps assets alive via shared_ptr and deduplicates
     * them by path.
     */
    class AssetFile
    {
        protected:
        std::string       m_path;     ///< Filesystem path used to load this asset.
        std::vector<char> m_contents; ///< Raw file bytes read from disk.
        AssetType         m_type;     ///< Discriminator tag for this asset kind.

        public:
        /**
         * @brief Constructs the asset and reads the file into m_contents.
         * @param _path Filesystem path to the asset file.
         * @param _type AssetType classification tag.
         */
        AssetFile(std::string _path, AssetType _type);
        ~AssetFile();

        /** @brief Returns the filesystem path used to load this asset. */
        std::string GetPath();
        /** @brief Returns the raw file contents as a std::string. */
        std::string GetContents();
        /** @brief Returns the AssetType tag for this asset. */
        AssetType GetAssetType();
    };
}

#endif