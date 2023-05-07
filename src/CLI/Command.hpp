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
    if (input_buffer ==  ".btree") 
    {
     fmt::print("Tree:\n");
     auto node = table.m_root;
     //std::span<nodePtr> nodePtrSpan{&table.m_pager.m_pages.data()->ptr, table.m_pager.m_pages.size()};
    //  std::vector<nodePtr> ordinals;
    //  std::transform(table.m_pager.m_pages.cbegin(), table.m_pager.m_pages.cend(), std::back_inserter(ordinals),
    //                [](NodePtr c) { return c.ptr; });
     node.print(&table.m_pager);
     //auto* leafNode = static_cast<LeafNode*>(node);
     //leafNode->print();
     return MetaCommandResult::SUCCESS;
    }
    return MetaCommandResult::UNRECOGNIZED_COMMAND;
  }

  bool exit = false;

};


