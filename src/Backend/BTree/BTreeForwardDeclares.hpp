#pragma once
#include <inttypes.h>
#include <variant>
#include <memory>
#include <type_traits>


// TODO make size correspond with pagetable size is os
inline constexpr size_t PAGE_SIZE = 4096;
inline constexpr size_t MaxDepth = 3;

//Forward declares
template<typename KeyType, typename ValueType, size_t PageSize= PAGE_SIZE, typename Allocator = std::allocator<ValueType>>
class LeafNode;
template<typename KeyType, typename ValueType, size_t Depth, size_t PageSize= PAGE_SIZE, typename Allocator = std::allocator<ValueType>>
class InternalNode ;
template<typename T> class traits;

// template specialization to indicate max depth
template<typename KeyType,typename ValueType, size_t PageSize, typename Allocator>
class InternalNode< KeyType,ValueType,MaxDepth,PageSize,Allocator> : public std::false_type {};

// TODO Variadic Template depending on max depth
template<typename KeyType, typename ValueType, size_t PageSize, typename Allocator>
using NodeVariant = std::variant
<
LeafNode<     KeyType,ValueType,PageSize,Allocator>*,
InternalNode< KeyType,ValueType,0,PageSize,Allocator>*,
InternalNode< KeyType,ValueType,1,PageSize,Allocator>*,
InternalNode< KeyType,ValueType,2,PageSize,Allocator>*
>;

struct Empty
{
    Empty() = default;
};

struct Pager;