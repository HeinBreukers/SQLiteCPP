#pragma once

#include <string>
#include <memory>

#include "BTreeForwardDeclares.hpp"

template<typename KeyType, typename ValueType, std::size_t PageSize>
class NodeAllocator
{
public:
    using LeafType = LeafNode<KeyType,ValueType,PageSize>;
    using LeafTypePtr = std::unique_ptr<LeafType>;

    template<std::size_t Depth>
    using InternalType = InternalNode<KeyType,ValueType,Depth,PageSize>;
    template<std::size_t Depth>
    using InternalTypePtr = std::shared_ptr<InternalNode<Depth>>;

    static [[nodiscard]] LeafTypePtr makeNewLeafPtr()
    {
        return std::make_unique<LeafType>();
    }
    
    template<std::size_t Depth>
    static [[nodiscard]] InternalTypePtr makeNewInternalPtr()
    {
        return std::make_shared<InternalType<Depth>>();
    }
};