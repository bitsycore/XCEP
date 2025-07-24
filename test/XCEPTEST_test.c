#include "XCEPTEST_test.h"

#define XCEP_IMPLEMENTATION
#include <XCEP.h>

#include <stdio.h>
#include <string.h>

// =========================================================
// MARK: Unit Test Tools
// =========================================================

#include "XCEPTEST_thread.h"

int XCEPTEST_g_tests_passed = 0;
int XCEPTEST_g_tests_failed = 0;

#define XCEPTEST_RUN_TEST(test_func) \
    printf("# Running " #test_func " #\n"); \
    if (test_func()) { \
        XCEPTEST_g_tests_passed++; \
        printf("   ...PASSED\n"); \
    } else { \
        XCEPTEST_g_tests_failed++; \
        printf("   ...FAILED\n"); \
    } \
    printf("\n")

#define XCEPTEST_BOOL2STR_0 "false"
#define XCEPTEST_BOOL2STR_1 "true"
#define XCEPTEST_BOOL2STR_IMPL(x) XCEPTEST_BOOL2STR_##x
#define XCEPTEST_BOOL2STR(x) XCEPTEST_BOOL2STR_IMPL(x)

// =========================================================
// MARK: Exception Codes
// =========================================================

enum XCEPTEST_ExceptionCodes {
    XCEPTEST_ERR_GENERIC_FAILURE = 100,
    XCEPTEST_ERR_FILE_NOT_FOUND = 101,
    XCEPTEST_ERR_NETWORK_TIMEOUT = 102,
    XCEPTEST_ERR_RETHROWN = 103,
    XCEPTEST_ERR_THROWN_FROM_CATCH = 104,
    XCEPTEST_ERR_PROPAGATED = 105,
    XCEPTEST_ERR_DEEP_RETHROW = 106,
    XCEPTEST_ERR_CLEANUP = 107,
    XCEPTEST_ERR_VOLATILE_TEST = 108,
    XCEPTEST_ERR_THREAD_BASE = 300
};

// =======================================================
// MARK: Test case 1: Successful path with no exceptions
// =======================================================

int test_no_throw() {
    volatile int status = 0;

    Try {
        printf("   Inside Try block, no throw.\n");
        status = 1;
    }
    CatchAll {
        printf("   This CatchAll block should NOT execute.\n");
        status = -1;
    }
    Finally {
        printf("   Finally block executed on success path.\n");
        if (status == 1) status = 2;
    }
    EndTry;
    return status == 2;
}

// =======================================================
// MARK: Test case 2: Catching a specific exception exception_result_code
// =======================================================

int test_simple_catch() {
    volatile int status = 0;

    Try {
        printf("   Inside Try block, about to throw.\n");
        Throw(XCEPTEST_ERR_FILE_NOT_FOUND, "file.txt not found");
        status = -1;
    }
    Catch(XCEPTEST_ERR_NETWORK_TIMEOUT) {
        printf("   This Catch(NETWORK) block should NOT execute.\n");
        status = -1;
    }
    Catch(XCEPTEST_ERR_FILE_NOT_FOUND) {
        printf("   Correctly caught specific exception: %d (%s)\n", CaughtException.code, CaughtException.message);
        status = 1;
    }
    CatchAll {
        printf("   This CatchAll block should NOT execute.\n");
        status = -1;
    }
    EndTry;

    return status == 1;
}

// =======================================================
// MARK: Test case 3: Using CatchAll for an unhandled specific exception_result_code
// =======================================================

int test_catch_all() {
    volatile int status = 0;

    Try {
        printf("   Inside Try block, about to throw.\n");
        Throw(XCEPTEST_ERR_GENERIC_FAILURE, "a generic failure occurred");
        status = -1;
    }
    Catch(XCEPTEST_ERR_FILE_NOT_FOUND) {
        printf("   This Catch(FILE) block should NOT execute.\n");
        status = -1;
    } CatchAll {
        printf("   Correctly caught exception with CatchAll: %d (%s)\n", CaughtException.code, CaughtException.message);
        status = 1;
    }
    Finally {
        printf("   Finally block executed on exception path.\n");
        if (status == 1) status = 2;
    }
    EndTry;

    return status == 2;
}

// =======================================================
// MARK: Test case 4: Nested Try/Catch where the inner exception is handled locally
// =======================================================

int test_nested_handled_exception() {

    volatile int outer_status = 0;
    volatile int inner_status = 0;

    Try {
        printf("   Outer Try block started.\n");
        outer_status = 1;
        Try {
            printf("   Inner Try block started, about to throw.\n");
            inner_status = 1;
            Throw(XCEPTEST_ERR_GENERIC_FAILURE, "inner problem");
            inner_status = -1;
        }
        CatchAll {
            printf("   Inner CatchAll executed as expected.\n");
            inner_status = 2;
        }
        EndTry;
        printf("   Returned to Outer Try block.\n");
        outer_status = 2;
    }
    CatchAll {
        printf("   Outer CatchAll should NOT have been executed.\n");
        outer_status = -1;
    }
    EndTry;

    return outer_status == 2 && inner_status == 2;
}


// =======================================================
// MARK: Test case 5: Rethrowing an exception from a Catch block
// =======================================================

int test_rethrow() {
    volatile int outer_catch_fired = 0;
    volatile int inner_catch_fired = 0;

    Try {
        printf("   Outer Try started.\n");
        Try {
            printf("   Inner Try started, about to throw.\n");
            Throw(XCEPTEST_ERR_RETHROWN, "to be rethrown");
        }
        Catch(XCEPTEST_ERR_RETHROWN) {
            printf("   Inner Catch executed, about to rethrow...\n");
            inner_catch_fired = 1;
            Rethrow;
            printf("   This line after Rethrow should NOT be printed.\n");
        }
        EndTry;
        printf("   This line in Outer Try should NOT be printed.\n");
    }
    Catch(XCEPTEST_ERR_RETHROWN) {
        printf("   Outer Catch correctly caught the re-thrown exception.\n");
        printf("   CaughtException details: %d (%s)\n", CaughtException.code, CaughtException.message);
        outer_catch_fired = 1;
    }
    EndTry;

    return inner_catch_fired == 1 && outer_catch_fired == 1;
}

// =======================================================
// MARK: Test case 6: Throwing a NEW exception from within a Catch block
// =======================================================

int test_throw_from_catch() {
    volatile int outer_catch_fired = 0;
    volatile int inner_catch_fired = 0;

    Try {
        printf("   Outer Try started.\n");
        Try {
            printf("   Inner Try started, about to throw original exception.\n");
            Throw(XCEPTEST_ERR_GENERIC_FAILURE, "original problem");
        }
        Catch(XCEPTEST_ERR_GENERIC_FAILURE) {
            printf("   Inner Catch executed, about to throw a NEW exception.\n");
            inner_catch_fired = 1;
            Throw(XCEPTEST_ERR_THROWN_FROM_CATCH, "new problem from catch");
        }
        EndTry;
    }
    Catch(XCEPTEST_ERR_THROWN_FROM_CATCH) {
        printf("   Outer Catch correctly caught the NEW exception.\n");
        printf("   CaughtException details: %d (%s)\n", CaughtException.code, CaughtException.message);
        outer_catch_fired = 1;
    }
    CatchAll {
        printf("   This CatchAll should NOT execute.\n");
        outer_catch_fired = -1;
    }
    EndTry;

    return inner_catch_fired == 1 && outer_catch_fired == 1;
}

// =======================================================
// MARK: Test case 7: Nested Uncaught Propagation
// =======================================================

int test_nested_uncaught_propagation() {
    volatile int status = 0;

    Try {
        printf("   Outer Try started.\n");
        Try {
            printf("   Inner Try started, will throw unhandled exception.\n");
            Throw(XCEPTEST_ERR_PROPAGATED, "propagate me");
        }
        Catch(XCEPTEST_ERR_GENERIC_FAILURE) {
            printf("   Inner Catch should NOT execute.\n");
            status = -1;
        }
        EndTry;
        printf("   This line in Outer Try should NOT be printed.\n");
        status = -1;
        }
    Catch(XCEPTEST_ERR_PROPAGATED) {
        printf("   Outer Catch correctly caught the propagated exception.\n");
        status = 1;
    }
    EndTry;

    return status == 1;
}

// =======================================================
// MARK: Test case 8: Nested Rethrow
// =======================================================

int test_nested_rethrow() {
    volatile int status = 0;

    Try { // Level 1
        printf("   Level 1 Try.\n");
        Try { // Level 2
            printf("   Level 2 Try, will throw.\n");
            Throw(XCEPTEST_ERR_DEEP_RETHROW, "deep problem");
        }
        CatchAll {
            printf("   Level 2 Catch, will rethrow.\n");
            Rethrow;
        }
        EndTry;
    }
    CatchAll {
        printf("   Level 1 Catch, caught rethrown exception: %d.\n", CaughtException.code);
        if (CaughtException.code == XCEPTEST_ERR_DEEP_RETHROW) {
            status = 1;
        }
    }
    EndTry;

    return status == 1;
}

// =======================================================
// MARK: Test case 9: Ressource cleanup with finally
// =======================================================

int test_resource_cleanup_with_finally() {
    volatile int resource_is_acquired = 0;

    Try {
        printf("   Acquiring resource...\n");
        resource_is_acquired = 1;
        printf("   Resource acquired, about to throw.\n");
        Throw(XCEPTEST_ERR_CLEANUP, "something went wrong");
    }
    CatchAll {
        printf("   Caught exception, resource state: %s.\n", resource_is_acquired ? "Acquired" : "Released");   // We expect it to still be acquired here.
    }
    Finally {
        printf("   Finally block executing, cleaning up resource.\n");
        if (resource_is_acquired) {
            resource_is_acquired = 0;
            printf("   Resource released.\n");
        }
    }
    EndTry;

    printf("   After Try/Finally, resource state: %s.\n", resource_is_acquired ? "Acquired" : "Released");

    return resource_is_acquired == 0;
}

// =======================================================
// MARK: Test case 10: Volatile variable is correct
// =======================================================

int test_volatile_variable_correctness() {
    volatile int v_var = 0;
    int non_v_var = 0;
    int success = 0;

    Try {
         v_var = 1;
         non_v_var = 1;
         Throw(XCEPTEST_ERR_VOLATILE_TEST, "testing volatile");
    }
    CatchAll {
        printf("   After longjmp: volatile var is %d, non-volatile var is %d.\n", v_var, non_v_var);
        printf("   NOTE: The C standard says the non-volatile variable's value is indeterminate!\n");
        if (v_var == 1) {
            printf("   Volatile variable correctly holds its value.\n");
            success = 1;
        } else {
            printf("   Volatile variable INCORRECTLY lost its value.\n");
            success = 0;
        }
    }
    EndTry;

    return success;
}

// =======================================================
// MARK: Test case 11: Multiple catch block
// =======================================================

int test_multiple_catch_blocks() {
    volatile int status = 0;

    Try {
        Throw(XCEPTEST_ERR_NETWORK_TIMEOUT, "a network error");
    }
    Catch(XCEPTEST_ERR_FILE_NOT_FOUND) {
        status = -1;
    }
    Catch(XCEPTEST_ERR_NETWORK_TIMEOUT) {
        printf("   Correctly caught NETWORK_TIMEOUT in a list of catches.\n");
        status = 1;
    }
    Catch(XCEPTEST_ERR_GENERIC_FAILURE) {
        status = -1;
    }
    EndTry;

    return status == 1;
}

// =======================================================
// MARK: Test case 12: Try finally only
// =======================================================

void function_with_try_finally() {
    volatile int finally_ran = 0;

    Try {
        printf("   Inside Try/Finally, about to throw.\n");
        Throw(XCEPTEST_ERR_PROPAGATED, "test finally propagation");
    }
    Finally {
        printf("   Finally block ran, proving cleanup occurred.\n");
        finally_ran = 1;
    }
    EndTry;

    printf("   Never reached !.\n");

    // never reaches here due to propagation.
}

int test_try_finally_only() {
    volatile int status = 0;

    Try {
        function_with_try_finally();
    }
    Catch(XCEPTEST_ERR_PROPAGATED) {
        printf("   Main test function caught exception propagated through a Finally.\n");
        status = 1;
    }
    EndTry;

    return status == 1;
}

// =======================================================
// MARK: Test case 13: Uncaught exception handling
// =======================================================

volatile int XCEPTEST_g_UncaughtHandlerFiredFlag = 0;

void XCEPTEST_uncaught_handler(const XCEP_t_Exception *ex) {
    printf("   Custom uncaught handler fired as expected for exception_result_code %d.\n", ex->code);
    if (ex->code == XCEPTEST_ERR_GENERIC_FAILURE) {
        XCEPTEST_g_UncaughtHandlerFiredFlag = 1;
    }
}

void function_that_throws_nakedly() {
    printf("   About to throw from a function without a Try block...\n");
    Throw(XCEPTEST_ERR_GENERIC_FAILURE, "this should be caught by the global handler");
}

int test_uncaught_exception() {
    XCEPTEST_g_UncaughtHandlerFiredFlag = 0;
    // Get the original handler to restore it later
    const XCEP_t_ExceptionHandler original_handler = XCEP_g_UncaughtExceptionHandler;

    printf("   Setting custom uncaught exception handler.\n");
    SetUncaughtExceptionHandler(XCEPTEST_uncaught_handler);

    function_that_throws_nakedly();

    printf("   Restoring original uncaught exception handler.\n");
    SetUncaughtExceptionHandler(original_handler);

    return XCEPTEST_g_UncaughtHandlerFiredFlag == 1;
}

// =======================================================
// MARK: Test case 14: Thread Safety
// =======================================================

#define NUM_THREADS_TO_TEST 100

typedef struct {
    int thread_id;
    int exception_result_code;
    char message[128];
    volatile int success_flag;
} XCEPTEST_t_ThreadData;

void* thread_worker(void *arg) {
    XCEPTEST_t_ThreadData *data = arg;
    data->success_flag = 0;

    Try {
        XCEPTEST_Sleep(10 + (data->thread_id % 10));
        Throw(data->exception_result_code, data->message);
    }
    CatchAll {
        // CRITICAL CHECK: Did we catch our OWN exception?
        if (CaughtException.code == data->exception_result_code && strcmp(CaughtException.message, data->message) == 0) {
            data->success_flag = 1;
        } else {
            // This would be a catastrophic failure of thread safety
            fprintf(stderr, "   [Thread %d] FAILED: Caught wrong exception! Expected %d, got %d.\n", data->thread_id, data->exception_result_code, CaughtException.code);
            data->success_flag = 0;
        }
    }
    EndTry;

    return NULL;
}

int test_thread_safety_scalable() {
    const int num_threads = NUM_THREADS_TO_TEST;
    XCEPTEST_t_Thread* threads = malloc(sizeof(XCEPTEST_t_Thread) * num_threads);
    XCEPTEST_t_ThreadData* all_thread_data = malloc(sizeof(XCEPTEST_t_ThreadData) * num_threads);
    if (!threads || !all_thread_data) {
        fprintf(stderr, "   Failed to allocate memory for thread management.\n");
        free(threads);
        free(all_thread_data);
        return 0;
    }
    printf("   Initializing and launching threads...\n");
    for (int i = 0; i < num_threads; ++i) {
        XCEPTEST_t_ThreadData *data = &all_thread_data[i];
        data->thread_id = i + 1;
        data->exception_result_code = XCEPTEST_ERR_THREAD_BASE + i;
        data->success_flag = 0;
        snprintf(data->message, sizeof(data->message), "Unique error from thread %d", data->thread_id);
        if (XCEPTEST_ThreadCreate(&threads[i], thread_worker, data) != 0) {
            fprintf(stderr, "   Failed to create thread %d.\n", data->thread_id);
            free(threads);
            free(all_thread_data);
            return 0;
        }
    }
    printf("   All threads launched (%d). Waiting for them to complete...\n", num_threads);
    for (int i = 0; i < num_threads; ++i) { XCEPTEST_ThreadJoin(threads[i]); }
    printf("   All threads finished. Verifying results...\n");
    int all_succeeded = 1;
    for (int i = 0; i < num_threads; ++i) {
        if (!all_thread_data[i].success_flag) {
            fprintf(stderr, "   VERIFICATION FAILED for thread %d.\n", all_thread_data[i].thread_id);
            all_succeeded = 0;
        }
    }
    free(threads);
    free(all_thread_data);
    return all_succeeded;
}


int XCEPTEST_RunTest() {

    printf("===== XCEP Test Suite =====\n\n");

    printf("    XCEP_CONF_ENABLE_THREAD_SAFE=" XCEPTEST_BOOL2STR(XCEP_CONF_ENABLE_THREAD_SAFE) "\n");
    printf("    XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO=" XCEPTEST_BOOL2STR(XCEP_CONF_ENABLE_EXTRA_EXCEPTION_INFO) "\n");
    printf("    XCEP_CONF_ENABLE_CUSTOM_TYPES=" XCEPTEST_BOOL2STR(XCEP_CONF_ENABLE_CUSTOM_TYPES) "\n");

    puts("");

    printf("====== Running Test =======\n\n");

    XCEPTEST_RUN_TEST(test_no_throw);
    XCEPTEST_RUN_TEST(test_simple_catch);
    XCEPTEST_RUN_TEST(test_catch_all);
    XCEPTEST_RUN_TEST(test_nested_handled_exception);
    XCEPTEST_RUN_TEST(test_rethrow);
    XCEPTEST_RUN_TEST(test_throw_from_catch);
    XCEPTEST_RUN_TEST(test_nested_uncaught_propagation);
    XCEPTEST_RUN_TEST(test_nested_rethrow);
    XCEPTEST_RUN_TEST(test_resource_cleanup_with_finally);
    XCEPTEST_RUN_TEST(test_volatile_variable_correctness);
    XCEPTEST_RUN_TEST(test_multiple_catch_blocks);
    XCEPTEST_RUN_TEST(test_try_finally_only);
    XCEPTEST_RUN_TEST(test_uncaught_exception);

#if XCEP_CONF_ENABLE_THREAD_SAFE
    XCEPTEST_RUN_TEST(test_thread_safety_scalable);
#else
    printf("===== Warnings =====\n");
    printf("   Thread safety tests are disabled.\n");
#endif


    printf("Result: %d passed, %d failed.\n", XCEPTEST_g_tests_passed, XCEPTEST_g_tests_failed);
    printf("===============================\n");
    return (XCEPTEST_g_tests_failed == 0) ? 0 : 1;
}
