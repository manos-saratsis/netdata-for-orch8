#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "log2journal-hashed-key.h"

// Mock functions if needed
static void *mock_malloc(size_t size) {
    return mock_ptr_type(void *);
}

static void test_hashed_key_cleanup(void **state) {
    HASHED_KEY k = {
        .key = strdup("test_key"),
        .len = 8,
        .flags = HK_HASHTABLE_ALLOCATED,
    };
    k.value.txt = strdup("test_value");
    k.value.len = 10;

    // Simulate cleanup
    hashed_key_cleanup(&k);

    // Assertions
    assert_null(k.key);
    assert_int_equal(k.len, 0);
    assert_int_equal(k.flags, HK_NONE);
}

static void test_hashed_key_set_auto_len(void **state) {
    HASHED_KEY k = {0};
    const char *test_key = "test_key";

    hashed_key_set(&k, test_key, -1);

    // Assertions
    assert_non_null(k.key);
    assert_string_equal(k.key, test_key);
    assert_int_equal(k.len, strlen(test_key));
    assert_int_not_equal(k.hash, 0);
}

static void test_hashed_key_set_explicit_len(void **state) {
    HASHED_KEY k = {0};
    const char *test_key = "test_key_long";

    hashed_key_set(&k, test_key, 8);  // Set explicit length

    // Assertions
    assert_non_null(k.key);
    assert_memory_equal(k.key, test_key, 8);
    assert_int_equal(k.len, 8);
    assert_int_not_equal(k.hash, 0);
}

static void test_hashed_keys_match_same_object(void **state) {
    HASHED_KEY k1 = {0};
    hashed_key_set(&k1, "test_key", -1);

    assert_true(hashed_keys_match(&k1, &k1));
}

static void test_hashed_keys_match_different_keys(void **state) {
    HASHED_KEY k1 = {0}, k2 = {0};
    hashed_key_set(&k1, "test_key1", -1);
    hashed_key_set(&k2, "test_key2", -1);

    assert_false(hashed_keys_match(&k1, &k2));
}

static void test_compare_keys(void **state) {
    HASHED_KEY k1 = {0}, k2 = {0}, k3 = {0};
    hashed_key_set(&k1, "a_key", -1);
    hashed_key_set(&k2, "b_key", -1);
    hashed_key_set(&k3, "a_key", -1);

    assert_true(compare_keys(&k1, &k1) == 0);
    assert_true(compare_keys(&k1, &k3) == 0);
    assert_true(compare_keys(&k1, &k2) < 0);
    assert_true(compare_keys(&k2, &k1) > 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_hashed_key_cleanup),
        cmocka_unit_test(test_hashed_key_set_auto_len),
        cmocka_unit_test(test_hashed_key_set_explicit_len),
        cmocka_unit_test(test_hashed_keys_match_same_object),
        cmocka_unit_test(test_hashed_keys_match_different_keys),
        cmocka_unit_test(test_compare_keys),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}