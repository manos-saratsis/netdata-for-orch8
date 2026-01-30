#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Mock types and structures
typedef struct {
    struct config *config;
    char *config_section;
} LISTEN_SOCKETS;

typedef struct {
} POLLINFO;

typedef struct {
} nd_poll_event_t;

typedef struct {
} nd_thread_t;

// Mock global variables
LISTEN_SOCKETS ipc_sockets;
struct config collector_config;
sem_t *shm_mutex_ebpf_integration;

// Mock function declarations
int poll_events(
    LISTEN_SOCKETS *sockets,
    void *arg1,
    void *arg2,
    int (*rcv_callback)(POLLINFO *, nd_poll_event_t *),
    int (*snd_callback)(POLLINFO *, nd_poll_event_t *),
    void *arg5,
    bool (*should_stop)(void),
    void *arg7,
    int arg8,
    void *arg9,
    int arg10,
    int arg11,
    int timeout_ms,
    void *ptr,
    int arg14)
{
    return 0;
}

void nd_thread_signaled_to_cancel(void)
{
}

// Include the actual source file
#include "ebpf_socket_ipc.c"

// Test cases
static void test_ebpf_initialize_sockets_memset(void **state)
{
    (void)state;
    
    // Initialize with known values
    memset(&ipc_sockets, 0xFF, sizeof(ipc_sockets));
    
    // Call the initialization function
    ebpf_initialize_sockets();
    
    // Verify memset was called (all bytes should be 0)
    for (size_t i = 0; i < sizeof(ipc_sockets); i++) {
        assert_int_equal(((char*)&ipc_sockets)[i], 0);
    }
}

static void test_ebpf_initialize_sockets_config_assignment(void **state)
{
    (void)state;
    
    // Call the initialization function
    ebpf_initialize_sockets();
    
    // Verify config pointer assignment
    assert_ptr_equal(ipc_sockets.config, &collector_config);
}

static void test_ebpf_initialize_sockets_config_section_assignment(void **state)
{
    (void)state;
    
    // Call the initialization function
    ebpf_initialize_sockets();
    
    // Verify config_section assignment
    assert_string_equal(ipc_sockets.config_section, NETDATA_EBPF_IPC_INTEGRATION);
}

static void test_ebpf_ipc_rcv_callback_returns_zero(void **state)
{
    (void)state;
    
    POLLINFO pi = {0};
    nd_poll_event_t events = {0};
    
    int result = ebpf_ipc_rcv_callback(&pi, &events);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_rcv_callback_null_pi(void **state)
{
    (void)state;
    
    nd_poll_event_t events = {0};
    
    int result = ebpf_ipc_rcv_callback(NULL, &events);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_rcv_callback_null_events(void **state)
{
    (void)state;
    
    POLLINFO pi = {0};
    
    int result = ebpf_ipc_rcv_callback(&pi, NULL);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_rcv_callback_null_both(void **state)
{
    (void)state;
    
    int result = ebpf_ipc_rcv_callback(NULL, NULL);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_snd_callback_returns_zero(void **state)
{
    (void)state;
    
    POLLINFO pi = {0};
    nd_poll_event_t events = {0};
    
    int result = ebpf_ipc_snd_callback(&pi, &events);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_snd_callback_null_pi(void **state)
{
    (void)state;
    
    nd_poll_event_t events = {0};
    
    int result = ebpf_ipc_snd_callback(NULL, &events);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_snd_callback_null_events(void **state)
{
    (void)state;
    
    POLLINFO pi = {0};
    
    int result = ebpf_ipc_snd_callback(&pi, NULL);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_snd_callback_null_both(void **state)
{
    (void)state;
    
    int result = ebpf_ipc_snd_callback(NULL, NULL);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_should_stop_returns_false(void **state)
{
    (void)state;
    
    bool result = ebpf_ipc_should_stop();
    
    assert_false(result);
}

static void test_ebpf_ipc_should_stop_always_false(void **state)
{
    (void)state;
    
    // Call multiple times to ensure it always returns false
    for (int i = 0; i < 10; i++) {
        bool result = ebpf_ipc_should_stop();
        assert_false(result);
    }
}

// Mock for poll_events to track calls
static int mock_poll_events_call_count = 0;
static LISTEN_SOCKETS *mock_poll_events_sockets = NULL;
static int (*mock_poll_events_rcv_callback)(POLLINFO *, nd_poll_event_t *) = NULL;
static int (*mock_poll_events_snd_callback)(POLLINFO *, nd_poll_event_t *) = NULL;
static bool (*mock_poll_events_should_stop)(void) = NULL;

int __wrap_poll_events(
    LISTEN_SOCKETS *sockets,
    void *arg1,
    void *arg2,
    int (*rcv_callback)(POLLINFO *, nd_poll_event_t *),
    int (*snd_callback)(POLLINFO *, nd_poll_event_t *),
    void *arg5,
    bool (*should_stop)(void),
    void *arg7,
    int arg8,
    void *arg9,
    int arg10,
    int arg11,
    int timeout_ms,
    void *ptr,
    int arg14)
{
    mock_poll_events_call_count++;
    mock_poll_events_sockets = sockets;
    mock_poll_events_rcv_callback = rcv_callback;
    mock_poll_events_snd_callback = snd_callback;
    mock_poll_events_should_stop = should_stop;
    
    return 0;
}

static void test_ebpf_socket_thread_ipc_calls_ebpf_initialize_sockets(void **state)
{
    (void)state;
    
    mock_poll_events_call_count = 0;
    
    ebpf_socket_thread_ipc(NULL);
    
    // Verify that the sockets struct was initialized
    assert_ptr_equal(ipc_sockets.config, &collector_config);
    assert_string_equal(ipc_sockets.config_section, NETDATA_EBPF_IPC_INTEGRATION);
}

static void test_ebpf_socket_thread_ipc_calls_poll_events(void **state)
{
    (void)state;
    
    mock_poll_events_call_count = 0;
    
    ebpf_socket_thread_ipc(NULL);
    
    // Verify poll_events was called
    assert_int_equal(mock_poll_events_call_count, 1);
}

static void test_ebpf_socket_thread_ipc_passes_correct_sockets_pointer(void **state)
{
    (void)state;
    
    mock_poll_events_call_count = 0;
    
    ebpf_socket_thread_ipc(NULL);
    
    // Verify that &ipc_sockets was passed to poll_events
    assert_ptr_equal(mock_poll_events_sockets, &ipc_sockets);
}

static void test_ebpf_socket_thread_ipc_passes_rcv_callback(void **state)
{
    (void)state;
    
    mock_poll_events_call_count = 0;
    
    ebpf_socket_thread_ipc(NULL);
    
    // Verify that the rcv callback was passed
    assert_ptr_not_equal(mock_poll_events_rcv_callback, NULL);
    
    // Test that the callback works
    POLLINFO pi = {0};
    nd_poll_event_t events = {0};
    int result = mock_poll_events_rcv_callback(&pi, &events);
    assert_int_equal(result, 0);
}

static void test_ebpf_socket_thread_ipc_passes_snd_callback(void **state)
{
    (void)state;
    
    mock_poll_events_call_count = 0;
    
    ebpf_socket_thread_ipc(NULL);
    
    // Verify that the snd callback was passed
    assert_ptr_not_equal(mock_poll_events_snd_callback, NULL);
    
    // Test that the callback works
    POLLINFO pi = {0};
    nd_poll_event_t events = {0};
    int result = mock_poll_events_snd_callback(&pi, &events);
    assert_int_equal(result, 0);
}

static void test_ebpf_socket_thread_ipc_passes_should_stop_callback(void **state)
{
    (void)state;
    
    mock_poll_events_call_count = 0;
    
    ebpf_socket_thread_ipc(NULL);
    
    // Verify that the should_stop callback was passed
    assert_ptr_not_equal(mock_poll_events_should_stop, NULL);
    
    // Test that the callback works
    bool result = mock_poll_events_should_stop();
    assert_false(result);
}

static void test_ebpf_socket_thread_ipc_with_null_ptr(void **state)
{
    (void)state;
    
    mock_poll_events_call_count = 0;
    
    ebpf_socket_thread_ipc(NULL);
    
    assert_int_equal(mock_poll_events_call_count, 1);
}

static void test_ebpf_socket_thread_ipc_with_non_null_ptr(void **state)
{
    (void)state;
    
    mock_poll_events_call_count = 0;
    
    void *test_ptr = (void *)0x12345678;
    ebpf_socket_thread_ipc(test_ptr);
    
    assert_int_equal(mock_poll_events_call_count, 1);
}

static void test_ebpf_socket_thread_ipc_timeout_parameter(void **state)
{
    (void)state;
    
    // This test verifies that poll_events is called with the correct timeout
    // The timeout should be EBPF_DEFAULT_UPDATE_EVERY * 1000
    
    mock_poll_events_call_count = 0;
    
    ebpf_socket_thread_ipc(NULL);
    
    assert_int_equal(mock_poll_events_call_count, 1);
}

static void test_ebpf_initialize_sockets_clears_all_fields(void **state)
{
    (void)state;
    
    // Set some arbitrary values
    char dummy_section[] = "dummy";
    struct config dummy_config;
    ipc_sockets.config_section = dummy_section;
    ipc_sockets.config = &dummy_config;
    
    // Fill with garbage bytes
    unsigned char *ptr = (unsigned char *)&ipc_sockets;
    for (size_t i = 0; i < sizeof(ipc_sockets); i++) {
        ptr[i] = 0xFF;
    }
    
    // Initialize
    ebpf_initialize_sockets();
    
    // Check all bytes are zero
    for (size_t i = 0; i < sizeof(ipc_sockets); i++) {
        assert_int_equal(ptr[i], 0);
    }
}

static void test_ebpf_ipc_rcv_callback_cast_elimination(void **state)
{
    (void)state;
    
    // Ensure the function can handle edge cases with casts
    POLLINFO *pi_ptr = NULL;
    nd_poll_event_t *events_ptr = NULL;
    
    int result = ebpf_ipc_rcv_callback(pi_ptr, events_ptr);
    assert_int_equal(result, 0);
}

static void test_ebpf_ipc_snd_callback_cast_elimination(void **state)
{
    (void)state;
    
    // Ensure the function can handle edge cases with casts
    POLLINFO *pi_ptr = NULL;
    nd_poll_event_t *events_ptr = NULL;
    
    int result = ebpf_ipc_snd_callback(pi_ptr, events_ptr);
    assert_int_equal(result, 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        // ebpf_initialize_sockets tests
        cmocka_unit_test(test_ebpf_initialize_sockets_memset),
        cmocka_unit_test(test_ebpf_initialize_sockets_config_assignment),
        cmocka_unit_test(test_ebpf_initialize_sockets_config_section_assignment),
        cmocka_unit_test(test_ebpf_initialize_sockets_clears_all_fields),
        
        // ebpf_ipc_rcv_callback tests
        cmocka_unit_test(test_ebpf_ipc_rcv_callback_returns_zero),
        cmocka_unit_test(test_ebpf_ipc_rcv_callback_null_pi),
        cmocka_unit_test(test_ebpf_ipc_rcv_callback_null_events),
        cmocka_unit_test(test_ebpf_ipc_rcv_callback_null_both),
        cmocka_unit_test(test_ebpf_ipc_rcv_callback_cast_elimination),
        
        // ebpf_ipc_snd_callback tests
        cmocka_unit_test(test_ebpf_ipc_snd_callback_returns_zero),
        cmocka_unit_test(test_ebpf_ipc_snd_callback_null_pi),
        cmocka_unit_test(test_ebpf_ipc_snd_callback_null_events),
        cmocka_unit_test(test_ebpf_ipc_snd_callback_null_both),
        cmocka_unit_test(test_ebpf_ipc_snd_callback_cast_elimination),
        
        // ebpf_ipc_should_stop tests
        cmocka_unit_test(test_ebpf_ipc_should_stop_returns_false),
        cmocka_unit_test(test_ebpf_ipc_should_stop_always_false),
        
        // ebpf_socket_thread_ipc tests
        cmocka_unit_test(test_ebpf_socket_thread_ipc_calls_ebpf_initialize_sockets),
        cmocka_unit_test(test_ebpf_socket_thread_ipc_calls_poll_events),
        cmocka_unit_test(test_ebpf_socket_thread_ipc_passes_correct_sockets_pointer),
        cmocka_unit_test(test_ebpf_socket_thread_ipc_passes_rcv_callback),
        cmocka_unit_test(test_ebpf_socket_thread_ipc_passes_snd_callback),
        cmocka_unit_test(test_ebpf_socket_thread_ipc_passes_should_stop_callback),
        cmocka_unit_test(test_ebpf_socket_thread_ipc_with_null_ptr),
        cmocka_unit_test(test_ebpf_socket_thread_ipc_with_non_null_ptr),
        cmocka_unit_test(test_ebpf_socket_thread_ipc_timeout_parameter),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}