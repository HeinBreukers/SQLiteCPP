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
  m_pager(filename),
  m_root(m_pager.getRoot())
  {
      // TODO move to pager constructor
      if (m_pager.m_numPages == 0) 
      {
        // New database file. Initialize page 0 as leaf node.
        // TODO make sure pager properly inits nodes
        //m_root = m_pager.fromPage(m_pager.getPage(0).get());
        m_root = m_pager.getRoot();
        //auto leafNode = std::get<LeafNode<>*>(m_root.ptr);
        //leafNode->init();
        m_root.isRoot()=true;
      }
  }

  // delete copy and move constructors because table flushes on destruction and pager has deleted copy constructor
  Table(const Table&) = delete;
  Table(Table&&) = delete;
  Table& operator=(const Table&) = delete;
  Table& operator=(Table&&) = delete;

  void createNewRoot(const nodePtr& rightChild) 
  {
    /*
    Handle splitting the root.
    Old root copied to new page, becomes left child.
    Address of right child passed in.
    Re-initialize root page to contain the new root node.
    New root node points to two children.
    */

    // Handle splitting the root.
    // TODO isnt always internalNode -> force root node to always be internal node
    auto oldRoot = m_root;
    oldRoot.isRoot()=false;
    
    // Old root copied to new page, becomes left child.
    // Create new internal node
    auto* newRoot = m_pager.getUnusedNode<NodeType::Internal>();
    // move new root to start of pager 
    m_pager.m_pages[0] = newRoot;
    // move old root to end of pager 
    m_pager.m_pages[1] = oldRoot;

    /* Root node is a new internal node with one key and two children */
    newRoot->m_header.m_isRoot = true;
    newRoot->m_header.m_numKeys = 1;
    newRoot->m_cells[0].m_child = oldRoot.ptr;
    uint32_t left_child_max_key = newRoot->maxKey();
    newRoot->m_cells[0].m_key = left_child_max_key;
    newRoot->m_header.m_rightChild = rightChild;
    m_root=newRoot;
  }
  
//private:
  Pager<> m_pager;
  NodePtr m_root;
};

