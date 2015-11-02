cmake_minimum_required(VERSION 3.3)
project(spooler)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(S_SOURCE_FILES sfs_api.h)
add_executable(server ${S_SOURCE_FILES})
target_link_libraries(server)
