// SPDX-License-Identifier: GPL-3.0-or-later
// Comprehensive test suite for disk-busy.h

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// Mock definitions for Netdata structures
typedef void RRDSET;
typedef void RRDDIM;
typedef long long collected_number;

// Macro definitions from Netdata
#define unlikely(x) (x)
#define RRD_ALGORITHM_INCREMENTAL 0
#define RRDSET_TYPE_AREA 0
#define NETDATA_CHART_PRIO_DISK_BUSY 1900
#define _COMMON_PLUGIN_NAME "test_plugin"
#define _COMMON_PLUGIN_MODULE_NAME "test_module"

// Mock state and call tracking
static int rrdset_create_called = 0;
static int rrddim_add_called = 0;
static int rrddim_set_by_pointer_called = 0;
static int rrdset_done_called = 0;
static int instance_labels_cb_called = 0;

static RRDSET *last_created_rrdset = NULL;
static RRDDIM *last_added_rrddim = NULL;
static collected_number last_set_value = 0;
static int last_rrddim_add_divisor = 0;
static int last_rrddim_add_multiplier = 0;

// Mock functions
static RRDSET* mock_rrdset_create_localhost(
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
    
    rrdset_create_called++;
    last_created_rrdset = (RRDSET*)malloc(1);
    return last_created_rrdset;
}

static RRDDIM* mock_rrddim_add(
    RRDSET *st,
    const char *name,
    const char *divisor,
    long multiplier,
    long divisor_val,
    int algorithm) {
    
    rrddim_add_called++;
    last_rrddim_add_multiplier = multiplier;
    last_rrddim_add_divisor = divisor_val;
    last_added_rrddim = (RRDDIM*)malloc(1);
    return last_added_rrddim;
}

static void mock_rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, collected_number value) {
    rrddim_set_by_pointer_called++;
    last_set_value = value;
}

static void mock_rrdset_done(RRDSET *st) {
    rrdset_done_called++;
}

// Redefine Netdata functions to use mocks
#define rrdset_create_localhost mock_rrdset_create_localhost
#define rrddim_add mock_rrddim_add
#define rrddim_set_by_pointer mock_rrddim_set_by_pointer
#define rrdset_done mock_rrdset_done

// Typedef for instance_labels_cb_t
typedef void (*instance_labels_cb_t)(RRDSET *st, void *data);

// Include the header under test
#include "disk-busy.h"

// Reset function for test isolation
static void reset_mocks(void) {
    rrdset_create_called = 0;
    rrddim_add_called = 0;
    rrddim_set_by_pointer_called = 0;
    rrdset_done_called = 0;
    instance_labels_cb_called = 0;
    last_set_value = 0;
    last_rrddim_add_divisor = 0;
    last_rrddim_add_multiplier = 0;
    
    if (last_created_rrdset) {
        free(last_created_rrdset);
        last_created_rrdset = NULL;
    }
    if (last_added_rrddim) {
        free(last_added_rrddim);
        last_added_rrddim = NULL;
    }
}

// Mock callback function for testing
static void test_instance_labels_cb(RRDSET *st, void *data) {
    instance_labels_cb_called++;
}

// Test 1: Initialize disk busy structure on first call
static int test_disk_busy_initialization(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 1, NULL, NULL);
    
    if (rrdset_create_called != 1) {
        printf("FAIL: test_disk_busy_initialization - rrdset_create_called should be 1, got %d\n", rrdset_create_called);
        return 0;
    }
    if (rrddim_add_called != 1) {
        printf("FAIL: test_disk_busy_initialization - rrddim_add_called should be 1, got %d\n", rrddim_add_called);
        return 0;
    }
    if (disk_busy.st_busy == NULL) {
        printf("FAIL: test_disk_busy_initialization - st_busy should not be NULL\n");
        return 0;
    }
    if (disk_busy.rd_busy == NULL) {
        printf("FAIL: test_disk_busy_initialization - rd_busy should not be NULL\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_initialization\n");
    return 1;
}

// Test 2: Skip initialization on subsequent calls
static int test_disk_busy_skip_initialization(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    // First call
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 1, NULL, NULL);
    int first_create_count = rrdset_create_called;
    
    // Second call with different values
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 200, 1, NULL, NULL);
    int second_create_count = rrdset_create_called;
    
    if (second_create_count != first_create_count) {
        printf("FAIL: test_disk_busy_skip_initialization - should not re-create rrdset\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_skip_initialization\n");
    return 1;
}

// Test 3: Update dimension with new value
static int test_disk_busy_update_value(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    uint64_t test_value = 12345;
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", test_value, 1, NULL, NULL);
    
    if (last_set_value != (collected_number)test_value) {
        printf("FAIL: test_disk_busy_update_value - expected %lld, got %lld\n", (long long)test_value, last_set_value);
        return 0;
    }
    if (rrddim_set_by_pointer_called != 1) {
        printf("FAIL: test_disk_busy_update_value - rrddim_set_by_pointer should be called once\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_update_value\n");
    return 1;
}

// Test 4: Call rrdset_done after update
static int test_disk_busy_rrdset_done(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 1, NULL, NULL);
    
    if (rrdset_done_called != 1) {
        printf("FAIL: test_disk_busy_rrdset_done - rrdset_done should be called once\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_rrdset_done\n");
    return 1;
}

// Test 5: Invoke callback when provided
static int test_disk_busy_callback_invoked(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 1, test_instance_labels_cb, NULL);
    
    if (instance_labels_cb_called != 1) {
        printf("FAIL: test_disk_busy_callback_invoked - callback should be called once\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_callback_invoked\n");
    return 1;
}

// Test 6: Skip callback when NULL
static int test_disk_busy_callback_null(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 1, NULL, NULL);
    
    if (instance_labels_cb_called != 0) {
        printf("FAIL: test_disk_busy_callback_null - callback should not be called\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_callback_null\n");
    return 1;
}

// Test 7: Handle zero busy_ms value
static int test_disk_busy_zero_value(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 0, 1, NULL, NULL);
    
    if (last_set_value != 0) {
        printf("FAIL: test_disk_busy_zero_value - expected 0, got %lld\n", last_set_value);
        return 0;
    }
    if (rrddim_set_by_pointer_called != 1) {
        printf("FAIL: test_disk_busy_zero_value - rrddim_set_by_pointer should be called\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_zero_value\n");
    return 1;
}

// Test 8: Handle max uint64_t value
static int test_disk_busy_max_value(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    uint64_t max_value = (uint64_t)-1; // Max uint64_t
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", max_value, 1, NULL, NULL);
    
    if (last_set_value != (collected_number)max_value) {
        printf("FAIL: test_disk_busy_max_value - value mismatch\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_max_value\n");
    return 1;
}

// Test 9: Verify correct dimension parameters on creation
static int test_disk_busy_dimension_params(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 1, NULL, NULL);
    
    if (last_rrddim_add_multiplier != 1) {
        printf("FAIL: test_disk_busy_dimension_params - multiplier should be 1, got %d\n", last_rrddim_add_multiplier);
        return 0;
    }
    if (last_rrddim_add_divisor != 1) {
        printf("FAIL: test_disk_busy_dimension_params - divisor should be 1, got %d\n", last_rrddim_add_divisor);
        return 0;
    }
    
    printf("PASS: test_disk_busy_dimension_params\n");
    return 1;
}

// Test 10: Multiple updates with different values
static int test_disk_busy_multiple_updates(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    uint64_t values[] = {100, 200, 300, 0, 999999};
    for (int i = 0; i < 5; i++) {
        common_disk_busy(&disk_busy, "disk0", "Disk 0", values[i], 1, NULL, NULL);
    }
    
    if (last_set_value != (collected_number)values[4]) {
        printf("FAIL: test_disk_busy_multiple_updates - last value mismatch\n");
        return 0;
    }
    if (rrddim_set_by_pointer_called != 5) {
        printf("FAIL: test_disk_busy_multiple_updates - should call set 5 times, got %d\n", rrddim_set_by_pointer_called);
        return 0;
    }
    if (rrdset_create_called != 1) {
        printf("FAIL: test_disk_busy_multiple_updates - should create rrdset only once\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_multiple_updates\n");
    return 1;
}

// Test 11: Different IDs and names
static int test_disk_busy_different_ids(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy1 = {0};
    ND_DISK_BUSY disk_busy2 = {0};
    
    common_disk_busy(&disk_busy1, "disk0", "Disk 0", 100, 1, NULL, NULL);
    int first_create = rrdset_create_called;
    
    common_disk_busy(&disk_busy2, "disk1", "Disk 1", 100, 1, NULL, NULL);
    int second_create = rrdset_create_called;
    
    if (second_create != first_create + 1) {
        printf("FAIL: test_disk_busy_different_ids - should create separate rrdsets\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_different_ids\n");
    return 1;
}

// Test 12: Callback with data pointer
static int test_disk_busy_callback_data(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    int callback_data = 42;
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 1, test_instance_labels_cb, &callback_data);
    
    if (instance_labels_cb_called != 1) {
        printf("FAIL: test_disk_busy_callback_data - callback should be called\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_callback_data\n");
    return 1;
}

// Test 13: Different update_every values
static int test_disk_busy_different_update_every(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 10, NULL, NULL);
    
    // Structure should still be properly initialized
    if (disk_busy.st_busy == NULL || disk_busy.rd_busy == NULL) {
        printf("FAIL: test_disk_busy_different_update_every - initialization failed\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_different_update_every\n");
    return 1;
}

// Test 14: Large busy_ms value (realistic disk busy time)
static int test_disk_busy_large_value(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    uint64_t large_value = 3600000; // 1 hour in ms
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", large_value, 1, NULL, NULL);
    
    if (last_set_value != (collected_number)large_value) {
        printf("FAIL: test_disk_busy_large_value - value mismatch\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_large_value\n");
    return 1;
}

// Test 15: Structure pointer is correctly populated
static int test_disk_busy_structure_population(void) {
    reset_mocks();
    ND_DISK_BUSY disk_busy = {0};
    
    common_disk_busy(&disk_busy, "disk0", "Disk 0", 100, 1, NULL, NULL);
    
    if (disk_busy.st_busy != last_created_rrdset) {
        printf("FAIL: test_disk_busy_structure_population - st_busy not set correctly\n");
        return 0;
    }
    if (disk_busy.rd_busy != last_added_rrddim) {
        printf("FAIL: test_disk_busy_structure_population - rd_busy not set correctly\n");
        return 0;
    }
    
    printf("PASS: test_disk_busy_structure_population\n");
    return 1;
}

// Main test runner
int main(void) {
    int passed = 0;
    int total = 0;
    
    total++; passed += test_disk_busy_initialization();
    total++; passed += test_disk_busy_skip_initialization();
    total++; passed += test_disk_busy_update_value();
    total++; passed += test_disk_busy_rrdset_done();
    total++; passed += test_disk_busy_callback_invoked();
    total++; passed += test_disk_busy_callback_null();
    total++; passed += test_disk_busy_zero_value();
    total++; passed += test_disk_busy_max_value();
    total++; passed += test_disk_busy_dimension_params();
    total++; passed += test_disk_busy_multiple_updates();
    total++; passed += test_disk_busy_different_ids();
    total++; passed += test_disk_busy_callback_data();
    total++; passed += test_disk_busy_different_update_every();
    total++; passed += test_disk_busy_large_value();
    total++; passed += test_disk_busy_structure_population();
    
    printf("\n========================================\n");
    printf("Tests passed: %d/%d\n", passed, total);
    printf("========================================\n");
    
    return (passed == total) ? 0 : 1;
}