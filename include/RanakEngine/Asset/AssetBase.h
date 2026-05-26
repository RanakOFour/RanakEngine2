#ifndef ASSETBASE_H
#define ASSETBASE_H

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
        TEXTURE,  ///< Texture
        MODEL,    ///< 3D Model
        SHADER,   ///< GLSL Shader Program
        AUDIO,    ///< Audio Clip
        LUASCRIPT ///< Lua Source Script
    };

    /**
     * @class AssetBase
     * @brief Base class for all engine assets loaded from disk.
     *
     * Stores the file path, raw byte contents, and an AssetType tag.
     * Concrete asset types (Texture, Shader, LuaFile, …) inherit from this
     * and perform type-specific parsing in their constructors.
     * The AssetManager keeps assets alive via shared_ptr and deduplicates
     * them by path.
     */
    class AssetBase
    {
        protected:
        std::string       m_path;     ///< Filesystem path used to load this asset.
        std::vector<char> m_rawContents; ///< Raw file bytes read from disk.
        AssetType         m_type;     ///< Discriminator tag for this asset kind.

        public:
        /**
         * @brief Constructs the asset and reads the file into m_contents.
         * @param _path Filesystem path to the asset file.
         * @param _type AssetType classification tag.
         */
        AssetBase(std::string _path, AssetType _type);
        ~AssetBase();

        /** @brief Returns the filesystem path used to load this asset. */
        std::string GetPath();
        /** @brief Returns the raw file contents as a std::string. */
        std::string GetContents();
        /** @brief Returns the AssetType tag for this asset. */
        AssetType GetAssetType();
    };
}

#endif