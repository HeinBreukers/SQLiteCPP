#pragma once

#include <vector>
#include <cinttypes>
#include <span>
#include <memory>
#include <cstring>
#include <fmt/format.h>

#include "../Backend/Pager.hpp"



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



struct Statement
{
    StatementType type;
    Row row_to_insert;
};



ExecuteResult execute_insert(const Statement& statement, Table& table) {
  if (table.num_rows >= TABLE_MAX_ROWS) {
    return ExecuteResult::TABLE_FULL;
  }

  auto& row_to_insert = statement.row_to_insert;
  auto cursor = table_end(table);
  auto slot = cursor.value();
  memcpy(slot.data(), serialize_row(row_to_insert).data(),slot.size());
  ++ table.num_rows;

  return ExecuteResult::SUCCESS;
}

ExecuteResult execute_select(Table& table) {
  auto cursor = table_start(table);
  Row row;

  while (!(cursor.end_of_table)) 
  {
    row = deserialize_row(cursor.value());
    row.print();
    cursor.advance();
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