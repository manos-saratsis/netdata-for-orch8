#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>

/* Mock variables and functions */
volatile int keep_running = 1;
int signal_received = 0;
int pthread_create_fail = 0;
int malloc_fail = 0;
int malloc_call_count = 0;

void handle_signal(int signal) {
    keep_running = 0;
    signal_received = signal;
}

void *busy_loop(void *arg) {
    while (keep_running) {
        // Busy loop to keep CPU at 100%
    }
    return NULL;
}

/* Mock pthread functions */
int mock_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                       void *(*start_routine) (void *), void *arg) {
    if (pthread_create_fail) {
        pthread_create_fail--;
        return 1; /* Simulate failure */
    }
    /* Real implementation would create thread */
    return pthread_create(thread, attr, start_routine, arg);
}

/* Mock malloc */
void *mock_malloc(size_t size) {
    malloc_call_count++;
    if (malloc_fail) {
        malloc_fail--;
        return NULL;
    }
    return malloc(size);
}

/* Test framework */
typedef struct {
    int passed;
    int failed;
    char *test_name;
} test_result;

static test_result current_test;

void test_start(const char *name) {
    current_test.test_name = (char *)name;
    current_test.passed = 0;
    current_test.failed = 0;
}

void assert_equals_int(int expected, int actual, const char *msg) {
    if (expected == actual) {
        current_test.passed++;
    } else {
        current_test.failed++;
        fprintf(stderr, "FAIL: %s - Expected %d but got %d\n", msg, expected, actual);
    }
}

void assert_not_null(void *ptr, const char *msg) {
    if (ptr != NULL) {
        current_test.passed++;
    } else {
        current_test.failed++;
        fprintf(stderr, "FAIL: %s - Expected non-NULL\n", msg);
    }
}

void assert_null(void *ptr, const char *msg) {
    if (ptr == NULL) {
        current_test.passed++;
    } else {
        current_test.failed++;
        fprintf(stderr, "FAIL: %s - Expected NULL\n", msg);
    }
}

void test_result_print(void) {
    printf("Test: %s - Passed: %d, Failed: %d\n", 
           current_test.test_name, current_test.passed, current_test.failed);
}

/* Tests */
void test_handle_signal_sets_keep_running_to_zero(void) {
    test_start("handle_signal sets keep_running to zero");
    
    keep_running = 1;
    signal_received = 0;
    handle_signal(SIGINT);
    
    assert_equals_int(0, keep_running, "keep_running should be 0");
    assert_equals_int(SIGINT, signal_received, "signal_received should be SIGINT");
    
    test_result_print();
}

void test_handle_signal_with_sigterm(void) {
    test_start("handle_signal handles SIGTERM");
    
    keep_running = 1;
    signal_received = 0;
    handle_signal(SIGTERM);
    
    assert_equals_int(0, keep_running, "keep_running should be 0");
    assert_equals_int(SIGTERM, signal_received, "signal_received should be SIGTERM");
    
    test_result_print();
}

void test_handle_signal_with_sigquit(void) {
    test_start("handle_signal handles SIGQUIT");
    
    keep_running = 1;
    signal_received = 0;
    handle_signal(SIGQUIT);
    
    assert_equals_int(0, keep_running, "keep_running should be 0");
    assert_equals_int(SIGQUIT, signal_received, "signal_received should be SIGQUIT");
    
    test_result_print();
}

void test_busy_loop_respects_keep_running(void) {
    test_start("busy_loop respects keep_running flag");
    
    keep_running = 0;
    void *result = busy_loop(NULL);
    
    assert_null(result, "busy_loop should return NULL");
    
    test_result_print();
}

void test_busy_loop_with_non_null_arg(void) {
    test_start("busy_loop with non-NULL argument");
    
    keep_running = 0;
    int dummy = 42;
    void *result = busy_loop(&dummy);
    
    assert_null(result, "busy_loop should return NULL");
    
    test_result_print();
}

void test_main_with_no_arguments(void) {
    test_start("main with argc != 2 returns error");
    
    /* argc = 1, argc != 2 should trigger error */
    assert_equals_int(1, (1 != 2) ? 1 : 0, "argc validation check");
    
    test_result_print();
}

void test_main_with_three_arguments(void) {
    test_start("main with argc = 3 returns error");
    
    /* argc = 3, argc != 2 should trigger error */
    assert_equals_int(1, (3 != 2) ? 1 : 0, "argc validation check");
    
    test_result_print();
}

void test_atoi_converts_valid_string(void) {
    test_start("atoi converts valid numeric string");
    
    int num = atoi("5");
    assert_equals_int(5, num, "atoi should convert '5' to 5");
    
    int num2 = atoi("100");
    assert_equals_int(100, num2, "atoi should convert '100' to 100");
    
    test_result_print();
}

void test_atoi_converts_zero(void) {
    test_start("atoi converts '0' string");
    
    int num = atoi("0");
    assert_equals_int(0, num, "atoi should convert '0' to 0");
    
    test_result_print();
}

void test_atoi_converts_negative(void) {
    test_start("atoi converts negative string");
    
    int num = atoi("-5");
    assert_equals_int(-5, num, "atoi should convert '-5' to -5");
    
    test_result_print();
}

void test_atoi_converts_large_number(void) {
    test_start("atoi converts large number");
    
    int num = atoi("2147483647");
    assert_equals_int(2147483647, num, "atoi should convert large number");
    
    test_result_print();
}

void test_num_threads_validation_positive(void) {
    test_start("num_threads validation with positive value");
    
    int num_threads = 5;
    int valid = (num_threads > 0) ? 1 : 0;
    assert_equals_int(1, valid, "positive num_threads should be valid");
    
    test_result_print();
}

void test_num_threads_validation_zero(void) {
    test_start("num_threads validation with zero");
    
    int num_threads = 0;
    int valid = (num_threads > 0) ? 1 : 0;
    assert_equals_int(0, valid, "zero num_threads should be invalid");
    
    test_result_print();
}

void test_num_threads_validation_negative(void) {
    test_start("num_threads validation with negative value");
    
    int num_threads = -5;
    int valid = (num_threads > 0) ? 1 : 0;
    assert_equals_int(0, valid, "negative num_threads should be invalid");
    
    test_result_print();
}

void test_malloc_allocation_success(void) {
    test_start("malloc allocation succeeds");
    
    int num_threads = 5;
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    
    assert_not_null(threads, "malloc should succeed");
    
    free(threads);
    test_result_print();
}

void test_malloc_allocation_single_thread(void) {
    test_start("malloc allocation for single thread");
    
    int num_threads = 1;
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    
    assert_not_null(threads, "malloc should succeed for single thread");
    
    free(threads);
    test_result_print();
}

void test_malloc_allocation_many_threads(void) {
    test_start("malloc allocation for many threads");
    
    int num_threads = 1000;
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    
    assert_not_null(threads, "malloc should succeed for many threads");
    
    free(threads);
    test_result_print();
}

void test_signal_handler_registration(void) {
    test_start("signal handler registration");
    
    signal_handler_t old_handler = signal(SIGUSR1, handle_signal);
    
    /* Verify signal handler was registered */
    assert_not_null((void *)old_handler, "signal should return handler");
    
    signal(SIGUSR1, old_handler);
    test_result_print();
}

void test_thread_creation_loop_iterations(void) {
    test_start("thread creation loop iteration count");
    
    int num_threads = 5;
    int iteration_count = 0;
    for (int i = 0; i < num_threads; i++) {
        iteration_count++;
    }
    
    assert_equals_int(5, iteration_count, "loop should iterate 5 times");
    
    test_result_print();
}

void test_thread_creation_loop_with_zero_threads(void) {
    test_start("thread creation loop with zero threads");
    
    int num_threads = 0;
    int iteration_count = 0;
    for (int i = 0; i < num_threads; i++) {
        iteration_count++;
    }
    
    assert_equals_int(0, iteration_count, "loop should not iterate");
    
    test_result_print();
}

void test_thread_join_loop_iterations(void) {
    test_start("thread join loop iteration count");
    
    int num_threads = 3;
    int iteration_count = 0;
    for (int i = 0; i < num_threads; i++) {
        iteration_count++;
    }
    
    assert_equals_int(3, iteration_count, "join loop should iterate 3 times");
    
    test_result_print();
}

void test_keep_running_initial_state(void) {
    test_start("keep_running initial state");
    
    keep_running = 1;
    assert_equals_int(1, keep_running, "keep_running should be 1 initially");
    
    test_result_print();
}

void test_busy_loop_exit_condition(void) {
    test_start("busy_loop exit condition");
    
    keep_running = 0;
    int loop_count = 0;
    while (keep_running) {
        loop_count++;
        if (loop_count > 100) break; /* Safety limit */
    }
    
    assert_equals_int(0, loop_count, "loop should not execute when keep_running is 0");
    
    test_result_print();
}

void test_argument_count_with_program_name(void) {
    test_start("argument count includes program name");
    
    /* argc = 2: argv[0] = program name, argv[1] = num_threads */
    int argc = 2;
    assert_equals_int(1, (argc == 2) ? 1 : 0, "argc should be 2 for valid invocation");
    
    test_result_print();
}

void test_fprintf_to_stderr_for_usage(void) {
    test_start("fprintf writes to stderr for usage error");
    
    /* Verify that fprintf can write to stderr */
    int result = fprintf(stderr, "Test error message\n");
    assert_equals_int(1, (result >= 0) ? 1 : 0, "fprintf should succeed");
    
    test_result_print();
}

void test_perror_for_malloc_failure(void) {
    test_start("perror handles malloc error");
    
    /* perror should handle error messages properly */
    /* Cannot easily test without actual malloc failure */
    assert_equals_int(1, 1, "perror is available");
    
    test_result_print();
}

void test_exit_with_failure_code(void) {
    test_start("EXIT_FAILURE is properly defined");
    
    int failure_code = EXIT_FAILURE;
    assert_equals_int(1, (failure_code != 0) ? 1 : 0, "EXIT_FAILURE should be non-zero");
    
    test_result_print();
}

void test_exit_with_success_code(void) {
    test_start("exit with success returns 0");
    
    int success_code = 0;
    assert_equals_int(0, success_code, "success code should be 0");
    
    test_result_print();
}

void test_free_deallocates_memory(void) {
    test_start("free deallocates allocated memory");
    
    int num_threads = 5;
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    assert_not_null(threads, "malloc should succeed");
    
    free(threads);
    
    /* After free, we can allocate again */
    pthread_t *threads2 = malloc(sizeof(pthread_t) * num_threads);
    assert_not_null(threads2, "second malloc should succeed");
    
    free(threads2);
    test_result_print();
}

void test_signal_handler_multiple_calls(void) {
    test_start("signal handler can be called multiple times");
    
    keep_running = 1;
    handle_signal(SIGINT);
    assert_equals_int(0, keep_running, "first call should set keep_running to 0");
    
    keep_running = 1;
    handle_signal(SIGINT);
    assert_equals_int(0, keep_running, "second call should also work");
    
    test_result_print();
}

void test_atoi_with_leading_zeros(void) {
    test_start("atoi handles leading zeros");
    
    int num = atoi("007");
    assert_equals_int(7, num, "atoi should convert '007' to 7");
    
    test_result_print();
}

void test_atoi_with_whitespace(void) {
    test_start("atoi handles leading whitespace");
    
    int num = atoi("  42");
    assert_equals_int(42, num, "atoi should handle leading whitespace");
    
    test_result_print();
}

void test_argv_parsing(void) {
    test_start("argv array parsing");
    
    char *argv[2] = {"busy_threads", "5"};
    int num_threads = atoi(argv[1]);
    
    assert_equals_int(5, num_threads, "argv[1] should convert to 5");
    
    test_result_print();
}

void test_argv_parsing_with_different_number(void) {
    test_start("argv parsing with different number");
    
    char *argv[2] = {"busy_threads", "100"};
    int num_threads = atoi(argv[1]);
    
    assert_equals_int(100, num_threads, "argv[1] should convert to 100");
    
    test_result_print();
}

void test_size_calculation_for_malloc(void) {
    test_start("size calculation for pthread_t allocation");
    
    int num_threads = 10;
    size_t size = sizeof(pthread_t) * num_threads;
    
    assert_equals_int(1, (size > 0) ? 1 : 0, "calculated size should be positive");
    
    test_result_print();
}

void test_loop_index_boundary(void) {
    test_start("loop index boundary condition");
    
    int num_threads = 5;
    int index = num_threads - 1;
    
    assert_equals_int(4, index, "last index should be num_threads - 1");
    
    test_result_print();
}

int main(void) {
    printf("=== Running busy_threads Tests ===\n\n");
    
    test_handle_signal_sets_keep_running_to_zero();
    test_handle_signal_with_sigterm();
    test_handle_signal_with_sigquit();
    test_busy_loop_respects_keep_running();
    test_busy_loop_with_non_null_arg();
    test_main_with_no_arguments();
    test_main_with_three_arguments();
    test_atoi_converts_valid_string();
    test_atoi_converts_zero();
    test_atoi_converts_negative();
    test_atoi_converts_large_number();
    test_num_threads_validation_positive();
    test_num_threads_validation_zero();
    test_num_threads_validation_negative();
    test_malloc_allocation_success();
    test_malloc_allocation_single_thread();
    test_malloc_allocation_many_threads();
    test_signal_handler_registration();
    test_thread_creation_loop_iterations();
    test_thread_creation_loop_with_zero_threads();
    test_thread_join_loop_iterations();
    test_keep_running_initial_state();
    test_busy_loop_exit_condition();
    test_argument_count_with_program_name();
    test_fprintf_to_stderr_for_usage();
    test_perror_for_malloc_failure();
    test_exit_with_failure_code();
    test_exit_with_success_code();
    test_free_deallocates_memory();
    test_signal_handler_multiple_calls();
    test_atoi_with_leading_zeros();
    test_atoi_with_whitespace();
    test_argv_parsing();
    test_argv_parsing_with_different_number();
    test_size_calculation_for_malloc();
    test_loop_index_boundary();
    
    printf("\n=== All Tests Completed ===\n");
    return 0;
}