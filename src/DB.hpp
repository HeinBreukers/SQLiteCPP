#pragma once

#include <string>
#include <iostream>
#include <fmt/format.h>
#include <sstream>
#include <vector>
#include <array>
#include <memory>
#include <span>


inline constexpr uint32_t ROW_SIZE = 12;
inline constexpr uint32_t PAGE_SIZE = 4096;
inline constexpr uint32_t TABLE_MAX_PAGES = 100;
inline constexpr uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
inline constexpr uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;


enum class MetaCommandResult
{
  SUCCESS,
  UNRECOGNIZED_COMMAND
};

enum class PrepareResult
{ 
    SUCCESS, 
    SYNTAX_ERROR,
    UNRECOGNIZED_STATEMENT 
};

enum class StatementType
{ 
    INSERT, 
    SELECT 
};

enum class ExecuteResult
{ 
    SUCCESS, 
    TABLE_FULL 
};

struct Row
{
    uint32_t id;
    uint32_t age;
    uint32_t lastvar;

    void print()
    {
      fmt::print("id: {}, age: {}, lastvar: {}\n",id,age,lastvar);
    }
};

struct Page
{
    // make size correspond with pagetable size is os
    std::array<uint8_t,PAGE_SIZE> m_rows;
};

struct Table{
    uint32_t num_rows = 0;
    // if size gets too large start thinking about allocatin on heap
    std::array<std::unique_ptr<Page>,TABLE_MAX_PAGES> m_pages{};
};

struct Statement
{
    StatementType type;
    Row row_to_insert;
};

void print_prompt() 
{ 
    fmt::print("db > "); 
}

MetaCommandResult do_meta_command(const std::string& input_buffer) {
  if (input_buffer == ".exit") {
    exit(EXIT_SUCCESS);
  } 
  else {
    return MetaCommandResult::UNRECOGNIZED_COMMAND;
  }
}

PrepareResult prepare_statement(const std::string& input_buffer, Statement& statement) 
{
  if (input_buffer.starts_with("insert")) {
    statement.type = StatementType::INSERT;
    std::stringstream s;
    s.str(input_buffer.substr(7));
    auto& row = statement.row_to_insert;
    s >>row.id >> row.age >> row.lastvar;
    if (s.fail()) {
      return PrepareResult::SYNTAX_ERROR;
    }
    return PrepareResult::SUCCESS;
  }
  if (input_buffer ==  "select") {
    statement.type = StatementType::SELECT;
    return PrepareResult::SUCCESS;
  }

  return PrepareResult::UNRECOGNIZED_STATEMENT;
}

std::vector<uint8_t> serialize_row(const Row& source) {
    std::vector<uint8_t> destination(3*sizeof(uint32_t));
    std::memcpy(&destination[0], &source, 3*sizeof(uint32_t)); 
    return destination;
}

Row deserialize_row(std::span<uint8_t> source) {
    Row destination;
    std::memcpy(&destination, &source[0], 3*sizeof(uint32_t));
    return destination;
}

std::span<uint8_t> row_slot(Table& table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    auto& page = table.m_pages[page_num];   
    if (!page) {
        // Allocate memory only when we try to access page
        page = std::make_unique<Page>();
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return std::span{&page->m_rows[byte_offset],ROW_SIZE};
}

ExecuteResult execute_insert(const Statement& statement, Table& table) {
  if (table.num_rows >= TABLE_MAX_ROWS) {
    return ExecuteResult::TABLE_FULL;
  }

  auto& row_to_insert = statement.row_to_insert;
  auto slot = row_slot(table, table.num_rows);
  memcpy(slot.data(), serialize_row(row_to_insert).data(),slot.size());
  ++ table.num_rows;

  return ExecuteResult::SUCCESS;
}

ExecuteResult execute_select(/*const Statement& statement,*/ Table& table) {
  Row row;
  for (uint32_t i = 0; i < table.num_rows; i++) {
    row = deserialize_row(row_slot(table, i));
    row.print();
  }
  return ExecuteResult::SUCCESS;
}

std::optional<ExecuteResult> execute_statement(const Statement& statement, Table& table) {
  switch (statement.type) 
  {
    case (StatementType::INSERT):
      return execute_insert(statement, table);
    case (StatementType::SELECT):
      return execute_select(table);
  }
  return std::nullopt;
}