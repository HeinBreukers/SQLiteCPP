#pragma once
#include <cstddef>
#include <inttypes.h>
#include <array>
#include <string>
#include <variant>
#include <fmt/core.h>

#include "BTreeForwardDeclares.hpp"

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

class BTreeException : public DBException 
{
public:
  BTreeException(const std::string& msg) : DBException(fmt::format("<BTree>: \"{}\"", msg)) {}
  virtual ~BTreeException() noexcept = default;
};

enum class NodeType: uint8_t
{
    Internal,
    Leaf
};

#pragma pack(1)
class CommonNodeHeader
{
public:
    NodeType m_nodeType;
    bool m_isRoot;
    nodePtr m_parent;
};
#pragma pack()

// CRTP base class Page
template<typename Node>
class Page
{
public:



    static constexpr size_t pageSize = traits<Node>::pageSize;
    std::array<char,pageSize>* toBytes()
    {
        // TODO WARNING UB
        return reinterpret_cast<std::array<char,pageSize>*>(this);
    }
    NodeType nodeType()
    {
        return commonNodeHeader()->m_nodeType;
    }
    bool& isRoot()
    {
        return commonNodeHeader()->m_isRoot;
    }
    nodePtr& parent()
    {
        return commonNodeHeader()->m_parent;
    }

    static void indent(uint32_t level) 
    {
        for (uint32_t i = 0; i < level; i++) 
        {
            fmt::print("  ");
        }
    }

protected:
    // Page should only be inherited, and not be directly addressable
    Page() = default;
    Page(const Page&) = default;
    Page(Page&&) noexcept = default;
    Page& operator=(const Page&) = default;
    Page& operator=(Page&&) noexcept = default;
    ~Page() = default; 
private:
    CommonNodeHeader* commonNodeHeader()
    {
        // TODO WARNING UB
        return reinterpret_cast<CommonNodeHeader*>(this);
    }
};