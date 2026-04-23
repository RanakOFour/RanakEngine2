
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
        // If a category with this name already exists, return the existing one to
        // prevent duplicate registrations (e.g. from loading the same scene twice).
        if (m_nameToSignature.count(_category.m_name))
        {
            return m_signatureToCategory[m_nameToSignature[_category.m_name]];
        }

        std::shared_ptr<Category> l_newCategoryPtr = std::make_shared<Category>();
        l_newCategoryPtr->m_name = _category.m_name;
        l_newCategoryPtr->m_fields = _category.m_fields;
        l_newCategoryPtr->m_originFile = _category.m_originFile;

        std::bitset<1024> l_newSignature;
        l_newSignature.set(m_size);

        m_nameToSignature[l_newCategoryPtr->m_name] = l_newSignature;

        l_newCategoryPtr->m_signature = l_newSignature;

        m_signatureToCategory[l_newSignature] = l_newCategoryPtr;

        m_size++;

        return l_newCategoryPtr;
    }

    std::weak_ptr<Category> CategoryFactory::ReloadCategory(std::shared_ptr<Category> _oldCategory, Category _newCategory)
    {
        // Check if the old category has any entities – if yes, abort reload to avoid data loss
        if (_oldCategory->GetSize() > 0)
        {
            Log::Error("Cannot reload category '" + _oldCategory->GetName() + "' because it is in use by entities.");
            return _oldCategory; // or return an empty weak_ptr to indicate failure
        }

        std::bitset<1024> l_signature = _oldCategory->GetSignature();
        std::string l_oldName = _oldCategory->GetName();

        // Remove old mapping
        m_signatureToCategory.erase(l_signature);
        m_nameToSignature.erase(l_oldName);

        // Create new category with the new definition but keep the same signature
        std::shared_ptr<Category> l_newCategoryPtr = std::make_shared<Category>();
        l_newCategoryPtr->m_name = _newCategory.m_name;
        l_newCategoryPtr->m_fields = _newCategory.m_fields;
        l_newCategoryPtr->m_signature = l_signature;  // preserve signature
        l_newCategoryPtr->m_originFile = _newCategory.GetOriginFile();

        // Insert new mappings
        m_nameToSignature[l_newCategoryPtr->m_name] = l_signature;
        m_signatureToCategory[l_signature] = l_newCategoryPtr;

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