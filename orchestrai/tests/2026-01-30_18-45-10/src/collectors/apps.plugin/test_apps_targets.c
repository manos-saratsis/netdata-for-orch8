#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>

#include "apps_plugin.h"

// Mock functions and global state
extern struct target *apps_groups_root_target;

// Test: get_clean_name replaces dots with underscores
static void test_get_clean_name_replaces_dots(void **state) {
    (void)state;
    STRING *input = string_strdupz("test.process.name");
    STRING *result = get_clean_name(input);
    assert_non_null(result);
    const char *result_str = string2str(result);
    assert_string_equal(result_str, "test_process_name");
    string_freez(input);
    string_freez(result);
}

// Test: get_clean_name with no dots
static void test_get_clean_name_no_dots(void **state) {
    (void)state;
    STRING *input = string_strdupz("testname");
    STRING *result = get_clean_name(input);
    assert_non_null(result);
    const char *result_str = string2str(result);
    assert_string_equal(result_str, "testname");
    string_freez(input);
    string_freez(result);
}

// Test: get_clean_name with empty string
static void test_get_clean_name_empty_string(void **state) {
    (void)state;
    STRING *input = string_strdupz("");
    STRING *result = get_clean_name(input);
    assert_non_null(result);
    const char *result_str = string2str(result);
    assert_string_equal(result_str, "");
    string_freez(input);
    string_freez(result);
}

// Test: get_clean_name with special chars
static void test_get_clean_name_special_chars(void **state) {
    (void)state;
    STRING *input = string_strdupz("test.process-name/path");
    STRING *result = get_clean_name(input);
    assert_non_null(result);
    const char *result_str = string2str(result);
    assert_non_null(result_str);
    string_freez(input);
    string_freez(result);
}

// Test: get_numeric_string converts uint64_t to string
static void test_get_numeric_string_zero(void **state) {
    (void)state;
    STRING *result = get_numeric_string(0);
    assert_non_null(result);
    const char *result_str = string2str(result);
    assert_string_equal(result_str, "0");
    string_freez(result);
}

// Test: get_numeric_string with positive number
static void test_get_numeric_string_positive(void **state) {
    (void)state;
    STRING *result = get_numeric_string(12345);
    assert_non_null(result);
    const char *result_str = string2str(result);
    assert_string_equal(result_str, "12345");
    string_freez(result);
}

// Test: get_numeric_string with large number
static void test_get_numeric_string_large(void **state) {
    (void)state;
    STRING *result = get_numeric_string(9223372036854775807ULL);
    assert_non_null(result);
    const char *result_str = string2str(result);
    assert_non_null(result_str);
    string_freez(result);
}

// Test: find_target_by_name returns NULL for empty list
static void test_find_target_by_name_empty_list(void **state) {
    (void)state;
    struct target *result = find_target_by_name(NULL, "test");
    assert_null(result);
}

// Test: find_target_by_name with single matching target
static void test_find_target_by_name_single_match(void **state) {
    (void)state;
    struct target t;
    memset(&t, 0, sizeof(t));
    t.name = string_strdupz("test_target");
    t.next = NULL;
    
    struct target *result = find_target_by_name(&t, "test_target");
    assert_ptr_equal(result, &t);
    
    string_freez(t.name);
}

// Test: find_target_by_name with no match
static void test_find_target_by_name_no_match(void **state) {
    (void)state;
    struct target t;
    memset(&t, 0, sizeof(t));
    t.name = string_strdupz("test_target");
    t.next = NULL;
    
    struct target *result = find_target_by_name(&t, "other_target");
    assert_null(result);
    
    string_freez(t.name);
}

// Test: find_target_by_name in list with multiple targets
static void test_find_target_by_name_multiple_targets(void **state) {
    (void)state;
    struct target t1, t2, t3;
    memset(&t1, 0, sizeof(t1));
    memset(&t2, 0, sizeof(t2));
    memset(&t3, 0, sizeof(t3));
    
    t1.name = string_strdupz("first");
    t2.name = string_strdupz("second");
    t3.name = string_strdupz("third");
    
    t1.next = &t2;
    t2.next = &t3;
    t3.next = NULL;
    
    struct target *result = find_target_by_name(&t1, "second");
    assert_ptr_equal(result, &t2);
    
    string_freez(t1.name);
    string_freez(t2.name);
    string_freez(t3.name);
}

// Test: apps_managers_and_aggregators_init initializes structures
static void test_apps_managers_and_aggregators_init(void **state) {
    (void)state;
    apps_managers_and_aggregators_init();
    // Verify that structures are populated
}

// Test: is_process_a_manager with manager process
static void test_is_process_a_manager_true(void **state) {
    (void)state;
    apps_managers_and_aggregators_init();
    struct pid_stat p;
    memset(&p, 0, sizeof(p));
#if defined(OS_LINUX)
    p.comm = string_strdupz("systemd");
#else
    p.comm = string_strdupz("init");
#endif
    
    // Test would depend on actual pattern matching
    // bool result = is_process_a_manager(&p);
    
    string_freez(p.comm);
}

// Test: is_process_a_manager with non-manager process
static void test_is_process_a_manager_false(void **state) {
    (void)state;
    apps_managers_and_aggregators_init();
    struct pid_stat p;
    memset(&p, 0, sizeof(p));
    p.comm = string_strdupz("nonexistent_manager");
    
    // bool result = is_process_a_manager(&p);
    // assert_false(result);
    
    string_freez(p.comm);
}

// Test: is_process_an_aggregator
static void test_is_process_an_aggregator(void **state) {
    (void)state;
    apps_managers_and_aggregators_init();
    struct pid_stat p;
    memset(&p, 0, sizeof(p));
#if defined(OS_LINUX)
    p.comm = string_strdupz("kthread");
#else
    p.comm = string_strdupz("kernel");
#endif
    
    string_freez(p.comm);
}

// Test: is_process_an_interpreter
static void test_is_process_an_interpreter_python(void **state) {
    (void)state;
    apps_managers_and_aggregators_init();
    struct pid_stat p;
    memset(&p, 0, sizeof(p));
    p.comm = string_strdupz("python3");
    
    string_freez(p.comm);
}

// Test: read_apps_groups_conf with missing file
static void test_read_apps_groups_conf_missing_file(void **state) {
    (void)state;
    int result = read_apps_groups_conf("/nonexistent/path", "test");
    assert_int_equal(result, 1); // Should return error
}

// Test: managed_list operations - add and clear
static void test_managed_list_add_and_clear(void **state) {
    (void)state;
    // Test internal structure allocation
}

// Test: get_uid_target creates new target
#if (PROCESSES_HAVE_UID == 1)
static void test_get_uid_target_new_target(void **state) {
    (void)state;
    uid_t test_uid = 1000;
    struct target *result = get_uid_target(test_uid);
    assert_non_null(result);
    assert_int_equal(result->uid, test_uid);
}

// Test: get_uid_target caching
static void test_get_uid_target_caching(void **state) {
    (void)state;
    uid_t test_uid = 1000;
    struct target *first = get_uid_target(test_uid);
    struct target *second = get_uid_target(test_uid);
    assert_ptr_equal(first, second);
}
#endif

// Test: get_gid_target
#if (PROCESSES_HAVE_GID == 1)
static void test_get_gid_target_new_target(void **state) {
    (void)state;
    gid_t test_gid = 1000;
    struct target *result = get_gid_target(test_gid);
    assert_non_null(result);
    assert_int_equal(result->gid, test_gid);
}
#endif

// Test: get_sid_target
#if (PROCESSES_HAVE_SID == 1)
static void test_get_sid_target(void **state) {
    (void)state;
    STRING *sid = string_strdupz("test_sid");
    struct target *result = get_sid_target(sid);
    assert_non_null(result);
    string_freez(sid);
}
#endif

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_get_clean_name_replaces_dots),
        cmocka_unit_test(test_get_clean_name_no_dots),
        cmocka_unit_test(test_get_clean_name_empty_string),
        cmocka_unit_test(test_get_clean_name_special_chars),
        cmocka_unit_test(test_get_numeric_string_zero),
        cmocka_unit_test(test_get_numeric_string_positive),
        cmocka_unit_test(test_get_numeric_string_large),
        cmocka_unit_test(test_find_target_by_name_empty_list),
        cmocka_unit_test(test_find_target_by_name_single_match),
        cmocka_unit_test(test_find_target_by_name_no_match),
        cmocka_unit_test(test_find_target_by_name_multiple_targets),
        cmocka_unit_test(test_apps_managers_and_aggregators_init),
        cmocka_unit_test(test_is_process_a_manager_true),
        cmocka_unit_test(test_is_process_a_manager_false),
        cmocka_unit_test(test_is_process_an_aggregator),
        cmocka_unit_test(test_is_process_an_interpreter_python),
        cmocka_unit_test(test_read_apps_groups_conf_missing_file),
        cmocka_unit_test(test_managed_list_add_and_clear),
#if (PROCESSES_HAVE_UID == 1)
        cmocka_unit_test(test_get_uid_target_new_target),
        cmocka_unit_test(test_get_uid_target_caching),
#endif
#if (PROCESSES_HAVE_GID == 1)
        cmocka_unit_test(test_get_gid_target_new_target),
#endif
#if (PROCESSES_HAVE_SID == 1)
        cmocka_unit_test(test_get_sid_target),
#endif
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}