#pragma once 

#include <array>
#include <string>
#include <memory>
#include <fstream>
#include <fmt/format.h>
#include <optional>
#include <vector>
#include <span>

#include <system_error>
#include <type_traits>

#include "BTree.hpp"

class PagerException : public DBException 
{
public:
  PagerException(const std::string& msg) : DBException(fmt::format("<Pager>: \"{}\"", msg)) {}
  virtual ~PagerException() noexcept = default;
};


// The Pager reads and writes Pages to/from disk
//template<std::size_t MaxPages>
struct Pager{
public: 
  //static constexpr size_t MaxPages = 100;

  Pager() = default;
  Pager(const Pager&) = delete;
  Pager(Pager&&) noexcept= default;
  Pager& operator=(const Pager&) = delete;
  Pager& operator=(Pager&&) noexcept= default;

  explicit Pager(const std::string& filename);
  ~Pager() = default;

  template<typename T>
  void WritePage(const std::array<char,PageSize>& page);

  template<typename T>
  std::array<char,PageSize> ReadPage(std::size_t pageNum);

  std::fstream m_fileStream;
  size_t m_fileLength{0};
  size_t m_numPages;
  //std::vector<std::unique_ptr<InternalNode<>>> m_internalNodes{};
  //std::vector<std::unique_ptr<LeafNode<>>> m_leafNodes{};
  // TODO maybe vector instead of array?
  //std::array<RootNode,MaxPages> m_pages{};
  // NodePtr has to be non owning since it is a variant of raw pointers
  // therefore vectors of leaf and internal nodes are created to own the nodes (see above), and the pages array has references to the nodes
  // the pages array is needed to read and write pages to disk in a structured format
  
private:
  // TODO split into newNode, node from file
  [[nodiscard]] RootNode& getNodeFromFile(size_t pageNum);
  [[nodiscard]] NodeType getNodeType(size_t pageNum);
  //[[nodiscard]] size_t getUnusedPageNum();
  //void flush(size_t pageNum);
  //[[nodiscard]] RootNode make_node(NodeType nodeType);

  //template<NodeType nodeType>
  //[[nodiscard]] decltype(auto) make_node();
  
};


//  template<NodeType nodeType>
// [[nodiscard]] std::pair<intType,RootNode> Pager::getUnusedNode() 
// { 
//   // Creates a new node at the end of the nodePtr array
//   auto pageNum = m_numPages++;
//   m_pages[pageNum] = {make_node<nodeType>()};
//   return {pageNum,m_pages[pageNum]}; 
// }

// template<NodeType nodeType>
// [[nodiscard]] decltype(auto) Pager::make_node()
// {
//   if constexpr (nodeType == NodeType::Internal)
//   {
//     return m_internalNodes.emplace_back(std::make_unique<InternalNode<>>()).get();
//   }
//   else if constexpr (nodeType == NodeType::Leaf)
//   {
//     return m_leafNodes.emplace_back(std::make_unique<LeafNode<>>()).get();
//   }
//   throw BTreeException(fmt::format("Cannot create invalid Node Type"));
// }



// template<size_t PageSize>
// void InternalNode<PageSize>::print(Pager* pager, uint32_t indentation_level)
// {
//     const uint32_t num_keys = m_header.m_numKeys;
//     this->indent(indentation_level);
//     fmt::print("- internal (size {})\n", num_keys);
//     for (uint32_t i = 0; i < num_keys; i++) {
//         nodePtr childPtr = pager->at(static_cast<size_t>(m_cells[i].m_child)).ptr;
//         std::visit([&](auto&& arg){arg->print(pager,indentation_level+1);}, childPtr);
//         this->indent(indentation_level + 1);
//         fmt::print("- key {}\n", m_cells[i].m_key);
//     }
//     nodePtr rightchildPtr = pager->at(static_cast<size_t>(m_header.m_rightChild)).ptr;
//     std::visit([&](auto&& arg){arg->print(pager,indentation_level+1);}, rightchildPtr);
// }
