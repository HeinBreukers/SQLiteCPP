#pragma once

#include "Pager.hpp"
#include "Table.hpp"
#include "BTree.hpp"

class CursorException : public DBException 
{
public:
  CursorException(const std::string& msg) : DBException(fmt::format("<Cursor>: \"{}\"", msg)) {}
  virtual ~CursorException() noexcept = default;
};

// Cursor iterates over cells of BTree node
class Cursor{
public:

  // TODO overload increment operators
  // TODO iterators
  void advance() 
  {
    ++m_cellNum;
    if (m_cellNum >= std::get<LeafNode<>*>(m_node)->m_header.m_numCells)
    {
      m_endOfTable = true;
    }
  }

  auto value() {
    return std::get<LeafNode<>*>(m_node)->m_cells[m_cellNum].m_value;
  }

  void insert(uint32_t key, const Row& value)
  {
    auto* leafNode = std::get<LeafNode<>*>(m_node);

    uint32_t num_cells = leafNode->m_header.m_numCells;
    if (num_cells >= LeafNode<>::maxCells()) {
      // Node full
      leaf_node_split_and_insert(key, value);
      return;
    }

    if (m_cellNum < num_cells) {
      // Make room for new cell
      for (uint32_t i = num_cells; i > m_cellNum; i--) {
        leafNode->m_cells[i] = leafNode->m_cells[i-1];
      }
    }
    leafNode->m_header.m_numCells +=1;
    leafNode->m_cells[m_cellNum].m_key = key;
    leafNode->m_cells[m_cellNum].m_value = value;
  }

  void leaf_node_split_and_insert([[maybe_unused]] uint32_t key, const Row& value) {
    /*
    Create a new node and move half the cells over.
    Insert the new value in one of the two nodes.
    Update parent or create a new parent.
    */
    auto* oldLeafNode = std::get<LeafNode<>*>(m_node);
    auto* newLeafNode = m_table.m_pager.getUnusedNode<NodeType::Leaf>();

    // TODO proper init
    //newLeafNode->init();
    /*
    All existing keys plus new key should be divided
    evenly between old (left) and new (right) nodes.
    Starting from the right, move each key to correct position.
    */
    for (int32_t i = LeafNode<>::maxCells(); i >= 0; i--) {
      LeafNode<>* destination_node;
      if (static_cast<size_t>(i) >= LeafNode<>::leftSplitCount()) {
        destination_node = newLeafNode;
      } else {
        destination_node = oldLeafNode;
      }
      size_t index_within_node = static_cast<size_t>(i) % LeafNode<>::leftSplitCount();
      LeafNodeCell& destination = destination_node->m_cells[index_within_node];

      if (static_cast<size_t>(i) == m_cellNum) {
        destination.m_value = value;
      } else if (static_cast<size_t>(i) > m_cellNum) {
        destination = oldLeafNode->m_cells[static_cast<size_t>(i-1)];
      } else {
        destination = oldLeafNode->m_cells[static_cast<size_t>(i)];
      }
    }
    /* Update cell count on both leaf nodes */
    oldLeafNode->m_header.m_numCells = LeafNode<>::leftSplitCount();
    newLeafNode->m_header.m_numCells = LeafNode<>::rightSplitCount();
    if (oldLeafNode->m_header.m_isRoot) {
      m_table.createNewRoot(newLeafNode);
    } else {
      throw CursorException("Need to implement updating parent after split");
    }
  }

  Table& m_table;
  nodePtr m_node;
  size_t m_cellNum;
  bool m_endOfTable; // Indicates a position one past the last element
};

// TODO forward table?
// TODO make member functions
// TODO make leafNode generic inernal vs leaf node
Cursor table_start(Table& table) {
  uint32_t num_cells = std::get<LeafNode<>*>(table.m_root)->m_header.m_numCells;
  return Cursor{.m_table = table, .m_node = table.m_root, .m_cellNum = 0, .m_endOfTable = (num_cells == 0)};
}

Cursor table_end(Table& table) {
  uint32_t num_cells = std::get<LeafNode<>*>(table.m_root)->m_header.m_numCells;
  return Cursor{.m_table = table, .m_node = table.m_root, .m_cellNum = num_cells, .m_endOfTable = true};


}

Cursor leaf_node_find(Table& table, nodePtr node, uint32_t key) 
{
  auto* leafNode = std::get<LeafNode<>*>(node);
  uint32_t num_cells = leafNode->m_header.m_numCells;


  // Binary search
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index) {
    uint32_t index = (min_index + one_past_max_index) / 2;
    uint32_t key_at_index = leafNode->m_cells[index].m_key;
    if (key == key_at_index) {
      return Cursor{.m_table = table, .m_node = node, .m_cellNum = index, .m_endOfTable = (num_cells == 0)};
    }
    if (key < key_at_index) {
      one_past_max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  // TODO RVO is not possible because of if statement
  return Cursor{.m_table = table, .m_node = node, .m_cellNum = min_index, .m_endOfTable = (num_cells == 0)};
}

Cursor table_find(Table& table, uint32_t key) 
{
  return std::visit([&](auto&& arg) ->Cursor 
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, InternalNode<>*>)
            throw CursorException("Need to implement searching an internal node");
        else if constexpr (std::is_same_v<T, LeafNode<>*>)
            return leaf_node_find(table, table.m_root, key);
    }, table.m_root);
}




