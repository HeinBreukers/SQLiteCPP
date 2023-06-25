#pragma once
#include <cstddef>
#include <inttypes.h>
#include <array>
#include <string>
#include <variant>
#include <fmt/core.h>
#include <memory>

#include "BTreeForwardDeclares.hpp"

class DBException : public std::exception 
{
public:
  DBException(const std::string& msg) : message(fmt::format("DB Exception {}",msg)) {}
  virtual ~DBException() noexcept = default;

  [[nodiscard]] const char* what() const noexcept override
  {
    return message.c_str();
  }

private:
  std::string message;
};

class BTreeException : public DBException 
{
public:
  BTreeException(const std::string& msg) : DBException(fmt::format("<BTree>: \"{}\"", msg)) {}
  virtual ~BTreeException() noexcept = default;
};

// enum class NodeType: uint8_t
// {
//     Internal,
//     Leaf
// };

// CRTP base class Page
template<typename Node>
class BTreeBase
{
protected:
    using KeyType = typename traits<Node>::keyType;
    using ValueType = typename traits<Node>::valueType;
    using NodeType = typename traits<Node>::type;
    using ParentType = typename traits<Node>::ParentType;
    using PtrType = typename traits<Node>::PtrType;

    using nodeVariant = NodeVariant<KeyType,ValueType>;

    Node* derived()
    {
        return static_cast<Node*>(this);
    }
public:
    static constexpr size_t pageSize = traits<Node>::pageSize;

    uint32_t maxKey()
    {
        return static_cast<Node*>(this)->maxKey();
    }

    static void indent(uint32_t level) 
    {
        for (uint32_t i = 0; i < level; i++) 
        {
            fmt::print("  ");
        }
    }

protected:
    // Page should only be inherited, and not be directly addressable
    BTreeBase() = default;
    BTreeBase(const BTreeBase&) = default;
    BTreeBase(BTreeBase&&) noexcept = default;
    BTreeBase& operator=(const BTreeBase&) = default;
    BTreeBase& operator=(BTreeBase&&) noexcept = default;
    ~BTreeBase() = default; 


    [[nodiscard]] uint32_t findIndex(KeyType key) 
    {
        // Binary search
        uint32_t min_index = 0;
        uint32_t one_past_max_index = derived()->m_size;
        while (one_past_max_index != min_index) {
            const uint32_t index = (min_index + one_past_max_index) / 2;
            const KeyType key_at_index = derived()->values[index].key;
            if (key == key_at_index) {
            return index;
            }
            if (key < key_at_index) {
            one_past_max_index = index;
            } else {
            min_index = index + 1;
            }
        }
        return min_index;
    }

    // if key > index && key < index+1 -> return val[index]
    //  
    //                 

    [[nodiscard]] int findIndexInternal(KeyType key) 
    {
        // Binary search
        int min_index = 0;
        // since size = leftMost + array size;
        int one_past_max_index = static_cast<int>(derived()->m_size)-1;
        while (one_past_max_index != min_index) {
            const int index = (min_index + one_past_max_index) / 2;
            const KeyType key_at_index = derived()->values[static_cast<uint32_t>(index)].key;
            if (key == key_at_index) {
            return index;
            }
            if (key < key_at_index) {
            one_past_max_index = index;
            } else {
            min_index = index + 1;
            }
        }
        return min_index-1;
    }

    // template<typename T, typename ValueType>
    // ValueType& emplace(uint32_t key, const ValueType& value)
    // {
    //     const uint32_t num_cells = this->m_size;
    //     // find cell
    //     // TODO 
    //     auto cellnum = findIndex(key);
    //     if (num_cells >= this->maxCells) {
    //         // Node full
    //         return leaf_node_split_and_insert(key, value,cellnum);
    //     }
    //     else
    //     {
            
    //         if (cellnum < num_cells) {
    //         // Make room for new cell
    //         for (uint32_t i = num_cells; i > cellnum; i--) {
    //             this->values[i] = this->values[i-1];
    //         }
    //         }
    //         this->m_size +=1;
    //         this->values[cellnum].key = key;
    //         this->values[cellnum].value = value;
    //         return this->values[cellnum];
    //     }
    // }

    void updateParent(PtrType rightChild)
    {
        if constexpr(std::is_same_v<NodeType,LeafNode<KeyType,ValueType>>)
        {
            auto& rightChildFirstRow = rightChild->values[0];
            rightChild->m_parent->emplace(rightChildFirstRow.key,std::move(rightChild));
        }
    }

    auto makeNewRoot(nodeVariant& root, PtrType rightChild)
    {
        auto oldRoot = std::move(std::get<PtrType>(root));
        auto newRoot = std::make_shared<ParentType>();
        oldRoot->m_parent = newRoot;
        rightChild->m_parent = newRoot;
        newRoot->leftMostChild = std::move(oldRoot);
        newRoot->values[0].key = rightChild->values[0].key;
        newRoot->values[0].value = std::move(rightChild);
        newRoot->m_size = 2;
        root = newRoot;
    }
};