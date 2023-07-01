#include "Cursor.hpp"

void Cursor::advance() 
{
  ++m_cellNum;
  if (m_cellNum >= std::get<LeafNode<>*>(m_node.ptr)->m_header.m_numCells)
  {
    m_endOfTable = true;
  }
}

// PreIncrement
Cursor& Cursor::operator++()
{
  advance();
  return *this;
} 	

void Cursor::insert(uint32_t key, const Row& value)
{
  auto* leafNode = std::get<LeafNode<>*>(m_node.ptr);

  const uint32_t num_cells = leafNode->m_header.m_numCells;
  if (num_cells >= LeafNode<>::maxCells) {
    // Node full
    leaf_node_split_and_insert(key, value);
    return;
  }

  if (m_cellNum < num_cells) {
    // Make room for new cell
    for (uint32_t i = num_cells; i > m_cellNum; i--) {
      leafNode->m_cells[i] = leafNode->m_cells[i-1];
    }
  }
  leafNode->m_header.m_numCells +=1;
  leafNode->m_cells[m_cellNum].m_key = key;
  leafNode->m_cells[m_cellNum].m_value = value;
}

void Cursor::leaf_node_split_and_insert(uint32_t key, const Row& value) {
  /*
  Create a new node and move half the cells over.
  Insert the new value in one of the two nodes.
  Update parent or create a new parent.
  */
  auto* oldLeafNode = std::get<LeafNode<>*>(m_node.ptr);
  auto [newLeafNodeIndex,newLeafNodePtr] = m_table.m_pager.getUnusedNode<NodeType::Leaf>();
  auto* newLeafNode = std::get<LeafNode<>*>(newLeafNodePtr.ptr);
  /*
  All existing keys plus new key should be divided
  evenly between old (left) and new (right) nodes.
  Starting from the right, move each key to correct position.
  */
  for (int32_t i = LeafNode<>::maxCells; i >= 0; i--) {
    LeafNode<>* destination_node = [&]()
    {
      if (static_cast<size_t>(i) >= LeafNode<>::leftSplitCount()) 
      {
        return newLeafNode;
      } 
      else 
      {
        return oldLeafNode;
      }
    }();
    
    size_t index_within_node = static_cast<size_t>(i) % LeafNode<>::leftSplitCount();
    LeafNodeCell& destination = destination_node->m_cells[index_within_node];

    if (static_cast<size_t>(i) == m_cellNum) 
    {
      destination.m_value = value;
      destination.m_key = key;
    } 
    else if (static_cast<size_t>(i) > m_cellNum) 
    {
      destination = oldLeafNode->m_cells[static_cast<size_t>(i-1)];
    } 
    else 
    {
      destination = oldLeafNode->m_cells[static_cast<size_t>(i)];
    }
  }
  /* Update cell count on both leaf nodes */
  oldLeafNode->m_header.m_numCells = LeafNode<>::leftSplitCount();
  newLeafNode->m_header.m_numCells = LeafNode<>::rightSplitCount();
  if (oldLeafNode->m_header.m_isRoot) {
    m_table.createNewRoot(newLeafNodeIndex);
  } else {
    throw CursorException("Need to implement updating parent after split");
  }
}


Cursor leaf_node_find(Table& table, NodePtr node, uint32_t key) 
{
  auto* leafNode = std::get<LeafNode<>*>(node.ptr);
  const uint32_t num_cells = leafNode->m_header.m_numCells;

  // Binary search
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index) {
    const uint32_t index = (min_index + one_past_max_index) / 2;
    const uint32_t key_at_index = leafNode->m_cells[index].m_key;
    if (key == key_at_index) {
      return Cursor{.m_table = table, .m_node = node, .m_cellNum = index, .m_endOfTable = (num_cells == 0)};
    }
    if (key < key_at_index) {
      one_past_max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  // TODO RVO is not possible because of if statement
  return Cursor{.m_table = table, .m_node = node, .m_cellNum = min_index, .m_endOfTable = (num_cells == 0)};
}

// TODO move out of header
Cursor internal_node_find(Table& table, NodePtr node, uint32_t key) 
{
  auto* internalNode = std::get<InternalNode<>*>(node.ptr);
  const uint32_t num_keys = internalNode->m_header.m_numKeys;

  /* Binary search to findIndex index of child to search */
  uint32_t min_index = 0;
  uint32_t max_index = num_keys; /* there is one more child than key */

  while (min_index != max_index) {
    const uint32_t index = (min_index + max_index) / 2;
    const uint32_t key_to_right = internalNode->m_cells[index].m_key;
    if (key_to_right >= key) {
      max_index = index;
    } else {
      min_index = index + 1;
    }
  }
  

  // TODO maybe move implementation
  NodePtr child = [&]()->NodePtr
  {
    if (min_index == num_keys) 
    {
      auto index = internalNode->m_header.m_rightChild;
      return table.m_pager.at(static_cast<size_t>(index));
    } 
    auto index = internalNode->m_cells[min_index].m_child;
    return table.m_pager.at(static_cast<size_t>(index));
  }();
  //internalNode->m_cells[min_index].m_child;
  
  
  return std::visit([&](auto&& arg) ->Cursor 
    {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, InternalNode<>*>)
          return internal_node_find(table, child, key);
      else if constexpr (std::is_same_v<T, LeafNode<>*>)
          return leaf_node_find(table, child, key);
    }, child.ptr);
}

// TODO move out of header
Cursor table_find(Table& table, uint32_t key) 
{
  //throw CursorException("Need to implement searching an internal node");
  return std::visit([&](auto&& arg) ->Cursor 
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, InternalNode<>*>)
        {
          return internal_node_find(table, table.m_root, key);
          //throw CursorException("Need to implement searching an internal node");
        }
        else if constexpr (std::is_same_v<T, LeafNode<>*>)
        {
            return leaf_node_find(table, table.m_root, key);
        }
    }, table.m_root.ptr);
}

// TODO forward table?
// TODO make member functions
// TODO make leafNode generic inernal vs leaf node
Cursor table_start(Table& table) {
  Cursor cursor =  table_find(table, 0);
  //cursor.m_endOfTable= table.m_pager.
  uint32_t num_cells = std::get<LeafNode<>*>(cursor.m_node.ptr)->m_header.m_numCells;
  cursor.m_endOfTable = (num_cells == 0);

  return cursor;
  // uint32_t num_cells = table.m_root->m_header.m_numKeys;
  // return Cursor{.m_table = table, .m_node = table.m_root, .m_cellNum = 0, .m_endOfTable = (num_cells == 0)};
}

