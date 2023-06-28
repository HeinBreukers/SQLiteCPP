#pragma once
#include <inttypes.h>
#include <variant>
#include <memory>
#include <type_traits>


// TODO make size correspond with pagetable size is os
inline constexpr size_t PAGE_SIZE = 4096;
inline constexpr size_t MaxDepth = 3;

//Forward declares
template<typename KeyType, typename ValueType, size_t PageSize = PAGE_SIZE>
class LeafNode;
template<typename KeyType, typename ValueType, size_t Depth, size_t PageSize = PAGE_SIZE>
class InternalNode ;
template<typename T> class traits;

// template specialization to indicate max depth
template<typename KeyType,typename ValueType, size_t PageSize>
class InternalNode< KeyType,ValueType,MaxDepth,PageSize> : public std::false_type {};

// TODO Variadic Template depending on max depth
template<typename KeyType, typename ValueType, size_t PageSize>
using NodeVariant = std::variant
<
std::unique_ptr< LeafNode<     KeyType,ValueType,PageSize>>,
std::shared_ptr< InternalNode< KeyType,ValueType,0,PageSize>>,
std::shared_ptr< InternalNode< KeyType,ValueType,1,PageSize>>,
std::shared_ptr< InternalNode< KeyType,ValueType,2,PageSize>>
>;

struct Empty
{
    Empty() = default;
};

struct Pager;