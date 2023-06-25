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




template<typename KeyType, typename ValueType,size_t Depth,size_t PageSize>
class traits<InternalNode<KeyType,ValueType,Depth,PageSize>>
{
public:
    using keyType = KeyType;
    using valueType = ValueType;
    using type = InternalNode<KeyType,ValueType,Depth,PageSize>;
    using ParentType = InternalNode<KeyType,ValueType,Depth+1,PageSize>;
    using PtrType = std::shared_ptr<type>;
    static constexpr size_t depth = Depth;
    static constexpr size_t pageSize = PageSize;
};


#pragma pack(1)
template<typename KeyType, typename ValueType, size_t Depth, size_t PageSize>
class alignas(PageSize) InternalNode: public BTreeBase<InternalNode<KeyType,ValueType,Depth,PageSize>>
{
public:
    using indexType = uint32_t;
    using ChildType = std::conditional_t<(Depth<1),LeafNode<KeyType,ValueType>,InternalNode<KeyType,ValueType,Depth-1>>;
    using ParentType = InternalNode<KeyType,ValueType,Depth+1>;

    struct ChildNode
    {
      KeyType key;
      std::unique_ptr<ChildType> value;
    };

    static constexpr size_t pageSize = PageSize;
    static constexpr size_t maxCells = (pageSize- sizeof(indexType)-sizeof(std::unique_ptr<ChildType>))/(sizeof(ChildNode));
    static constexpr size_t filler   = pageSize- sizeof(indexType) -sizeof(std::unique_ptr<ChildType>) - sizeof(std::array<ChildNode,maxCells>);

    
    
public:
    InternalNode() = default;


    KeyType maxKey() 
    {
        return values[m_size-1].key; 
    }

    ChildType& emplace(const KeyType& key,std::unique_ptr<ChildType> child)
    {
      const indexType num_cells = m_size;
      // findIndex cell
      auto cellnum = this->findIndexInternal(key)+1;
      if(cellnum==-1)
      {
        for (indexType i = num_cells; i > 0; i--) 
        {
            values[i] = std::move(values[i-1]);
        }
        values[0].key= leftMostChild->values[0].key;
        values[0].value=std::move(leftMostChild); 
        leftMostChild = std::move(child);
        m_size +=1;
        return *leftMostChild.get();
      }
      else
      {
        auto index  = static_cast<indexType>(cellnum);
        if (index < num_cells) 
        {
          // Make room for new cell
          for (indexType i = num_cells; i > index; i--) 
          {
              values[i] = std::move(values[i-1]);
          }
        }
        m_size +=1;
        values[index].key = key;
        values[index].value = std::move(child);
        return *(values[index].value.get());
      }
    }

    LeafNode<KeyType,ValueType>& findLeaf(const KeyType& key)
    {
      auto index = this->findIndexInternal(key);
      if(index==-1)
      {
        return *leftMostChild.get();
      }
      if constexpr(std::is_same_v<LeafNode<KeyType,ValueType>, ChildType>)
      {
        return *(values[static_cast<indexType>(index)].value.get());
      }
      return values[static_cast<indexType>(index)].value.get()->findLeaf(key);
    } 


    void print(uint32_t indentation_level = 0) const
    {
      fmt::print("- internal ({})\n", m_size);
      leftMostChild->print(indentation_level+1);
      for (indexType i = 0; i < m_size-1; i++) 
      {
        values[i].value->print(indentation_level+1);
      }
    }

    indexType m_size = 0;
    std::unique_ptr<ChildType> leftMostChild;
    std::array<ChildNode,maxCells> values;
    
private:
    // empty filler for writing complete page to disk
    [[no_unique_address]] typename std::conditional<(filler != 0),std::array<uint8_t,filler>, Empty>::type m_filler;
};
#pragma pack()

// TODO 
//static_assert(sizeof(InternalNode<>)==PAGE_SIZE);