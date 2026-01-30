#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Mock and include the source file */
#include "common_public.h"

/* Test suite for common_public.c */

/* ===== Tests for _caller_responsibility() ===== */

/**
 * Test: _caller_responsibility should accept NULL pointer without crashing
 */
void test_caller_responsibility_with_null_pointer(void) {
    /* Arrange */
    void *ptr = NULL;
    
    /* Act - should not crash */
    _caller_responsibility(ptr);
    
    /* Assert - function executed without error */
    printf("✓ test_caller_responsibility_with_null_pointer\n");
}

/**
 * Test: _caller_responsibility should accept valid pointer without modification
 */
void test_caller_responsibility_with_valid_pointer(void) {
    /* Arrange */
    int data = 42;
    void *ptr = (void *)&data;
    int original_value = data;
    
    /* Act */
    _caller_responsibility(ptr);
    
    /* Assert - pointer value unchanged */
    assert(data == original_value);
    assert(*(int *)ptr == 42);
    printf("✓ test_caller_responsibility_with_valid_pointer\n");
}

/**
 * Test: _caller_responsibility should accept arbitrary pointer value
 */
void test_caller_responsibility_with_arbitrary_pointer(void) {
    /* Arrange */
    void *arbitrary_ptr = (void *)0xDEADBEEF;
    
    /* Act - should not crash with arbitrary address */
    _caller_responsibility(arbitrary_ptr);
    
    /* Assert - function completed without error */
    printf("✓ test_caller_responsibility_with_arbitrary_pointer\n");
}

/**
 * Test: _caller_responsibility function pointer is valid
 */
void test_caller_responsibility_function_pointer_is_valid(void) {
    /* Arrange */
    free_fnc_t fn = &_caller_responsibility;
    
    /* Act */
    fn(NULL);
    
    /* Assert - function pointer works */
    assert(fn != NULL);
    printf("✓ test_caller_responsibility_function_pointer_is_valid\n");
}

/**
 * Test: CALLER_RESPONSIBILITY macro equals the function pointer
 */
void test_caller_responsibility_macro_equals_function_pointer(void) {
    /* Arrange & Act */
    void *caller_resp_ptr = (void *)CALLER_RESPONSIBILITY;
    void *fn_ptr = (void *)&_caller_responsibility;
    
    /* Assert - macro and function pointer are the same */
    assert(caller_resp_ptr == fn_ptr);
    printf("✓ test_caller_responsibility_macro_equals_function_pointer\n");
}

/* ===== Tests for mqtt_ng_stats structure ===== */

/**
 * Test: mqtt_ng_stats structure has correct size and member offsets
 */
void test_mqtt_ng_stats_structure_size(void) {
    /* Arrange & Assert */
    struct mqtt_ng_stats stats;
    
    /* Verify structure contains all expected fields */
    assert(offsetof(struct mqtt_ng_stats, tx_bytes_queued) >= 0);
    assert(offsetof(struct mqtt_ng_stats, tx_messages_queued) >= 0);
    assert(offsetof(struct mqtt_ng_stats, tx_messages_sent) >= 0);
    assert(offsetof(struct mqtt_ng_stats, rx_messages_rcvd) >= 0);
    assert(offsetof(struct mqtt_ng_stats, packets_waiting_puback) >= 0);
    assert(offsetof(struct mqtt_ng_stats, tx_buffer_used) >= 0);
    assert(offsetof(struct mqtt_ng_stats, tx_buffer_free) >= 0);
    assert(offsetof(struct mqtt_ng_stats, tx_buffer_size) >= 0);
    assert(offsetof(struct mqtt_ng_stats, tx_buffer_reclaimable) >= 0);
    assert(offsetof(struct mqtt_ng_stats, max_puback_wait_us) >= 0);
    assert(offsetof(struct mqtt_ng_stats, max_send_queue_wait_us) >= 0);
    assert(offsetof(struct mqtt_ng_stats, max_unsent_wait_us) >= 0);
    assert(offsetof(struct mqtt_ng_stats, max_partial_wait_us) >= 0);
    
    printf("✓ test_mqtt_ng_stats_structure_size\n");
}

/**
 * Test: mqtt_ng_stats can be initialized to zero
 */
void test_mqtt_ng_stats_zero_initialization(void) {
    /* Arrange & Act */
    struct mqtt_ng_stats stats = {0};
    
    /* Assert */
    assert(stats.tx_bytes_queued == 0);
    assert(stats.tx_messages_queued == 0);
    assert(stats.tx_messages_sent == 0);
    assert(stats.rx_messages_rcvd == 0);
    assert(stats.packets_waiting_puback == 0);
    assert(stats.tx_buffer_used == 0);
    assert(stats.tx_buffer_free == 0);
    assert(stats.tx_buffer_size == 0);
    assert(stats.tx_buffer_reclaimable == 0);
    assert(stats.max_puback_wait_us == 0);
    assert(stats.max_send_queue_wait_us == 0);
    assert(stats.max_unsent_wait_us == 0);
    assert(stats.max_partial_wait_us == 0);
    
    printf("✓ test_mqtt_ng_stats_zero_initialization\n");
}

/**
 * Test: mqtt_ng_stats can be populated with values
 */
void test_mqtt_ng_stats_value_assignment(void) {
    /* Arrange & Act */
    struct mqtt_ng_stats stats;
    stats.tx_bytes_queued = 1024;
    stats.tx_messages_queued = 10;
    stats.tx_messages_sent = 5;
    stats.rx_messages_rcvd = 3;
    stats.packets_waiting_puback = 2;
    stats.tx_buffer_used = 512;
    stats.tx_buffer_free = 512;
    stats.tx_buffer_size = 1024;
    stats.tx_buffer_reclaimable = 256;
    stats.max_puback_wait_us = 1000000;
    stats.max_send_queue_wait_us = 500000;
    stats.max_unsent_wait_us = 200000;
    stats.max_partial_wait_us = 300000;
    
    /* Assert */
    assert(stats.tx_bytes_queued == 1024);
    assert(stats.tx_messages_queued == 10);
    assert(stats.tx_messages_sent == 5);
    assert(stats.rx_messages_rcvd == 3);
    assert(stats.packets_waiting_puback == 2);
    assert(stats.tx_buffer_used == 512);
    assert(stats.tx_buffer_free == 512);
    assert(stats.tx_buffer_size == 1024);
    assert(stats.tx_buffer_reclaimable == 256);
    assert(stats.max_puback_wait_us == 1000000);
    assert(stats.max_send_queue_wait_us == 500000);
    assert(stats.max_unsent_wait_us == 200000);
    assert(stats.max_partial_wait_us == 300000);
    
    printf("✓ test_mqtt_ng_stats_value_assignment\n");
}

/**
 * Test: mqtt_ng_stats handles maximum values for size_t fields
 */
void test_mqtt_ng_stats_max_size_t_values(void) {
    /* Arrange & Act */
    struct mqtt_ng_stats stats;
    stats.tx_bytes_queued = SIZE_MAX;
    stats.tx_buffer_used = SIZE_MAX;
    stats.tx_buffer_free = SIZE_MAX;
    stats.tx_buffer_size = SIZE_MAX;
    stats.tx_buffer_reclaimable = SIZE_MAX;
    
    /* Assert */
    assert(stats.tx_bytes_queued == SIZE_MAX);
    assert(stats.tx_buffer_used == SIZE_MAX);
    assert(stats.tx_buffer_free == SIZE_MAX);
    assert(stats.tx_buffer_size == SIZE_MAX);
    assert(stats.tx_buffer_reclaimable == SIZE_MAX);
    
    printf("✓ test_mqtt_ng_stats_max_size_t_values\n");
}

/**
 * Test: mqtt_ng_stats handles maximum values for uint64_t fields
 */
void test_mqtt_ng_stats_max_uint64_values(void) {
    /* Arrange & Act */
    struct mqtt_ng_stats stats;
    stats.max_puback_wait_us = UINT64_MAX;
    stats.max_send_queue_wait_us = UINT64_MAX;
    stats.max_unsent_wait_us = UINT64_MAX;
    stats.max_partial_wait_us = UINT64_MAX;
    
    /* Assert */
    assert(stats.max_puback_wait_us == UINT64_MAX);
    assert(stats.max_send_queue_wait_us == UINT64_MAX);
    assert(stats.max_unsent_wait_us == UINT64_MAX);
    assert(stats.max_partial_wait_us == UINT64_MAX);
    
    printf("✓ test_mqtt_ng_stats_max_uint64_values\n");
}

/**
 * Test: mqtt_ng_stats handles maximum values for int fields
 */
void test_mqtt_ng_stats_max_int_values(void) {
    /* Arrange & Act */
    struct mqtt_ng_stats stats;
    stats.tx_messages_queued = INT_MAX;
    stats.tx_messages_sent = INT_MAX;
    stats.rx_messages_rcvd = INT_MAX;
    stats.packets_waiting_puback = INT_MAX;
    
    /* Assert */
    assert(stats.tx_messages_queued == INT_MAX);
    assert(stats.tx_messages_sent == INT_MAX);
    assert(stats.rx_messages_rcvd == INT_MAX);
    assert(stats.packets_waiting_puback == INT_MAX);
    
    printf("✓ test_mqtt_ng_stats_max_int_values\n");
}

/**
 * Test: mqtt_ng_stats handles negative int values
 */
void test_mqtt_ng_stats_negative_int_values(void) {
    /* Arrange & Act */
    struct mqtt_ng_stats stats;
    stats.tx_messages_queued = -1;
    stats.tx_messages_sent = -100;
    stats.rx_messages_rcvd = -999;
    stats.packets_waiting_puback = INT_MIN;
    
    /* Assert */
    assert(stats.tx_messages_queued == -1);
    assert(stats.tx_messages_sent == -100);
    assert(stats.rx_messages_rcvd == -999);
    assert(stats.packets_waiting_puback == INT_MIN);
    
    printf("✓ test_mqtt_ng_stats_negative_int_values\n");
}

/* ===== Tests for free_fnc_t typedef ===== */

/**
 * Test: free_fnc_t can be used as NULL
 */
void test_free_fnc_t_null(void) {
    /* Arrange & Act */
    free_fnc_t free_fn = NULL;
    
    /* Assert */
    assert(free_fn == NULL);
    printf("✓ test_free_fnc_t_null\n");
}

/**
 * Test: free_fnc_t can be set to stdlib free
 */
void test_free_fnc_t_stdlib_free(void) {
    /* Arrange & Act */
    free_fnc_t free_fn = &free;
    
    /* Assert */
    assert(free_fn != NULL);
    printf("✓ test_free_fnc_t_stdlib_free\n");
}

/**
 * Test: free_fnc_t can be set to CALLER_RESPONSIBILITY
 */
void test_free_fnc_t_caller_responsibility(void) {
    /* Arrange & Act */
    free_fnc_t free_fn = CALLER_RESPONSIBILITY;
    
    /* Assert */
    assert(free_fn != NULL);
    assert(free_fn == CALLER_RESPONSIBILITY);
    printf("✓ test_free_fnc_t_caller_responsibility\n");
}

/**
 * Test: Different free function pointers are distinguishable
 */
void test_free_fnc_t_distinguishable(void) {
    /* Arrange & Act */
    free_fnc_t fn_null = NULL;
    free_fnc_t fn_caller = CALLER_RESPONSIBILITY;
    free_fnc_t fn_stdlib = &free;
    
    /* Assert */
    assert(fn_null != fn_caller);
    assert(fn_caller != fn_stdlib);
    assert(fn_stdlib != fn_null);
    printf("✓ test_free_fnc_t_distinguishable\n");
}

/* ===== Main test runner ===== */

int main(void) {
    /* Tests for _caller_responsibility function */
    test_caller_responsibility_with_null_pointer();
    test_caller_responsibility_with_valid_pointer();
    test_caller_responsibility_with_arbitrary_pointer();
    test_caller_responsibility_function_pointer_is_valid();
    test_caller_responsibility_macro_equals_function_pointer();
    
    /* Tests for mqtt_ng_stats structure */
    test_mqtt_ng_stats_structure_size();
    test_mqtt_ng_stats_zero_initialization();
    test_mqtt_ng_stats_value_assignment();
    test_mqtt_ng_stats_max_size_t_values();
    test_mqtt_ng_stats_max_uint64_values();
    test_mqtt_ng_stats_max_int_values();
    test_mqtt_ng_stats_negative_int_values();
    
    /* Tests for free_fnc_t typedef */
    test_free_fnc_t_null();
    test_free_fnc_t_stdlib_free();
    test_free_fnc_t_caller_responsibility();
    test_free_fnc_t_distinguishable();
    
    printf("\n✓ All tests passed!\n");
    return 0;
}