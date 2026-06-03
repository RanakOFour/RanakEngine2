#ifndef RANAKASSETS_H
#define RANAKASSETS_H

#include "RanakEngine/Asset/LuaScript.h"
#include "RanakEngine/Asset/AudioSample.h"
#include "RanakEngine/Asset/Model.h"
#include "RanakEngine/Asset/Shader.h"
#include "RanakEngine/Asset/Texture.h"
#include "RanakEngine/Asset/AssetCache.h"
#include <memory>

namespace RanakEngine
{
    class LuaEngine;

namespace Asset
{
    /**
     * @brief Convenience wrapper that forwards to the module's cached AssetCache.
     *
     * @code{.cpp}
     *   auto tex = RanakEngine::Asset::Load<Texture>("resources/icon.png");
     * @endcode
     *
     * Defined in Assets.cpp and explicitly instantiated for each asset type, so the
     * module's globals (the cache, defaults, Lua table) stay private to that file.
     *
     * @tparam T   Asset type (Texture, Model, Shader, AudioSample, LuaScript).
     * @param _path Filesystem path to the asset file.
     * @return Weak pointer to the loaded (or cached) asset.
     */
    template<typename T>
    std::weak_ptr<T> Load(std::string _path);

    std::shared_ptr<Asset::Shader> GetDefaultShader();
    std::shared_ptr<Asset::Model> GetDefaultModel();

    void CreateIfNotExists(const std::string& _path, const char* _data);
    void CreateIfNotExists(const std::string& _path, const char* _data, const unsigned int _size);

    std::filesystem::path GetTempDir();
    std::filesystem::path GetDataDir();

    /** @brief Creates and returns the global asset cache. */
    std::shared_ptr<AssetCache> Init(LuaEngine& _engine);
    /** @brief Releases all cached assets and shuts down the asset subsystem. */
    void Stop();
}
}

#endif