// #include "CommandProcessor.hpp"
// #include "VirtualMachine.hpp"
// #include "Pager.hpp"
// #include "Command.hpp"
#include "BTree.hpp"

#include <iostream>

void print_prompt() 
{ 
    fmt::print("db > "); 
}

int main(/*int argc, char* argv[]*/) 
{
  // std::string input_buffer;
  // const std::string dbFile = "file.db";
  // Table table(dbFile);
  // MetaCommand metaCommand;
  // while (!metaCommand.exit) {
  //   print_prompt();
  //   std::getline(std::cin, input_buffer);

  //   if (input_buffer.front() == '.') {
  //     switch (metaCommand.do_meta_command(input_buffer,table)) {
  //       case (MetaCommandResult::SUCCESS):
  //         continue;
  //       case (MetaCommandResult::UNRECOGNIZED_COMMAND):
  //         fmt::print("Unrecognized command '{}'\n", input_buffer);
  //         continue;
  //     }
  //   } 

  //   Statement statement{};
  //   switch (prepare_statement(input_buffer, statement)) {
  //     case (PrepareResult::SUCCESS):
  //       break;
  //     case (PrepareResult::SYNTAX_ERROR):
  //       fmt::print("Syntax error. Could not parse statement.\n");
  //       continue;
  //     case (PrepareResult::UNRECOGNIZED_STATEMENT):
  //       fmt::print("Unrecognized keyword at start of '{}'.\n", input_buffer);
  //       continue;
  //   }

  //   switch (execute_statement(statement, table)) {
  //     case (ExecuteResult::SUCCESS):
  //       fmt::print("Executed.\n");
  //       break;
  //     case (ExecuteResult::TABLE_FULL):
  //       fmt::print("Error: Table full.\n");
  //       break;
  //     case (ExecuteResult::DUPLICATE_KEY):
  //       fmt::print("Error: Duplicate Key.\n");
  //       break;
  //   }
   
  // }

  BTree<uint32_t,int> btree;

  btree.emplace(1,1);
  //auto& val = btree.emplace(2,4);
  
  
  btree.emplace(3,12345);
  btree.emplace(4,1);
  //fmt::print("{}\n",btree.at(3));
  //TODO make key immutable
  //val = 5;
  btree.print();

  BTree<uint32_t,std::array<int,250>> btreeBig;
  btreeBig.emplace(8,{1,2,3});
  btreeBig.emplace(7,{1,2,3});
  btreeBig.emplace(6,{1,2,3});
  btreeBig.emplace(2,{1,2,3});
  btreeBig.emplace(3,{1,2,3});
  btreeBig.emplace(5,{1,2,3});
  btreeBig.emplace(1,{1,2,3});
  btreeBig.emplace(4,{1,2,3});


  //std::array<int,500> ret={1,2,3};
  
  btreeBig.print();
  // TODO at internal node
  //fmt::print("{}\n", btreeBig.at(3));


  BTree<int,std::array<int,1000>> btreeD;
  int i;
  for(i = InternalNode<int,std::array<int,1000>,0>::maxValues+100; i> 0; --i)
  {
      btreeD.emplace(i,{1,2,3});
  }
  // for(i = 1; i< InternalNode<int,std::array<int,1000>,0>::maxValues+100; ++i)
  // {
  //     btreeD.emplace(i,{1,2,3});
  // }
  //btreeD.print();


  const size_t pagesize = 128;
  BTree<int,long long, pagesize> btreePage;

  for(i = 1; i<256; ++i)
  {
      btreePage.emplace(i,1);
  }

  btreePage.print();

  return 0;
}


