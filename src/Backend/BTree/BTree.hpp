#pragma once

#include "BTreeForwardDeclares.hpp"
#include "BTreeBase.hpp"
#include "LeafNode.hpp"
#include "InternalNode.hpp"


#include <memory>
#include <variant>
#include <type_traits>

#include <map>


template<typename KeyType, typename ValueType>
class BTree
{
private:
    using nodeVariant = NodeVariant<KeyType,ValueType>;

public:

    ValueType& emplace(const KeyType& key, const ValueType& value)
    {
        auto& ret =  std::visit([&](auto&& t_ptr) ->ValueType&
        {
            auto& leafnode = t_ptr->findLeaf(key);
            return leafnode.emplace(key,value,m_root);
        },m_root);
        ++ m_size;
        return ret;
    }

    ValueType& at(const KeyType& key)
    {
        return std::visit([&](auto&& t_ptr) ->ValueType&
        {
            auto& leafnode = t_ptr->findLeaf(key);
            return leafnode.at(key);
        },m_root);
    }

    std::size_t size()
    {
        return m_size;
    }

    void print() const
    {
        std::visit([](auto&& t_ptr)->void
        {
            t_ptr->print();
        },m_root);
    }
private:
    nodeVariant m_root = std::make_unique<LeafNode<KeyType, ValueType>>();
    std::size_t m_size = 0;
private:
    static_assert(sizeof(LeafNode<KeyType, ValueType>)==PAGE_SIZE);
    static_assert(sizeof(InternalNode<KeyType,ValueType,0>)==PAGE_SIZE);
};