#pragma once
#include <cstddef>
#include <inttypes.h>
#include <array>
#include <bit>
#include <variant>

#include "Row.hpp"
#include "BTreeBase.hpp"


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

//TODO instead of pack(1) calculate alignment between header and array
#pragma pack(1)
template<size_t PageSize>
class alignas(PageSize) LeafNode: public Page<LeafNode<PageSize>>
{
public:
    static constexpr size_t pageSize = PageSize;
    static constexpr size_t numCells = (pageSize- sizeof(LeafNodeHeader))/sizeof(LeafNodeCell);
    static constexpr size_t filler   = (pageSize- sizeof(LeafNodeHeader)) - sizeof(std::array<LeafNodeCell,numCells>);
public:
    LeafNode():
    m_header(LeafNodeHeader{{NodeType::Leaf, false, nodePtr{}}, 0})
    {
    }

    LeafNode(bool isRoot, nodePtr parent, uint32_t t_numCells):
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
        uint32_t num_keys = m_header.m_numCells;
        this->indent(indentation_level);
        fmt::print("- leaf (size {})\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++) 
        {
            this->indent(indentation_level);
            fmt::print("- {}\n", m_cells[i].m_key);
        }
    }

    LeafNodeHeader m_header;
    std::array<LeafNodeCell,numCells> m_cells = {};
private:
    // empty filler for writing complete page to disk
    [[no_unique_address]] typename std::conditional<(filler != 0) ,std::array<uint8_t,filler>, Empty>::type m_filler;
};
#pragma pack()

static_assert(sizeof(LeafNode<>)==PAGE_SIZE);

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
class alignas(PageSize) InternalNode: public Page<InternalNode<PageSize>>
{
private:
    static constexpr size_t pageSize = PageSize;
    static constexpr size_t numCells = (pageSize- sizeof(InternalNodeHeader))/sizeof(InternalNodeCell);
    static constexpr size_t filler   = (pageSize- sizeof(InternalNodeHeader)) - sizeof(std::array<InternalNodeCell,numCells>);
public:
    InternalNode():
    m_header(InternalNodeHeader{{NodeType::Internal, false, nodePtr{}}, 0, nodePtr{} })
    {
    }

    InternalNode(bool isRoot, nodePtr parent, uint32_t numKeys, nodePtr rightChild):
    m_header(InternalNodeHeader{{NodeType::Internal, isRoot, parent}, numKeys, rightChild})
    {
    }

    uint32_t maxKey() 
    {
        return m_cells[m_header.m_numKeys].m_key; 
    }

    void print(uint32_t indentation_level = 0)
    {
        uint32_t num_keys = m_header.m_numKeys;
        this->indent(indentation_level);
        fmt::print("- internal (size {})\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++) {
            std::visit([&](auto&& arg){arg->print(++indentation_level);}, m_cells[i].m_child);
            this->indent(indentation_level + 1);
            fmt::print("- key {}\n", m_cells[i].m_key);
        }
        std::visit([&](auto&& arg){arg->print(++indentation_level);}, m_header.m_rightChild);
    }

    InternalNodeHeader m_header;
    std::array<InternalNodeCell,numCells> m_cells;
private:
    // empty filler for writing complete page to disk
    [[no_unique_address]] typename std::conditional<(filler != 0),std::array<uint8_t,filler>, Empty>::type m_filler;
};
#pragma pack()

static_assert(sizeof(InternalNode<>)==PAGE_SIZE);

