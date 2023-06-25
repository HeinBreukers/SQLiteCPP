#pragma once

#include <fmt/format.h>
#include <type_traits>

#include "BTreeForwardDeclares.hpp"
#include "BTreeBase.hpp"
#include "LeafNode.hpp"
#include "InternalNode.hpp"

// class RootNode
// {
// public:
//     RootNode() = default;

//     // RootNode(nodeVariant&& nPtr):
//     // ptr(std::move(nPtr))
//     // {
//     // }

//     explicit operator bool () const;
//     RootNode& copyFrom(const RootNode& src);
//     //void print(Pager* pager) const;
//     //[[nodiscard]] NodeType nodeType() const;
//     //[[nodiscard]] std::array<char,PAGE_SIZE>* toBytes();
//     //[[nodiscard]] uint32_t maxKey();

//     nodeVariant ptr;
    
// private:
//     static void copy(const nodeVariant& src, nodeVariant& dest);
//     //[[nodiscard]] bool IsNotNull() const;
// };