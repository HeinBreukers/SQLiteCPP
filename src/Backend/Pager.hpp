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
#include "NodePtr.hpp"


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
  static constexpr size_t MaxPages = 100;

  Pager() = default;
  Pager(const Pager&) = delete;
  Pager(Pager&&) noexcept= default;
  Pager& operator=(const Pager&) = delete;
  Pager& operator=(Pager&&) noexcept= default;

  explicit Pager(const std::string& filename)
  {
    m_fileStream.open(filename,std::ios::in | std::ios::out | std::ios::binary );
    
    if (!(m_fileStream.is_open()&&m_fileStream.good())) {
      
      throw PagerException(fmt::format("Unable to open file {}", filename));
    }

    // get file length
    m_fileStream.seekg(0, std::ios::end);
    m_fileLength = static_cast<size_t>(m_fileStream.tellg());
    m_fileStream.seekg(0, std::ios::beg);

    m_numPages = m_fileLength/PAGE_SIZE;
    
    if(!m_numPages)
    {
      // new file
      m_pages[0] = getUnusedNode<NodeType::Leaf>().second;
      m_pages[0].isRoot()=true;
    }

    if (m_fileLength % PAGE_SIZE != 0) {
      throw PagerException("Db file is not a whole number of pages. Corrupt file.");
    }
  }
  
  ~Pager()
  {
    for (size_t i = 0; i < m_numPages; i++) {
      if(!m_pages[i])
      {
        continue;
      }

      flush(i);
    }
  }

  [[nodiscard]] NodePtr getRoot() 
  {
    return at(0);
  }

  // Creates a new node at the end of the nodePtr array
  template<NodeType nodeType>
  [[nodiscard]] std::pair<intType,NodePtr> getUnusedNode() 
  { 
    auto pageNum = m_numPages++;
    m_pages[pageNum] = {make_node<nodeType>()};
    return {pageNum,m_pages[pageNum]}; 
  }

  [[nodiscard]] NodePtr& at(size_t index)
  {
    if(!m_pages[index])
    {
      return getNodeFromFile(index);
    }
    return m_pages[index];
  }

  std::fstream m_fileStream;
  size_t m_fileLength{0};
  size_t m_numPages;
  std::vector<std::unique_ptr<InternalNode<>>> m_internalNodes{};
  std::vector<std::unique_ptr<LeafNode<>>> m_leafNodes{};
  // TODO maybe vector instead of array?
  // NodePtr has to be non owning since it is a variant of raw pointers
  // therefore vectors of leaf and internal nodes are created to own the nodes (see above), and the pages array has references to the nodes
  // the pages array is needed to read and write pages to disk in a structured format
  std::array<NodePtr,MaxPages> m_pages{};
  
private:
  // TODO split into newNode, node from file
  //template<NodeType nodeType>
  [[nodiscard]] NodePtr& getNodeFromFile(size_t pageNum) 
  {
    if (pageNum > MaxPages) 
    {
      throw PagerException("Page Number out of bounds");
    }

    if (!m_pages[pageNum]) 
    {
      size_t num_pages = m_fileLength / PAGE_SIZE;

      // We might save a partial page at the end of the file
      if (m_fileLength % PAGE_SIZE) {
        num_pages += 1;
      }
      
      if (pageNum < num_pages) 
      {
        // Cache miss. Allocate memory and load from file.
        auto nodeType = getNodeType(pageNum);
        m_pages[pageNum] = make_node(nodeType);
        NodePtr& page = m_pages[pageNum];

        m_fileStream.seekg(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg);
        
        // read page or till eof
        auto* bytes = page.toBytes()->data();
        if(!m_fileStream.read(bytes,PAGE_SIZE))
        {
          if(m_fileStream.eof())
          {
            // if eof then clear error state
            m_fileStream.clear();
          }
          else
          {
            throw PagerException("Error reading file");
          }
        }
        
      }

      //m_pages[pageNum] = std::move(page);

      if (pageNum >= m_numPages) 
      {
        m_numPages = pageNum + 1;
      }
    }

    return m_pages[pageNum];
  }

  NodeType getNodeType(size_t pageNum)
  {
    m_fileStream.seekg(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg);
    
    // read page or till eof
    std::array<char, sizeof(NodeType)> nodeTypeBytes;
    if(!m_fileStream.read(nodeTypeBytes.data(),sizeof(NodeType)))
    {
      if(m_fileStream.eof())
      {
        // if eof then clear error state
        m_fileStream.clear();
      }
      else
      {
        throw PagerException("Error reading file");
      }
    }
    return *reinterpret_cast<NodeType*>(nodeTypeBytes.data());
  }
  /*
  Until we start recycling free pages, new pages will always
  go onto the end of the database file
  */
  [[nodiscard]] size_t getUnusedPageNum() 
  { 
    return m_numPages; 
  }

  void flush(size_t pageNum) {
    if (!m_pages[pageNum]) {
      throw PagerException("Tried to flush null page");
    }
    if(!m_fileStream.seekp(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg))
    {
      throw PagerException("Error seeking file");
    }
    if(!m_fileStream.write(m_pages[pageNum].toBytes()->data(), static_cast<std::streamsize>(PAGE_SIZE)))
    {
      throw PagerException("Error writing file");
    }
  }

  template<NodeType nodeType>
  [[nodiscard]] decltype(auto) make_node()
  {
    if constexpr (nodeType == NodeType::Internal)
    {
      return m_internalNodes.emplace_back(std::make_unique<InternalNode<>>()).get();
    }
    else if constexpr (nodeType == NodeType::Leaf)
    {
      return m_leafNodes.emplace_back(std::make_unique<LeafNode<>>()).get();
    }
    throw BTreeException(fmt::format("Cannot create invalid Node Type"));
  }

  [[nodiscard]] NodePtr make_node(NodeType nodeType)
  {
    if (nodeType == NodeType::Internal)
    {
      return {m_internalNodes.emplace_back(std::make_unique<InternalNode<>>()).get()};
    }
    else if (nodeType == NodeType::Leaf)
    {
      return {m_leafNodes.emplace_back(std::make_unique<LeafNode<>>()).get()};
    }
    throw BTreeException(fmt::format("Cannot create invalid Node Type"));
  }
};


template<size_t PageSize>
void InternalNode<PageSize>::print(Pager* pager, uint32_t indentation_level)
{
    const uint32_t num_keys = m_header.m_numKeys;
    this->indent(indentation_level);
    fmt::print("- internal (size {})\n", num_keys);
    for (uint32_t i = 0; i < num_keys; i++) {
        nodePtr childPtr = pager->at(static_cast<size_t>(m_cells[i].m_child)).ptr;
        std::visit([&](auto&& arg){arg->print(pager,indentation_level+1);}, childPtr);
        this->indent(indentation_level + 1);
        fmt::print("- key {}\n", m_cells[i].m_key);
    }
    nodePtr rightchildPtr = pager->at(static_cast<size_t>(m_header.m_rightChild)).ptr;
    std::visit([&](auto&& arg){arg->print(pager,indentation_level+1);}, rightchildPtr);
}

void NodePtr::print(Pager* pager) const
{
    std::visit([&pager](auto&& t_ptr)->void
    {
        t_ptr->print(pager);
    },ptr);
};