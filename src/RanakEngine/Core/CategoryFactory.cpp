
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/LuaContext.h"

#include "RanakEngine/Asset/LuaFile.h"

#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    CategoryFactory::CategoryFactory()
    : m_signatureToCategory()
    , m_nameToSignature()
    , m_size(0)
    {
    }

    CategoryFactory::~CategoryFactory()
    {

    }

    std::weak_ptr<Category> CategoryFactory::RegisterCategory(Category _category)
    {
        std::shared_ptr<Category> l_newCategoryPtr = std::make_shared<Category>();
        l_newCategoryPtr->m_name = _category.m_name;
        l_newCategoryPtr->m_baseAttributeTable = _category.m_baseAttributeTable;
        l_newCategoryPtr->m_originFile = _category.m_originFile;

        std::bitset<1024> l_newSignature;
        l_newSignature.set(m_size);

        m_nameToSignature[l_newCategoryPtr->m_name] = l_newSignature;

        l_newCategoryPtr->m_signature = l_newSignature;

        m_signatureToCategory[l_newSignature] = l_newCategoryPtr;

        m_size++;

        return l_newCategoryPtr;
    }

    std::weak_ptr<Category> CategoryFactory::ReloadCategory(Category _category, std::bitset<1024> _signature, std::string _oldName)
    {
        std::shared_ptr<Category> l_newCategoryPtr = std::make_shared<Category>(_category);
        l_newCategoryPtr->m_signature = _signature;

        // Remove the old category from our maps
        auto l_signatureIt = m_nameToSignature.find(_oldName);
        if (l_signatureIt != m_nameToSignature.end())
        {
            std::bitset<1024> l_signature = l_signatureIt->second;
            m_signatureToCategory.erase(l_signature);
            m_nameToSignature.erase(l_signatureIt);
        }
        
        m_nameToSignature[l_newCategoryPtr->GetName()] = _signature;
        m_signatureToCategory[_signature] = l_newCategoryPtr;
        
        m_size++;

        return l_newCategoryPtr;
    }

    std::weak_ptr<Category> CategoryFactory::GetByName(std::string _name)
    {
        if(m_nameToSignature.find(_name) != m_nameToSignature.end())
        {
            std::bitset<1024> l_signature = m_nameToSignature[_name];
            return m_signatureToCategory[l_signature];
        }

        
        return std::weak_ptr<Category>();
    }

    std::weak_ptr<Category> CategoryFactory::GetBySignature(std::bitset<1024> _signature)
    {
        if(m_signatureToCategory.find(_signature) == m_signatureToCategory.end())
        {
            return std::weak_ptr<Category>();
        }

        return m_signatureToCategory[_signature];
    }

    std::vector<std::weak_ptr<Category>> CategoryFactory::GetCategories(std::bitset<1024> _combinedSignature)
    {
        std::vector<std::weak_ptr<Category>> l_toReturn;
        for(auto l_pair : m_signatureToCategory)
        {
            if((l_pair.first & _combinedSignature).any())
            {
                l_toReturn.push_back(l_pair.second);
            }
        }

        return l_toReturn;
    }

    void CategoryFactory::RenameCategory(std::bitset<1024> _signature, std::string _name)
    {
        std::string l_oldName = m_signatureToCategory[_signature]->GetName();
        m_signatureToCategory[_signature]->m_name = _name;

        m_nameToSignature.erase(l_oldName);
        m_nameToSignature[_name] = m_signatureToCategory[_signature]->m_signature;
    }

    std::string CategoryFactory::GetCategoryNames()
    {
        std::string l_toReturn;
        for(auto& l_pair : m_nameToSignature)
        {
            l_toReturn += l_pair.first + ";";
        }

        return l_toReturn;
    }
}