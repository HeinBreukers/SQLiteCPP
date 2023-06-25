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
    // rootnode = internal node
    // values are correct
    // size is correct
    EXPECT_EQ(val, ret);
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
    EXPECT_EQ(val, ret);
}