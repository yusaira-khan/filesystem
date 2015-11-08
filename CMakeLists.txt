cmake_minimum_required(VERSION 3.3)
project(filesystem)
find_package(PkgConfig)
pkg_check_modules(PC_FUSE "fuse" QUIET)
set(FUSE_DEFINITIONS "${PC_FUSE_CFLAGS_OTHER}")
find_path(
        FUSE_INCLUDE_DIRS
        NAMES fuse_common.h fuse_lowlevel.h fuse.h
        PATHS "${PC_FUSE_INCLUDE_DIRS}"
        DOC "Include directories for FUSE"
)
find_library(
        FUSE_LIBRARIES
        NAMES "fuse"
        PATHS "${PC_FUSE_LIBRARY_DIRS}"
        DOC "Libraries for FUSE"
)
#execute__process(COMMAND pkg-config fuse --cflags --libs OUTPUT_VARIABLE FUSE_FLAGS)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
add_definitions(${FUSE_DEFINITIONS})
include_directories(${FUSE_INCLUDE_DIRS})


set(S_SOURCE_FILES sfs_api.h)
add_executable(server ${S_SOURCE_FILES})
target_link_libraries(server)
target_link_libraries(myapp ${FUSE_LIBRARIES})