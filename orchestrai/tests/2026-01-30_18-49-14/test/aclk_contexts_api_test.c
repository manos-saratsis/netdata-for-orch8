#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock types for schema wrappers
typedef struct {
    int dummy;
} contexts_snapshot_t;

typedef struct {
    int dummy;
} contexts_updated_t;

typedef struct {
    int dummy;
} update_node_collectors;

typedef struct {
    int dummy;
} update_node_info;

// Mock implementations of the interface functions
void aclk_send_contexts_snapshot(contexts_snapshot_t data) {
    // Mock implementation - track if called
    function_called();
}

void aclk_send_contexts_updated(contexts_updated_t data) {
    // Mock implementation - track if called
    function_called();
}

void aclk_update_node_collectors(struct update_node_collectors *collectors) {
    // Mock implementation - track if called
    function_called();
}

void aclk_update_node_info(struct update_node_info *info) {
    // Mock implementation - track if called
    function_called();
}

// Helper function to track function calls
static void function_called(void) {
    // Mock - increments internal counter
}

// Test: aclk_send_contexts_snapshot with valid data
static void test_aclk_send_contexts_snapshot_valid_data(void **state) {
    contexts_snapshot_t snapshot = {.dummy = 1};
    
    // Should not raise any errors
    aclk_send_contexts_snapshot(snapshot);
}

// Test: aclk_send_contexts_snapshot with null-like data
static void test_aclk_send_contexts_snapshot_zero_data(void **state) {
    contexts_snapshot_t snapshot = {.dummy = 0};
    
    // Should handle zero data gracefully
    aclk_send_contexts_snapshot(snapshot);
}

// Test: aclk_send_contexts_updated with valid data
static void test_aclk_send_contexts_updated_valid_data(void **state) {
    contexts_updated_t update = {.dummy = 1};
    
    // Should not raise any errors
    aclk_send_contexts_updated(update);
}

// Test: aclk_send_contexts_updated with zero data
static void test_aclk_send_contexts_updated_zero_data(void **state) {
    contexts_updated_t update = {.dummy = 0};
    
    // Should handle zero data gracefully
    aclk_send_contexts_updated(update);
}

// Test: aclk_update_node_collectors with valid pointer
static void test_aclk_update_node_collectors_valid_pointer(void **state) {
    struct update_node_collectors collectors = {.dummy = 1};
    
    // Should not raise any errors
    aclk_update_node_collectors(&collectors);
}

// Test: aclk_update_node_collectors with null pointer
static void test_aclk_update_node_collectors_null_pointer(void **state) {
    // Function should handle NULL gracefully
    // Note: actual implementation should check for NULL
    aclk_update_node_collectors(NULL);
}

// Test: aclk_update_node_info with valid pointer
static void test_aclk_update_node_info_valid_pointer(void **state) {
    struct update_node_info info = {.dummy = 1};
    
    // Should not raise any errors
    aclk_update_node_info(&info);
}

// Test: aclk_update_node_info with null pointer
static void test_aclk_update_node_info_null_pointer(void **state) {
    // Function should handle NULL gracefully
    // Note: actual implementation should check for NULL
    aclk_update_node_info(NULL);
}

// Test: Multiple consecutive calls to aclk_send_contexts_snapshot
static void test_aclk_send_contexts_snapshot_multiple_calls(void **state) {
    contexts_snapshot_t snapshot1 = {.dummy = 1};
    contexts_snapshot_t snapshot2 = {.dummy = 2};
    contexts_snapshot_t snapshot3 = {.dummy = 3};
    
    // All calls should succeed
    aclk_send_contexts_snapshot(snapshot1);
    aclk_send_contexts_snapshot(snapshot2);
    aclk_send_contexts_snapshot(snapshot3);
}

// Test: Multiple consecutive calls to aclk_send_contexts_updated
static void test_aclk_send_contexts_updated_multiple_calls(void **state) {
    contexts_updated_t update1 = {.dummy = 1};
    contexts_updated_t update2 = {.dummy = 2};
    contexts_updated_t update3 = {.dummy = 3};
    
    // All calls should succeed
    aclk_send_contexts_updated(update1);
    aclk_send_contexts_updated(update2);
    aclk_send_contexts_updated(update3);
}

// Test: Multiple consecutive calls to aclk_update_node_collectors
static void test_aclk_update_node_collectors_multiple_calls(void **state) {
    struct update_node_collectors collectors1 = {.dummy = 1};
    struct update_node_collectors collectors2 = {.dummy = 2};
    struct update_node_collectors collectors3 = {.dummy = 3};
    
    // All calls should succeed
    aclk_update_node_collectors(&collectors1);
    aclk_update_node_collectors(&collectors2);
    aclk_update_node_collectors(&collectors3);
}

// Test: Multiple consecutive calls to aclk_update_node_info
static void test_aclk_update_node_info_multiple_calls(void **state) {
    struct update_node_info info1 = {.dummy = 1};
    struct update_node_info info2 = {.dummy = 2};
    struct update_node_info info3 = {.dummy = 3};
    
    // All calls should succeed
    aclk_update_node_info(&info1);
    aclk_update_node_info(&info2);
    aclk_update_node_info(&info3);
}

// Test: Interleaved calls to all four functions
static void test_interleaved_function_calls(void **state) {
    contexts_snapshot_t snapshot = {.dummy = 1};
    contexts_updated_t update = {.dummy = 2};
    struct update_node_collectors collectors = {.dummy = 3};
    struct update_node_info info = {.dummy = 4};
    
    // All calls should succeed in any order
    aclk_send_contexts_snapshot(snapshot);
    aclk_update_node_collectors(&collectors);
    aclk_send_contexts_updated(update);
    aclk_update_node_info(&info);
}

// Test: aclk_send_contexts_snapshot with large dummy value
static void test_aclk_send_contexts_snapshot_large_value(void **state) {
    contexts_snapshot_t snapshot = {.dummy = 0x7FFFFFFF};
    
    // Should handle large values
    aclk_send_contexts_snapshot(snapshot);
}

// Test: aclk_send_contexts_updated with large dummy value
static void test_aclk_send_contexts_updated_large_value(void **state) {
    contexts_updated_t update = {.dummy = 0x7FFFFFFF};
    
    // Should handle large values
    aclk_send_contexts_updated(update);
}

// Test: aclk_update_node_collectors with large dummy value
static void test_aclk_update_node_collectors_large_value(void **state) {
    struct update_node_collectors collectors = {.dummy = 0x7FFFFFFF};
    
    // Should handle large values
    aclk_update_node_collectors(&collectors);
}

// Test: aclk_update_node_info with large dummy value
static void test_aclk_update_node_info_large_value(void **state) {
    struct update_node_info info = {.dummy = 0x7FFFFFFF};
    
    // Should handle large values
    aclk_update_node_info(&info);
}

// Test: aclk_send_contexts_snapshot with negative dummy value
static void test_aclk_send_contexts_snapshot_negative_value(void **state) {
    contexts_snapshot_t snapshot = {.dummy = -1};
    
    // Should handle negative values
    aclk_send_contexts_snapshot(snapshot);
}

// Test: aclk_send_contexts_updated with negative dummy value
static void test_aclk_send_contexts_updated_negative_value(void **state) {
    contexts_updated_t update = {.dummy = -1};
    
    // Should handle negative values
    aclk_send_contexts_updated(update);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_send_contexts_snapshot_valid_data),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_zero_data),
        cmocka_unit_test(test_aclk_send_contexts_updated_valid_data),
        cmocka_unit_test(test_aclk_send_contexts_updated_zero_data),
        cmocka_unit_test(test_aclk_update_node_collectors_valid_pointer),
        cmocka_unit_test(test_aclk_update_node_collectors_null_pointer),
        cmocka_unit_test(test_aclk_update_node_info_valid_pointer),
        cmocka_unit_test(test_aclk_update_node_info_null_pointer),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_multiple_calls),
        cmocka_unit_test(test_aclk_send_contexts_updated_multiple_calls),
        cmocka_unit_test(test_aclk_update_node_collectors_multiple_calls),
        cmocka_unit_test(test_aclk_update_node_info_multiple_calls),
        cmocka_unit_test(test_interleaved_function_calls),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_large_value),
        cmocka_unit_test(test_aclk_send_contexts_updated_large_value),
        cmocka_unit_test(test_aclk_update_node_collectors_large_value),
        cmocka_unit_test(test_aclk_update_node_info_large_value),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_negative_value),
        cmocka_unit_test(test_aclk_send_contexts_updated_negative_value),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}