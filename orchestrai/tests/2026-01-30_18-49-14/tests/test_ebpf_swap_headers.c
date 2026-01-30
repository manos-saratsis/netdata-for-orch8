#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Test ebpf_swap.h macros and constants

// Test: NETDATA_EBPF_MODULE_NAME_SWAP macro
static void test_module_name_swap_macro(void **state) {
    (void)state;
    
    const char *expected = "swap";
    assert_string_equal(NETDATA_EBPF_MODULE_NAME_SWAP, expected);
}

// Test: NETDATA_SWAP_SLEEP_MS macro
static void test_swap_sleep_ms_macro(void **state) {
    (void)state;
    
    assert_int_equal(NETDATA_SWAP_SLEEP_MS, 850000ULL);
}

// Test: NETDATA_MEM_SWAP_CHART macro
static void test_mem_swap_chart_macro(void **state) {
    (void)state;
    
    const char *expected = "swapcalls";
    assert_string_equal(NETDATA_MEM_SWAP_CHART, expected);
}

// Test: NETDATA_MEM_SWAP_READ_CHART macro
static void test_mem_swap_read_chart_macro(void **state) {
    (void)state;
    
    const char *expected = "swap_read_call";
    assert_string_equal(NETDATA_MEM_SWAP_READ_CHART, expected);
}

// Test: NETDATA_MEM_SWAP_WRITE_CHART macro
static void test_mem_swap_write_chart_macro(void **state) {
    (void)state;
    
    const char *expected = "swap_write_call";
    assert_string_equal(NETDATA_MEM_SWAP_WRITE_CHART, expected);
}

// Test: NETDATA_SWAP_SUBMENU macro
static void test_swap_submenu_macro(void **state) {
    (void)state;
    
    const char *expected = "swap";
    assert_string_equal(NETDATA_SWAP_SUBMENU, expected);
}

// Test: NETDATA_DIRECTORY_SWAP_CONFIG_FILE macro
static void test_directory_swap_config_file_macro(void **state) {
    (void)state;
    
    const char *expected = "swap.conf";
    assert_string_equal(NETDATA_DIRECTORY_SWAP_CONFIG_FILE, expected);
}

// Test: NETDATA_CGROUP_SWAP_READ_CONTEXT macro
static void test_cgroup_swap_read_context_macro(void **state) {
    (void)state;
    
    const char *expected = "cgroup.swap_read";
    assert_string_equal(NETDATA_CGROUP_SWAP_READ_CONTEXT, expected);
}

// Test: NETDATA_CGROUP_SWAP_WRITE_CONTEXT macro
static void test_cgroup_swap_write_context_macro(void **state) {
    (void)state;
    
    const char *expected = "cgroup.swap_write";
    assert_string_equal(NETDATA_CGROUP_SWAP_WRITE_CONTEXT, expected);
}

// Test: NETDATA_SYSTEMD_SWAP_READ_CONTEXT macro
static void test_systemd_swap_read_context_macro(void **state) {
    (void)state;
    
    const char *expected = "systemd.service.swap_read";
    assert_string_equal(NETDATA_SYSTEMD_SWAP_READ_CONTEXT, expected);
}

// Test: NETDATA_SYSTEMD_SWAP_WRITE_CONTEXT macro
static void test_systemd_swap_write_context_macro(void **state) {
    (void)state;
    
    const char *expected = "systemd.service.swap_write";
    assert_string_equal(NETDATA_SYSTEMD_SWAP_WRITE_CONTEXT, expected);
}

// Test: enum swap_tables values
static void test_enum_swap_tables_values(void **state) {
    (void)state;
    
    assert_int_equal(NETDATA_PID_SWAP_TABLE, 0);
    assert_int_equal(NETDATA_SWAP_CONTROLLER, 1);
    assert_int_equal(NETDATA_SWAP_GLOBAL_TABLE, 2);
}

// Test: enum swap_counters values
static void test_enum_swap_counters_values(void **state) {
    (void)state;
    
    assert_int_equal(NETDATA_KEY_SWAP_READPAGE_CALL, 0);
    assert_int_equal(NETDATA_KEY_SWAP_WRITEPAGE_CALL, 1);
    assert_int_equal(NETDATA_SWAP_END, 2);
}

// Test: Multiple macro references
static void test_multiple_macro_references(void **state) {
    (void)state;
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s:%s", NETDATA_MEM_SWAP_CHART, NETDATA_SWAP_SUBMENU);
    
    assert_string_equal(buffer, "swapcalls:swap");
}

// Test: Chart context combinations
static void test_chart_context_combinations(void **state) {
    (void)state;
    
    // Verify all context strings are unique
    assert_string_not_equal(NETDATA_CGROUP_SWAP_READ_CONTEXT, NETDATA_CGROUP_SWAP_WRITE_CONTEXT);
    assert_string_not_equal(NETDATA_CGROUP_SWAP_READ_CONTEXT, NETDATA_SYSTEMD_SWAP_READ_CONTEXT);
    assert_string_not_equal(NETDATA_SYSTEMD_SWAP_READ_CONTEXT, NETDATA_SYSTEMD_SWAP_WRITE_CONTEXT);
}

// Test: Enum sequential values
static void test_enum_sequential_values(void **state) {
    (void)state;
    
    // Ensure enum values are sequential for proper array indexing
    assert_int_equal(NETDATA_KEY_SWAP_READPAGE_CALL + 1, NETDATA_KEY_SWAP_WRITEPAGE_CALL);
    assert_int_equal(NETDATA_KEY_SWAP_WRITEPAGE_CALL + 1, NETDATA_SWAP_END);
}

// Test: Config file name validity
static void test_config_file_name_validity(void **state) {
    (void)state;
    
    const char *config = NETDATA_DIRECTORY_SWAP_CONFIG_FILE;
    assert_true(strlen(config) > 0);
    assert_string_equal(config, "swap.conf");
    assert_true(strchr(config, '.') != NULL);
}

// Test: Module name and description separation
static void test_module_description_macro(void **state) {
    (void)state;
    
    const char *desc = NETDATA_EBPF_SWAP_MODULE_DESC;
    assert_true(strlen(desc) > 0);
    assert_true(strstr(desc, "swap") != NULL);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_module_name_swap_macro),
        cmocka_unit_test(test_swap_sleep_ms_macro),
        cmocka_unit_test(test_mem_swap_chart_macro),
        cmocka_unit_test(test_mem_swap_read_chart_macro),
        cmocka_unit_test(test_mem_swap_write_chart_macro),
        cmocka_unit_test(test_swap_submenu_macro),
        cmocka_unit_test(test_directory_swap_config_file_macro),
        cmocka_unit_test(test_cgroup_swap_read_context_macro),
        cmocka_unit_test(test_cgroup_swap_write_context_macro),
        cmocka_unit_test(test_systemd_swap_read_context_macro),
        cmocka_unit_test(test_systemd_swap_write_context_macro),
        cmocka_unit_test(test_enum_swap_tables_values),
        cmocka_unit_test(test_enum_swap_counters_values),
        cmocka_unit_test(test_multiple_macro_references),
        cmocka_unit_test(test_chart_context_combinations),
        cmocka_unit_test(test_enum_sequential_values),
        cmocka_unit_test(test_config_file_name_validity),
        cmocka_unit_test(test_module_description_macro),
    };

    return cmocka_run_tests(tests);
}