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
    using indexType = typename traits<Node>::indexType;
    

    static constexpr size_t pageSize = traits<Node>::pageSize;
    static constexpr size_t maxValues = traits<Node>::maxValues;

    using LeafType = LeafNode<KeyType,ValueType,pageSize>;


    using nodeVariant = NodeVariant<KeyType,ValueType,pageSize>;

    Node* derived()
    {
        return static_cast<Node*>(this);
    }
public:
    

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

    [[nodiscard]] static constexpr indexType rightSplitCount() noexcept
    {
        return (maxValues+1)/2;
    }

    [[nodiscard]] static constexpr indexType leftSplitCount() noexcept
    {
        return (maxValues+1) - rightSplitCount();
    }


    [[nodiscard]] uint32_t findIndex(KeyType key) 
    {
        // Binary search
        uint32_t min_index = 0;
        uint32_t one_past_max_index = [&]()
        {
            if constexpr(std::is_same_v<NodeType,LeafType>)
            {
                return derived()->m_size;
            }
            else
            {
                return derived()->m_size-1;
            }
        }();
        while (one_past_max_index != min_index) {
            const uint32_t index = (min_index + one_past_max_index) / 2;
            const KeyType key_at_index = [&]()
            {
                if constexpr(std::is_same_v<NodeType,LeafType>)
                {
                    return derived()->values[index].key;
                }
                else
                {
                    return derived()->keys[index];
                }
            }();
            if (key == key_at_index) {
                if constexpr(std::is_same_v<NodeType,LeafType>)
                {
                    return index;
                }
                else
                {
                    return index + 1;
                }
            }
            if (key < key_at_index) {
                one_past_max_index = index;
            } else {
                min_index = index + 1;
            }
        }
        return min_index;
    }         

    // TODO Test
    void updateParent(const KeyType& key,PtrType rightChild,nodeVariant& root)
    {
        if constexpr(ParentType::value)
        {
            rightChild->m_parent->emplace(key,std::move(rightChild),root);
        }
        else
        {
            throw(std::out_of_range("Maximum Tree depth exeeded"));
            // throw exception max depth reached
        }
    }

    auto makeNewRoot(nodeVariant& root, PtrType&& rightChild)
    {
        if constexpr(ParentType::value)
        {
            auto oldRoot = std::move(std::get<PtrType>(root));
            auto newRoot = std::make_shared<ParentType>();
            oldRoot->m_parent = newRoot;
            rightChild->m_parent = newRoot;
            if constexpr(std::is_same_v<NodeType,LeafType>)
            {
                newRoot->keys[0] = rightChild->values[0].key;
            }
            else
            {
                newRoot->keys[0] = oldRoot->keys[oldRoot->m_size-1];
            }
            newRoot->values[0] = std::move(oldRoot);
            newRoot->values[1] = std::move(rightChild);
            newRoot->m_size = 2;
            root = newRoot;
        }
        else
        {
            throw(std::out_of_range("Maximum Tree depth exeeded"));
            // throw exception
        }
    }
};