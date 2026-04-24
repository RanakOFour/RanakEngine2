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
    namespace
    {
        static std::shared_ptr<Asset::Manager> AssetManager; ///< Module-private singleton handle (set by Init()).
        static std::shared_ptr<Asset::Shader> DefaultShader; ///< Default shader used for drawing entities without an explicit shader.
        static std::shared_ptr<Asset::Model> DefaultModel;   ///< Default model used for drawing entities without an explicit model.
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
    static std::weak_ptr<T> Load(std::string _path)
    {
        assert(AssetManager != nullptr && "Asset::Load() called before AssetManager was initialised! Did you forget to call Asset::Init()?");
        return AssetManager->Load<T>(_path);
    };

    static std::shared_ptr<Asset::Shader> GetDefaultShader()
    {
        assert(DefaultShader != nullptr && "DefaultShader was not initialised! Did you forget to call Asset::Init()?");
        return DefaultShader;
    }

    static std::shared_ptr<Asset::Model> GetDefaultModel()
    {
        assert(DefaultModel != nullptr && "DefaultModel was not initialised! Did you forget to call Asset::Init()?");
        return DefaultModel;
    }

    /** @brief Registers asset-related Lua bindings with the active LuaContext. */
    void DefineLuaLib();
    /** @brief Creates and returns the Asset::Manager singleton. */
    std::shared_ptr<Asset::Manager> Init();
    /** @brief Releases all cached assets and shuts down the asset subsystem. */
    void Stop();
}

#endif