#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <memory>
#include <map>
#include <filesystem>

#include "RanakEngine/Log.h"
#include "RanakEngine/Asset/AssetBase.h"

namespace RanakEngine::Asset
{
    /**
     * @class AssetCache
     * @brief Dictionary manager responsible for loading and caching all engine assets.
     *
     * Assets are keyed by their filesystem path (or a semi-colon-separated pair of paths
     * for shader programs).  The first call to Load<T>(path) constructs and stores the
     * asset; subsequent calls return the cached instance.  This ensures each file is
     * only resident once in memory.
     */
    class AssetCache
    {
        private:
        std::map<std::string, std::shared_ptr<AssetBase>> m_resourceMap; ///< Path -> loaded asset cache.

        public:
        AssetCache();
        ~AssetCache();
        
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
                
                m_resourceMap[_path] = std::static_pointer_cast<AssetBase>(l_newAsset);
            }
                        
            return std::static_pointer_cast<T>(m_resourceMap[_path]);
        };
    };
}

#endif