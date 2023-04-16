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
  m_rootPageNum(0)
  {
      // TODO move to pager constructor
      if (m_pager.m_numPages == 0) 
      {
        // New database file. Initialize page 0 as leaf node.
        // TODO make sure pager properly inits nodes
        m_root = m_pager.fromPage(m_pager.getPage(0).get());
        auto leafNode = std::get<LeafNode<>*>(m_root);
        //leafNode->init();
        leafNode->m_header.m_isRoot = true;
      }
  }

  // ~Table()
  // {


  // }

  // delete copy and move constructors because table flushes on destruction and pager has deleted copy constructor
  Table(const Table&) = delete;
  Table(Table&&) = delete;
  Table& operator=(const Table&) = delete;
  Table& operator=(Table&&) = delete;

  void createNewRoot(size_t rightChildPageNum) {
    /*
    Handle splitting the root.
    Old root copied to new page, becomes left child.
    Address of right child passed in.
    Re-initialize root page to contain the new root node.
    New root node points to two children.
    */

    // TODO create proper factory where node is returned instead of page
    auto* rootPage = m_pager.getPage(m_rootPageNum).get();
    m_root = m_pager.fromPage(rootPage);
    auto* rootNode = std::get<InternalNode<>*>(m_root);
    auto* rightChildPage = m_pager.getPage(rightChildPageNum).get();
    auto rightChild = m_pager.fromPage(rightChildPage);

    auto leftChildPageNum = m_pager.getUnusedPageNum();
    auto* leftChildPage = m_pager.getPage(leftChildPageNum).get();
    auto leftChild = m_pager.fromPage(leftChildPage);
    
    /* Left child has data copied from old root */
    memcpy(leftChildPage, rootPage, Page<>::size);
    std::visit([](auto&& arg){arg->m_header.m_isRoot = false;}, leftChild);
    
    /* Root node is a new internal node with one key and two children */
    //rootNode->init();
    rootNode->m_header.m_isRoot = true;
    rootNode->m_header.m_numKeys = 1;
    rootNode->m_cells[0].m_child = leftChild;
    uint32_t left_child_max_key =  std::visit([](auto&& arg)-> uint32_t {return arg->maxKey();}, leftChild);
    rootNode->m_cells[0].m_key = left_child_max_key;
    rootNode->m_header.m_rightChild = rightChild;
  }
//private:
  Pager<> m_pager;
  size_t m_rootPageNum;

  // BTree 
  nodePtr m_root;
  std::vector<std::unique_ptr<InternalNode<>>> m_internalNodes{};
  std::vector<std::unique_ptr<LeafNode<>>> m_leafNodes{};
  
};

