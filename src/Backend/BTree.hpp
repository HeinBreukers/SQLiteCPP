#pragma once
#include <cstddef>
#include <inttypes.h>
#include <array>
#include <bit>
#include <variant>

//#include "Pager.hpp"
//#include "OldBTree.hpp"

inline constexpr size_t ROW_SIZE = 12;
inline constexpr size_t PAGE_SIZE = 4096;
inline constexpr size_t TABLE_MAX_PAGES = 100;

struct Row
{
  uint32_t id;
  uint32_t age;
  uint32_t lastvar;

  void print()
  {
    fmt::print("id: {}, age: {}, lastvar: {}\n",id,age,lastvar);
  }
};

// TODO move out of header
std::vector<char> serialize_row(const Row& source) {
    std::vector<char> destination(3*sizeof(uint32_t));
    std::memcpy(&destination[0], &source, 3*sizeof(uint32_t)); 
    return destination;
}

Row deserialize_row(std::span<char> source) {
    Row destination;
    std::memcpy(&destination, &source[0], 3*sizeof(uint32_t));
    return destination;
}



class DBException : public std::exception 
{
public:
  DBException(const std::string& msg) : message(fmt::format("DB Exception {}",msg)) {}
  virtual ~DBException() noexcept = default;

  const char* what() const noexcept override
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

void indent(uint32_t level) 
{
    for (uint32_t i = 0; i < level; i++) {
        fmt::print("  ");
    }
}


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
        num_keys = m_header.m_numCells;
        indent(indentation_level);
        fmt::print("- leaf (size {})\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++) 
        {
            indent(indentation_level);
            fmt::print("- {}\n", m_cells[i].m_key);
        }
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

    void print(uint32_t indentation_level = 0)
    {
        uint32_t num_keys;
        num_keys = m_header.m_numKeys;
        indent(indentation_level);
        fmt::print("- internal (size {})\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++) {
            std::visit([&](auto&& arg){arg->print(++indentation_level);}, m_cells[i].m_child);
            indent(indentation_level + 1);
            fmt::print("- key {}\n", m_cells[i].m_key);
        }
        std::visit([&](auto&& arg){arg->print(++indentation_level);}, m_header.m_rightChild);
    }

    InternalNodeHeader m_header;
    std::array<InternalNodeCell,numCells> m_cells;
private:
    // empty filler for writing complete page to disk
    [[no_unique_address]] std::conditional<(filler != 0),std::array<int,filler>, Empty>::type m_filler;
};
#pragma pack()

