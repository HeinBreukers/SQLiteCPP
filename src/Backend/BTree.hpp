#pragma once
#include <cstddef>
#include <inttypes.h>

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
class LeafNodeBody
{
public:
    uint32_t m_key;
    Row m_value;
};
#pragma pack()






class InternalNode
{
    
};

class LeafNode
{

};

template<std::size_t order>
class BTree
{
public:

private:
};

template<typename Derived>
class BTreeNode
{
    NodeType nodeType;
    bool isRoot;
    BTreeNode* parent;
}

uint32_t* leaf_node_num_cells(void* node) {
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void* leaf_node_cell(void* node, uint32_t cell_num) {
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initialize_leaf_node(void* node) { *leaf_node_num_cells(node) = 0; }