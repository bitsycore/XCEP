add_executable(test test.c
        other.c
        other.h)
target_link_libraries(test PRIVATE xcep)