#pragma once
#include <cstddef>
#include <inttypes.h>
#include <array>
#include <bit>
#include <variant>
#include <span>
#include <memory>
#include <fmt/ranges.h>

#include "Row.hpp"
#include "BTreeBase.hpp"

template<typename KeyType, typename ValueType,size_t PageSize>
class traits<LeafNode<KeyType, ValueType, PageSize>>
{
public:
    using keyType = KeyType;
    using valueType = ValueType;
    using type = LeafNode<KeyType, ValueType, PageSize>;
    using ParentType = InternalNode<KeyType,ValueType,0,PageSize>;
    using PtrType = std::unique_ptr<type>;
    using ParentPtrType = std::shared_ptr<ParentType>;
    using indexType = uint32_t;

    static constexpr size_t pageSize = PageSize;
    static constexpr size_t maxValues = type::maxValues;
};

//TODO instead of pack(1) calculate alignment between header and array
#pragma pack(1)
template<typename KeyType, typename ValueType, size_t PageSize>
class alignas(PageSize) LeafNode: public BTreeBase<LeafNode<KeyType, ValueType, PageSize>>
{
public:

    using RowType = Row<KeyType,ValueType>; 
    using ParentType = InternalNode<KeyType,ValueType,0,PageSize>;
    using ParentPtrType = std::shared_ptr<ParentType>;
    using indexType = uint32_t;
    using Base = BTreeBase<LeafNode<KeyType, ValueType, PageSize>>;
    using LeafType = LeafNode<KeyType,ValueType,PageSize>;


    static constexpr size_t pageSize = PageSize;
    static_assert(pageSize>(sizeof(indexType)+ sizeof(ParentPtrType)), "PageSize too small");
    static constexpr size_t maxValues = (pageSize- sizeof(indexType)- sizeof(ParentPtrType))/sizeof(RowType);
    static constexpr size_t filler   = pageSize- sizeof(indexType)- sizeof(ParentPtrType) - sizeof(std::array<RowType,maxValues>);
public:
    LeafNode()=default;

    [[nodiscard]] indexType maxKey() 
    {
        return values[m_size-1].key; 
    }

    // [[nodiscard]] static constexpr size_t maxCells() noexcept
    // {
    //     return maxCells;
    // }

    ValueType& at(const KeyType& key)
    {
        auto& row = values[this->findIndex(key)];
        if(row.key!=key)
        {
            throw(std::out_of_range("Key not in BTree"));
        }
        return row.value;
    }

    LeafType& findLeaf([[maybe_unused]] const KeyType& key)
    {
        return *this;
    }

    ValueType& emplace(const KeyType& key, const ValueType& value, typename Base::nodeVariant& root)
    {
        const indexType num_cells = m_size;

        auto cellnum = this->findIndex(key);
        if (num_cells >= maxValues) {
            // Node full
            return leaf_node_split_and_insert(key, value,cellnum,root);
        }

        auto& row = values[cellnum];
        if(row.key==key && cellnum<m_size)
        {
            throw(std::out_of_range("Duplicate Key"));
        }
        
        if (cellnum < num_cells) {
        // Make room for new cell
        for (uint32_t i = num_cells; i > cellnum; i--) {
            values[i] = values[i-1];
        }
        }
        m_size +=1;
        row = values[cellnum];
        row.key = key;
        row.value = value;
        return row.value;
        
    }

    ValueType& leaf_node_split_and_insert(const KeyType& key, const ValueType& value,indexType cellnum, typename Base::nodeVariant& root) 
    {
        /*
        Create a new node and move half the cells over.
        Insert the new value in one of the two nodes.
        Update parent or create a new parent.
        */
        auto newLeaf = std::make_unique<LeafType>();
        /*
        All existing keys plus new key should be divided
        evenly between old (left) and new (right) nodes.
        Starting from the right, move each key to correct position.
        */
        ValueType* ret;
        for (int32_t i = maxValues; i >= 0; i--) {
            LeafType* destination_node = [&]()
            {
            if (static_cast<indexType>(i) >= Base::leftSplitCount()) 
            {
                return newLeaf.get();
            } 
            else 
            {
                return this;
            }
            }();
            
            indexType index_within_node = static_cast<indexType>(i) % Base::leftSplitCount();
            RowType& destination = destination_node->values[index_within_node];

            if (static_cast<indexType>(i) == cellnum) 
            {
                destination.value = value;
                destination.key = key;
                ret=&(destination.value);
            } 
            else if (static_cast<indexType>(i) > cellnum) 
            {
                destination = values[static_cast<indexType>(i-1)];
            } 
            else 
            {
                destination = values[static_cast<indexType>(i)];
            }
        }
        /* Update cell count on both leaf nodes */
        m_size = Base::leftSplitCount();
        newLeaf->m_size = Base::rightSplitCount();
        
        if(m_parent)
        {
            newLeaf->m_parent = m_parent;
            this->updateParent(newLeaf->values[0].key,std::move(newLeaf),root);
        }
        else
        {
            this->makeNewRoot(root, std::move(newLeaf));
        }

        return *ret;
    }

    

    void print(uint32_t indentation_level = 0) const
    {
        const indexType num_keys = m_size;
        this->indent(indentation_level);
        fmt::print("- leaf (size {})\n", num_keys);
        for (indexType i = 0; i < num_keys; i++) 
        {
            this->indent(indentation_level+1);
            fmt::print("- {}, \n", values[i].key);
        }
    }

    indexType m_size = 0;
    ParentPtrType m_parent = nullptr;
    std::array<RowType,maxValues> values = {};
private:

    // empty filler for writing complete page to disk
    [[no_unique_address]] typename std::conditional<(filler != 0) ,std::array<uint8_t,filler>, Empty>::type m_filler;
};
#pragma pack()

