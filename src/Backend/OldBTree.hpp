#pragma once
#include <cstddef>
#include <inttypes.h>
#include <array>
#include <bit>
#include <variant>

#include "Pager.hpp"

class BTreeException : public DBException 
{
public:
  BTreeException(const std::string& msg) : DBException(fmt::format("<BTree>: \"{}\"", msg)) {}
  virtual ~BTreeException() noexcept = default;
};

enum class NodeType: uint8_t
{
    Internal,
    Leaf
};


#pragma pack(1)
class CommonNodeHeader
{
public:
    NodeType nodeType;
    bool isRoot;
    uint32_t* parent;
};
#pragma pack()
#pragma pack(1)
class LeafNodeHeader : public CommonNodeHeader
{
public:
    uint32_t numCells;
};
#pragma pack()
#pragma pack(1)
class LeafNodeCell
{
public:
    uint32_t m_key;
    Row m_value;
};
#pragma pack()

class CommonNode: public Page<>
{
public: 
    CommonNodeHeader* Header()
    {
        // TODO replace by bitcast
        return reinterpret_cast<CommonNodeHeader*>(&m_memPool[0]);
    }

protected:
    void indent(uint32_t level) 
    {
        for (uint32_t i = 0; i < level; i++) {
            fmt::print("  ");
        }
    }
};


class LeafNode;
class InternalNode;

using nodePtr = std::variant<LeafNode*,InternalNode*>;

// union nodePtr
// {
//     LeafNode* leafPtr;
//     InternalNode* internalPtr;
// };

class LeafNode: public CommonNode
{
public:
    LeafNode()
    {
        init();
    }
    
    // TODO make private so only RAII
    void init() 
    {
        Header()->nodeType=NodeType::Leaf;
        Header()->isRoot=false;
        Header()->numCells=0;
    }

    LeafNodeHeader* Header() 
    {
        // TODO replace by bitcast
        return reinterpret_cast<LeafNodeHeader*>(&m_memPool[0]);
    }

    uint32_t maxKey() 
    {
        return Cell(Header()->numCells)->m_key; 
    }

    LeafNodeCell* Cell(size_t index)
    {
        return reinterpret_cast<LeafNodeCell*>(&m_memPool[0]+ sizeof(LeafNodeHeader)+index*sizeof(LeafNodeCell));
    }

    [[nodiscard]] static constexpr size_t maxCells() noexcept
    {
        return (Page::size - sizeof(LeafNodeHeader))/sizeof(LeafNodeCell);
    }

    [[nodiscard]] static constexpr size_t rightSplitCount() noexcept
    {
        return (maxCells()+1)/2;
    }

    [[nodiscard]] static constexpr size_t leftSplitCount() noexcept
    {
        return (maxCells()+1) - rightSplitCount();
    }

    void print(uint32_t indentation_level = 0)
    {
        uint32_t num_keys;
        num_keys = Header()->numCells;
        indent(indentation_level);
        fmt::print("- leaf (size {})\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++) 
        {
            indent(indentation_level);
            fmt::print("- {}\n", Cell(i)->m_key);
        }
    }

};

#pragma pack(1)
class InternalNodeHeader : public CommonNodeHeader
{
public:
    uint32_t numKeys;
    nodePtr rightChild;
};
#pragma pack()
#pragma pack(1)
class InternalNodeCell
{
public:
    nodePtr m_child;
    uint32_t m_key;
};
#pragma pack()

class InternalNode: public CommonNode
{
public:
    InternalNode()
    {
        init();
    }
    
    // TODO make private so only RAII
    void init() 
    {
        Header()->nodeType=NodeType::Internal;
        Header()->isRoot=false;
        Header()->numKeys=0;
    }


    InternalNodeHeader* Header() 
    {
        // TODO replace by bitcast
        return reinterpret_cast<InternalNodeHeader*>(&m_memPool[0]);
    }

    uint32_t maxKey() 
    {
        return Cell(Header()->numKeys)->m_key; 
    }

    InternalNodeCell* Cell(size_t index)
    {
        uint32_t num_keys = Header()->numKeys;
        if (index > num_keys) {
            throw BTreeException(fmt::format("Tried to access child_num {} > num_keys {}", index, num_keys));
        } else if (index == num_keys) {
            // TODO fix dangerous cast from commonnode to internalnodecell
            return std::visit([](auto&& arg){return reinterpret_cast<InternalNodeCell*>(arg);}, Header()->rightChild);
        } else {
            return reinterpret_cast<InternalNodeCell*>(&m_memPool[0]+ sizeof(InternalNodeHeader)+index*sizeof(InternalNodeCell));
        }
    }

    void print(uint32_t indentation_level = 0)
    {
        uint32_t num_keys;
        num_keys = Header()->numKeys;
        indent(indentation_level);
        fmt::print("- internal (size {})\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++) {
            std::visit([&](auto&& arg){arg->print(++indentation_level);}, Cell(i)->m_child);
            indent(indentation_level + 1);
            fmt::print("- key {}\n", Cell(i)->m_key);
        }
        std::visit([&](auto&& arg){arg->print(++indentation_level);}, Header()->rightChild);
    }
};


nodePtr fromPage(Page<>* page)
{
    auto nodeType = static_cast<CommonNode*>(page)->Header()->nodeType;
    switch (nodeType)
    {
    case (NodeType::Internal):
        return static_cast<InternalNode*>(page);
        break;
    case (NodeType::Leaf):
        return static_cast<LeafNode*>(page);
        break;

    }
    throw BTreeException(fmt::format("Invalid Node Type"));
}