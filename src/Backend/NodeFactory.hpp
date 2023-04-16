#pragma once

#include "BTree.hpp"

// TODO migrate to "Table" once done
class NodeFactory
{
    template<NodeType nodeType, bool isRoot>
    decltype(auto) NewNode()
    {
        if constexpr(nodeType == NodeType::Internal)
        {
            InternalNode* ptr;
            return  ptr;
        }
        else if constexpr(nodeType == NodeType::Leaf)
        {
            LeafNode* ptr;
            return  ptr;
        }
    }

    nodePtr InitNodeFromFile(Page<>* page)
    {
        auto nodeType = static_cast<CommonNode*>(page)->Header()->nodeType;
        switch (nodeType)
        {
        case (NodeType::Internal):
            return static_cast<InternalNode*>(page);
            break;
        case (NodeType::Leaf):
            return static_cast<LeafNode*>(page);
            break;

        }
        throw BTreeException(fmt::format("Invalid Node Type"));
    }
};