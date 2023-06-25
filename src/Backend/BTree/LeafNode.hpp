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
    static constexpr size_t pageSize = PageSize;
    

};

//TODO instead of pack(1) calculate alignment between header and array
#pragma pack(1)
template<typename KeyType, typename ValueType, size_t PageSize>
class alignas(PageSize) LeafNode: public BTreeBase<LeafNode<KeyType, ValueType, PageSize>>
{
public:
    // using KeyType = uint32_t;
    // using ValueType = int; 
    using RowType = Row<KeyType,ValueType>; 
    using ParentType = InternalNode<KeyType,ValueType,0>;
    using ParentPtrType = std::shared_ptr<ParentType>;
    using indexType = uint32_t;
    using Base = BTreeBase<LeafNode<KeyType, ValueType, PageSize>>;


    static constexpr size_t pageSize = PageSize;
    static constexpr size_t maxCells = (pageSize- sizeof(indexType)- sizeof(ParentPtrType))/sizeof(RowType);
    static constexpr size_t filler   = pageSize- sizeof(indexType)- sizeof(ParentPtrType) - sizeof(std::array<RowType,maxCells>);
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

    LeafNode<KeyType,ValueType>& findLeaf([[maybe_unused]] const KeyType& key)
    {
        return *this;
    }

    ValueType& emplace(const KeyType& key, const ValueType& value, typename Base::nodeVariant& root)
    {
        const indexType num_cells = m_size;
        // findIndex cell
        // TODO 
        auto cellnum = this->findIndex(key);
        auto& row = values[cellnum];
        if(row.key==key)
        {
            throw(std::out_of_range("Duplicate Key"));
        }
        

        if (num_cells >= maxCells) {
            // Node full
            return leaf_node_split_and_insert(key, value,cellnum,root);
        }
        else
        {
            
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
    }

    // [[nodiscard]] uint32_t findIndex(uint32_t key) 
    // {
    //     // Binary search
    //     uint32_t min_index = 0;
    //     uint32_t one_past_max_index = m_size;
    //     while (one_past_max_index != min_index) {
    //         const uint32_t index = (min_index + one_past_max_index) / 2;
    //         const uint32_t key_at_index = values[index].key;
    //         if (key == key_at_index) {
    //         return index;
    //         }
    //         if (key < key_at_index) {
    //         one_past_max_index = index;
    //         } else {
    //         min_index = index + 1;
    //         }
    //     }
    //     return min_index;
    // }

    ValueType& leaf_node_split_and_insert(const KeyType& key, const ValueType& value,indexType cellnum, typename Base::nodeVariant& root) 
    {
        /*
        Create a new node and move half the cells over.
        Insert the new value in one of the two nodes.
        Update parent or create a new parent.
        */
        auto newLeaf = std::make_unique<LeafNode<KeyType,ValueType>>();
        /*
        All existing keys plus new key should be divided
        evenly between old (left) and new (right) nodes.
        Starting from the right, move each key to correct position.
        */
        ValueType* ret;
        for (int32_t i = LeafNode<KeyType,ValueType>::maxCells; i >= 0; i--) {
            LeafNode<KeyType,ValueType>* destination_node = [&]()
            {
            if (static_cast<indexType>(i) >= LeafNode<KeyType,ValueType>::leftSplitCount()) 
            {
                return newLeaf.get();
            } 
            else 
            {
                return this;
            }
            }();
            
            indexType index_within_node = static_cast<indexType>(i) % LeafNode<KeyType,ValueType>::leftSplitCount();
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
        m_size = LeafNode<KeyType,ValueType>::leftSplitCount();
        newLeaf->m_size = LeafNode<KeyType,ValueType>::rightSplitCount();
        
        // TODO
        if(m_parent)
        {
            newLeaf->m_parent = m_parent;
            this->updateParent(std::move(newLeaf),newLeaf->values[0].key);
        }
        else
        {
            this->makeNewRoot(root, std::move(newLeaf));
        }

        return *ret;
    }

    [[nodiscard]] static constexpr indexType rightSplitCount() noexcept
    {
        return (maxCells+1)/2;
    }

    [[nodiscard]] static constexpr indexType leftSplitCount() noexcept
    {
        return (maxCells+1) - rightSplitCount();
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
    //std::array<uint32_t,maxCells> keys = {};
    ParentPtrType m_parent = nullptr;
    std::array<RowType,maxCells> values = {};
private:

    //split
    // empty filler for writing complete page to disk
    [[no_unique_address]] typename std::conditional<(filler != 0) ,std::array<uint8_t,filler>, Empty>::type m_filler;
};
#pragma pack()

