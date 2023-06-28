#include <gtest/gtest.h>


#include "BTree.hpp"


class BTreeTest : public ::testing::Test {
 protected:
  
  
  void SetUp() override 
  {
    // close filestream that created dbfile;
  }

  void TearDown() override 
  {
    // delete db file

  }

};

TEST_F(BTreeTest, Insert) 
{
    BTree<uint32_t,int> btree;
    auto& val = btree.emplace(2,4);
    EXPECT_EQ(val, 4);
}

TEST_F(BTreeTest, At) 
{
    BTree<uint32_t,int> btree;
    btree.emplace(1,3);
    btree.emplace(2,4);
    btree.emplace(3,5);
    btree.emplace(80,90);
    auto val = btree.at(1);
    EXPECT_EQ(val, 3);
    val = btree.at(2);
    EXPECT_EQ(val, 4);
    val = btree.at(3);
    EXPECT_EQ(val, 5);
    val = btree.at(80);
    EXPECT_EQ(val, 90);
}

TEST_F(BTreeTest, Size) 
{
    BTree<uint32_t,int> btree;
    btree.emplace(1,4);
    btree.emplace(2,4);
    btree.emplace(3,4);
    btree.emplace(4,4);
    EXPECT_EQ(btree.size(), 4);
}

TEST_F(BTreeTest, IndexOutOfRange) 
{
    BTree<uint32_t,int> btree;
    EXPECT_THROW( auto val = btree.at(1);, std::out_of_range );
}

TEST_F(BTreeTest, DuplicateKey) 
{
    BTree<uint32_t,int> btree;
    btree.emplace(3,4);
    EXPECT_THROW(btree.emplace(3,5);, std::out_of_range );
}

TEST_F(BTreeTest, SplitLeafNewRoot) 
{
    BTree<uint32_t,std::array<int,1000>> btree;
    btree.emplace(2,{1,2,3});
    auto& val = btree.emplace(4,{1,2,3});
    std::array<int,1000> ret={1,2,3};
    auto& rootnode = std::get<std::shared_ptr<InternalNode<uint32_t,std::array<int,1000>,0>>>(btree.m_root);

    EXPECT_EQ(val, ret);
    EXPECT_EQ(btree.size(), 2);
    EXPECT_EQ(rootnode->m_size, 2);
    EXPECT_EQ(rootnode->keys[0], 4);
}

TEST_F(BTreeTest, SplitLeafUpdateParent) 
{
    BTree<uint32_t,std::array<int,500>> btree;
    btree.emplace(2,{1,2,3});
    btree.emplace(4,{1,2,3});
    auto& val = btree.emplace(5,{1,2,3});
    std::array<int,500> ret={1,2,3};
    auto& rootnode = std::get<std::shared_ptr<InternalNode<uint32_t,std::array<int,500>,0>>>(btree.m_root);

    EXPECT_EQ(val, ret);
    EXPECT_EQ(btree.size(), 3);
    EXPECT_EQ(rootnode->m_size, 2);
    EXPECT_EQ(rootnode->keys[0], 5);
}

// test updating leftMostChild
TEST_F(BTreeTest, InsertSmallerKey) 
{
    BTree<uint32_t,std::array<int,500>> btree;
    btree.emplace(8,{1,2,3});
    btree.emplace(10,{1,2,3});
    auto& val = btree.emplace(1,{1,2,3});
    std::array<int,500> ret={1,2,3};
    // check BTree internals for internal node with 
    auto& rootnode = std::get<std::shared_ptr<InternalNode<uint32_t,std::array<int,500>,0>>>(btree.m_root);

    EXPECT_EQ(val, ret);
    EXPECT_EQ(btree.size(), 3);
    EXPECT_EQ(rootnode->m_size, 2);
    EXPECT_EQ(rootnode->keys[0], 10);
}

TEST_F(BTreeTest, SplitInternalNewRoot) 
{
    BTree<int,std::array<int,1000>> btree;
    int i;
    for(i = 1; i< InternalNode<int,std::array<int,1000>,0>::maxValues+100; ++i)
    {
        btree.emplace(i,{1,2,3});
    }
    auto& val = btree.at(i-1);
    std::array<int,1000> ret={1,2,3};

    auto& rootnode = std::get<std::shared_ptr<InternalNode<int,std::array<int,1000>,1>>>(btree.m_root);

    EXPECT_EQ(val, ret);
    EXPECT_EQ(btree.size(), (InternalNode<int,std::array<int,1000>,0>::maxValues+99));
    EXPECT_EQ(rootnode->m_size, 2);
    EXPECT_EQ(rootnode->keys[0], ((InternalNode<int,std::array<int,1000>,0>::maxValues+1)/2+1));
}

TEST_F(BTreeTest, SplitInternalNewRootDecreasing) 
{
    const size_t overshoot = 100;
    BTree<int,std::array<int,1000>> btree;
    int i;
    for(i = InternalNode<int,std::array<int,1000>,0>::maxValues+overshoot; i> 0; --i)
    {
        btree.emplace(i,{1,2,3});
    }
    auto& val = btree.at(InternalNode<int,std::array<int,1000>,0>::maxValues);
    std::array<int,1000> ret={1,2,3};

    auto& rootnode = std::get<std::shared_ptr<InternalNode<int,std::array<int,1000>,1>>>(btree.m_root);

    EXPECT_EQ(val, ret);
    EXPECT_EQ(btree.size(), (InternalNode<int,std::array<int,1000>,0>::maxValues+overshoot));
    EXPECT_EQ(rootnode->m_size, 2);
    EXPECT_EQ(rootnode->keys[0], ((InternalNode<int,std::array<int,1000>,0>::maxValues+1)/2+overshoot));
}

TEST_F(BTreeTest, SplitInternalUpdateParent) 
{
    const size_t pagesize = 128;
    BTree<int,long long, pagesize> btree;
    for(int i = 0; i<256; ++i)
    {
        btree.emplace(i,i);
    }
    auto& val = btree.at(128);

    auto& rootnode = std::get<std::shared_ptr<InternalNode<int,long long,2, pagesize>>>(btree.m_root);

    EXPECT_EQ(val, 128);
    EXPECT_EQ(btree.size(), 256);
    EXPECT_EQ(rootnode->m_size, 4);
    EXPECT_EQ(rootnode->keys[0], 60);
}

// TEST_F(BTreeTest, NewInternalRoot) 
// {
//     BTree<int,std::array<int,1000>> btree;
//     int i;
//     for(i = 1; i< InternalNode<int,std::array<int,1000>,0>::maxValues+100; ++i)
//     {
//         btree.emplace(i,{1,2,3});
//     }

//     auto& val = btree.at(i-1);
//     std::array<int,1000> ret={1,2,3};
//     // rootnode = internal node
//     // values are correct
//     // size is correct
//     EXPECT_EQ(val, ret);
// }

// Test if default constructed value of key dus not give duplicate key error
TEST_F(BTreeTest, DefaultKey) 
{
    BTree<int,int> btree;
    auto val = btree.emplace(0,0);
    EXPECT_EQ(val, 0);
}