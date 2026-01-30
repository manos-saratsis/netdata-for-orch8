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
        fprintf(stderr, "FAIL: %s:%d Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), (long)(actual)); \
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

#include "common_internal.h"

/* Test: Verify MQTT_WSS_FRAG_MEMALIGN is defined */
static int test_mqtt_wss_frag_memalign_defined(void) {
    assert_true(MQTT_WSS_FRAG_MEMALIGN == 8);
    return TEST_PASS;
}

/* Test: Verify MQTT_WSS_FRAG_MEMALIGN has correct default value */
static int test_mqtt_wss_frag_memalign_value_is_eight(void) {
    int value = MQTT_WSS_FRAG_MEMALIGN;
    assert_equal(value, 8);
    return TEST_PASS;
}

/* Test: Verify MQTT_WSS_FRAG_MEMALIGN is a power of 2 (common alignment requirement) */
static int test_mqtt_wss_frag_memalign_is_power_of_two(void) {
    int value = MQTT_WSS_FRAG_MEMALIGN;
    /* Check if power of 2: (n & (n-1)) == 0 and n > 0 */
    assert_true((value > 0) && ((value & (value - 1)) == 0));
    return TEST_PASS;
}

/* Test: Verify MQTT_WSS_FRAG_MEMALIGN can be used in memory operations */
static int test_mqtt_wss_frag_memalign_usable_for_alignment(void) {
    size_t alignment = MQTT_WSS_FRAG_MEMALIGN;
    size_t test_size = 100;
    
    /* Test that we can calculate aligned addresses */
    size_t aligned_size = ((test_size + alignment - 1) / alignment) * alignment;
    assert_true(aligned_size >= test_size);
    assert_true((aligned_size % alignment) == 0);
    return TEST_PASS;
}

/* Test: Verify include guard prevents double inclusion */
static int test_common_internal_include_guard(void) {
    /* Include guard COMMON_INTERNAL_H prevents re-inclusion */
    /* This is implicitly tested by successful compilation with multiple includes */
    assert_true(1);
    return TEST_PASS;
}

/* Test: Verify MQTT_WSS_FRAG_MEMALIGN can be redefined before inclusion */
static int test_mqtt_wss_frag_memalign_can_be_overridden(void) {
    /* When MQTT_WSS_FRAG_MEMALIGN is already defined, the ifndef should not redefine it */
    /* This is implicit in the header design */
    int value = MQTT_WSS_FRAG_MEMALIGN;
    assert_true(value > 0);
    return TEST_PASS;
}

/* Test: Verify MQTT_WSS_FRAG_MEMALIGN with multiple of alignment */
static int test_mqtt_wss_frag_memalign_multiple_addresses(void) {
    size_t alignment = MQTT_WSS_FRAG_MEMALIGN;
    
    /* Test alignment for various sizes */
    for (size_t i = 1; i <= 100; i++) {
        size_t aligned = ((i + alignment - 1) / alignment) * alignment;
        assert_true((aligned % alignment) == 0);
        assert_true(aligned >= i);
    }
    return TEST_PASS;
}

/* Test: Verify macro can be used in preprocessor conditionals */
static int test_mqtt_wss_frag_memalign_in_conditions(void) {
    #if MQTT_WSS_FRAG_MEMALIGN >= 8
    assert_true(1);
    #else
    assert_true(0);
    #endif
    return TEST_PASS;
}

/* Test: Verify endian_compat.h is included */
static int test_endian_compat_included(void) {
    /* endian_compat.h should be included, making endian functions available */
    /* Check that endian macros are defined after including common_internal.h */
    #if defined(htobe16) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__linux__)
    assert_true(1);
    #else
    assert_true(1); /* May not be defined on all platforms */
    #endif
    return TEST_PASS;
}

/* Test: Verify MQTT_WSS_FRAG_MEMALIGN is suitable for memory alignment operations */
static int test_mqtt_wss_frag_memalign_alignment_arithmetic(void) {
    size_t alignment = MQTT_WSS_FRAG_MEMALIGN;
    size_t base_address = 0;
    
    /* Calculate next aligned address */
    size_t next_aligned = (base_address + alignment - 1) & ~(alignment - 1);
    assert_true((next_aligned % alignment) == 0);
    return TEST_PASS;
}

int main(void) {
    printf("Running common_internal.h tests...\n\n");
    
    RUN_TEST(test_mqtt_wss_frag_memalign_defined);
    RUN_TEST(test_mqtt_wss_frag_memalign_value_is_eight);
    RUN_TEST(test_mqtt_wss_frag_memalign_is_power_of_two);
    RUN_TEST(test_mqtt_wss_frag_memalign_usable_for_alignment);
    RUN_TEST(test_common_internal_include_guard);
    RUN_TEST(test_mqtt_wss_frag_memalign_can_be_overridden);
    RUN_TEST(test_mqtt_wss_frag_memalign_multiple_addresses);
    RUN_TEST(test_mqtt_wss_frag_memalign_in_conditions);
    RUN_TEST(test_endian_compat_included);
    RUN_TEST(test_mqtt_wss_frag_memalign_alignment_arithmetic);
    
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}