#include "Table.hpp"
#include "InternalNode.hpp"

void Table::createNewRoot(intType rightChildIndex) 
{
/*
Handle splitting the root.
Old root copied to new page, becomes left child.
Address of right child passed in.
Re-initialize root page to contain the new root node.
New root node points to two children.
*/

// Handle splitting the root.
auto oldRoot = m_root;
oldRoot.isRoot()=false;

// Old root copied to new page, becomes left child.
// Create new internal node
auto [newIndex,newRootPtr] = m_pager.getUnusedNode<NodeType::Internal>();
auto* newRoot = std::get<InternalNode<>*>(newRootPtr.ptr);
// move new root to start of pager 
m_pager.at(0) = newRootPtr;
// move old root to end of pager 
m_pager.at(m_pager.m_numPages-1) = oldRoot;

/* Root node is a new internal node with one key and two children */
newRoot->m_header.m_isRoot = true;
newRoot->m_header.m_numKeys = 1;
newRoot->m_cells[0].m_child = newIndex;
const uint32_t left_child_max_key = oldRoot.maxKey();
newRoot->m_cells[0].m_key = left_child_max_key;
newRoot->m_header.m_rightChild = rightChildIndex;
m_root=newRootPtr;
// TODO
//oldRoot.parent()=m_root;
}