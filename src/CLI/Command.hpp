#pragma once

#include <string>
#include <fmt/core.h>

#include "Table.hpp"

enum class MetaCommandResult
{
  SUCCESS,
  UNRECOGNIZED_COMMAND
};

class MetaCommand
{
public:
  MetaCommandResult do_meta_command(const std::string& input_buffer, Table& table) 
  {
    if (input_buffer == ".exit") {
      exit = true;
      //exit(EXIT_SUCCESS);
      return MetaCommandResult::SUCCESS;
    } 
    if (input_buffer ==  ".btree") 
    {
     fmt::print("Tree:\n");
     auto node = table.m_root;
     node.print(&table.m_pager);
     return MetaCommandResult::SUCCESS;
    }
    return MetaCommandResult::UNRECOGNIZED_COMMAND;
  }

  bool exit = false;

};


