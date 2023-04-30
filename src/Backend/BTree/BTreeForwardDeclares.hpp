#pragma once
#include <inttypes.h>
#include <variant>

inline constexpr size_t ROW_SIZE = 12;
// TODO make size correspond with pagetable size is os
inline constexpr size_t PAGE_SIZE = 4096;
inline constexpr size_t TABLE_MAX_PAGES = 100;

//Forward declares
template<size_t PageSize = PAGE_SIZE>
class LeafNode;
template<size_t PageSize = PAGE_SIZE>
class InternalNode;
template<typename T> class traits;

template<size_t PageSize>
class traits<LeafNode<PageSize>>
{
public:
    static constexpr size_t pageSize = PageSize;
};


template<size_t PageSize>
class traits<InternalNode<PageSize>>
{
public:
    static constexpr size_t pageSize = PageSize;
};

using nodePtr = std::variant<LeafNode<>*,InternalNode<>*>;

struct Empty
{
    Empty() = default;
};