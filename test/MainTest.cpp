#include <gtest/gtest.h>
#include <filesystem>

#include "../src/Backend/Pager.hpp"
#include "../src/Core/CommandProcessor.hpp"
#include "../src/Core/VirtualMachine.hpp"
#include "../src/CLI/Command.hpp"


class DBTest : public ::testing::Test {
 protected:
  
  
  void SetUp() override 
  {
    // close filestream that created dbfile;
    newFile.close();
  }

  void TearDown() override 
  {
    // delete db file
    std::filesystem::remove(filename);
  }
  
  std::string filename = "tests.db";
  std::ofstream newFile{filename};
  MetaCommand metaCommand;
  Table table{filename};
  Statement statement;
};

TEST_F(DBTest, Basic) {

  //std::string input = ".exit";
  EXPECT_EQ(1,1);
}

TEST_F(DBTest, NormalExit) {

  std::string input = ".exit";
  EXPECT_EQ(metaCommand.exit, false);
  auto res = metaCommand.do_meta_command(input,table);
  EXPECT_EQ(metaCommand.exit, true);
  //EXPECT_EXIT(metaCommand.do_meta_command(input),testing::ExitedWithCode(0), "");
}

TEST_F(DBTest, PrepareCorrectInsert) {

  std::string input = "insert 1 2 3";
  auto res = prepare_statement(input, statement);
  EXPECT_EQ(res,PrepareResult::SUCCESS);
}

TEST_F(DBTest, PrepareIncorrectInsert) {

  std::string input = "insert 1 2";
  auto res = prepare_statement(input, statement);
  EXPECT_EQ(res,PrepareResult::SYNTAX_ERROR);
}

TEST_F(DBTest, PrepareUnrecognizedCommand) {

  std::string input = "nonsense";
  auto res = prepare_statement(input, statement);
  EXPECT_EQ(res,PrepareResult::UNRECOGNIZED_STATEMENT);
}

TEST_F(DBTest, PrepareSelect) {

  std::string input = "select";
  auto res = prepare_statement(input, statement);
  EXPECT_EQ(res,PrepareResult::SUCCESS);
}

TEST_F(DBTest, ExecuteInsert) {

  std::string input = "insert 1 2 3";
  prepare_statement(input, statement);
  auto res = execute_statement(statement, table).value();
  EXPECT_EQ(res,ExecuteResult::SUCCESS);
}

TEST_F(DBTest, ExecuteSelect) {
  
  std::string input = "insert 1 2 3";
  prepare_statement(input, statement);
  execute_statement(statement, table).value();

  input = "select";
  prepare_statement(input, statement);
  testing::internal::CaptureStdout();
  auto res = execute_statement(statement, table).value();
  std::string output = testing::internal::GetCapturedStdout();
  EXPECT_EQ(res,ExecuteResult::SUCCESS);
  EXPECT_EQ(output,"id: 1, age: 2, lastvar: 3\n");
}

TEST_F(DBTest, Persistance) {
  std::string input = "insert 1 2 3";
  prepare_statement(input, statement);
  execute_statement(statement, table).value();
  input = ".exit";
  metaCommand.do_meta_command(input,table);
  EXPECT_EQ(metaCommand.exit, true);
  // Table does not have explicit destructor anymore 
  // TODO make DB class that behaves like an actual executable
  //table.~Table();


  Table newTable{filename};
  input = "select";
  prepare_statement(input, statement);
  testing::internal::CaptureStdout();
  auto res = execute_statement(statement, newTable).value();
  std::string output = testing::internal::GetCapturedStdout();
  EXPECT_EQ(res,ExecuteResult::SUCCESS);
  EXPECT_EQ(output,"id: 1, age: 2, lastvar: 3\n");
}

//TODO Test Table Full
TEST_F(DBTest, TableFull) {

  EXPECT_EQ(1,1);
}

//TODO Test table implementation
//TODO test buffer overflows
