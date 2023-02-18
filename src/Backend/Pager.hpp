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
// TODO make constructor private
template<size_t PageSize = PAGE_SIZE>
class alignas(PageSize) Page
{
public:
  static constexpr size_t size = PageSize;
  std::array<char,PageSize> m_memPool;
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
      
      throw PagerException(fmt::format("Unable to open file {}", filename));
    }

    m_fileStream.seekg(0, std::ios::end);
    m_fileLength = static_cast<size_t>(m_fileStream.tellg());
    m_fileStream.seekg(0, std::ios::beg);

    m_numPages = m_fileLength/PAGE_SIZE;

    if (m_fileLength % PAGE_SIZE != 0) {
      throw PagerException("Db file is not a whole number of pages. Corrupt file.");
    }

    // TODO look into unnecessary 
    for(auto& page: m_pages)
    {
      page = nullptr;
    }
  }
  
  std::unique_ptr<Page<>>& getPage(size_t pageNum) 
  {
    if (pageNum > MaxPages) 
    {
      //fmt::print("Tried to fetch page number out of bounds. {} > {}\n", pageNum,MaxPages);
      throw PagerException("Page Number out of bounds");
    }

    if (!m_pages[pageNum]) {
      // Cache miss. Allocate memory and load from file.
      auto page = std::make_unique<Page<>>();
      size_t num_pages = m_fileLength / PAGE_SIZE;

      // We might save a partial page at the end of the file
      if (m_fileLength % PAGE_SIZE) {
        num_pages += 1;
      }

      if (pageNum < num_pages) {
        m_fileStream.seekg(static_cast<std::streamoff>(pageNum * PAGE_SIZE), std::ios::beg);
        // TODO m_rows.size();
        
        // read page or till eof
        if(!m_fileStream.read(page->m_memPool.data(),PAGE_SIZE))
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
    if(!m_fileStream.write(m_pages[pageNum]->m_memPool.data(), static_cast<std::streamsize>(PAGE_SIZE)))
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
  std::array<std::unique_ptr<Page<>>,MaxPages> m_pages{};
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


