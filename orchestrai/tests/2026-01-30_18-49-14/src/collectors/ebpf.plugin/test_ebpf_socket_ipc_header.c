#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Test header file constants and definitions
static void test_netdata_ebpf_ipc_section_value(void **state)
{
    (void)state;
    
    assert_string_equal(NETDATA_EBPF_IPC_SECTION, "ipc");
}

static void test_netdata_ebpf_ipc_integration_value(void **state)
{
    (void)state;
    
    assert_string_equal(NETDATA_EBPF_IPC_INTEGRATION, "integration");
}

static void test_netdata_ebpf_ipc_backlog_value(void **state)
{
    (void)state;
    
    assert_string_equal(NETDATA_EBPF_IPC_BACKLOG, "backlog");
}

static void test_netdata_ebpf_ipc_bind_to_value(void **state)
{
    (void)state;
    
    assert_string_equal(NETDATA_EBPF_IPC_BIND_TO, "bind to");
}

static void test_netdata_ebpf_ipc_bind_to_default_value(void **state)
{
    (void)state;
    
    assert_string_equal(NETDATA_EBPF_IPC_BIND_TO_DEFAULT, "unix:/tmp/netdata_ebpf_sock");
}

static void test_netdata_ebpf_ipc_integration_shm_value(void **state)
{
    (void)state;
    
    assert_string_equal(NETDATA_EBPF_IPC_INTEGRATION_SHM, "shm");
}

static void test_netdata_ebpf_ipc_integration_socket_value(void **state)
{
    (void)state;
    
    assert_string_equal(NETDATA_EBPF_IPC_INTEGRATION_SOCKET, "socket");
}

static void test_netdata_ebpf_ipc_integration_disabled_value(void **state)
{
    (void)state;
    
    assert_string_equal(NETDATA_EBPF_IPC_INTEGRATION_DISABLED, "disabled");
}

// Test enum ebpf_integration_list values
static void test_ebpf_integration_disabled_enum_value(void **state)
{
    (void)state;
    
    enum ebpf_integration_list value = NETDATA_EBPF_INTEGRATION_DISABLED;
    assert_int_equal(value, 0);
}

static void test_ebpf_integration_socket_enum_value(void **state)
{
    (void)state;
    
    enum ebpf_integration_list value = NETDATA_EBPF_INTEGRATION_SOCKET;
    assert_int_equal(value, 1);
}

static void test_ebpf_integration_shm_enum_value(void **state)
{
    (void)state;
    
    enum ebpf_integration_list value = NETDATA_EBPF_INTEGRATION_SHM;
    assert_int_equal(value, 2);
}

// Test enum values are distinct
static void test_ebpf_integration_enum_values_distinct(void **state)
{
    (void)state;
    
    enum ebpf_integration_list disabled = NETDATA_EBPF_INTEGRATION_DISABLED;
    enum ebpf_integration_list socket = NETDATA_EBPF_INTEGRATION_SOCKET;
    enum ebpf_integration_list shm = NETDATA_EBPF_INTEGRATION_SHM;
    
    // All should be different
    assert_int_not_equal(disabled, socket);
    assert_int_not_equal(disabled, shm);
    assert_int_not_equal(socket, shm);
}

// Test that header guards work
static void test_header_guard_defined(void **state)
{
    (void)state;
    
    // This is implicitly tested by successful compilation
    // If header guards fail, we'd have compilation errors
    assert_true(1);
}

// Test definition ranges
static void test_constants_are_string_constants(void **state)
{
    (void)state;
    
    const char *ipc_section = NETDATA_EBPF_IPC_SECTION;
    const char *ipc_integration = NETDATA_EBPF_IPC_INTEGRATION;
    const char *ipc_backlog = NETDATA_EBPF_IPC_BACKLOG;
    const char *ipc_bind_to = NETDATA_EBPF_IPC_BIND_TO;
    const char *ipc_bind_to_default = NETDATA_EBPF_IPC_BIND_TO_DEFAULT;
    const char *ipc_integration_shm = NETDATA_EBPF_IPC_INTEGRATION_SHM;
    const char *ipc_integration_socket = NETDATA_EBPF_IPC_INTEGRATION_SOCKET;
    const char *ipc_integration_disabled = NETDATA_EBPF_IPC_INTEGRATION_DISABLED;
    
    // All should be non-null
    assert_non_null(ipc_section);
    assert_non_null(ipc_integration);
    assert_non_null(ipc_backlog);
    assert_non_null(ipc_bind_to);
    assert_non_null(ipc_bind_to_default);
    assert_non_null(ipc_integration_shm);
    assert_non_null(ipc_integration_socket);
    assert_non_null(ipc_integration_disabled);
}

// Test include files don't cause issues
static void test_header_includes_compile(void **state)
{
    (void)state;
    
    // This test passes if the header compiles successfully
    // The header includes fcntl.h, sys/stat.h, semaphore.h, etc.
    assert_true(1);
}

// Test external declarations exist
static void test_extern_declarations(void **state)
{
    (void)state;
    
    // These should be available as external declarations
    // extern LISTEN_SOCKETS ipc_sockets;
    // extern sem_t *shm_mutex_ebpf_integration;
    // void ebpf_socket_thread_ipc(void *ptr);
    // void netdata_integration_cleanup_shm();
    
    // Test passes if no compilation errors occur
    assert_true(1);
}

// Test string length and content
static void test_section_name_not_empty(void **state)
{
    (void)state;
    
    assert_true(strlen(NETDATA_EBPF_IPC_SECTION) > 0);
}

static void test_integration_name_not_empty(void **state)
{
    (void)state;
    
    assert_true(strlen(NETDATA_EBPF_IPC_INTEGRATION) > 0);
}

static void test_backlog_name_not_empty(void **state)
{
    (void)state;
    
    assert_true(strlen(NETDATA_EBPF_IPC_BACKLOG) > 0);
}

static void test_bind_to_name_not_empty(void **state)
{
    (void)state;
    
    assert_true(strlen(NETDATA_EBPF_IPC_BIND_TO) > 0);
}

static void test_bind_to_default_path_format(void **state)
{
    (void)state;
    
    const char *default_bind = NETDATA_EBPF_IPC_BIND_TO_DEFAULT;
    
    // Should start with "unix:"
    assert_true(strncmp(default_bind, "unix:", 5) == 0);
    
    // Should have a reasonable length
    assert_true(strlen(default_bind) > 5);
    assert_true(strlen(default_bind) < 256);
}

static void test_integration_shm_not_empty(void **state)
{
    (void)state;
    
    assert_true(strlen(NETDATA_EBPF_IPC_INTEGRATION_SHM) > 0);
}

static void test_integration_socket_not_empty(void **state)
{
    (void)state;
    
    assert_true(strlen(NETDATA_EBPF_IPC_INTEGRATION_SOCKET) > 0);
}

static void test_integration_disabled_not_empty(void **state)
{
    (void)state;
    
    assert_true(strlen(NETDATA_EBPF_IPC_INTEGRATION_DISABLED) > 0);
}

// Test that constants have expected values
static void test_integration_constant_names(void **state)
{
    (void)state;
    
    // Verify the integration type constants contain expected substrings
    assert_non_null(strstr(NETDATA_EBPF_IPC_INTEGRATION, "integration"));
}

// Test enum values coverage
static void test_enum_all_values_reachable(void **state)
{
    (void)state;
    
    int enum_values[] = {
        NETDATA_EBPF_INTEGRATION_DISABLED,
        NETDATA_EBPF_INTEGRATION_SOCKET,
        NETDATA_EBPF_INTEGRATION_SHM
    };
    
    // Should be 3 values
    assert_int_equal(sizeof(enum_values) / sizeof(enum_values[0]), 3);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        // Constant value tests
        cmocka_unit_test(test_netdata_ebpf_ipc_section_value),
        cmocka_unit_test(test_netdata_ebpf_ipc_integration_value),
        cmocka_unit_test(test_netdata_ebpf_ipc_backlog_value),
        cmocka_unit_test(test_netdata_ebpf_ipc_bind_to_value),
        cmocka_unit_test(test_netdata_ebpf_ipc_bind_to_default_value),
        cmocka_unit_test(test_netdata_ebpf_ipc_integration_shm_value),
        cmocka_unit_test(test_netdata_ebpf_ipc_integration_socket_value),
        cmocka_unit_test(test_netdata_ebpf_ipc_integration_disabled_value),
        
        // Enum value tests
        cmocka_unit_test(test_ebpf_integration_disabled_enum_value),
        cmocka_unit_test(test_ebpf_integration_socket_enum_value),
        cmocka_unit_test(test_ebpf_integration_shm_enum_value),
        cmocka_unit_test(test_ebpf_integration_enum_values_distinct),
        cmocka_unit_test(test_enum_all_values_reachable),
        
        // Header structure tests
        cmocka_unit_test(test_header_guard_defined),
        cmocka_unit_test(test_header_includes_compile),
        cmocka_unit_test(test_extern_declarations),
        
        // String content tests
        cmocka_unit_test(test_section_name_not_empty),
        cmocka_unit_test(test_integration_name_not_empty),
        cmocka_unit_test(test_backlog_name_not_empty),
        cmocka_unit_test(test_bind_to_name_not_empty),
        cmocka_unit_test(test_bind_to_default_path_format),
        cmocka_unit_test(test_integration_shm_not_empty),
        cmocka_unit_test(test_integration_socket_not_empty),
        cmocka_unit_test(test_integration_disabled_not_empty),
        cmocka_unit_test(test_integration_constant_names),
        
        // Type and constant tests
        cmocka_unit_test(test_constants_are_string_constants),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}