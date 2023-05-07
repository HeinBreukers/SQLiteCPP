#pragma once

#include <vector>
#include <cinttypes>
#include <fmt/core.h>

struct Row
{
  uint32_t id;
  uint32_t age;
  std::array<uint32_t,126> lastvar;

  void print()
  {
    fmt::print("id: {}, age: {}, lastvar: {}\n",id,age,lastvar[0]);
  }
};

// TODO move out of header
std::vector<char> serialize_row(const Row& source) {
    std::vector<char> destination(sizeof(Row));
    std::memcpy(&destination[0], &source, sizeof(Row)); 
    return destination;
}

Row deserialize_row(std::span<char> source) {
    Row destination;
    std::memcpy(&destination, &source[0], sizeof(Row));
    return destination;
}
