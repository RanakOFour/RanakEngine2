#ifndef RANAKRESOURCES_H
#define RANAKRESOURCES_H

#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Asset/Audio.h"
#include "RanakEngine/Asset/Model.h"
#include "RanakEngine/Asset/Shader.h"
#include "RanakEngine/Asset/Texture.h"
#include "RanakEngine/Asset/AssetManager.h"

namespace RanakEngine::Asset
{
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
        auto l_manager = Manager::Instance().lock();
        assert(l_manager && "Asset::Load() called before AssetManager was initialised! Did you forget to call Asset::Init()?");
        return l_manager->Load<T>(_path);
    }

    std::shared_ptr<Asset::Shader> GetDefaultShader();
    
    std::shared_ptr<Asset::Model> GetDefaultModel();

    void CreateIfNotExists(const std::string& _path, const char* _data);
    void CreateIfNotExists(const std::string& _path, const char* _data, const unsigned int _size);

    std::filesystem::path GetTempDir();
    std::filesystem::path GetDataDir();

    /** @brief Registers asset-related Lua bindings with the active LuaContext. */
    void DefineLuaLib();
    /** @brief Creates and returns the Asset::Manager singleton. */
    std::shared_ptr<Asset::Manager> Init();
    /** @brief Releases all cached assets and shuts down the asset subsystem. */
    void Stop();
}

#endif