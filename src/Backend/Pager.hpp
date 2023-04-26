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

#include "BTree.hpp"


class PagerException : public DBException 
{
public:
  PagerException(const std::string& msg) : DBException(fmt::format("<Pager>: \"{}\"", msg)) {}
  virtual ~PagerException() noexcept = default;
};


// The Pager reads and writes Pages to/from disk
template<std::size_t MaxPages = TABLE_MAX_PAGES>
struct Pager{
public: 

  Pager() = default;
  Pager(const Pager&) = delete;
  Pager(Pager&&) noexcept= default;
  Pager& operator=(const Pager&) = delete;
  Pager& operator=(Pager&&) noexcept= default;

  Pager(const std::string& filename)
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

    if (m_fileLength % PAGE_SIZE != 0) {
      throw PagerException("Db file is not a whole number of pages. Corrupt file.");
    }

    // TODO look into unnecessary 
    // for(auto& page: m_pages)
    // {
    //   page = nullptr;
    // }
  }
  
  ~Pager()
  {
      for (size_t i = 0; i < m_numPages; i++) {
        //if (!m_pages[i]) {
        if(!IsNotNull(m_pages[i]))
        {
          continue;
        }

        // TODO RAII
        flush(i);
        //m_pages[i] = nullptr;
      }
  }

  // TODO
  [[nodiscard]] nodePtr getRoot() 
  {
    return getNode<NodeType::Leaf>(0);
  }

  //  TODO should make_unique new node
  // Creates a new node at the end of the nodePtr array
  template<NodeType nodeType>
  [[nodiscard]] decltype(auto) getUnusedNode() 
  { 
    auto idx = getUnusedPageNum();
    if constexpr (nodeType == NodeType::Internal)
    {
      auto newInternal = m_internalNodes.emplace_back(std::make_unique<InternalNode<>>()).get();
      m_pages[idx]= newInternal;
      return newInternal;
    }
    else if constexpr (nodeType == NodeType::Leaf)
    {
      auto newLeaf = m_leafNodes.emplace_back(std::make_unique<LeafNode<>>()).get();
      m_pages[idx]= newLeaf;
      return newLeaf;
    }

    throw BTreeException(fmt::format("Invalid Node Type"));
  }

  nodePtr copyToNewNode(const nodePtr& node)
  {
    //auto* page = getUnusedPage().get();

    return std::visit( [&](auto&& src)->nodePtr
    { 
      NodeType nodeType = src->m_header.m_nodeType;
      if (nodeType == NodeType::Internal)
      {
        InternalNode<>* ptr = getUnusedNode<NodeType::Internal>();
        memcpy(ptr,src,PAGE_SIZE);
        // TODO figure out why it doesnt work
        //*ptr=*src;
        return ptr;
      }
      else if (nodeType == NodeType::Leaf)
      {
        LeafNode<>* ptr = getUnusedNode<NodeType::Leaf>();
        memcpy(ptr,src,PAGE_SIZE);
        //*ptr=*src;
        return ptr;
      }
      throw BTreeException(fmt::format("Invalid Node Type"));
    }, node);
  }

  std::fstream m_fileStream;
  size_t m_fileLength{0};
  size_t m_numPages;
  std::vector<std::unique_ptr<InternalNode<>>> m_internalNodes{};
  std::vector<std::unique_ptr<LeafNode<>>> m_leafNodes{};
  // TODO maybe vecor instead of array?
  // nodePtr has to be non owning since it is a variant of raw ptrs
  std::array<nodePtr,MaxPages> m_pages{};
  
private:
  // [[nodiscard]] std::unique_ptr<Page<>>& getPage(size_t pageNum) 
  // {
  //   if (pageNum > MaxPages) 
  //   {
  //     //fmt::print("Tried to fetch page number out of bounds. {} > {}\n", pageNum,MaxPages);
  //     throw PagerException("Page Number out of bounds");
  //   }

  //   if (!m_pages[pageNum]) {
  //     // Cache miss. Allocate memory and load from file.
  //     // TODO construct inplace in array, then use ref to edit page
  //     auto page = std::make_unique<Page<>>();
  //     size_t num_pages = m_fileLength / PAGE_SIZE;

  //     // We might save a partial page at the end of the file
  //     if (m_fileLength % PAGE_SIZE) {
  //       num_pages += 1;
  //     }

  //     if (pageNum < num_pages) {
  //       m_fileStream.seekg(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg);
        
  //       // read page or till eof
  //       if(!m_fileStream.read(page->toBytes()->data(),PAGE_SIZE))
  //       {
  //         if(m_fileStream.eof())
  //         {
  //           // if eof then clear error state
  //           m_fileStream.clear();
  //         }
  //         else
  //         {
  //           throw PagerException("Error reading file");
  //         }
  //       }
        
  //     }

  //     m_pages[pageNum] = std::move(page);

  //     if (pageNum >= m_numPages) 
  //     {
  //       m_numPages = pageNum + 1;
  //     }
  //   }

  //   return m_pages[pageNum];
  // }

  template<NodeType nodeType>
  [[nodiscard]] decltype(auto) getNode(size_t pageNum) 
  {
    if (pageNum > MaxPages) 
    {
      //fmt::print("Tried to fetch page number out of bounds. {} > {}\n", pageNum,MaxPages);
      throw PagerException("Page Number out of bounds");
    }

    if (!IsNotNull(m_pages[pageNum])) 
    {
      // Cache miss. Allocate memory and load from file.
      // TODO construct inplace in array, then use ref to edit page
      nodePtr page;
      if constexpr (nodeType == NodeType::Internal)
      {
        page = m_internalNodes.emplace_back(std::make_unique<InternalNode<>>()).get();

      }
      else if constexpr (nodeType == NodeType::Leaf)
      {
        page = m_leafNodes.emplace_back(std::make_unique<LeafNode<>>()).get();
      }
      else
      {
        throw BTreeException(fmt::format("Invalid Node Type"));
      }
      //auto page = std::make_unique<Page<>>();
      size_t num_pages = m_fileLength / PAGE_SIZE;

      // We might save a partial page at the end of the file
      if (m_fileLength % PAGE_SIZE) {
        num_pages += 1;
      }

      if (pageNum < num_pages) {
        m_fileStream.seekg(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg);
        
        // read page or till eof
      
        //if(!m_fileStream.read(page->toBytes()->data(),PAGE_SIZE))
        auto bytes = std::visit([&](auto&& arg){return arg->toBytes()->data();},page);
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

      m_pages[pageNum] = std::move(page);

      if (pageNum >= m_numPages) 
      {
        m_numPages = pageNum + 1;
      }
    }

    return m_pages[pageNum];
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
    if (!IsNotNull(m_pages[pageNum])) {
      throw PagerException("Tried to flush null page");
    }
    if(!m_fileStream.seekp(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg))
    {
      throw PagerException("Error seeking file");
    }
    if(!m_fileStream.write(std::visit([&](auto&& arg){return arg->toBytes()->data();},m_pages[pageNum]), static_cast<std::streamsize>(PAGE_SIZE)))
    {
      throw PagerException("Error writing file");
    }
  }

  // [[nodiscard]] nodePtr fromPage(Page<>* page)
  // {
  //     auto nodeType = *reinterpret_cast<NodeType*>(page);
  //     //auto nodeType = static_cast<CommonNode*>(page)->Header()->m_nodeType;
  //     switch (nodeType)
  //     {
  //     case (NodeType::Internal):
  //         return reinterpret_cast<InternalNode<>*>(page);
  //         break;
  //     case (NodeType::Leaf):
  //         return reinterpret_cast<LeafNode<>*>(page);
  //         break;

  //     }
  //     throw BTreeException(fmt::format("Invalid Node Type"));
  // }

  // [[nodiscard]] std::unique_ptr<Page<>>& getUnusedPage() 
  // { 
  //   return getPage(getUnusedPageNum()); 
  // }
  bool IsNotNull(const nodePtr& ptr)
  {
      return std::visit([&](auto&& arg)->bool
      {
        if(arg)
        {
          return true;
        }
        return false;
      },ptr);
  }
};




