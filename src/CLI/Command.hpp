#pragma once

#include <string>
#include <fmt/core.h>

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
    else if (input_buffer ==  ".btree") 
    {
     fmt::print("Tree:\n");
     auto* page = table.m_pager.getPage(0).get();
     auto node = table.m_pager.fromPage(page);
     std::visit([&](auto&& arg){arg->print();}, node);
     //auto* leafNode = static_cast<LeafNode*>(node);
     //leafNode->print();
     return MetaCommandResult::SUCCESS;
    }
    else {
      return MetaCommandResult::UNRECOGNIZED_COMMAND;
    }
  }

  bool exit = false;

};


