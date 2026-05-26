#ifndef RANAKRESOURCES_H
#define RANAKRESOURCES_H

#include "RanakEngine/Asset/LuaScript.h"
#include "RanakEngine/Asset/AudioSample.h"
#include "RanakEngine/Asset/Model.h"
#include "RanakEngine/Asset/Shader.h"
#include "RanakEngine/Asset/Texture.h"
#include "RanakEngine/Asset/AssetCache.h"
#include <memory>

namespace RanakEngine::Asset
{
    namespace
    {
        std::shared_ptr<AssetCache> g_AssetCache;
        std::shared_ptr<Asset::Shader> g_DefaultShader; ///< Default shader used for drawing entities without an explicit shader.
        std::shared_ptr<Asset::Model> g_DefaultModel;   ///< Default model used for drawing entities without an explicit model.
        sol::table g_AssetTable;
    }

    /**
     * @brief Convenience wrapper that forwards to Asset::Manager::Load<T>().
     *
     * Allows callers to load assets without holding a reference to the AssetManager:
     * @code{.cpp}
     *   auto tex = RanakEngine::Asset::Load<Texture>("resources/icon.png");
     * @endcode
     *
     * @tparam T   Asset type (Texture, Audio, Shader, Model, LuaFile, …).
     * @param _path Filesystem path to the asset file.
     * @return Weak pointer to the loaded (or cached) asset.
     */
    template<typename T>
    inline std::weak_ptr<T> Load(std::string _path)
    {
        g_AssetCache = std::make_shared<AssetCache>();
        
        assert(g_AssetCache);
        
        return g_AssetCache;
    }

    std::shared_ptr<Asset::Shader> GetDefaultShader();
    std::shared_ptr<Asset::Model> GetDefaultModel();

    void CreateIfNotExists(const std::string& _path, const char* _data);
    void CreateIfNotExists(const std::string& _path, const char* _data, const unsigned int _size);

    std::filesystem::path GetTempDir();
    std::filesystem::path GetDataDir();

    /** @brief Creates and returns the global asset cache. */
    std::shared_ptr<AssetCache> Init();
    /** @brief Releases all cached assets and shuts down the asset subsystem. */
    void Stop();
}

#endif