cmake_minimum_required(VERSION 3.2)
project(db_src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wnull-dereference -Wdouble-promotion -Wformat=2" )
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast " )


# --- Fetch KafkaWrapper --------------------------------------------------------------
include(FetchContent)


FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 8.1.1
        )

FetchContent_MakeAvailable(fmt)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Backend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Backend/BTree)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/CLI)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)

set(SOURCE
# Backend/Cursor.cpp
Backend/BTree/Row.cpp
Backend/BTree/RootNode.cpp
# Backend/Table.cpp
# Backend/Pager.cpp
)

set(HEADER
Core/CommandProcessor.hpp
Core/VirtualMachine.hpp)

add_executable(db main.cpp ${SOURCE} ${HEADER})
target_link_libraries(db fmt)

add_library(SQLiteCPP
STATIC
      ${HEADER}
      ${SOURCE}
)

target_link_libraries(SQLiteCPP PUBLIC fmt )
target_compile_options(SQLiteCPP PRIVATE -Werror -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wnull-dereference -Wdouble-promotion -Wformat=2)
# if GCC
#target_compile_options(SQLiteCPP PRIVATE -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast)
target_include_directories(SQLiteCPP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Backend)
target_include_directories(SQLiteCPP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Backend/BTree)
target_include_directories(SQLiteCPP PUBLIC ${date_SOURCE_DIR}/CLI)
target_include_directories(SQLiteCPP PUBLIC ${date_SOURCE_DIR}/Core)