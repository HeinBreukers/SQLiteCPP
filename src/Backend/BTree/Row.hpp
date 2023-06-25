#pragma once

#include <vector>
#include <cinttypes>
#include <fmt/core.h>

#include <array>
#include <span>
#include <type_traits>

template<typename Key, typename Value> requires (std::is_trivially_copyable_v<Key>&&std::is_trivially_copyable_v<Value>)
struct Row
{
  Key key;
  Value value;

  void print()
  {
    fmt::print("key: {}, value: {}\n",key,value);
  }
};

template<typename Key, typename Value>
std::array<char,sizeof(Row<Key,Value>)> serialize_row(const Row<Key,Value>& source) {
    std::array<char,sizeof(Row<Key,Value>)> destination;
    std::memcpy(&destination[0], &source, sizeof(Row<Key,Value>)); 
    return destination;
}

template<typename Key, typename Value>
Row<Key,Value> deserialize_row(std::span<char> source) {
    Row<Key,Value> destination;
    std::memcpy(&destination, source.data(), sizeof(Row<Key,Value>));
    return destination;
}


// std::vector<char> serialize_row(const Row& source);
// Row deserialize_row(std::span<char> source);
