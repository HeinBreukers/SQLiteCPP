cmake_minimum_required(VERSION 3.2)
project(db_src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2" )


# --- Fetch KafkaWrapper --------------------------------------------------------------
include(FetchContent)


FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 8.1.1
        )

FetchContent_MakeAvailable(fmt)

include_directories(FFT)

set(SOURCE_FILES main.cpp)

set(HEADER_FILES main.cpp)

add_executable(db ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(db fmt)
#target_link_libraries(divider division)
#install(TARGETS divider DESTINATION ${DIVISIBLE_INSTALL_BIN_DIR})
