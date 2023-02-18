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

class Cursor{
public:

  // TODO overload increment operators
  // TODO iterators
  void advance() 
  {
    auto page = table.m_pager.getPage(table.m_rootPageNum).get(); 

    ++m_cellNum;
    if (m_cellNum >= static_cast<LeafNode*>(page)->Header()->numCells)
    {
      end_of_table = true;
    }
  }

  auto value() {
    auto page = table.m_pager.getPage(m_pageNum).get();
    return static_cast<LeafNode*>(page)->Cell(m_cellNum)->m_value;
  }

  void insert(uint32_t key, const Row& value)
  {
    auto node = table.m_pager.getPage(m_pageNum).get();
    auto* leafNode = static_cast<LeafNode*>(node);

    uint32_t num_cells = leafNode->Header()->numCells;
    if (num_cells >= leafNode->maxCells()) {
      // Node full
      throw CursorException("Need to implement splitting a leaf node.");
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

  Table& table;
  size_t m_pageNum;
  size_t m_cellNum;
  bool end_of_table;  // Indicates a position one past the last element
};

// TODO forward table?
// TODO make member functions
// TODO make leafNode generic inernal vs leaf node
Cursor table_start(Table& table) {
  auto root_node = table.m_pager.getPage(table.m_rootPageNum).get(); 
  uint32_t num_cells = static_cast<LeafNode*>(root_node)->Header()->numCells;
  return Cursor{.table = table, .m_pageNum = table.m_rootPageNum, .m_cellNum = 0, .end_of_table = (num_cells == 0)};
}

Cursor table_end(Table& table) {
  auto root_node = table.m_pager.getPage(table.m_rootPageNum).get(); 
  uint32_t num_cells = static_cast<LeafNode*>(root_node)->Header()->numCells;
  return Cursor{.table = table, .m_pageNum = table.m_rootPageNum, .m_cellNum = num_cells, .end_of_table = true};


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
      return Cursor{.table = table, .m_pageNum = page_num, .m_cellNum = index, .end_of_table = (num_cells == 0)};;
    }
    if (key < key_at_index) {
      one_past_max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  return Cursor{.table = table, .m_pageNum = page_num, .m_cellNum = min_index, .end_of_table = (num_cells == 0)};
}

Cursor table_find(Table& table, uint32_t key) 
{
  auto* root_node = table.m_pager.getPage(table.m_rootPageNum).get(); 
  auto* commonNode = static_cast<CommonNode*>(root_node);

  // TODO dynamic casts
  if (commonNode->CommonHeader()->nodeType== NodeType::Leaf) {
    return leaf_node_find(table, table.m_rootPageNum, key);
  } else {
    throw CursorException("Need to implement searching an internal node");
  }
}


