cmake_minimum_required(VERSION 2.8)
include_directories(../../include)
include_directories(.)

file(GLOB_RECURSE SRC *.cpp)

add_library(bh_filter_preprocessor SHARED ${SRC})

#We depend on bh.so
target_link_libraries(bh_filter_preprocessor bh)

install(TARGETS bh_filter_preprocessor DESTINATION lib COMPONENT bohrium)
