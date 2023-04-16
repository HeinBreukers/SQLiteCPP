#pragma once
#include <cstddef>
#include <inttypes.h>
#include <array>
#include <bit>
#include <variant>

//#include "Pager.hpp"
//#include "OldBTree.hpp"

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

struct Empty
{};

#pragma pack(1)
class CommonNodeHeader
{
public:
    NodeType m_nodeType;
    bool m_isRoot;
    uint32_t* m_parent;
};
#pragma pack()
#pragma pack(1)
class LeafNodeHeader : public CommonNodeHeader
{
public:
    uint32_t m_numCells;
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

class CommonNode
{
// public: 
//     CommonNodeHeader* Header()
//     {
//         // TODO replace by bitcast
//         return reinterpret_cast<CommonNodeHeader*>(&m_memPool[0]);
//     }

protected:
    void indent(uint32_t level) 
    {
        for (uint32_t i = 0; i < level; i++) {
            fmt::print("  ");
        }
    }
};



template<size_t PageSize = PAGE_SIZE>
class LeafNode;
template<size_t PageSize = PAGE_SIZE>
class InternalNode;

using nodePtr = std::variant<LeafNode<>*,InternalNode<>*>;



//TODO instead of pack(1) calculate alignment between header and array
#pragma pack(1)
template<size_t PageSize>
class alignas(PageSize) LeafNode
{
private:
    static constexpr size_t pageSize = PageSize;
    static constexpr size_t numCells = (pageSize- sizeof(LeafNodeHeader))/sizeof(LeafNodeCell);
    static constexpr size_t filler   = (pageSize- sizeof(LeafNodeHeader)) - sizeof(std::array<LeafNodeCell,numCells>);
public:
    LeafNode():
    m_header(LeafNodeHeader{{NodeType::Leaf, false, nullptr}, 0})
    {
    }

    LeafNode(bool isRoot, uint32_t* parent, uint32_t t_numCells):
    m_header(LeafNodeHeader{{NodeType::Leaf, isRoot, parent}, t_numCells})
    {
    }

    [[nodiscard]] uint32_t maxKey() 
    {
        return m_cells[m_header.m_numCells].m_key; 
    }

    [[nodiscard]] static constexpr size_t maxCells() noexcept
    {
        return numCells;
    }

    LeafNodeHeader m_header;
    std::array<LeafNodeCell,numCells> m_cells;
private:
    // empty filler for writing complete page to disk
    [[no_unique_address]] std::conditional<(filler != 0) ,std::array<int,filler>, Empty>::type m_filler;
};
#pragma pack()


#pragma pack(1)
class InternalNodeHeader : public CommonNodeHeader
{
public:
    uint32_t m_numKeys;
    nodePtr m_rightChild;
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

#pragma pack(1)
template<size_t PageSize>
class alignas(PageSize) InternalNode
{
private:
    static constexpr size_t pageSize = PageSize;
    static constexpr size_t numCells = (pageSize- sizeof(InternalNodeHeader))/sizeof(InternalNodeCell);
    static constexpr size_t filler   = (pageSize- sizeof(InternalNodeHeader)) - sizeof(std::array<InternalNodeCell,numCells>);
public:
    InternalNode():
    m_header(InternalNodeHeader{{NodeType::Internal, false, nullptr}, 0, nodePtr{} })
    {
    }

    InternalNode(bool isRoot, uint32_t* parent, uint32_t numKeys, nodePtr rightChild):
    m_header(InternalNodeHeader{{NodeType::Internal, isRoot, parent}, numKeys, rightChild})
    {
    }

    uint32_t maxKey() 
    {
        return m_cells[m_header.m_numKeys].m_key; 
    }

    InternalNodeHeader m_header;
    std::array<InternalNodeCell,numCells> m_cells;
private:
    // empty filler for writing complete page to disk
    [[no_unique_address]] std::conditional<(filler != 0),std::array<int,filler>, Empty>::type m_filler;
};
#pragma pack()

