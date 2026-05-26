#include "RanakEngine/Asset/AssetBase.h"

namespace RanakEngine::Asset
{
    AssetBase::AssetBase(std::string _filePath, AssetType _type) 
    : m_rawContents()
    , m_type(_type)
    , m_path(_filePath)
    {
    }

    AssetBase::~AssetBase()
    {
        
    }

    std::string AssetBase::GetPath()
    {
        return m_path;
    }

    std::string AssetBase::GetContents()
    {
        return std::string(m_rawContents.data());
    }

    AssetType AssetBase::GetAssetType()
    {
        return m_type;
    }
}