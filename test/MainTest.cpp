#include <gtest/gtest.h>
#include "../src/DB.hpp"

class DBTest : public ::testing::Test {
 protected:
  void SetUp() override 
  {

  }

  void TearDown() override 
  {

  }
  
  Table table;
  Statement statement;
};

TEST_F(DBTest, NormalExit) {

  std::string input = ".exit";
  EXPECT_EXIT(do_meta_command(input),testing::ExitedWithCode(0), "");
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

//TODO Test table implementation
//TODO test buffer overflows
//TODO Test Table Full