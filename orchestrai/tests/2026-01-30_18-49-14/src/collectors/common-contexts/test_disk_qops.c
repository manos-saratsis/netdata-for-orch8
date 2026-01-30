#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Mock declarations for Netdata structures and functions
typedef struct {
    void *data;
} RRDSET;

typedef struct {
    void *data;
} RRDDIM;

typedef long long collected_number;

typedef enum {
    RRD_ALGORITHM_ABSOLUTE = 1,
} RRD_ALGORITHM;

typedef enum {
    RRDSET_TYPE_LINE = 0,
} RRDSET_TYPE;

typedef void (*instance_labels_cb_t)(RRDSET *st, void *data);

#define _COMMON_PLUGIN_NAME "test_plugin"
#define _COMMON_PLUGIN_MODULE_NAME "test_module"
#define NETDATA_CHART_PRIO_DISK_QOPS 100
#define unlikely(x) (x)

// Mock functions
RRDSET *__wrap_rrdset_create_localhost(
    const char *type,
    const char *id,
    const char *name,
    const char *units,
    const char *family,
    const char *title,
    const char *context,
    const char *plugin,
    const char *module,
    int priority,
    int update_every,
    int chart_type
) {
    check_expected(type);
    check_expected(id);
    check_expected(name);
    check_expected(units);
    check_expected(family);
    check_expected(title);
    check_expected(context);
    check_expected_ptr(plugin);
    check_expected_ptr(module);
    check_expected(priority);
    check_expected(update_every);
    check_expected(chart_type);
    return mock_ptr_type(RRDSET *);
}

RRDDIM *__wrap_rrddim_add(
    RRDSET *st,
    const char *name,
    const char *divisor,
    long multiplier,
    long divisor_num,
    int algorithm
) {
    check_expected_ptr(st);
    check_expected(name);
    check_expected(multiplier);
    check_expected(divisor_num);
    check_expected(algorithm);
    return mock_ptr_type(RRDDIM *);
}

void __wrap_rrddim_set_by_pointer(
    RRDSET *st,
    RRDDIM *rd,
    collected_number value
) {
    check_expected_ptr(st);
    check_expected_ptr(rd);
    check_expected(value);
}

void __wrap_rrdset_done(RRDSET *st) {
    check_expected_ptr(st);
}

#define rrdset_create_localhost __wrap_rrdset_create_localhost
#define rrddim_add __wrap_rrddim_add
#define rrddim_set_by_pointer __wrap_rrddim_set_by_pointer
#define rrdset_done __wrap_rrdset_done

#include "disk-qops.h"

// Test: First call with NULL st_qops creates new rrdset and rrddim
static void test_disk_qops_first_call_creates_rrdset(void **state) {
    ND_DISK_QOPS d = { .st_qops = NULL, .rd_qops = NULL };
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    
    expect_string(__wrap_rrdset_create_localhost, type, "disk_qops");
    expect_string(__wrap_rrdset_create_localhost, id, "test_id");
    expect_string(__wrap_rrdset_create_localhost, name, "test_name");
    expect_string(__wrap_rrdset_create_localhost, units, "ops");
    expect_string(__wrap_rrdset_create_localhost, family, "disk.qops");
    expect_string(__wrap_rrdset_create_localhost, title, "Disk Current I/O Operations");
    expect_string(__wrap_rrdset_create_localhost, context, "operations");
    expect_string(__wrap_rrdset_create_localhost, plugin, _COMMON_PLUGIN_NAME);
    expect_string(__wrap_rrdset_create_localhost, module, _COMMON_PLUGIN_MODULE_NAME);
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_QOPS);
    expect_value(__wrap_rrdset_create_localhost, update_every, 10);
    expect_value(__wrap_rrdset_create_localhost, chart_type, RRDSET_TYPE_LINE);
    will_return(__wrap_rrdset_create_localhost, &mock_st);
    
    expect_value(__wrap_rrddim_add, st, &mock_st);
    expect_string(__wrap_rrddim_add, name, "operations");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_num, 1);
    expect_value(__wrap_rrddim_add, algorithm, RRD_ALGORITHM_ABSOLUTE);
    will_return(__wrap_rrddim_add, &mock_rd);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 100);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "test_id", "test_name", 100, 10, NULL, NULL);
    
    assert_ptr_equal(d.st_qops, &mock_st);
    assert_ptr_equal(d.rd_qops, &mock_rd);
}

// Test: First call with callback invokes callback
static void test_disk_qops_first_call_with_callback(void **state) {
    ND_DISK_QOPS d = { .st_qops = NULL, .rd_qops = NULL };
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    int cb_called = 0;
    
    expect_string(__wrap_rrdset_create_localhost, type, "disk_qops");
    expect_string(__wrap_rrdset_create_localhost, id, "id1");
    expect_string(__wrap_rrdset_create_localhost, name, "name1");
    expect_string(__wrap_rrdset_create_localhost, units, "ops");
    expect_string(__wrap_rrdset_create_localhost, family, "disk.qops");
    expect_string(__wrap_rrdset_create_localhost, title, "Disk Current I/O Operations");
    expect_string(__wrap_rrdset_create_localhost, context, "operations");
    expect_string(__wrap_rrdset_create_localhost, plugin, _COMMON_PLUGIN_NAME);
    expect_string(__wrap_rrdset_create_localhost, module, _COMMON_PLUGIN_MODULE_NAME);
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_QOPS);
    expect_value(__wrap_rrdset_create_localhost, update_every, 5);
    expect_value(__wrap_rrdset_create_localhost, chart_type, RRDSET_TYPE_LINE);
    will_return(__wrap_rrdset_create_localhost, &mock_st);
    
    expect_value(__wrap_rrddim_add, st, &mock_st);
    expect_string(__wrap_rrddim_add, name, "operations");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_num, 1);
    expect_value(__wrap_rrddim_add, algorithm, RRD_ALGORITHM_ABSOLUTE);
    will_return(__wrap_rrddim_add, &mock_rd);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 50);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    instance_labels_cb_t test_cb = (instance_labels_cb_t)&cb_called;
    common_disk_qops(&d, "id1", "name1", 50, 5, test_cb, &cb_called);
    
    assert_ptr_equal(d.st_qops, &mock_st);
    assert_ptr_equal(d.rd_qops, &mock_rd);
}

// Test: Subsequent call skips rrdset creation
static void test_disk_qops_subsequent_call_skips_creation(void **state) {
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    ND_DISK_QOPS d = { .st_qops = &mock_st, .rd_qops = &mock_rd };
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 200);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "test_id", "test_name", 200, 10, NULL, NULL);
    
    assert_ptr_equal(d.st_qops, &mock_st);
    assert_ptr_equal(d.rd_qops, &mock_rd);
}

// Test: Zero queued_ops value
static void test_disk_qops_zero_queued_ops(void **state) {
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    ND_DISK_QOPS d = { .st_qops = &mock_st, .rd_qops = &mock_rd };
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "test_id", "test_name", 0, 10, NULL, NULL);
}

// Test: Large queued_ops value
static void test_disk_qops_large_queued_ops(void **state) {
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    ND_DISK_QOPS d = { .st_qops = &mock_st, .rd_qops = &mock_rd };
    uint64_t large_value = UINT64_MAX / 2;
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, (collected_number)large_value);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "test_id", "test_name", large_value, 10, NULL, NULL);
}

// Test: Different update_every values
static void test_disk_qops_different_update_every(void **state) {
    ND_DISK_QOPS d = { .st_qops = NULL, .rd_qops = NULL };
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    
    expect_string(__wrap_rrdset_create_localhost, type, "disk_qops");
    expect_string(__wrap_rrdset_create_localhost, id, "test_id");
    expect_string(__wrap_rrdset_create_localhost, name, "test_name");
    expect_string(__wrap_rrdset_create_localhost, units, "ops");
    expect_string(__wrap_rrdset_create_localhost, family, "disk.qops");
    expect_string(__wrap_rrdset_create_localhost, title, "Disk Current I/O Operations");
    expect_string(__wrap_rrdset_create_localhost, context, "operations");
    expect_string(__wrap_rrdset_create_localhost, plugin, _COMMON_PLUGIN_NAME);
    expect_string(__wrap_rrdset_create_localhost, module, _COMMON_PLUGIN_MODULE_NAME);
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_QOPS);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, RRDSET_TYPE_LINE);
    will_return(__wrap_rrdset_create_localhost, &mock_st);
    
    expect_value(__wrap_rrddim_add, st, &mock_st);
    expect_string(__wrap_rrddim_add, name, "operations");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_num, 1);
    expect_value(__wrap_rrddim_add, algorithm, RRD_ALGORITHM_ABSOLUTE);
    will_return(__wrap_rrddim_add, &mock_rd);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 42);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "test_id", "test_name", 42, 1, NULL, NULL);
}

// Test: Empty id and name strings
static void test_disk_qops_empty_id_name(void **state) {
    ND_DISK_QOPS d = { .st_qops = NULL, .rd_qops = NULL };
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    
    expect_string(__wrap_rrdset_create_localhost, type, "disk_qops");
    expect_string(__wrap_rrdset_create_localhost, id, "");
    expect_string(__wrap_rrdset_create_localhost, name, "");
    expect_string(__wrap_rrdset_create_localhost, units, "ops");
    expect_string(__wrap_rrdset_create_localhost, family, "disk.qops");
    expect_string(__wrap_rrdset_create_localhost, title, "Disk Current I/O Operations");
    expect_string(__wrap_rrdset_create_localhost, context, "operations");
    expect_string(__wrap_rrdset_create_localhost, plugin, _COMMON_PLUGIN_NAME);
    expect_string(__wrap_rrdset_create_localhost, module, _COMMON_PLUGIN_MODULE_NAME);
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_QOPS);
    expect_value(__wrap_rrdset_create_localhost, update_every, 10);
    expect_value(__wrap_rrdset_create_localhost, chart_type, RRDSET_TYPE_LINE);
    will_return(__wrap_rrdset_create_localhost, &mock_st);
    
    expect_value(__wrap_rrddim_add, st, &mock_st);
    expect_string(__wrap_rrddim_add, name, "operations");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_num, 1);
    expect_value(__wrap_rrddim_add, algorithm, RRD_ALGORITHM_ABSOLUTE);
    will_return(__wrap_rrddim_add, &mock_rd);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "", "", 0, 10, NULL, NULL);
}

// Test: Special characters in id and name
static void test_disk_qops_special_chars_in_names(void **state) {
    ND_DISK_QOPS d = { .st_qops = NULL, .rd_qops = NULL };
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    
    expect_string(__wrap_rrdset_create_localhost, type, "disk_qops");
    expect_string(__wrap_rrdset_create_localhost, id, "disk-sda1");
    expect_string(__wrap_rrdset_create_localhost, name, "Samsung SSD 970");
    expect_string(__wrap_rrdset_create_localhost, units, "ops");
    expect_string(__wrap_rrdset_create_localhost, family, "disk.qops");
    expect_string(__wrap_rrdset_create_localhost, title, "Disk Current I/O Operations");
    expect_string(__wrap_rrdset_create_localhost, context, "operations");
    expect_string(__wrap_rrdset_create_localhost, plugin, _COMMON_PLUGIN_NAME);
    expect_string(__wrap_rrdset_create_localhost, module, _COMMON_PLUGIN_MODULE_NAME);
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_QOPS);
    expect_value(__wrap_rrdset_create_localhost, update_every, 10);
    expect_value(__wrap_rrdset_create_localhost, chart_type, RRDSET_TYPE_LINE);
    will_return(__wrap_rrdset_create_localhost, &mock_st);
    
    expect_value(__wrap_rrddim_add, st, &mock_st);
    expect_string(__wrap_rrddim_add, name, "operations");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_num, 1);
    expect_value(__wrap_rrddim_add, algorithm, RRD_ALGORITHM_ABSOLUTE);
    will_return(__wrap_rrddim_add, &mock_rd);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 75);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "disk-sda1", "Samsung SSD 970", 75, 10, NULL, NULL);
}

// Test: Multiple consecutive calls
static void test_disk_qops_multiple_consecutive_calls(void **state) {
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    ND_DISK_QOPS d = { .st_qops = &mock_st, .rd_qops = &mock_rd };
    
    // First call
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 10);
    expect_value(__wrap_rrdset_done, st, &mock_st);
    common_disk_qops(&d, "id", "name", 10, 10, NULL, NULL);
    
    // Second call
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 20);
    expect_value(__wrap_rrdset_done, st, &mock_st);
    common_disk_qops(&d, "id", "name", 20, 10, NULL, NULL);
    
    // Third call
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 30);
    expect_value(__wrap_rrdset_done, st, &mock_st);
    common_disk_qops(&d, "id", "name", 30, 10, NULL, NULL);
}

// Test: Maximum uint64_t value
static void test_disk_qops_max_uint64_value(void **state) {
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    ND_DISK_QOPS d = { .st_qops = &mock_st, .rd_qops = &mock_rd };
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, (collected_number)UINT64_MAX);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "test_id", "test_name", UINT64_MAX, 10, NULL, NULL);
}

// Test: Negative update_every (edge case)
static void test_disk_qops_negative_update_every(void **state) {
    ND_DISK_QOPS d = { .st_qops = NULL, .rd_qops = NULL };
    RRDSET mock_st = { 0 };
    RRDDIM mock_rd = { 0 };
    
    expect_string(__wrap_rrdset_create_localhost, type, "disk_qops");
    expect_string(__wrap_rrdset_create_localhost, id, "test_id");
    expect_string(__wrap_rrdset_create_localhost, name, "test_name");
    expect_string(__wrap_rrdset_create_localhost, units, "ops");
    expect_string(__wrap_rrdset_create_localhost, family, "disk.qops");
    expect_string(__wrap_rrdset_create_localhost, title, "Disk Current I/O Operations");
    expect_string(__wrap_rrdset_create_localhost, context, "operations");
    expect_string(__wrap_rrdset_create_localhost, plugin, _COMMON_PLUGIN_NAME);
    expect_string(__wrap_rrdset_create_localhost, module, _COMMON_PLUGIN_MODULE_NAME);
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_QOPS);
    expect_value(__wrap_rrdset_create_localhost, update_every, -1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, RRDSET_TYPE_LINE);
    will_return(__wrap_rrdset_create_localhost, &mock_st);
    
    expect_value(__wrap_rrddim_add, st, &mock_st);
    expect_string(__wrap_rrddim_add, name, "operations");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_num, 1);
    expect_value(__wrap_rrddim_add, algorithm, RRD_ALGORITHM_ABSOLUTE);
    will_return(__wrap_rrddim_add, &mock_rd);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_st);
    expect_value(__wrap_rrddim_set_by_pointer, rd, &mock_rd);
    expect_value(__wrap_rrddim_set_by_pointer, value, 99);
    
    expect_value(__wrap_rrdset_done, st, &mock_st);
    
    common_disk_qops(&d, "test_id", "test_name", 99, -1, NULL, NULL);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_disk_qops_first_call_creates_rrdset),
        cmocka_unit_test(test_disk_qops_first_call_with_callback),
        cmocka_unit_test(test_disk_qops_subsequent_call_skips_creation),
        cmocka_unit_test(test_disk_qops_zero_queued_ops),
        cmocka_unit_test(test_disk_qops_large_queued_ops),
        cmocka_unit_test(test_disk_qops_different_update_every),
        cmocka_unit_test(test_disk_qops_empty_id_name),
        cmocka_unit_test(test_disk_qops_special_chars_in_names),
        cmocka_unit_test(test_disk_qops_multiple_consecutive_calls),
        cmocka_unit_test(test_disk_qops_max_uint64_value),
        cmocka_unit_test(test_disk_qops_negative_update_every),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}