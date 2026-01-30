#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>

// Mock definitions
#define _COMMON_PLUGIN_NAME "test_plugin"
#define _COMMON_PLUGIN_MODULE_NAME "test_module"

// Forward declarations for mocked structures
typedef struct {
    int dummy;
} RRDSET;

typedef struct {
    int dummy;
} RRDDIM;

typedef void (*instance_labels_cb_t)(RRDSET *st, void *data);

// Mock constants
#define NETDATA_CHART_PRIO_DISK_AVGSZ 3800
#define RRD_ALGORITHM_ABSOLUTE 3
#define RRDSET_TYPE_AREA 0

// Mock function declarations
RRDSET *rrdset_create_localhost(const char *type, const char *id, const char *name,
                                const char *family, const char *context,
                                const char *title, const char *units,
                                const char *plugin, const char *module,
                                int priority, int update_every, int chart_type);
RRDDIM *rrddim_add(RRDSET *st, const char *id, const char *name,
                   int multiplier, int divisor, int algorithm);
void rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, int64_t value);
void rrdset_done(RRDSET *st);

// Mock implementations
static RRDSET mock_rrdset = {.dummy = 0};
static RRDDIM mock_rrddim_reads = {.dummy = 1};
static RRDDIM mock_rrddim_writes = {.dummy = 2};

RRDSET *__wrap_rrdset_create_localhost(const char *type, const char *id, const char *name,
                                        const char *family, const char *context,
                                        const char *title, const char *units,
                                        const char *plugin, const char *module,
                                        int priority, int update_every, int chart_type) {
    check_expected_ptr(type);
    check_expected_ptr(id);
    check_expected_ptr(name);
    check_expected_ptr(family);
    check_expected_ptr(context);
    check_expected_ptr(title);
    check_expected_ptr(units);
    check_expected_ptr(plugin);
    check_expected_ptr(module);
    check_expected(priority);
    check_expected(update_every);
    check_expected(chart_type);
    return &mock_rrdset;
}

RRDDIM *__wrap_rrddim_add(RRDSET *st, const char *id, const char *name,
                          int multiplier, int divisor, int algorithm) {
    check_expected_ptr(st);
    check_expected_ptr(id);
    check_expected_ptr(name);
    check_expected(multiplier);
    check_expected(divisor);
    check_expected(algorithm);
    
    if (strcmp(id, "reads") == 0) {
        return &mock_rrddim_reads;
    }
    return &mock_rrddim_writes;
}

void __wrap_rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, int64_t value) {
    check_expected_ptr(st);
    check_expected_ptr(rd);
    check_expected(value);
}

void __wrap_rrdset_done(RRDSET *st) {
    check_expected_ptr(st);
}

// Include the header file to test
#define rrdset_create_localhost __wrap_rrdset_create_localhost
#define rrddim_add __wrap_rrddim_add
#define rrddim_set_by_pointer __wrap_rrddim_set_by_pointer
#define rrdset_done __wrap_rrdset_done

#include "../../src/collectors/common-contexts/disk-avgsz.h"

// Test: common_disk_avgsz should initialize when st_avgsz is NULL
static void test_common_disk_avgsz_init(void **state) {
    ND_DISK_AVGSZ d = {.st_avgsz = NULL, .rd_avgsz_reads = NULL, .rd_avgsz_writes = NULL};
    
    expect_value(__wrap_rrdset_create_localhost, type, "disk_avgsz");
    expect_string(__wrap_rrdset_create_localhost, id, "sda");
    expect_string(__wrap_rrdset_create_localhost, name, "Disk A");
    expect_string(__wrap_rrdset_create_localhost, family, "io");
    expect_string(__wrap_rrdset_create_localhost, context, "disk.avgsz");
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_AVGSZ);
    expect_value(__wrap_rrdset_create_localhost, update_every, 10);
    expect_value(__wrap_rrdset_create_localhost, chart_type, RRDSET_TYPE_AREA);
    
    // rrddim_add calls for reads
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_string(__wrap_rrddim_add, id, "reads");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor, 1024);
    expect_value(__wrap_rrddim_add, algorithm, RRD_ALGORITHM_ABSOLUTE);
    
    // rrddim_add calls for writes
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_string(__wrap_rrddim_add, id, "writes");
    expect_value(__wrap_rrddim_add, multiplier, -1);
    expect_value(__wrap_rrddim_add, divisor, 1024);
    expect_value(__wrap_rrddim_add, algorithm, RRD_ALGORITHM_ABSOLUTE);
    
    // rrddim_set_by_pointer calls
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 1024);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 2048);
    
    expect_value(__wrap_rrdset_done, st, &mock_rrdset);
    
    common_disk_avgsz(&d, "sda", "Disk A", 1024, 2048, 10, NULL, NULL);
    
    assert_ptr_equal(d.st_avgsz, &mock_rrdset);
    assert_ptr_equal(d.rd_avgsz_reads, &mock_rrddim_reads);
    assert_ptr_equal(d.rd_avgsz_writes, &mock_rrddim_writes);
}

// Test: common_disk_avgsz should not reinitialize when st_avgsz is already set
static void test_common_disk_avgsz_skip_reinit(void **state) {
    ND_DISK_AVGSZ d = {
        .st_avgsz = &mock_rrdset,
        .rd_avgsz_reads = &mock_rrddim_reads,
        .rd_avgsz_writes = &mock_rrddim_writes
    };
    
    // Only expect the set and done calls, not the initialization
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 5000);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 6000);
    
    expect_value(__wrap_rrdset_done, st, &mock_rrdset);
    
    common_disk_avgsz(&d, "sdb", "Disk B", 5000, 6000, 10, NULL, NULL);
    
    // Verify structure wasn't modified
    assert_ptr_equal(d.st_avgsz, &mock_rrdset);
}

// Test: common_disk_avgsz with zero values
static void test_common_disk_avgsz_zero_values(void **state) {
    ND_DISK_AVGSZ d = {.st_avgsz = NULL, .rd_avgsz_reads = NULL, .rd_avgsz_writes = NULL};
    
    expect_value(__wrap_rrdset_create_localhost, type, "disk_avgsz");
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_AVGSZ);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor, 1024);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, multiplier, -1);
    expect_value(__wrap_rrddim_add, divisor, 1024);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    
    expect_value(__wrap_rrdset_done, st, &mock_rrdset);
    
    common_disk_avgsz(&d, "sdc", "Disk C", 0, 0, 1, NULL, NULL);
    
    assert_ptr_equal(d.st_avgsz, &mock_rrdset);
}

// Test: common_disk_avgsz with callback
static void test_common_disk_avgsz_with_callback(void **state) {
    ND_DISK_AVGSZ d = {.st_avgsz = NULL, .rd_avgsz_reads = NULL, .rd_avgsz_writes = NULL};
    
    int callback_called = 0;
    void callback_func(RRDSET *st, void *data) {
        *(int *)data = 1;
    }
    
    expect_value(__wrap_rrdset_create_localhost, type, "disk_avgsz");
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_AVGSZ);
    expect_value(__wrap_rrdset_create_localhost, update_every, 10);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, multiplier, 1);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, multiplier, -1);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 1000);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 2000);
    
    expect_value(__wrap_rrdset_done, st, &mock_rrdset);
    
    common_disk_avgsz(&d, "sdd", "Disk D", 1000, 2000, 10, callback_func, &callback_called);
    
    assert_ptr_equal(d.st_avgsz, &mock_rrdset);
}

// Test: common_disk_avgsz with maximum values
static void test_common_disk_avgsz_max_values(void **state) {
    ND_DISK_AVGSZ d = {.st_avgsz = NULL, .rd_avgsz_reads = NULL, .rd_avgsz_writes = NULL};
    
    uint64_t max_val = 0xFFFFFFFFFFFFFFFFULL;
    
    expect_value(__wrap_rrdset_create_localhost, type, "disk_avgsz");
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_AVGSZ);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, multiplier, 1);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, multiplier, -1);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    
    expect_value(__wrap_rrdset_done, st, &mock_rrdset);
    
    common_disk_avgsz(&d, "sde", "Disk E", max_val, max_val, 10, NULL, NULL);
    
    assert_ptr_equal(d.st_avgsz, &mock_rrdset);
}

// Test: common_disk_avgsz with different update_every values
static void test_common_disk_avgsz_different_update_every(void **state) {
    ND_DISK_AVGSZ d = {.st_avgsz = NULL, .rd_avgsz_reads = NULL, .rd_avgsz_writes = NULL};
    
    expect_value(__wrap_rrdset_create_localhost, type, "disk_avgsz");
    expect_value(__wrap_rrdset_create_localhost, update_every, 60);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, multiplier, 1);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, multiplier, -1);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    
    expect_value(__wrap_rrdset_done, st, &mock_rrdset);
    
    common_disk_avgsz(&d, "sdf", "Disk F", 512, 512, 60, NULL, NULL);
}

// Test: common_disk_avgsz dimension multiplier correctness
static void test_common_disk_avgsz_multipliers(void **state) {
    ND_DISK_AVGSZ d = {.st_avgsz = NULL, .rd_avgsz_reads = NULL, .rd_avgsz_writes = NULL};
    
    expect_value(__wrap_rrdset_create_localhost, type, "disk_avgsz");
    expect_value(__wrap_rrdset_create_localhost, priority, NETDATA_CHART_PRIO_DISK_AVGSZ);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, id, "reads");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor, 1024);
    
    expect_value(__wrap_rrddim_add, st, &mock_rrdset);
    expect_value(__wrap_rrddim_add, id, "writes");
    expect_value(__wrap_rrddim_add, multiplier, -1);
    expect_value(__wrap_rrddim_add, divisor, 1024);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 2048);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, &mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, value, 4096);
    
    expect_value(__wrap_rrdset_done, st, &mock_rrdset);
    
    common_disk_avgsz(&d, "sdg", "Disk G", 2048, 4096, 10, NULL, NULL);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_common_disk_avgsz_init),
        cmocka_unit_test(test_common_disk_avgsz_skip_reinit),
        cmocka_unit_test(test_common_disk_avgsz_zero_values),
        cmocka_unit_test(test_common_disk_avgsz_with_callback),
        cmocka_unit_test(test_common_disk_avgsz_max_values),
        cmocka_unit_test(test_common_disk_avgsz_different_update_every),
        cmocka_unit_test(test_common_disk_avgsz_multipliers),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}