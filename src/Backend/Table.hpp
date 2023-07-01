#pragma once

#include "Pager.hpp"
#include "BTree.hpp"

class TableException : public DBException 
{
public:
  TableException(const std::string& msg) : DBException(fmt::format("<Table>: \"{}\"", msg)) {}
  virtual ~TableException() noexcept = default;
};

// DataBase Table
class Table{
public:
  explicit Table(const std::string& filename):
  m_pager(filename)
  {
  }

  // delete copy and move constructors because table flushes on destruction and pager has deleted copy constructor
  Table(const Table&) = delete;
  Table(Table&&) = delete;
  Table& operator=(const Table&) = delete;
  Table& operator=(Table&&) = delete;
  ~Table() = default;
  
//private:
  BTree btree;
  Pager m_pager;
};

