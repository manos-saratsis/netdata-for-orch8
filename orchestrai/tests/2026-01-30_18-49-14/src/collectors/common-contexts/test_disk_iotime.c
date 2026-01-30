#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Mock definitions for Netdata types and functions */
typedef struct {
    void *data;
} RRDSET;

typedef struct {
    void *data;
} RRDDIM;

#define RRD_ALGORITHM_INCREMENTAL 1
#define RRDSET_TYPE_AREA 1
#define NETDATA_CHART_PRIO_DISK_IOTIME 1000

typedef long long collected_number;
typedef void (*instance_labels_cb_t)(RRDSET *st, void *data);

#define unlikely(x) (x)
#define _COMMON_PLUGIN_NAME "test_plugin"
#define _COMMON_PLUGIN_MODULE_NAME "test_module"

/* Mock functions */
RRDSET* mock_rrdset_create_localhost(
        const char *type,
        const char *id,
        const char *name,
        const char *context,
        const char *unit,
        const char *title,
        const char *units,
        const char *plugin,
        const char *module,
        int prio,
        int update_every,
        int chart_type) {
    RRDSET *st = (RRDSET *)malloc(sizeof(RRDSET));
    st->data = (void *)1;
    return st;
}

RRDDIM* mock_rrddim_add(
        RRDSET *st,
        const char *id,
        const char *name,
        int multiplier,
        int divisor,
        int algorithm) {
    RRDDIM *rd = (RRDDIM *)malloc(sizeof(RRDDIM));
    rd->data = (void *)1;
    return rd;
}

void mock_rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, collected_number value) {
    /* Mock implementation - track calls */
}

void mock_rrdset_done(RRDSET *st) {
    /* Mock implementation - track calls */
}

#define rrdset_create_localhost mock_rrdset_create_localhost
#define rrddim_add mock_rrddim_add
#define rrddim_set_by_pointer mock_rrddim_set_by_pointer
#define rrdset_done mock_rrdset_done

#include "disk-iotime.h"

/* Test state tracking */
static int callback_called = 0;
static RRDSET *callback_st = NULL;
static void *callback_data_ptr = NULL;

void test_callback(RRDSET *st, void *data) {
    callback_called = 1;
    callback_st = st;
    callback_data_ptr = data;
}

/* Test: Initial creation - st_iotime is NULL */
static void test_disk_iotime_initial_creation(void **state) {
    ND_DISK_IOTIME d = {0};
    
    assert_null(d.st_iotime);
    assert_null(d.rd_reads_ms);
    assert_null(d.rd_writes_ms);
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, NULL, NULL);
    
    /* After first call, st_iotime should be created */
    assert_non_null(d.st_iotime);
    assert_non_null(d.rd_reads_ms);
    assert_non_null(d.rd_writes_ms);
}

/* Test: Second call should not recreate st_iotime */
static void test_disk_iotime_no_recreation(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, NULL, NULL);
    RRDSET *first_st = d.st_iotime;
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1500, 2500, 10, NULL, NULL);
    RRDSET *second_st = d.st_iotime;
    
    assert_ptr_equal(first_st, second_st);
}

/* Test: Callback is called when provided and st_iotime is NULL */
static void test_disk_iotime_callback_called(void **state) {
    ND_DISK_IOTIME d = {0};
    int test_data = 42;
    
    callback_called = 0;
    callback_st = NULL;
    callback_data_ptr = NULL;
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, test_callback, &test_data);
    
    assert_int_equal(callback_called, 1);
    assert_non_null(callback_st);
    assert_ptr_equal(callback_data_ptr, &test_data);
}

/* Test: Callback is not called on second invocation */
static void test_disk_iotime_callback_not_called_twice(void **state) {
    ND_DISK_IOTIME d = {0};
    int test_data = 42;
    
    callback_called = 0;
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, test_callback, &test_data);
    assert_int_equal(callback_called, 1);
    
    callback_called = 0;
    common_disk_iotime(&d, "disk1", "Disk 1", 1500, 2500, 10, test_callback, &test_data);
    assert_int_equal(callback_called, 0);
}

/* Test: Callback is not called when cb is NULL */
static void test_disk_iotime_no_callback_when_null(void **state) {
    ND_DISK_IOTIME d = {0};
    
    callback_called = 0;
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, NULL, NULL);
    
    assert_int_equal(callback_called, 0);
}

/* Test: Different ID values */
static void test_disk_iotime_different_ids(void **state) {
    ND_DISK_IOTIME d1 = {0};
    ND_DISK_IOTIME d2 = {0};
    
    common_disk_iotime(&d1, "disk1", "Disk 1", 1000, 2000, 10, NULL, NULL);
    common_disk_iotime(&d2, "disk2", "Disk 2", 1000, 2000, 10, NULL, NULL);
    
    assert_non_null(d1.st_iotime);
    assert_non_null(d2.st_iotime);
    assert_ptr_not_equal(d1.st_iotime, d2.st_iotime);
}

/* Test: Zero reads_ms and writes_ms */
static void test_disk_iotime_zero_values(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "disk1", "Disk 1", 0, 0, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
    assert_non_null(d.rd_reads_ms);
    assert_non_null(d.rd_writes_ms);
}

/* Test: Large uint64_t values */
static void test_disk_iotime_large_values(void **state) {
    ND_DISK_IOTIME d = {0};
    uint64_t max_val = 0xFFFFFFFFFFFFFFFFULL;
    
    common_disk_iotime(&d, "disk1", "Disk 1", max_val, max_val, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Different update_every values */
static void test_disk_iotime_different_update_every(void **state) {
    ND_DISK_IOTIME d1 = {0};
    ND_DISK_IOTIME d2 = {0};
    ND_DISK_IOTIME d3 = {0};
    
    common_disk_iotime(&d1, "disk1", "Disk 1", 1000, 2000, 1, NULL, NULL);
    common_disk_iotime(&d2, "disk1", "Disk 1", 1000, 2000, 10, NULL, NULL);
    common_disk_iotime(&d3, "disk1", "Disk 1", 1000, 2000, 60, NULL, NULL);
    
    assert_non_null(d1.st_iotime);
    assert_non_null(d2.st_iotime);
    assert_non_null(d3.st_iotime);
}

/* Test: Empty name string */
static void test_disk_iotime_empty_name(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "disk1", "", 1000, 2000, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Empty id string */
static void test_disk_iotime_empty_id(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "", "Disk 1", 1000, 2000, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Single value update - reads_ms only */
static void test_disk_iotime_update_reads_only(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "disk1", "Disk 1", 5000, 0, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Single value update - writes_ms only */
static void test_disk_iotime_update_writes_only(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "disk1", "Disk 1", 0, 5000, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Multiple consecutive updates */
static void test_disk_iotime_multiple_updates(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, NULL, NULL);
    common_disk_iotime(&d, "disk1", "Disk 1", 2000, 3000, 10, NULL, NULL);
    common_disk_iotime(&d, "disk1", "Disk 1", 3000, 4000, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
    assert_ptr_equal(d.st_iotime, d.st_iotime);
}

/* Test: Pre-initialized structure */
static void test_disk_iotime_preinitialized(void **state) {
    RRDSET mock_st = {0};
    RRDDIM mock_rd1 = {0};
    RRDDIM mock_rd2 = {0};
    
    ND_DISK_IOTIME d = {
        .st_iotime = &mock_st,
        .rd_reads_ms = &mock_rd1,
        .rd_writes_ms = &mock_rd2
    };
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, NULL, NULL);
    
    /* Should not recreate since st_iotime is already set */
    assert_ptr_equal(d.st_iotime, &mock_st);
}

/* Test: Callback with NULL data pointer */
static void test_disk_iotime_callback_null_data(void **state) {
    ND_DISK_IOTIME d = {0};
    
    callback_called = 0;
    callback_data_ptr = (void *)0xDEADBEEF;
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, test_callback, NULL);
    
    assert_int_equal(callback_called, 1);
    assert_null(callback_data_ptr);
}

/* Test: Negative update_every (edge case) */
static void test_disk_iotime_negative_update_every(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, -10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Very small update_every value */
static void test_disk_iotime_small_update_every(void **state) {
    ND_DISK_IOTIME d = {0};
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 0, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Long name string */
static void test_disk_iotime_long_name(void **state) {
    ND_DISK_IOTIME d = {0};
    const char *long_name = "Very Long Disk Name With Many Characters And Numbers 12345678901234567890";
    
    common_disk_iotime(&d, "disk1", long_name, 1000, 2000, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Long id string */
static void test_disk_iotime_long_id(void **state) {
    ND_DISK_IOTIME d = {0};
    const char *long_id = "very_long_disk_id_with_many_segments_12345678901234567890";
    
    common_disk_iotime(&d, long_id, "Disk 1", 1000, 2000, 10, NULL, NULL);
    
    assert_non_null(d.st_iotime);
}

/* Test: Callback called with correct st parameter */
static void test_disk_iotime_callback_st_parameter(void **state) {
    ND_DISK_IOTIME d = {0};
    
    callback_called = 0;
    callback_st = NULL;
    
    common_disk_iotime(&d, "disk1", "Disk 1", 1000, 2000, 10, test_callback, NULL);
    
    assert_ptr_equal(callback_st, d.st_iotime);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_disk_iotime_initial_creation),
        cmocka_unit_test(test_disk_iotime_no_recreation),
        cmocka_unit_test(test_disk_iotime_callback_called),
        cmocka_unit_test(test_disk_iotime_callback_not_called_twice),
        cmocka_unit_test(test_disk_iotime_no_callback_when_null),
        cmocka_unit_test(test_disk_iotime_different_ids),
        cmocka_unit_test(test_disk_iotime_zero_values),
        cmocka_unit_test(test_disk_iotime_large_values),
        cmocka_unit_test(test_disk_iotime_different_update_every),
        cmocka_unit_test(test_disk_iotime_empty_name),
        cmocka_unit_test(test_disk_iotime_empty_id),
        cmocka_unit_test(test_disk_iotime_update_reads_only),
        cmocka_unit_test(test_disk_iotime_update_writes_only),
        cmocka_unit_test(test_disk_iotime_multiple_updates),
        cmocka_unit_test(test_disk_iotime_preinitialized),
        cmocka_unit_test(test_disk_iotime_callback_null_data),
        cmocka_unit_test(test_disk_iotime_negative_update_every),
        cmocka_unit_test(test_disk_iotime_small_update_every),
        cmocka_unit_test(test_disk_iotime_long_name),
        cmocka_unit_test(test_disk_iotime_long_id),
        cmocka_unit_test(test_disk_iotime_callback_st_parameter),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}