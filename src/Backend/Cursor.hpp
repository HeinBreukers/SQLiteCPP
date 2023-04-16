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

// Cursor iterates through BTree nodes
class Cursor{
public:

  // TODO overload increment operators
  // TODO iterators
  void advance() 
  {
    auto page = m_table.m_pager.getPage(m_table.m_rootPageNum).get(); 

    ++m_cellNum;
    if (m_cellNum >= static_cast<LeafNode<>*>(page)->Header()->numCells)
    {
      m_endOfTable = true;
    }
  }

  auto value() {
    auto page = m_table.m_pager.getPage(m_pageNum).get();
    return static_cast<LeafNode<>*>(page)->Cell(m_cellNum)->m_value;
  }

  void insert(uint32_t key, const Row& value)
  {
    auto node = m_table.m_pager.getPage(m_pageNum).get();
    auto* leafNode = static_cast<LeafNode*>(node);

    uint32_t num_cells = leafNode->Header()->numCells;
    if (num_cells >= LeafNode::maxCells()) {
      // Node full
      leaf_node_split_and_insert(key, value);
      return;
    }

    if (m_cellNum < num_cells) {
      // Make room for new cell
      for (uint32_t i = num_cells; i > m_cellNum; i--) {
        memcpy(leafNode->Cell(i) ,leafNode->Cell(i-1) ,
              sizeof(LeafNodeCell));
      }
    }
    leafNode->Header()->numCells +=1;
    leafNode->Cell(m_cellNum)->m_key = key;
    leafNode->Cell(m_cellNum)->m_value = value;
    //serialize_row(value, leafNode->Cell(m_cellNum)->m_value);
  }

  void leaf_node_split_and_insert([[maybe_unused]] uint32_t key, const Row& value) {
    /*
    Create a new node and move half the cells over.
    Insert the new value in one of the two nodes.
    Update parent or create a new parent.
    */
    auto* oldNode = m_table.m_pager.getPage(m_pageNum).get();
    auto* oldLeafNode = static_cast<LeafNode<>*>(oldNode);
    size_t newPageNum = m_table.m_pager.getUnusedPageNum();
    auto* newNode = m_table.m_pager.getPage(newPageNum).get();
    auto* newLeafNode = static_cast<LeafNode<>*>(newNode);
    newLeafNode->init();
    /*
    All existing keys plus new key should be divided
    evenly between old (left) and new (right) nodes.
    Starting from the right, move each key to correct position.
    */
    for (int32_t i = LeafNode::maxCells(); i >= 0; i--) {
      LeafNode* destination_node;
      if (static_cast<size_t>(i) >= LeafNode::leftSplitCount()) {
        destination_node = newLeafNode;
      } else {
        destination_node = oldLeafNode;
      }
      size_t index_within_node = static_cast<size_t>(i) % LeafNode::leftSplitCount();
      LeafNodeCell* destination = destination_node->Cell(index_within_node);

      if (static_cast<size_t>(i) == m_cellNum) {
        destination->m_value = value;
      } else if (static_cast<size_t>(i) > m_cellNum) {
        memcpy(destination, oldLeafNode->Cell(static_cast<size_t>(i)-1), sizeof(LeafNodeCell));
      } else {
        memcpy(destination, oldLeafNode->Cell(static_cast<size_t>(i)), sizeof(LeafNodeCell));
      }
    }
    /* Update cell count on both leaf nodes */
    oldLeafNode->Header()->numCells = LeafNode::leftSplitCount();
    newLeafNode->Header()->numCells = LeafNode::rightSplitCount();
    if (oldLeafNode->Header()->isRoot) {
      m_table.createNewRoot(newPageNum);
    } else {
      throw CursorException("Need to implement updating parent after split");
    }
  }

  Table& m_table;
  size_t m_pageNum;
  size_t m_cellNum;
  bool m_endOfTable;  // Indicates a position one past the last element
};

// TODO forward table?
// TODO make member functions
// TODO make leafNode generic inernal vs leaf node
Cursor table_start(Table& table) {
  auto root_node = table.m_pager.getPage(table.m_rootPageNum).get(); 
  uint32_t num_cells = static_cast<LeafNode*>(root_node)->Header()->numCells;
  return Cursor{.m_table = table, .m_pageNum = table.m_rootPageNum, .m_cellNum = 0, .m_endOfTable = (num_cells == 0)};
}

Cursor table_end(Table& table) {
  auto root_node = table.m_pager.getPage(table.m_rootPageNum).get(); 
  uint32_t num_cells = static_cast<LeafNode*>(root_node)->Header()->numCells;
  return Cursor{.m_table = table, .m_pageNum = table.m_rootPageNum, .m_cellNum = num_cells, .m_endOfTable = true};


}

Cursor leaf_node_find(Table& table, size_t page_num, uint32_t key) 
{
  auto* node = table.m_pager.getPage(page_num).get(); 
  auto* leafNode = static_cast<LeafNode*>(node);
  uint32_t num_cells = leafNode->Header()->numCells;


  // Binary search
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index) {
    uint32_t index = (min_index + one_past_max_index) / 2;
    uint32_t key_at_index = leafNode->Cell(index)->m_key;
    if (key == key_at_index) {
      return Cursor{.m_table = table, .m_pageNum = page_num, .m_cellNum = index, .m_endOfTable = (num_cells == 0)};
    }
    if (key < key_at_index) {
      one_past_max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  // TODO RVO is not possible because of if statement
  return Cursor{.m_table = table, .m_pageNum = page_num, .m_cellNum = min_index, .m_endOfTable = (num_cells == 0)};
}

Cursor table_find(Table& table, uint32_t key) 
{
  auto* root_node = table.m_pager.getPage(table.m_rootPageNum).get(); 
  auto node = fromPage(root_node);

  return std::visit([&](auto&& arg) ->Cursor 
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, InternalNode*>)
            throw CursorException("Need to implement searching an internal node");
        else if constexpr (std::is_same_v<T, LeafNode*>)
            return leaf_node_find(table, table.m_rootPageNum, key);
    }, node);
}




