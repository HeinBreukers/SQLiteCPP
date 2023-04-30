#pragma once

#include <fmt/format.h>
#include <type_traits>

#include "BTreeForwardDeclares.hpp"
#include "BTreeBase.hpp"
#include "BTree.hpp"

class NodePtr
{
public:
    NodePtr() = default;

    explicit NodePtr(const nodePtr& nPtr):
    ptr(nPtr)
    {}

    // TODO make concept isNodeType
    template<typename Node> requires (std::is_same_v<std::decay_t<Node>,LeafNode<>*> || std::is_same_v<std::decay_t<Node>,InternalNode<>*>)
    NodePtr(Node&& nPtr):
    ptr(std::forward<Node>(nPtr))
    {}

    template<typename Node> requires (std::is_same_v<std::decay_t<Node>,LeafNode<>*> || std::is_same_v<std::decay_t<Node>,InternalNode<>*>)
    NodePtr& operator=(Node&& nPtr)
    {
        ptr = std::forward<Node>(nPtr);
        return *this;
    }

    explicit operator bool () const 
    { 
        return IsNotNull();
    }  

    NodePtr& copyFrom(const NodePtr& src)
    {
        NodePtr::copy(src.ptr,ptr);
        return *this;
    }

    void print() const
    {
        std::visit([](auto&& t_ptr)->void
        {
           t_ptr->print();
        },ptr);
    };

    [[nodiscard]] NodeType nodeType() const
    {
        return std::visit([](auto&& t_ptr)
        {
            return t_ptr->m_header.m_nodeType;
        },ptr);
    }

    [[nodiscard]] std::array<char,PAGE_SIZE>* toBytes()
    {
        return std::visit([](auto&& t_ptr)
        {
            return t_ptr->toBytes();
        },ptr);
    }

    [[nodiscard]] bool& isRoot()
    {
        return std::visit([](auto&& t_ptr) ->bool&
        {
            return t_ptr->m_header.m_isRoot;
        },ptr);
    }

    [[nodiscard]] NodePtr parent() const
    {
        return NodePtr{std::visit([](auto&& t_ptr)
        {
            return t_ptr->m_header.m_parent;
        },ptr)};
    }

    nodePtr ptr;
private:
    static void copy(const nodePtr& src, nodePtr& dest)
    {
        std::visit([](auto&& t_src, auto&& t_dest)
        {
            if constexpr (std::is_same_v<decltype(t_src),decltype(t_dest)>)
            {
                *t_dest=*t_src;
            }   
            else
            {
                throw BTreeException(fmt::format("Tried to copy different types"));
            }
        }, src, dest);
    }

    [[nodiscard]] bool IsNotNull() const noexcept
    {
        return std::visit([](auto&& t_ptr)->bool
        {
            if(t_ptr)
            {
                return true;
            }
            return false;
        },ptr);
    }
};