#include "RanakEngine/Asset/AssetManager.h"

namespace RanakEngine::Asset
{
    Asset::Manager::Manager()
    : m_resourceMap()
    {
        m_resourceMap.clear();
    }

    Asset::Manager::~Manager()
    {

    }
    
    std::shared_ptr<Asset::Manager> Asset::Manager::Init()
    {
        std::shared_ptr<Asset::Manager> l_toReturn;
        Asset::Manager* l_manager = new Asset::Manager();
        l_toReturn.reset(l_manager);

        l_toReturn->m_self = l_toReturn;

        return l_toReturn;
    }

    std::weak_ptr<Asset::Manager> Asset::Manager::Instance()
    {
        return m_self;
    }
}