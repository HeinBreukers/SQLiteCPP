cmake_minimum_required(VERSION 3.14)
project(db_test)

# GoogleTest requires at least C++14  

include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 8.1.1
        )

FetchContent_MakeAvailable(fmt)

# For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/Backend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/Backend/BTree)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/CLI)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/Core)

# add_executable(
#   MainTest
#   MainTest.cpp
# )
# target_link_libraries(
#   MainTest
#   GTest::gtest_main
#   SQLiteCPP
# )

add_executable(
  BTreeTest
  BTreeTest.cpp
)
target_link_libraries(
  BTreeTest
  GTest::gtest_main
  SQLiteCPP
)

#add_test(multiply_gtests test1)


