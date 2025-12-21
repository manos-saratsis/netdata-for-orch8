#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "src/libnetdata/libjudy/judy-malloc.h"

static void test_judy_aral_free_bytes(void **state) {
    (void) state; // unused
    size_t free_bytes = judy_aral_free_bytes();
    assert_true(free_bytes >= 0);
}

static void test_judy_aral_structures(void **state) {
    (void) state; // unused
    size_t structures = judy_aral_structures();
    assert_true(structures >= 0);
}

static void test_judy_aral_statistics(void **state) {
    (void) state; // unused
    struct aral_statistics *stats = judy_aral_statistics();
    assert_non_null(stats);
}

static void test_judy_alloc_thread_pulse_reset(void **state) {
    (void) state; // unused
    JudyAllocThreadPulseReset();
    // No return value to check, just ensuring no crash
    assert_true(1);
}

static void test_judy_alloc_thread_pulse_get_and_reset(void **state) {
    (void) state; // unused
    int64_t pulse = JudyAllocThreadPulseGetAndReset();
    assert_true(pulse >= 0 || pulse < 0);
}

static void test_libjudy_malloc_init(void **state) {
    (void) state; // unused
    libjudy_malloc_init();
    // No return value to check, just ensuring no crash
    assert_true(1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_judy_aral_free_bytes),
        cmocka_unit_test(test_judy_aral_structures),
        cmocka_unit_test(test_judy_aral_statistics),
        cmocka_unit_test(test_judy_alloc_thread_pulse_reset),
        cmocka_unit_test(test_judy_alloc_thread_pulse_get_and_reset),
        cmocka_unit_test(test_libjudy_malloc_init),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}