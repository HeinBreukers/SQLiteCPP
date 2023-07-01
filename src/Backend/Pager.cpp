#include "Pager.hpp"
#include "LeafNode.hpp"

Pager::Pager(const std::string& filename)
{
m_fileStream.open(filename,std::ios::in | std::ios::out | std::ios::binary );

if (!(m_fileStream.is_open()&&m_fileStream.good())) {
    
    throw PagerException(fmt::format("Unable to open file {}", filename));
}

// get file length
m_fileStream.seekg(0, std::ios::end);
m_fileLength = static_cast<size_t>(m_fileStream.tellg());
m_fileStream.seekg(0, std::ios::beg);

// m_numPages = m_fileLength/PAGE_SIZE;

// if(!m_numPages)
// {
//     // new file
//     m_pages[0] = getUnusedNode<NodeType::Leaf>().second;
//     m_pages[0].isRoot()=true;
// }

if (m_fileLength % PAGE_SIZE != 0) {
    throw PagerException("Db file is not a whole number of pages. Corrupt file.");
}
}
  
  Pager::~Pager()
  {
    for (size_t i = 0; i < m_numPages; i++) {
      if(!m_pages[i])
      {
        continue;
      }

      flush(i);
    }
  }

  // [[nodiscard]] RootNode Pager::getRoot() 
  // {
  //   return at(0);
  // }

  // [[nodiscard]] RootNode& Pager::at(size_t index)
  // {
  //   if(!m_pages[index])
  //   {
  //     return getNodeFromFile(index);
  //   }
  //   return m_pages[index];
  // }


[[nodiscard]] RootNode& Pager::getNodeFromFile(size_t pageNum) 
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
        RootNode& page = m_pages[pageNum];

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

  NodeType Pager::getNodeType(size_t pageNum)
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
  
  [[nodiscard]] size_t Pager::getUnusedPageNum() 
  { 
    /*
  Until we start recycling free pages, new pages will always
  go onto the end of the database file
  */
    return m_numPages; 
  }

  void Pager::flush(size_t pageNum) {
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

  [[nodiscard]] RootNode Pager::make_node(NodeType nodeType)
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