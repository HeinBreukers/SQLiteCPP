#pragma once

#include "Pager.hpp"
#include "BTree.hpp"

class TableException : public DBException 
{
public:
  TableException(const std::string& msg) : DBException(fmt::format("<Table>: \"{}\"", msg)) {}
  virtual ~TableException() noexcept = default;
};

class Table{
public:
    explicit Table(const std::string& filename):
    m_pager(filename),
    m_rootPageNum(0)
    {
        if (m_pager.m_numPages == 0) 
        {
          // New database file. Initialize page 0 as leaf node.
          auto root_node = m_pager.getPage(0).get();
          static_cast<LeafNode*>(root_node)->init();
        }
    }

    ~Table()
    {

      for (size_t i = 0; i < m_pager.m_numPages; i++) {
        if (!m_pager.m_pages[i]) {
          continue;
        }

        // TODO RAII
        m_pager.flush(i);
        m_pager.m_pages[i] = nullptr;
      }
    }

    // delete copy and move constructors because table flushes on destruction and pager has deleted copy constructor
    Table(const Table&) = delete;
    Table(Table&&) = delete;

    
//private:
    Pager<> m_pager;
    size_t m_rootPageNum;
    //Pager<>* pager;
};

