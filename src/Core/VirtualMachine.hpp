#pragma once

#include <vector>
#include <cinttypes>
#include <span>
#include <memory>
#include <cstring>
#include <fmt/format.h>

#include "../Backend/Pager.hpp"
#include "../Backend/Cursor.hpp"


enum class StatementType
{ 
    INSERT, 
    SELECT 
};

enum class ExecuteResult
{ 
    SUCCESS, 
    TABLE_FULL,
    DUPLICATE_KEY
};



struct Statement
{
    StatementType type;
    Row row_to_insert;
};



ExecuteResult execute_insert(const Statement& statement, Table& table) {
  auto* node = table.m_pager.getPage(table.m_rootPageNum).get();
  auto* leafNode = static_cast<LeafNode*>(node);

  auto numCells = leafNode->Header()->numCells;
  if ( numCells >= leafNode->maxCells()) 
  {  
    return ExecuteResult::TABLE_FULL;
  }

  auto& row_to_insert = statement.row_to_insert;

  uint32_t key_to_insert = row_to_insert.id;
  auto cursor = table_find(table, key_to_insert);
  if (cursor.m_cellNum < numCells) {
    uint32_t key_at_index = leafNode->Cell(cursor.m_cellNum)->m_key;
    if (key_at_index == key_to_insert) {
      return ExecuteResult::DUPLICATE_KEY;
    }
  }
  cursor.insert(row_to_insert.id,row_to_insert);

  return ExecuteResult::SUCCESS;
}

ExecuteResult execute_select(Table& table) {
  auto cursor = table_start(table);
  
  Row row;

  while (!(cursor.end_of_table)) 
  {
    row = cursor.value();
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