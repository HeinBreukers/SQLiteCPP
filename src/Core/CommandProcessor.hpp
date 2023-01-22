#pragma once

#include <string>
#include <iostream>
#include <fmt/format.h>
#include <sstream>
#include <memory>
#include <span>

#include "VirtualMachine.hpp"


enum class PrepareResult
{ 
    SUCCESS, 
    SYNTAX_ERROR,
    UNRECOGNIZED_STATEMENT 
};

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