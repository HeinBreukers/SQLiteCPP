#pragma once

#include <string>

enum class MetaCommandResult
{
  SUCCESS,
  UNRECOGNIZED_COMMAND
};

class MetaCommand
{
public:
  MetaCommandResult do_meta_command(const std::string& input_buffer) 
  {
    if (input_buffer == ".exit") {
      exit = true;
      //exit(EXIT_SUCCESS);
      return MetaCommandResult::SUCCESS;
    } 
    else {
      return MetaCommandResult::UNRECOGNIZED_COMMAND;
    }
  }

  bool exit = false;

};


