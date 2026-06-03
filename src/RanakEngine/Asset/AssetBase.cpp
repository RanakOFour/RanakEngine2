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
        // m_rawContents is not NUL-terminated; build with explicit length so we
        // don't read past the buffer into heap garbage.
        return std::string(m_rawContents.data(), m_rawContents.size());
    }

    AssetType AssetBase::GetAssetType()
    {
        return m_type;
    }
}