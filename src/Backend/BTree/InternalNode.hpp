#pragma once

#include <cstddef>
#include <inttypes.h>
#include <array>
#include <bit>
#include <variant>
#include <span>
#include <memory>

#include "Row.hpp"
#include "BTreeBase.hpp"




template<typename KeyType, typename ValueType, size_t Depth,size_t PageSize, typename Allocator>
class traits<InternalNode<KeyType,ValueType,Depth,PageSize,Allocator>>
{
public:
    using indexType = uint32_t;

    using keyType = KeyType;
    using valueType = ValueType;
    using allocator = Allocator;

    using type = InternalNode<KeyType,ValueType,Depth,PageSize,Allocator>;
    using ParentType = InternalNode<KeyType,ValueType,Depth+1,PageSize,Allocator>;
    

    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<type>;
    using NodeAllocatorTraits = typename std::allocator_traits<Allocator>::template rebind_traits<type>;

    using PtrType = typename NodeAllocatorTraits::pointer;

    using ParentAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<InternalNode<KeyType,ValueType,Depth+1,PageSize,Allocator>>;
    using ParentAllocatorTraits = typename std::allocator_traits<Allocator>::template rebind_traits<InternalNode<KeyType,ValueType,Depth+1,PageSize,Allocator>>;

    using ParentPtrType = typename ParentAllocatorTraits::pointer;

    
    static constexpr size_t depth = Depth;
    static constexpr size_t pageSize = PageSize;
    static constexpr size_t maxValues = type::maxValues;
};


#pragma pack(1)
template<typename KeyType, typename ValueType, size_t Depth, size_t PageSize, typename Allocator>
class alignas(PageSize) InternalNode: public BTreeBase<InternalNode<KeyType,ValueType,Depth,PageSize,Allocator>>, public std::true_type
{
public:
    using indexType = uint32_t;

    using type = InternalNode<KeyType,ValueType,Depth,PageSize,Allocator>;
    using LeafType = LeafNode<KeyType,ValueType,PageSize,Allocator>;
    using ChildType = std::conditional_t<(Depth<1),LeafType,InternalNode<KeyType,ValueType,Depth-1,PageSize,Allocator>>;

    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<type>;
    using NodeAllocatorTraits = typename std::allocator_traits<Allocator>::template rebind_traits<type>;

    using PtrType = typename NodeAllocatorTraits::pointer;

    using ParentAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<InternalNode<KeyType,ValueType,Depth+1,PageSize,Allocator>>;
    using ParentAllocatorTraits = typename std::allocator_traits<Allocator>::template rebind_traits<InternalNode<KeyType,ValueType,Depth+1,PageSize,Allocator>>;

    using ParentPtrType = typename ParentAllocatorTraits::pointer;

    using ChildAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<ChildType>;
    using ChildAllocatorTraits = typename std::allocator_traits<Allocator>::template rebind_traits<ChildType>;

    using ChildPtrType = typename ChildAllocatorTraits::pointer;
    

    using Base = BTreeBase<InternalNode<KeyType,ValueType,Depth,PageSize,Allocator>>;

    struct ChildNode
    {
      KeyType key;
      std::unique_ptr<ChildType> value;
    };

    static constexpr size_t pageSize = PageSize;
    static_assert(pageSize>(sizeof(indexType)+ sizeof(ParentPtrType)), "PageSize too small");
    static constexpr size_t maxValues = (pageSize- sizeof(indexType)- sizeof(ParentPtrType))/(sizeof(KeyType)+sizeof(ChildPtrType));
    static constexpr size_t maxKeys = maxValues-1;
    static constexpr size_t filler   = pageSize- sizeof(indexType) -sizeof(ParentPtrType) - sizeof(std::array<KeyType,maxKeys>) - sizeof(std::array<ChildPtrType,maxValues>);

public:
    InternalNode() = default;
    InternalNode(const InternalNode&) = delete;
    InternalNode(InternalNode&&)=delete;
    InternalNode& operator=(const InternalNode&) = delete;
    InternalNode& operator=(InternalNode&&) = delete;
    ~InternalNode()
    {
        for(indexType i=0; i< m_size; ++i)
        {
            auto& childPtr = values[i];
            auto allocator = std::move(childPtr->get_allocator());
            ChildAllocatorTraits::destroy(allocator,childPtr);
            ChildAllocatorTraits::deallocate(allocator,childPtr,1);
            
        }
    }


    ChildType& emplace(const KeyType& key, ChildPtrType&& child,typename Base::nodeVariant& root)
    {
      const indexType num_cells = m_size;
      const indexType num_keys = num_cells-1;
      auto keyIndex = this->findIndex(key);
      auto valueIndex = keyIndex+1;
      if (num_cells >= maxValues) 
      {
          // Node full
          return internal_node_split_and_insert(key, std::move(child),valueIndex,root);
      }
      // findIndex cell
      if (valueIndex < num_cells) 
      {
        // Make room for new cell
        for (indexType i = num_cells; i > valueIndex; i--) 
        {
            values[i] = std::move(values[i-1]);
        }
      }
      if (keyIndex < num_keys) 
      {
        // Make room for new cell
        for (indexType i = num_keys; i > keyIndex; i--) 
        {
            keys[i] = std::move(keys[i-1]);
        }
      }
      m_size +=1;
      keys[keyIndex] = key;
      values[valueIndex] = std::move(child);
      return *(values[valueIndex]);
      
    }

    ChildType& internal_node_split_and_insert(const KeyType& key,  ChildPtrType&& value,indexType cellnum, typename Base::nodeVariant& root) 
    {
        /*
        Create a new node and move half the cells over.
        Update children
        Insert the new value in one of the two nodes.
        Update parent or create a new parent.
        */
        //auto newInternalNode = std::make_shared<InternalType>();
        // this->m_nodeAllocator.allocate(1);
        auto newInternalNode = NodeAllocatorTraits::allocate(this->get_allocator(),1);
        NodeAllocatorTraits::construct(this->m_nodeAllocator,newInternalNode);
        /*
        All existing keys plus new key should be divided
        evenly between old (left) and new (right) nodes.
        Starting from the right, move each key to correct position.
        */
        ChildType* ret;
        // TODO split loop in two for keys and values for better data locality?
        for (int32_t i = maxValues; i >= 0; i--) {
            PtrType destination_node = [&]()
            {
            if (static_cast<indexType>(i) >= Base::leftSplitCount()) 
            {
                return newInternalNode;
            } 
            else 
            {
                return this;
            }
            }();
            
            indexType index_within_node = static_cast<indexType>(i) % Base::leftSplitCount();
            ChildPtrType& destinationValue = destination_node->values[index_within_node];
            KeyType& destinationKey = destination_node->keys[index_within_node];
            //auto& destinationValueParent = ;

            if (static_cast<indexType>(i) == cellnum) 
            {
                destinationValue = std::move(value);
                destination_node->keys[index_within_node-1] = std::move(key);
                ret=destinationValue;
            } 
            else if (static_cast<indexType>(i) > cellnum) 
            {
                destinationValue = std::move(values[static_cast<indexType>(i-1)]);
                destinationKey = std::move(keys[static_cast<indexType>(i-1)]);
            } 
            else 
            {
                destinationValue = std::move(values[static_cast<indexType>(i)]);
                // TODO for i==maxValue index falls out of range of keys
                if(i<maxKeys)
                {
                    destinationKey = std::move(keys[static_cast<indexType>(i)]);
                }
            }

            if (static_cast<indexType>(i) >= Base::leftSplitCount()) 
            {
                destinationValue->m_parent = newInternalNode;
            }
        }
        /* Update cell count on both leaf nodes */
        m_size = Base::leftSplitCount();
        newInternalNode->m_size = Base::rightSplitCount();
        
        if(m_parent)
        {
            newInternalNode->m_parent = m_parent;
            this->updateParent(keys[--m_size],std::move(newInternalNode),root);
        }
        else
        {
            // current node is root since parent == nullptr
            this->makeNewRoot(root, std::move(newInternalNode));
        }

        return *ret;
    }

    LeafType& findLeaf(const KeyType& key)
    {
      auto index = this->findIndex(key);
      
      if constexpr(std::is_same_v<LeafType, ChildType>)
      {
        return *(values[static_cast<indexType>(index)]);
      }
      return values[static_cast<indexType>(index)]->findLeaf(key);
    } 


    void print(uint32_t indentation_level = 0) const
    {
      this->indent(indentation_level);
      fmt::print("- internal ({})\n", m_size);
      for (indexType i = 0; i < m_size; i++) 
      {
        values[i]->print(indentation_level+1);
        if(i<(m_size-1))
        {
            this->indent(indentation_level);
            fmt::print("- Key ({})\n", keys[i]);
        }
      }
    }

    indexType m_size = 0;
    ParentPtrType m_parent = nullptr;
    std::array<KeyType,maxKeys> keys;
    std::array<ChildPtrType,maxValues> values;

private:
    // empty filler for writing complete page to disk
    [[no_unique_address]] typename std::conditional<(filler != 0),std::array<uint8_t,filler>, Empty>::type m_filler;
};
#pragma pack()
