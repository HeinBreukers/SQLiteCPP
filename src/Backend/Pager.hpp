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
inline constexpr size_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
inline constexpr size_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

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

struct Page
{
  // make size correspond with pagetable size is os
  std::array<char,PAGE_SIZE> m_rows;
};

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
      //fmt::print("Unable to open file\n");
      throw PagerException("Unable to open file");
    }

    m_fileStream.seekg(0, std::ios::end);
    m_fileLength = static_cast<size_t>(m_fileStream.tellg());
    m_fileStream.seekg(0, std::ios::beg);

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
    }

    return m_pages[pageNum];
  }

  void flush(size_t pageNum, size_t size) {
  if (!m_pages[pageNum]) {
    //fmt::print("Tried to flush null page\n");
    throw PagerException("Tried to flush null page");
  }
  if(!m_fileStream.seekp(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg))
  {
    //fmt::print("Error seeking\n");
    throw PagerException("Error seeking file");
  }
  if(!m_fileStream.write(m_pages[pageNum]->m_rows.data(), static_cast<std::streamsize>(size)))
  {
    //fmt::print("Error writing\n");
    throw PagerException("Error writing file");
  }
}

  std::fstream m_fileStream;
  size_t m_fileLength{0};
  // if size gets too large start thinking about allocatin on heap
  // unique ptr because heap allocation is preffered for large sizes
  std::array<std::unique_ptr<Page>,MaxPages> m_pages{};
};

class Table{
public:
    explicit Table(const std::string& filename):
    m_pager(filename),
    num_rows(m_pager.m_fileLength / ROW_SIZE)
    {}

    ~Table()
    {
      size_t num_full_pages = num_rows / ROWS_PER_PAGE;

      for (size_t i = 0; i < num_full_pages; i++) {
        if (!m_pager.m_pages[i]) {
          continue;
        }
        m_pager.flush(i,PAGE_SIZE);
        m_pager.m_pages[i] = nullptr;
      }

      // There may be a partial page to write to the end of the file
      // This should not be needed after we switch to a B-tree
      size_t num_additional_rows = num_rows % ROWS_PER_PAGE;
      if (num_additional_rows > 0) {
        size_t page_num = num_full_pages;
        if (m_pager.m_pages[page_num]) {
          m_pager.flush(page_num, num_additional_rows * ROW_SIZE);
          m_pager.m_pages[page_num] = nullptr;
        }
      }

      m_pager.m_fileStream.close();
      if(m_pager.m_fileStream.fail())
      {
        fmt::print("Error closing db file.\n");
        //TODO throw PagerException("Error closing db file");
      }
    }

    // delete copy and move constructors because table flushes on destruction and pager has deleted copy constructor
    Table(const Table&) = delete;
    Table(Table&&) = delete;
//private:
    Pager<> m_pager;
    size_t num_rows;
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
    row_num += 1;
    if (row_num >= table.num_rows) {
      end_of_table = true;
    }
  }

  std::span<char> value() {
    size_t page_num = row_num / ROWS_PER_PAGE;
    auto& page = table.m_pager.getPage(page_num);
    size_t row_offset = row_num % ROWS_PER_PAGE;
    size_t byte_offset = row_offset * ROW_SIZE;
    return std::span{&page->m_rows[byte_offset],ROW_SIZE};
  }

  Table& table;
  size_t row_num;
  bool end_of_table;  // Indicates a position one past the last element
} ;

// TODO forward table?
Cursor table_start(Table& table) {
  return Cursor{table,0,(table.num_rows == 0)};
}

Cursor table_end(Table& table) {
  return Cursor{table,table.num_rows,true};
}