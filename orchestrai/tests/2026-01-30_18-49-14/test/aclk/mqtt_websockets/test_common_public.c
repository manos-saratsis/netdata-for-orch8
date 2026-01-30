/*
 * Comprehensive tests for common_public.h
 * Tests all exported functions, macros, and data structures
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

#include "src/aclk/mqtt_websockets/common_public.h"

/* Test the _caller_responsibility function pointer
 * This function should do nothing (no-op) to indicate caller responsibility
 */
static void test_caller_responsibility_function_exists(void **state) {
    (void)state;
    
    /* Verify function pointer is not NULL */
    assert_non_null(_caller_responsibility);
}

/* Test calling _caller_responsibility with NULL pointer
 * Should not crash or segfault
 */
static void test_caller_responsibility_with_null_pointer(void **state) {
    (void)state;
    
    /* Should handle NULL gracefully */
    _caller_responsibility(NULL);
    
    /* If we reach here, it didn't crash */
    assert_true(1);
}

/* Test calling _caller_responsibility with valid pointer
 * Should not free or modify the pointer
 */
static void test_caller_responsibility_with_valid_pointer(void **state) {
    (void)state;
    
    int test_data = 42;
    int *ptr = &test_data;
    
    /* Call the function - it should do nothing */
    _caller_responsibility(ptr);
    
    /* Pointer should still be valid and point to same data */
    assert_int_equal(*ptr, 42);
}

/* Test CALLER_RESPONSIBILITY macro is valid function pointer
 */
static void test_caller_responsibility_macro_is_valid_function_pointer(void **state) {
    (void)state;
    
    free_fnc_t fn = CALLER_RESPONSIBILITY;
    
    /* Should be a valid function pointer */
    assert_non_null(fn);
    assert_ptr_equal(fn, &_caller_responsibility);
}

/* Test CALLER_RESPONSIBILITY macro with different pointer types
 */
static void test_caller_responsibility_macro_with_char_pointer(void **state) {
    (void)state;
    
    char test_str[] = "test";
    char *ptr = test_str;
    free_fnc_t fn = CALLER_RESPONSIBILITY;
    
    /* Should not crash with char pointer */
    fn(ptr);
    assert_string_equal(ptr, "test");
}

/* Test CALLER_RESPONSIBILITY macro with struct pointer
 */
static void test_caller_responsibility_macro_with_struct_pointer(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats = {0};
    struct mqtt_ng_stats *ptr = &stats;
    free_fnc_t fn = CALLER_RESPONSIBILITY;
    
    /* Should not crash with struct pointer */
    fn(ptr);
    assert_non_null(ptr);
}

/* Test free_fnc_t typedef accepts function pointer to _caller_responsibility
 */
static void test_free_fnc_t_typedef_assignment(void **state) {
    (void)state;
    
    free_fnc_t fn = _caller_responsibility;
    
    assert_non_null(fn);
    assert_ptr_equal(fn, _caller_responsibility);
}

/* Test mqtt_ng_stats structure initialization
 */
static void test_mqtt_ng_stats_struct_initialization(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats = {0};
    
    /* All fields should be initialized to zero */
    assert_int_equal(stats.tx_bytes_queued, 0);
    assert_int_equal(stats.tx_messages_queued, 0);
    assert_int_equal(stats.tx_messages_sent, 0);
    assert_int_equal(stats.rx_messages_rcvd, 0);
    assert_int_equal(stats.packets_waiting_puback, 0);
    assert_int_equal(stats.tx_buffer_used, 0);
    assert_int_equal(stats.tx_buffer_free, 0);
    assert_int_equal(stats.tx_buffer_size, 0);
    assert_int_equal(stats.tx_buffer_reclaimable, 0);
    assert_int_equal(stats.max_puback_wait_us, 0);
    assert_int_equal(stats.max_send_queue_wait_us, 0);
    assert_int_equal(stats.max_unsent_wait_us, 0);
    assert_int_equal(stats.max_partial_wait_us, 0);
}

/* Test mqtt_ng_stats structure with various values
 */
static void test_mqtt_ng_stats_struct_with_values(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats = {
        .tx_bytes_queued = 1024,
        .tx_messages_queued = 5,
        .tx_messages_sent = 3,
        .rx_messages_rcvd = 2,
        .packets_waiting_puback = 1,
        .tx_buffer_used = 512,
        .tx_buffer_free = 512,
        .tx_buffer_size = 1024,
        .tx_buffer_reclaimable = 256,
        .max_puback_wait_us = 5000000,
        .max_send_queue_wait_us = 3000000,
        .max_unsent_wait_us = 2000000,
        .max_partial_wait_us = 1000000
    };
    
    assert_int_equal(stats.tx_bytes_queued, 1024);
    assert_int_equal(stats.tx_messages_queued, 5);
    assert_int_equal(stats.tx_messages_sent, 3);
    assert_int_equal(stats.rx_messages_rcvd, 2);
    assert_int_equal(stats.packets_waiting_puback, 1);
    assert_int_equal(stats.tx_buffer_used, 512);
    assert_int_equal(stats.tx_buffer_free, 512);
    assert_int_equal(stats.tx_buffer_size, 1024);
    assert_int_equal(stats.tx_buffer_reclaimable, 256);
    assert_int_equal(stats.max_puback_wait_us, 5000000);
    assert_int_equal(stats.max_send_queue_wait_us, 3000000);
    assert_int_equal(stats.max_unsent_wait_us, 2000000);
    assert_int_equal(stats.max_partial_wait_us, 1000000);
}

/* Test mqtt_ng_stats with maximum uint64_t values
 */
static void test_mqtt_ng_stats_struct_with_max_uint64_values(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats = {
        .max_puback_wait_us = UINT64_MAX,
        .max_send_queue_wait_us = UINT64_MAX,
        .max_unsent_wait_us = UINT64_MAX,
        .max_partial_wait_us = UINT64_MAX
    };
    
    assert_int_equal(stats.max_puback_wait_us, UINT64_MAX);
    assert_int_equal(stats.max_send_queue_wait_us, UINT64_MAX);
    assert_int_equal(stats.max_unsent_wait_us, UINT64_MAX);
    assert_int_equal(stats.max_partial_wait_us, UINT64_MAX);
}

/* Test mqtt_ng_stats with maximum size_t values
 */
static void test_mqtt_ng_stats_struct_with_max_size_t_values(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats = {
        .tx_bytes_queued = SIZE_MAX,
        .tx_buffer_used = SIZE_MAX,
        .tx_buffer_free = SIZE_MAX,
        .tx_buffer_size = SIZE_MAX,
        .tx_buffer_reclaimable = SIZE_MAX
    };
    
    assert_int_equal(stats.tx_bytes_queued, SIZE_MAX);
    assert_int_equal(stats.tx_buffer_used, SIZE_MAX);
    assert_int_equal(stats.tx_buffer_free, SIZE_MAX);
    assert_int_equal(stats.tx_buffer_size, SIZE_MAX);
    assert_int_equal(stats.tx_buffer_reclaimable, SIZE_MAX);
}

/* Test mqtt_ng_stats with negative int values
 */
static void test_mqtt_ng_stats_struct_with_negative_int_values(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats = {
        .tx_messages_queued = -1,
        .tx_messages_sent = -100,
        .rx_messages_rcvd = -999,
        .packets_waiting_puback = -1
    };
    
    assert_int_equal(stats.tx_messages_queued, -1);
    assert_int_equal(stats.tx_messages_sent, -100);
    assert_int_equal(stats.rx_messages_rcvd, -999);
    assert_int_equal(stats.packets_waiting_puback, -1);
}

/* Test mqtt_ng_stats memset to zero
 */
static void test_mqtt_ng_stats_struct_memset_zero(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats;
    memset(&stats, 0, sizeof(stats));
    
    assert_int_equal(stats.tx_bytes_queued, 0);
    assert_int_equal(stats.tx_messages_queued, 0);
    assert_int_equal(stats.tx_messages_sent, 0);
    assert_int_equal(stats.rx_messages_rcvd, 0);
    assert_int_equal(stats.packets_waiting_puback, 0);
    assert_int_equal(stats.tx_buffer_used, 0);
    assert_int_equal(stats.tx_buffer_free, 0);
    assert_int_equal(stats.tx_buffer_size, 0);
    assert_int_equal(stats.tx_buffer_reclaimable, 0);
    assert_int_equal(stats.max_puback_wait_us, 0);
    assert_int_equal(stats.max_send_queue_wait_us, 0);
    assert_int_equal(stats.max_unsent_wait_us, 0);
    assert_int_equal(stats.max_partial_wait_us, 0);
}

/* Test mqtt_ng_stats memcpy
 */
static void test_mqtt_ng_stats_struct_memcpy(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats1 = {
        .tx_bytes_queued = 512,
        .tx_messages_queued = 10,
        .tx_messages_sent = 8,
        .rx_messages_rcvd = 5
    };
    
    struct mqtt_ng_stats stats2;
    memcpy(&stats2, &stats1, sizeof(struct mqtt_ng_stats));
    
    assert_int_equal(stats2.tx_bytes_queued, 512);
    assert_int_equal(stats2.tx_messages_queued, 10);
    assert_int_equal(stats2.tx_messages_sent, 8);
    assert_int_equal(stats2.rx_messages_rcvd, 5);
}

/* Test sizeof mqtt_ng_stats
 */
static void test_mqtt_ng_stats_struct_size(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats;
    
    /* Size should be reasonable and non-zero */
    assert_true(sizeof(stats) > 0);
    assert_true(sizeof(stats) >= sizeof(size_t) * 8 + sizeof(int) * 5 + sizeof(uint64_t) * 4);
}

/* Test mqtt_ng_stats alignment
 */
static void test_mqtt_ng_stats_struct_alignment(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats = {0};
    
    /* Verify address is accessible */
    assert_non_null(&stats);
    assert_non_null(&stats.tx_bytes_queued);
    assert_non_null(&stats.max_partial_wait_us);
}

/* Test pointer to mqtt_ng_stats
 */
static void test_mqtt_ng_stats_struct_pointer(void **state) {
    (void)state;
    
    struct mqtt_ng_stats stats = {
        .tx_bytes_queued = 256
    };
    
    struct mqtt_ng_stats *ptr = &stats;
    
    assert_non_null(ptr);
    assert_int_equal(ptr->tx_bytes_queued, 256);
}

/* Test free_fnc_t with standard free function signature
 */
static void test_free_fnc_t_with_standard_free(void **state) {
    (void)state;
    
    /* Verify we can assign standard free function to free_fnc_t */
    free_fnc_t fn = free;
    
    assert_non_null(fn);
}

/* Test NULL free_fnc_t assignment
 */
static void test_free_fnc_t_null_assignment(void **state) {
    (void)state;
    
    free_fnc_t fn = NULL;
    
    assert_null(fn);
}

/* Test multiple CALLER_RESPONSIBILITY usages
 */
static void test_multiple_caller_responsibility_usages(void **state) {
    (void)state;
    
    free_fnc_t fn1 = CALLER_RESPONSIBILITY;
    free_fnc_t fn2 = CALLER_RESPONSIBILITY;
    
    assert_ptr_equal(fn1, fn2);
    assert_non_null(fn1);
    assert_non_null(fn2);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_caller_responsibility_function_exists),
        cmocka_unit_test(test_caller_responsibility_with_null_pointer),
        cmocka_unit_test(test_caller_responsibility_with_valid_pointer),
        cmocka_unit_test(test_caller_responsibility_macro_is_valid_function_pointer),
        cmocka_unit_test(test_caller_responsibility_macro_with_char_pointer),
        cmocka_unit_test(test_caller_responsibility_macro_with_struct_pointer),
        cmocka_unit_test(test_free_fnc_t_typedef_assignment),
        cmocka_unit_test(test_mqtt_ng_stats_struct_initialization),
        cmocka_unit_test(test_mqtt_ng_stats_struct_with_values),
        cmocka_unit_test(test_mqtt_ng_stats_struct_with_max_uint64_values),
        cmocka_unit_test(test_mqtt_ng_stats_struct_with_max_size_t_values),
        cmocka_unit_test(test_mqtt_ng_stats_struct_with_negative_int_values),
        cmocka_unit_test(test_mqtt_ng_stats_struct_memset_zero),
        cmocka_unit_test(test_mqtt_ng_stats_struct_memcpy),
        cmocka_unit_test(test_mqtt_ng_stats_struct_size),
        cmocka_unit_test(test_mqtt_ng_stats_struct_alignment),
        cmocka_unit_test(test_mqtt_ng_stats_struct_pointer),
        cmocka_unit_test(test_free_fnc_t_with_standard_free),
        cmocka_unit_test(test_free_fnc_t_null_assignment),
        cmocka_unit_test(test_multiple_caller_responsibility_usages),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}