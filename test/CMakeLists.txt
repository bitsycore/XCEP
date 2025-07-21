add_executable(test test_suite.c
        main.c
        test_suite.h
        cross_thread.c
        cross_thread.h)
target_link_libraries(test PRIVATE xcep)