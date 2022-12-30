#pragma once 

struct Pager{
  int file_descriptor;
  uint32_t file_length;
  void* pages[TABLE_MAX_PAGES];
};