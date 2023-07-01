#pragma once

#include <array>
#include <string>

// Option 1

// Calculates location of BTree Node on DiskFile
// Nodes are ordered according to DFS

// This means the following BTree:

//                   Root
//                  /    \
//             Inter1    Inter2
//             /  \        /  \
//         Leaf1 Leaf2  Leaf3 Leaf 4

// Will have the following pageNum ordering:

// Root,Inter1,Leaf1,Leaf2,Inter2,Leaf3,Leaf4
// 0   ,1     ,2    ,3    ,4     ,5    ,6

// The pageNums will be mapped to a PageLocation on the DiskFile according to a Page Table

//      Root,Inter1,Leaf1,Leaf2,Inter2,Leaf3,Leaf4
//      0   ,1     ,2    ,3    ,4     ,5    ,6
// Meta,6   ,2     ,0    ,1    ,5     ,4    ,3

// The Meta Page(s) at the beginning of the Diskfile Information will include informatoin about the DiskFile, 
// Including the page mapping table

// Using this structure the location of a Node on the Diskfile can be determined by examining the BTree shape
// A restructuring of the BTree does not result in a restructuring of the Diskfile,
// Pages can still be added to the end of the DiskFile, no expensive diskFile restructuring has to be done
// but instead the pageTable will be reordered

// a total of TOTAL_DATA_SIZE/PAGE_SIZE extra data is needed for storing the pageTable

// Option 2


// Use memory mapped file
// Make a Page Allocator for the BTree, where the pointers inside the BTree point directly to page numbers
// use offset pointers such as boost::interprocess::offset_ptr
// use allocator_traits
// use pointer_traits


class Serializer
{


};