#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Mock types and macros */
typedef struct {
    void *ptr;
} RRDSET;

typedef struct {
    void *ptr;
} RRDDIM;

typedef void (*instance_labels_cb_t)(RRDSET *st, void *data);

#define NETDATA_CHART_PRIO_DISK_OPS 1
#define RRD_ALGORITHM_INCREMENTAL 1
#define RRDSET_TYPE_LINE 1
#define _COMMON_PLUGIN_NAME "test_plugin"
#define _COMMON_PLUGIN_MODULE_NAME "test_module"

typedef long long collected_number;

/* Mock functions */
RRDSET *__wrap_rrdset_create_localhost(
    const char *type,
    const char *id,
    const char *name,
    const char *family,
    const char *context,
    const char *title,
    const char *units,
    const char *plugin,
    const char *module,
    long priority,
    int update_every,
    int chart_type) {
    
    RRDSET *st = malloc(sizeof(RRDSET));
    st->ptr = malloc(1);
    return st;
}

RRDDIM *__wrap_rrddim_add(
    RRDSET *st,
    const char *id,
    const char *name,
    long multiplier,
    long divisor,
    int algorithm) {
    
    RRDDIM *rd = malloc(sizeof(RRDDIM));
    rd->ptr = malloc(1);
    return rd;
}

void __wrap_rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, collected_number value) {
    check_expected_ptr(st);
    check_expected_ptr(rd);
    check_expected(value);
}

void __wrap_rrdset_done(RRDSET *st) {
    check_expected_ptr(st);
}

/* Test setup and teardown */
static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    return 0;
}

/* Test: common_disk_ops creates rrdset when not initialized */
static void test_disk_ops_creates_rrdset_first_call(void **state) {
    ND_DISK_OPS d = {0};
    
    expect_value(__wrap_rrddim_set_by_pointer, value, 100);
    expect_value(__wrap_rrddim_set_by_pointer, value, 200);
    expect_any(__wrap_rrdset_done, st);
    
    common_disk_ops(&d, "disk0", "Disk 0", 100, 200, 10, NULL, NULL);
    
    assert_non_null(d.st_ops);
    assert_non_null(d.rd_ops_reads);
    assert_non_null(d.rd_ops_writes);
}

/* Test: common_disk_ops reuses rrdset on subsequent calls */
static void test_disk_ops_reuses_rrdset_second_call(void **state) {
    ND_DISK_OPS d = {0};
    
    expect_value(__wrap_rrddim_set_by_pointer, value, 100);
    expect_value(__wrap_rrddim_set_by_pointer, value, 200);
    expect_any(__wrap_rrdset_done, st);
    
    common_disk_ops(&d, "disk0", "Disk 0", 100, 200, 10, NULL, NULL);
    
    RRDSET *first_st = d.st_ops;
    RRDDIM *first_reads = d.rd_ops_reads;
    RRDDIM *first_writes = d.rd_ops_writes;
    
    expect_value(__wrap_rrddim_set_by_pointer, value, 150);
    expect_value(__wrap_rrddim_set_by_pointer, value, 250);
    expect_any(__wrap_rrdset_done, st);
    
    common_disk_ops(&d, "disk0", "Disk 0", 150, 250, 10, NULL, NULL);
    
    assert_ptr_equal(d.st_ops, first_st);
    assert_ptr_equal(d.rd_ops_reads, first_reads);
    assert_ptr_equal(d.rd_ops_writes, first_writes);
}

/* Test: common_disk_ops with zero values */
static void test_disk_ops_with_zero_values(void **state) {
    ND_DISK_OPS d = {0};
    
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    expect_any(__wrap_rrdset_done, st);
    
    common_disk_ops(&d, "disk0", "Disk 0", 0, 0, 10, NULL, NULL);
    
    assert_non_null(d.st_ops);
}

/* Test: common_disk_ops with maximum uint64_t values */
static void test_disk_ops_with_max_values(void **state) {
    ND_DISK_OPS d = {0};
    uint64_t max_val = UINT64_MAX;
    
    expect_value(__wrap_rrddim_set_by_pointer, value, (collected_number)max_val);
    expect_value(__wrap_rrddim_set_by_pointer, value, (collected_number)max_val);
    expect_any(__wrap_rrdset_done, st);
    
    common_disk_ops(&d, "disk0", "Disk 0", max_val, max_val, 10, NULL, NULL);
    
    assert_non_null(d.st_ops);
}

/* Test: common_disk_ops with callback provided */
static void test_disk_ops_with_callback(void **state) {
    ND_DISK_OPS d = {0};
    int callback_called = 0;
    
    void mock_callback(RRDSET *st, void *data) {
        assert_non_null(st);
        assert_non_null(data);
        *(int *)data = 1;
    }
    
    expect_value(__wrap_rrddim_set_by_pointer, value, 100);
    expect_value(__wrap_rrddim_set_by_pointer, value, 200);
    expect_any(__wrap_rrdset_done, st);
    
    common_disk_ops(&d, "disk0", "Disk 0", 100, 200, 10, mock_callback, &callback_called);
    
    assert_int_equal(callback_called, 1);
}

/* Test: common_disk_ops with different update_every values */
static void test_disk_ops_various_update_every(void **state) {
    for (int update_every = 1; update_every <= 60; update_every += 10) {
        ND_DISK_OPS d = {0};
        
        expect_value(__wrap_rrddim_set_by_pointer, value, 100);
        expect_value(__wrap_rrddim_set_by_pointer, value, 200);
        expect_any(__wrap_rrdset_done, st);
        
        common_disk_ops(&d, "disk0", "Disk 0", 100, 200, update_every, NULL, NULL);
        
        assert_non_null(d.st_ops);
    }
}

/* Test: common_disk_ops with different id and name values */
static void test_disk_ops_various_ids_names(void **state) {
    const char *ids[] = {"disk0", "disk1", "sda", "nvme0n1", ""};
    const char *names[] = {"Disk 0", "Disk 1", "SATA", "NVMe", NULL};
    
    for (int i = 0; i < 5; i++) {
        ND_DISK_OPS d = {0};
        
        expect_value(__wrap_rrddim_set_by_pointer, value, 100);
        expect_value(__wrap_rrddim_set_by_pointer, value, 200);
        expect_any(__wrap_rrdset_done, st);
        
        common_disk_ops(&d, ids[i], names[i], 100, 200, 10, NULL, NULL);
        
        assert_non_null(d.st_ops);
    }
}

/* Test: common_disk_ops with different read and write values */
static void test_disk_ops_different_read_write_values(void **state) {
    ND_DISK_OPS d = {0};
    
    expect_value(__wrap_rrddim_set_by_pointer, value, 5000);
    expect_value(__wrap_rrddim_set_by_pointer, value, 2000);
    expect_any(__wrap_rrdset_done, st);
    
    common_disk_ops(&d, "disk0", "Disk 0", 5000, 2000, 10, NULL, NULL);
    
    assert_non_null(d.st_ops);
}

/* Test: common_disk_ops without callback (callback NULL) */
static void test_disk_ops_without_callback(void **state) {
    ND_DISK_OPS d = {0};
    
    expect_value(__wrap_rrddim_set_by_pointer, value, 100);
    expect_value(__wrap_rrddim_set_by_pointer, value, 200);
    expect_any(__wrap_rrdset_done, st);
    
    common_disk_ops(&d, "disk0", "Disk 0", 100, 200, 10, NULL, NULL);
    
    assert_non_null(d.st_ops);
}

/* Test: common_disk_ops multiple sequential updates */
static void test_disk_ops_multiple_sequential_updates(void **state) {
    ND_DISK_OPS d = {0};
    
    for (int i = 0; i < 5; i++) {
        expect_value(__wrap_rrddim_set_by_pointer, value, 100 * (i + 1));
        expect_value(__wrap_rrddim_set_by_pointer, value, 200 * (i + 1));
        expect_any(__wrap_rrdset_done, st);
        
        common_disk_ops(&d, "disk0", "Disk 0", 100 * (i + 1), 200 * (i + 1), 10, NULL, NULL);
    }
    
    assert_non_null(d.st_ops);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_disk_ops_creates_rrdset_first_call, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_reuses_rrdset_second_call, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_with_zero_values, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_with_max_values, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_with_callback, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_various_update_every, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_various_ids_names, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_different_read_write_values, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_without_callback, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_ops_multiple_sequential_updates, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}