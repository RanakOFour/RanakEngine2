#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <memory>
#include <map>
#include <filesystem>

#include "RanakEngine/Log.h"
#include "RanakEngine/Asset/AssetFile.h"

namespace RanakEngine::Asset
{
    /**
     * @class Manager
     * @brief Singleton asset manager responsible for loading and caching all engine assets.
     *
     * Assets are keyed by their filesystem path (or a semi-colon-separated pair of paths
     * for shader programs).  The first call to Load<T>(path) constructs and stores the
     * asset; subsequent calls return the cached instance.  This ensures each file is
     * only resident once in memory.
     *
     * Obtain the global instance via Manager::Instance() after Manager::Init() has been
     * called during engine start-up.  The convenience wrapper RE::Asset::Load<T>() in
     * Assets.h calls through to this class.
     */
    class Manager
    {
        private:
        static inline std::weak_ptr<Asset::Manager> m_self; ///< Singleton back-reference.
        std::map<std::string, std::shared_ptr<AssetFile>> m_resourceMap; ///< Path -> loaded asset cache.

        Manager();
        public:
        ~Manager();
        /** @brief Creates and returns the singleton Manager instance. */
        static std::shared_ptr<Asset::Manager> Init();
        /** @brief Returns a weak pointer to the singleton Manager. */
        static std::weak_ptr<Asset::Manager> Instance();

        /**
         * @brief Re-registers an already constructed asset so it is tracked by path.
         *
         * If the path is already in the cache the existing entry is kept.
         *
         * @tparam T  Concrete AssetFile subclass.
         * @param _asset Weak pointer to the asset to register.
         * @return Weak pointer to the (possibly pre-existing) cached entry.
         */
        template<typename T>
        std::weak_ptr<T> CreateNewReference(std::weak_ptr<T> _asset)
        {
            std::shared_ptr<AssetFile> l_assetRef = std::static_pointer_cast<AssetFile>(_asset.lock());

            if(m_resourceMap.find(l_assetRef->GetPath()) == m_resourceMap.end())
            {
                m_resourceMap[l_assetRef->GetPath()] = l_assetRef;
            }

            return std::static_pointer_cast<T>(m_resourceMap[l_assetRef->GetPath()]);
        }
        
        /**
         * @brief Loads (or retrieves from cache) an asset of type T.
         *
         * For single-file assets, _path is the filesystem path.  For shader programs,
         * _path is two paths separated by ';' (fragment;vertex or compute).
         * Returns an expired weak_ptr and logs a warning if any path does not exist.
         *
         * @tparam T  Concrete AssetFile subclass (Texture, Shader, LuaFile, …).
         * @param _path  Path or semi-colon-delimited pair of paths.
         * @return Weak pointer to the loaded (or cached) asset.
         */
        template<typename T>
        std::weak_ptr<T> Load(std::string _path)
        {
            if(_path.find_first_of(';') == _path.npos)
            {
                std::filesystem::path l_fsPath(_path);
                if(!std::filesystem::exists(l_fsPath))
                {
                    Log::Warning("Asset::Load<" + std::string(typeid(T).name()) + ">, File does not exist: " + _path);
                    return std::weak_ptr<T>();
                }
            }
            else
            {
                int l_splitPos = _path.find_first_of(';');

                std::string l_path1 = _path.substr(0, l_splitPos);
                std::string l_path2 = _path.substr(l_splitPos + 1, _path.size() - l_splitPos);

                if(!std::filesystem::exists(l_path1))
                {
                    Log::Warning("Asset::Load<" + std::string(typeid(T).name()) + ">, File does not exist: " + _path);
                    return std::weak_ptr<T>();
                }

                if(!std::filesystem::exists(l_path2))
                {
                    Log::Warning("Asset::Load<" + std::string(typeid(T).name()) + ">, File does not exist: " + _path);
                    return std::weak_ptr<T>();
                }
            }
            

            if(m_resourceMap.find(_path) == m_resourceMap.end())
            {
                std::shared_ptr<T> l_newAsset;
                
                l_newAsset = std::make_shared<T>(_path);
                
                m_resourceMap[_path] = std::static_pointer_cast<AssetFile>(l_newAsset);
            }
            
            return std::static_pointer_cast<T>(m_resourceMap[_path]);
        };
    };
}

#endif