cmake_minimum_required(VERSION 3.3)
project(filesystem)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=gnu99")

add_definitions(${FUSE_DEFINITIONS})
include_directories(${FUSE_INCLUDE_DIRS})
add_executable(sfs disk_emu.c sfs_api.c fuse_wrappers.c sfs_api.h)
target_link_libraries(sfs ${FUSE_LIBRARIES})

add_executable(test1 disk_emu.c disk_emu.h sfs_api.c sfs_test.c sfs_api.h)
target_link_libraries(test1 ${FUSE_LIBRARIES})

add_executable(test2 disk_emu.c sfs_api.c sfs_test2.c sfs_api.h)
target_link_libraries(test2 ${FUSE_LIBRARIES})


