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

inline constexpr size_t ROW_SIZE = 12;
inline constexpr size_t PAGE_SIZE = 4096;
inline constexpr size_t TABLE_MAX_PAGES = 100;

class DBException : public std::exception 
{
public:
  DBException(const std::string& msg) : message(fmt::format("DB Exception {}",msg)) {}
  virtual ~DBException() noexcept = default;

  const char* what() const noexcept override
  {
    return message.c_str();
  }

private:
  std::string message;
};

class PagerException : public DBException 
{
public:
  PagerException(const std::string& msg) : DBException(fmt::format("<Pager>: \"{}\"", msg)) {}
  virtual ~PagerException() noexcept = default;
};

struct Row
{
  uint32_t id;
  uint32_t age;
  uint32_t lastvar;

  void print()
  {
    fmt::print("id: {}, age: {}, lastvar: {}\n",id,age,lastvar);
  }
};

// TODO make size correspond with pagetable size is os
template<size_t PageSize = PAGE_SIZE>
struct alignas(PageSize) Page
{
  
  //std::array<char,PageSize> m_rows;
};

template<std::size_t MaxPages = TABLE_MAX_PAGES>
struct Pager{
public: 

  Pager() = default;
  Pager(const Pager&) = delete;
  Pager(Pager&&) noexcept= default;
  Pager& operator=(const Pager&) = delete;
  Pager& operator=(Pager&&) noexcept= default;

  Pager(const std::string& filename):
  {
    m_fileStream.open(filename,std::ios::in | std::ios::out | std::ios::binary );
    
    if (!(m_fileStream.is_open()&&m_fileStream.good())) {
      
      throw PagerException(fmt::format("Unable to open file {}", filename));
    }

    m_fileStream.seekg(0, std::ios::end);
    m_fileLength = static_cast<size_t>(m_fileStream.tellg());
    m_fileStream.seekg(0, std::ios::beg);

    m_numPages = m_fileLength/PAGE_SIZE;

    if (file_length % PAGE_SIZE != 0) {
      throw PagerException("Db file is not a whole number of pages. Corrupt file.");
    }

    // TODO look into unnecessary 
    for(auto& page: m_pages)
    {
      page = nullptr;
    }
  }
  
  std::unique_ptr<Page>& getPage(size_t pageNum) 
  {
    if (pageNum > MaxPages) 
    {
      //fmt::print("Tried to fetch page number out of bounds. {} > {}\n", pageNum,MaxPages);
      throw PagerException("Page Number out of bounds");
    }

    if (!m_pages[pageNum]) {
      // Cache miss. Allocate memory and load from file.
      auto page = std::make_unique<Page>();
      size_t num_pages = m_fileLength / PAGE_SIZE;

      // We might save a partial page at the end of the file
      if (m_fileLength % PAGE_SIZE) {
        num_pages += 1;
      }

      if (pageNum < num_pages) {
        m_fileStream.seekg(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg);
        // TODO m_rows.size();
        
        // read page or till eof
        if(!m_fileStream.read(page->m_rows.data(),PAGE_SIZE))
        {
          if(m_fileStream.eof())
          {
            // if eof then clear error state
            m_fileStream.clear();
          }
          else
          {
            //fmt::print("Error reading file\n");
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

  void flush(size_t pageNum) {
    if (!m_pages[pageNum]) {
      //fmt::print("Tried to flush null page\n");
      throw PagerException("Tried to flush null page");
    }
    if(!m_fileStream.seekp(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg))
    {
      //fmt::print("Error seeking\n");
      throw PagerException("Error seeking file");
    }
    if(!m_fileStream.write(m_pages[pageNum]->m_rows.data(), static_cast<std::streamsize>(PAGE_SIZE)))
    {
      //fmt::print("Error writing\n");
      throw PagerException("Error writing file");
    }
  }

  std::fstream m_fileStream;
  size_t m_fileLength{0};
  size_t m_numPages;
  // if size gets too large start thinking about allocatin on heap
  // unique ptr because heap allocation is preffered for large sizes
  std::array<std::unique_ptr<Page>,MaxPages> m_pages{};
};

class Table{
public:
    explicit Table(const std::string& filename):
    m_pager(filename)
    //,num_rows(m_pager.m_fileLength / ROW_SIZE)
    {}

    ~Table()
    {

      for (size_t i = 0; i < m_pager.m_numPages; i++) {
        if (!m_pager.m_pages[i]) {
          continue;
        }
        m_pager.flush(i);
        m_pager.m_pages[i] = nullptr;
      }

      // filestream is RAII object so no need to close manually
      // m_pager.m_fileStream.close();
      // if(m_pager.m_fileStream.fail())
      // {
      //   //throw PagerException("Error closing db file");
      //   fmt::print("Error closing db file.\n");
      //   //TODO throw PagerException("Error closing db file");
      // }
    }

    // delete copy and move constructors because table flushes on destruction and pager has deleted copy constructor
    Table(const Table&) = delete;
    Table(Table&&) = delete;
//private:
    Pager<> m_pager;
    size_t m_rootPageNum;
    //Pager<>* pager;
};

std::vector<char> serialize_row(const Row& source) {
    std::vector<char> destination(3*sizeof(uint32_t));
    std::memcpy(&destination[0], &source, 3*sizeof(uint32_t)); 
    return destination;
}

Row deserialize_row(std::span<char> source) {
    Row destination;
    std::memcpy(&destination, &source[0], 3*sizeof(uint32_t));
    return destination;
}


class Cursor{
public:

  // TODO overload increment operators
  void advance() 
  {
    auto& node = table.m_pager.getPage(table.m_rootPageNum); 

    ++m_cellNum;
    if (m_cellNum >= (*leaf_node_num_cells(node)))
    {
      end_of_table = true;
    }
  }

  auto value() {
    auto& page = table.m_pager.getPage(m_pageNum);
    return leaf_node_value(page, m_cellNum);
    //return std::span{&page->m_rows[byte_offset],ROW_SIZE};
  }

  Table& table;
  size_t m_pageNum;
  size_t m_cellNum;
  bool end_of_table;  // Indicates a position one past the last element
};

// TODO forward table?
// TODO make member functions
Cursor table_start(Table& table) {
  auto& root_node = table.m_pager.getPage(table.m_rootPageNum); 
  uint32_t num_cells = *leaf_node_num_cells(root_node);
  return Cursor{.table = table, .m_pageNum = table.m_rootPageNum, .m_cellNum = 0, .end_of_table = (num_cells == 0)};
}

Cursor table_end(Table& table) {
  auto& root_node = table.m_pager.getPage(table.m_rootPageNum); 
  uint32_t num_cells = *leaf_node_num_cells(root_node);
  return Cursor{.table = table, .m_pageNum = table.m_rootPageNum, .m_cellNum = num_cells, .end_of_table = true};
}