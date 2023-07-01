#pragma once

#include "Pager.hpp"
#include "Table.hpp"
#include "NodePtr.hpp"
#include "LeafNode.hpp"

class CursorException : public DBException 
{
public:
  CursorException(const std::string& msg) : DBException(fmt::format("<Cursor>: \"{}\"", msg)) {}
  virtual ~CursorException() noexcept = default;
};

// Cursor iterates over cells of BTree node
class Cursor{
public:

  // TODO iterators
  void advance();
  Cursor& operator++();

  auto value()
  {
    return std::get<LeafNode<>*>(m_node.ptr)->m_cells[m_cellNum].m_value;
  }
  void insert(uint32_t key, const Row& value);
  void leaf_node_split_and_insert(uint32_t key, const Row& value);

  // TODO ref data member to smart ptr(or something else)
  Table& m_table;
  RootNode m_node;
  size_t m_cellNum;
  bool m_endOfTable; // Indicates a position one past the last element
};


// TODO make table member functions
Cursor leaf_node_find(Table& table, RootNode node, uint32_t key);

Cursor internal_node_find(Table& table, RootNode node, uint32_t key);

Cursor table_find(Table& table, uint32_t key);

// TODO forward table?
// TODO make member functions
// TODO make leafNode generic inernal vs leaf node
Cursor table_start(Table& table);



