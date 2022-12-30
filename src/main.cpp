#include "DB.hpp"


int main(/*int argc, char* argv[]*/) 
{
  std::string input_buffer;
  Table table;
  while (true) {
    print_prompt();
    std::getline(std::cin, input_buffer);

    if (input_buffer.front() == '.') {
      switch (do_meta_command(input_buffer)) {
        case (MetaCommandResult::SUCCESS):
          continue;
        case (MetaCommandResult::UNRECOGNIZED_COMMAND):
          fmt::print("Unrecognized command '{}'\n", input_buffer);
          continue;
      }
    } 

    Statement statement;
    switch (prepare_statement(input_buffer, statement)) {
      case (PrepareResult::SUCCESS):
        break;
      case (PrepareResult::SYNTAX_ERROR):
          fmt::print("Syntax error. Could not parse statement.\n");
          continue;
      case (PrepareResult::UNRECOGNIZED_STATEMENT):
        fmt::print("Unrecognized keyword at start of '{}'.\n", input_buffer);
        continue;
    }

    switch (execute_statement(statement, table).value()) {
      case (ExecuteResult::SUCCESS):
        fmt::print("Executed.\n");
        break;
      case (ExecuteResult::TABLE_FULL):
        fmt::print("Error: Table full.\n");
        break;
    }
   
  }
  return 0;
}


