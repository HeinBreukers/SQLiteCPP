#pragma once
#include <inttypes.h>
#include <variant>
#include <memory>

inline constexpr size_t ROW_SIZE = 12;
// TODO make size correspond with pagetable size is os
inline constexpr size_t PAGE_SIZE = 4096;
inline constexpr size_t TABLE_MAX_PAGES = 100;

//Forward declares
template<typename KeyType, typename ValueType, size_t PageSize = PAGE_SIZE>
class LeafNode;
template<typename KeyType, typename ValueType, size_t Depth, size_t PageSize = PAGE_SIZE>
class InternalNode;
template<typename T> class traits;





using intType = std::conditional<sizeof(void*)==sizeof(int64_t),int64_t,int32_t>::type;

template<typename KeyType, typename ValueType>
using NodeVariant = std::variant<std::unique_ptr<LeafNode<KeyType, ValueType>>,std::shared_ptr<InternalNode<KeyType,ValueType,0>>>;//,InternalNode<1>,InternalNode<2>,InternalNode<3>>;

struct Empty
{
    Empty() = default;
};

struct Pager;