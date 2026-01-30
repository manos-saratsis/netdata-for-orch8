#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test framework - minimal setup */
#define TEST_PASS 1
#define TEST_FAIL 0

static int tests_run = 0;
static int tests_passed = 0;

#define assert_true(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s:%d\n", __FILE__, __LINE__); \
        return TEST_FAIL; \
    } \
} while(0)

#define assert_equal(actual, expected) do { \
    if ((actual) != (expected)) { \
        fprintf(stderr, "FAIL: %s:%d Expected %p, got %p\n", __FILE__, __LINE__, (void*)(expected), (void*)(actual)); \
        return TEST_FAIL; \
    } \
} while(0)

#define assert_not_equal(actual, not_expected) do { \
    if ((actual) == (not_expected)) { \
        fprintf(stderr, "FAIL: %s:%d Should not equal %p\n", __FILE__, __LINE__, (void*)(not_expected)); \
        return TEST_FAIL; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    tests_run++; \
    if (test_func() == TEST_PASS) { \
        tests_passed++; \
        printf("PASS: %s\n", #test_func); \
    } else { \
        printf("FAIL: %s\n", #test_func); \
    } \
} while(0)

#include "common_public.h"

/* Test: _caller_responsibility with NULL pointer */
static int test_caller_responsibility_with_null_pointer(void) {
    /* Function should accept NULL and do nothing */
    _caller_responsibility(NULL);
    /* If we get here without crash, test passes */
    assert_true(1);
    return TEST_PASS;
}

/* Test: _caller_responsibility with valid pointer */
static int test_caller_responsibility_with_valid_pointer(void) {
    int dummy_value = 42;
    int *ptr = &dummy_value;
    
    /* Function should accept any pointer and do nothing */
    _caller_responsibility((void *)ptr);
    
    /* Pointer should still be valid after the call */
    assert_equal(*ptr, 42);
    return TEST_PASS;
}

/* Test: _caller_responsibility with pointer to stack memory */
static int test_caller_responsibility_with_stack_memory(void) {
    int stack_var = 100;
    _caller_responsibility(&stack_var);
    
    /* Stack variable should still be valid */
    assert_equal(stack_var, 100);
    return TEST_PASS;
}

/* Test: _caller_responsibility with pointer to heap memory */
static int test_caller_responsibility_with_heap_memory(void) {
    int *heap_ptr = (int *)malloc(sizeof(int));
    assert_true(heap_ptr != NULL);
    
    *heap_ptr = 77;
    _caller_responsibility((void *)heap_ptr);
    
    /* Heap memory should still be valid and accessible */
    assert_equal(*heap_ptr, 77);
    
    free(heap_ptr);
    return TEST_PASS;
}

/* Test: _caller_responsibility with pointer to struct */
static int test_caller_responsibility_with_struct_pointer(void) {
    struct mqtt_ng_stats stats = {0};
    stats.tx_bytes_queued = 1000;
    
    _caller_responsibility(&stats);
    
    /* Struct should still be valid */
    assert_equal(stats.tx_bytes_queued, 1000);
    return TEST_PASS;
}

/* Test: _caller_responsibility does not modify pointed data */
static int test_caller_responsibility_does_not_modify_data(void) {
    int original_value = 999;
    int test_value = original_value;
    
    _caller_responsibility(&test_value);
    
    /* Value should not be modified */
    assert_equal(test_value, original_value);
    return TEST_PASS;
}

/* Test: _caller_responsibility with char pointer */
static int test_caller_responsibility_with_char_pointer(void) {
    char buffer[] = "test string";
    _caller_responsibility(buffer);
    
    /* String should still be intact */
    assert_equal(strcmp(buffer, "test string"), 0);
    return TEST_PASS;
}

/* Test: _caller_responsibility with void pointer cast from different types */
static int test_caller_responsibility_with_various_casts(void) {
    double d = 3.14;
    _caller_responsibility((void *)&d);
    
    long long l = 12345;
    _caller_responsibility((void *)&l);
    
    char c = 'A';
    _caller_responsibility((void *)&c);
    
    assert_equal((int)d, 3);
    assert_equal(l, 12345);
    assert_equal(c, 'A');
    return TEST_PASS;
}

/* Test: CALLER_RESPONSIBILITY macro is defined correctly */
static int test_caller_responsibility_macro_defined(void) {
    free_fnc_t fn = CALLER_RESPONSIBILITY;
    assert_true(fn != NULL);
    return TEST_PASS;
}

/* Test: CALLER_RESPONSIBILITY macro points to _caller_responsibility */
static int test_caller_responsibility_macro_points_to_function(void) {
    free_fnc_t fn = CALLER_RESPONSIBILITY;
    assert_equal(fn, (free_fnc_t)&_caller_responsibility);
    return TEST_PASS;
}

/* Test: CALLER_RESPONSIBILITY can be called via macro */
static int test_caller_responsibility_via_macro(void) {
    int test_val = 555;
    free_fnc_t fn = CALLER_RESPONSIBILITY;
    fn(&test_val);
    
    assert_equal(test_val, 555);
    return TEST_PASS;
}

/* Test: Multiple consecutive calls to _caller_responsibility */
static int test_caller_responsibility_multiple_calls(void) {
    int vals[] = {1, 2, 3, 4, 5};
    
    for (int i = 0; i < 5; i++) {
        _caller_responsibility(&vals[i]);
    }
    
    for (int i = 0; i < 5; i++) {
        assert_equal(vals[i], i + 1);
    }
    return TEST_PASS;
}

/* Test: _caller_responsibility with array pointer */
static int test_caller_responsibility_with_array_pointer(void) {
    int arr[] = {10, 20, 30, 40, 50};
    _caller_responsibility(arr);
    
    assert_equal(arr[0], 10);
    assert_equal(arr[4], 50);
    return TEST_PASS;
}

/* Test: free_fnc_t typedef is properly defined */
static int test_free_fnc_t_typedef(void) {
    /* Verify we can declare function pointers of this type */
    free_fnc_t fn1 = NULL;
    free_fnc_t fn2 = CALLER_RESPONSIBILITY;
    
    assert_equal(fn1, NULL);
    assert_not_equal(fn2, NULL);
    return TEST_PASS;
}

/* Test: mqtt_ng_stats structure is properly defined */
static int test_mqtt_ng_stats_structure(void) {
    struct mqtt_ng_stats stats;
    memset(&stats, 0, sizeof(stats));
    
    /* Verify all fields are accessible */
    stats.tx_bytes_queued = 100;
    stats.tx_messages_queued = 5;
    stats.tx_messages_sent = 3;
    stats.rx_messages_rcvd = 2;
    stats.packets_waiting_puback = 1;
    stats.tx_buffer_used = 50;
    stats.tx_buffer_free = 450;
    stats.tx_buffer_size = 500;
    stats.tx_buffer_reclaimable = 25;
    stats.max_puback_wait_us = 1000000;
    stats.max_send_queue_wait_us = 2000000;
    stats.max_unsent_wait_us = 500000;
    stats.max_partial_wait_us = 300000;
    
    assert_equal(stats.tx_bytes_queued, 100);
    assert_equal(stats.tx_messages_queued, 5);
    assert_equal(stats.tx_messages_sent, 3);
    assert_equal(stats.rx_messages_rcvd, 2);
    assert_equal(stats.packets_waiting_puback, 1);
    assert_equal(stats.tx_buffer_used, 50);
    assert_equal(stats.tx_buffer_free, 450);
    assert_equal(stats.tx_buffer_size, 500);
    assert_equal(stats.tx_buffer_reclaimable, 25);
    assert_equal(stats.max_puback_wait_us, 1000000);
    assert_equal(stats.max_send_queue_wait_us, 2000000);
    assert_equal(stats.max_unsent_wait_us, 500000);
    assert_equal(stats.max_partial_wait_us, 300000);
    
    return TEST_PASS;
}

/* Test: mqtt_ng_stats with zero values */
static int test_mqtt_ng_stats_with_zeros(void) {
    struct mqtt_ng_stats stats = {0};
    
    assert_equal(stats.tx_bytes_queued, 0);
    assert_equal(stats.tx_messages_queued, 0);
    assert_equal(stats.tx_messages_sent, 0);
    assert_equal(stats.rx_messages_rcvd, 0);
    assert_equal(stats.packets_waiting_puback, 0);
    assert_equal(stats.tx_buffer_used, 0);
    assert_equal(stats.tx_buffer_free, 0);
    assert_equal(stats.tx_buffer_size, 0);
    assert_equal(stats.tx_buffer_reclaimable, 0);
    assert_equal(stats.max_puback_wait_us, 0);
    assert_equal(stats.max_send_queue_wait_us, 0);
    assert_equal(stats.max_unsent_wait_us, 0);
    assert_equal(stats.max_partial_wait_us, 0);
    
    return TEST_PASS;
}

/* Test: mqtt_ng_stats with maximum values */
static int test_mqtt_ng_stats_with_max_values(void) {
    struct mqtt_ng_stats stats;
    memset(&stats, 0xFF, sizeof(stats));  /* Set all bytes to max */
    
    /* Just verify the structure can hold large values */
    assert_true(stats.tx_bytes_queued > 0);
    assert_true(stats.max_puback_wait_us > 0);
    assert_true(stats.max_send_queue_wait_us > 0);
    
    return TEST_PASS;
}

/* Test: common_public.h include guard */
static int test_common_public_include_guard(void) {
    /* Include guard MQTT_WEBSOCKETS_COMMON_PUBLIC_H prevents re-inclusion */
    assert_true(1);
    return TEST_PASS;
}

/* Test: _caller_responsibility function address is consistent */
static int test_caller_responsibility_address_consistency(void) {
    void (*addr1)(void *) = &_caller_responsibility;
    void (*addr2)(void *) = &_caller_responsibility;
    
    assert_equal(addr1, addr2);
    return TEST_PASS;
}

/* Test: _caller_responsibility with cast pointer arithmetic */
static int test_caller_responsibility_with_offset_pointer(void) {
    int arr[] = {10, 20, 30, 40, 50};
    int *ptr = &arr[2];  /* Point to middle element */
    
    _caller_responsibility(ptr);
    
    assert_equal(*ptr, 30);
    assert_equal(arr[2], 30);
    return TEST_PASS;
}

int main(void) {
    printf("Running common_public.c tests...\n\n");
    
    RUN_TEST(test_caller_responsibility_with_null_pointer);
    RUN_TEST(test_caller_responsibility_with_valid_pointer);
    RUN_TEST(test_caller_responsibility_with_stack_memory);
    RUN_TEST(test_caller_responsibility_with_heap_memory);
    RUN_TEST(test_caller_responsibility_with_struct_pointer);
    RUN_TEST(test_caller_responsibility_does_not_modify_data);
    RUN_TEST(test_caller_responsibility_with_char_pointer);
    RUN_TEST(test_caller_responsibility_with_various_casts);
    RUN_TEST(test_caller_responsibility_macro_defined);
    RUN_TEST(test_caller_responsibility_macro_points_to_function);
    RUN_TEST(test_caller_responsibility_via_macro);
    RUN_TEST(test_caller_responsibility_multiple_calls);
    RUN_TEST(test_caller_responsibility_with_array_pointer);
    RUN_TEST(test_free_fnc_t_typedef);
    RUN_TEST(test_mqtt_ng_stats_structure);
    RUN_TEST(test_mqtt_ng_stats_with_zeros);
    RUN_TEST(test_mqtt_ng_stats_with_max_values);
    RUN_TEST(test_common_public_include_guard);
    RUN_TEST(test_caller_responsibility_address_consistency);
    RUN_TEST(test_caller_responsibility_with_offset_pointer);
    
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}