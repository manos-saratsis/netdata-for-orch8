#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "stacktrace-common.h"
#include "stacktrace.h"

// Mock buffer for testing
typedef struct {
    char data[4096];
    size_t len;
} MOCK_BUFFER;

// Test stacktrace_backend()
void test_stacktrace_backend() {
    const char *backend = stacktrace_backend();
    assert(strcmp(backend, "none") == 0);
}

// Test stacktrace_available()
void test_stacktrace_available() {
    bool available = stacktrace_available();
    assert(available == false);
}

// Test impl_stacktrace_init()
void test_impl_stacktrace_init() {
    // Verify no crash
    impl_stacktrace_init();
}

// Test stacktrace_flush()
void test_stacktrace_flush() {
    // Verify no crash
    stacktrace_flush();
}

// Test stacktrace_capture_is_async_signal_safe()
void test_stacktrace_capture_is_async_signal_safe() {
    bool safe = stacktrace_capture_is_async_signal_safe();
    assert(safe == false);
}

// Test stacktrace_capture()
void test_stacktrace_capture() {
    MOCK_BUFFER wb = {0};
    stacktrace_capture((BUFFER *)&wb);

    // Verify a default message is added
    assert(strstr(wb.data, NO_STACK_TRACE_PREFIX "no back-end available") != NULL);
}

// Test impl_stacktrace_get_frames()
void test_impl_stacktrace_get_frames() {
    void *frames[10];
    int max_frames = 10;
    int skip_frames = 2;

    // Test with valid input
    int frames_count = impl_stacktrace_get_frames(frames, max_frames, skip_frames);
    assert(frames_count == 1);  // Returns 1 dummy frame
    assert(frames[0] != NULL);  // Frame is not NULL

    // Test with invalid input
    frames_count = impl_stacktrace_get_frames(NULL, 0, 0);
    assert(frames_count == 0);
}

// Test impl_stacktrace_to_buffer()
void test_impl_stacktrace_to_buffer() {
    MOCK_BUFFER wb = {0};
    struct stacktrace st = {0};
    st.hash = 42;

    impl_stacktrace_to_buffer((STACKTRACE)&st, (BUFFER *)&wb);

    // Verify output contains expected message and hash
    char expected_prefix[] = NO_STACK_TRACE_PREFIX "no back-end available (id: 42)";
    assert(strstr(wb.data, expected_prefix) != NULL);
}

int main() {
    test_stacktrace_backend();
    test_stacktrace_available();
    test_impl_stacktrace_init();
    test_stacktrace_flush();
    test_stacktrace_capture_is_async_signal_safe();
    test_stacktrace_capture();
    test_impl_stacktrace_get_frames();
    test_impl_stacktrace_to_buffer();

    printf("All stacktrace-none tests passed!\n");
    return 0;
}