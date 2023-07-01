#pragma once

#include "BTreeForwardDeclares.hpp"
#include "BTreeBase.hpp"
#include "LeafNode.hpp"
#include "InternalNode.hpp"


#include <memory>
#include <variant>
#include <type_traits>

#include <map>


template<typename KeyType, typename ValueType, size_t PageSize = PAGE_SIZE, typename Allocator = std::allocator<ValueType>>
class BTree
{
private:
    using nodeVariant = NodeVariant<KeyType,ValueType,PageSize,Allocator>;
    
public:
    // TODO conform to allocatorAwareContainer constraints
    // https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer
    // TODO implement destroy
    // for now delete copy and move constructors
    // TODO replace by allocator traits construct
    BTree():
    m_root( new LeafNode<KeyType, ValueType,PageSize,Allocator>())
    {}

    BTree(const BTree&) = delete;
    BTree(BTree&&)=delete;
    BTree& operator=(const BTree&) = delete;
    BTree& operator=(BTree&&) = delete;
    ~BTree() = default;

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

    nodeVariant m_root;
    std::size_t m_size = 0;
private:
    // TODO fix for different page sizes
    static_assert(sizeof(LeafNode<KeyType, ValueType,PageSize,Allocator>)==PageSize);
    static_assert(sizeof(InternalNode<KeyType,ValueType,0,PageSize,Allocator>)==PageSize);
    static_assert(sizeof(InternalNode<KeyType,ValueType,1,PageSize,Allocator>)==PageSize);
};