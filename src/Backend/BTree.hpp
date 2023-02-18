#pragma once
#include <cstddef>
#include <inttypes.h>
#include <array>
#include <bit>

#include "Pager.hpp"

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
};


class LeafNode: public CommonNode
{
public:
    LeafNode()
    {
        init();
    }
    
    void init()
    {
        Header()->nodeType=NodeType::Leaf;
        Header()->numCells=0;
    }

    LeafNodeHeader* Header()
    {
        // TODO replace by bitcast
        return reinterpret_cast<LeafNodeHeader*>(&m_memPool[0]);
    }

    LeafNodeCell* Cell(size_t index)
    {
        return reinterpret_cast<LeafNodeCell*>(&m_memPool[0]+ sizeof(LeafNodeHeader)+index*sizeof(LeafNodeCell));
    }

    [[nodiscard]] size_t maxCells() noexcept
    {
        return (Page::size - sizeof(LeafNodeHeader))/sizeof(LeafNodeCell);
    }

    void print()
    {
        uint32_t num_cells = Header()->numCells;
        fmt::print("leaf (size {})\n", num_cells);
        for (uint32_t i = 0; i < num_cells; i++) {
            uint32_t key = Cell(i)->m_key;
            fmt::print("  - {} : {}\n", i, key);
        }
    }

};

// uint32_t* leaf_node_num_cells(void* node) {
//   return node + LEAF_NODE_NUM_CELLS_OFFSET;
// }

// void* leaf_node_cell(void* node, uint32_t cell_num) {
//   return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
// }

// uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
//   return leaf_node_cell(node, cell_num);
// }

// void* leaf_node_value(void* node, uint32_t cell_num) {
//   return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
// }

// void initialize_leaf_node(void* node) { *leaf_node_num_cells(node) = 0; }