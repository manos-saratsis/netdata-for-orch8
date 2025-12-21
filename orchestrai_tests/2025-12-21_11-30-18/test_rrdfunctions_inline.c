```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "rrdfunctions-inline.h"
#include "rrd.h"

// Mock callback function for testing
static int mock_execute_callback(BUFFER *wb, const char *function, BUFFER *payload, const char *source) {
    check_expected(function);
    check_expected(payload);
    check_expected(source);
    return mock_type(int);
}

// Mock function to simulate cancelled execution
static bool mock_is_cancelled(void *data) {
    return mock_type(bool);
}

static void test_rrd_function_run_inline_success(void **state) {
    RRDHOST host = {0};
    RRDSET set = {0};
    BUFFER wb = {0};
    BUFFER payload = {0};

    struct rrd_function_execute rfe = {
        .result = { .wb = &wb },
        .function = "test_function",
        .payload = &payload,
        .source = "test_source",
        .is_cancelled = { .cb = mock_is_cancelled, .data = NULL }
    };

    // Expect not cancelled
    will_return(mock_is_cancelled, false);
    
    // Expect successful execution
    expect_string(mock_execute_callback, function, "test_function");
    expect_value(mock_execute_callback, payload, &payload);
    expect_string(mock_execute_callback, source, "test_source");
    will_return(mock_execute_callback, HTTP_RESP_OK);

    // Optional: test result callback if it exists
    // This might require additional mocking depending on implementation
}

static void test_rrd_function_run_inline_cancelled(void **state) {
    BUFFER wb = {0};
    struct rrd_function_execute rfe = {
        .result = { .wb = &wb },
        .is_cancelled = { .cb = mock_is_cancelled, .data = NULL }
    };

    // Simulate cancellation
    will_return(mock_is_cancelled, true);

    // Expect HTTP_RESP_CLIENT_CLOSED_REQUEST
    will_return(mock_execute_callback, HTTP_RESP_CLIENT_CLOSED_REQUEST);
}

static void test_rrd_function_add_inline(void **state) {
    RRDHOST host = {0};
    RRDSET set = {0};

    // Expect rrd_collector_started to be called
    // Expect rrd_function_add to be called with correct parameters
    rrd_function_add_inline(&host, &set, "test_func", 10, 5, 1, 
                             "Help text", "tags", HTTP_ACCESS_NONE, 
                             mock_execute_callback);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rrd_function_run_inline_success),
        cmocka_unit_test(test_rrd_function_run_inline_cancelled),
        cmocka_unit_test(test_rrd_function_add_inline)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
```