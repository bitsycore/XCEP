add_executable(
        test
        main.c
        XCEPTEST_test.c
        XCEPTEST_test.h
        XCEPTEST_thread.c
        XCEPTEST_thread.h
)

target_link_libraries(test PRIVATE xcep)