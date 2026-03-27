#ifndef CATEGORYFACTORY_H
#define CATEGORYFACTORY_H

#include <memory>
#include <map>
#include <unordered_map>
#include <bitset>
#include <vector>

#include "sol/sol.hpp"

namespace RanakEngine
{
namespace Asset
{
    class LuaFile;
}
namespace Core
{
    class LuaContext;
    class Category;

    /**
     * @class CategoryFactory
     * @brief Registry that owns all Category instances and manages their signatures.
     *
     * Each Category is assigned a unique bit in a 1024-bit signature when it is
     * first registered.  The factory provides O(1) lookup by name or by signature,
     * and supports hot-reload by swapping old Category data in place.
     *
     * Accessed exclusively through LuaContext::CreateCategory() /
     * LuaContext::ReloadCategory(); not intended for direct external use.
     */
    class CategoryFactory
    {
        friend LuaContext;
        private:
        int m_size; ///< Number of categories registered so far (used to assign the next free bit).

        // Unordered because bitset and string do not have operator< by default.
        std::unordered_map<std::bitset<1024>, std::shared_ptr<Category>> m_signatureToCategory; ///< Signature -> Category.
        std::map<std::string, std::bitset<1024>> m_nameToSignature; ///< Category name -> its signature.

        /**
         * @brief Registers a new Category, assigns it a unique signature bit, and stores it.
         * @param _category The Category to register (moved in).
         * @return Weak pointer to the stored Category.
         */
        std::weak_ptr<Category> RegisterCategory(Category _category);

        /**
         * @brief Replaces _oldCategory with _newCategory in place, preserving entity membership.
         *
         * If _oldCategory still has members (GetSize() > 0) the reload is aborted.
         *
         * @param _oldCategory Shared pointer to the category being replaced.
         * @param _newCategory New category data to swap in.
         * @return Weak pointer to the updated Category.
         */
        std::weak_ptr<Category> ReloadCategory(std::shared_ptr<Category> _oldCategory, Category _newCategory);

        public:
        CategoryFactory();
        ~CategoryFactory();

        /**
         * @brief Looks up a Category by name.
         * @param _name Category name to search for.
         * @return Weak pointer to the matching Category, or expired if not found.
         */
        std::weak_ptr<Category> GetByName(std::string _name);
        /**
         * @brief Looks up a Category by its exact signature.
         * @param _signature The 1024-bit signature to match.
         * @return Weak pointer to the matching Category, or expired if not found.
         */
        std::weak_ptr<Category> GetBySignature(std::bitset<1024> _signature);
        /**
         * @brief Returns all Categories whose individual signatures are set in the combined mask.
         * @param _combinedSignature Bitwise OR of one or more category signatures.
         * @return Vector of weak pointers to all matching Categories.
         */
        std::vector<std::weak_ptr<Category>> GetCategories(std::bitset<1024> _combinedSignature);
        /**
         * @brief Renames a registered Category (updates the name->signature map).
         * @param _signature Signature of the category to rename.
         * @param _name      New name to assign.
         */
        void RenameCategory(std::bitset<1024> _signature, std::string _name);
        /** @brief Returns a semi-colon separated list of all registered category names. */
        std::string GetCategoryNames();
    };
}
}

#endif