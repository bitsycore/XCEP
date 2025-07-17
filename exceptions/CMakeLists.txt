add_library(exception STATIC Exceptions.c)
target_include_directories(exception PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})