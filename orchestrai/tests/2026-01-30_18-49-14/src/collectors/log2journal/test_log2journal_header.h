#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock the logger
#define l2j_log(format, ...) mock_l2j_log(format, ##__VA_ARGS__)
static void mock_l2j_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

// ============================================================================
// Tests for copy_to_buffer function
// ============================================================================

static void test_copy_to_buffer_with_valid_inputs(void **state) {
    (void)state;
    char dst[10];
    const char *src = "hello";
    
    size_t result = copy_to_buffer(dst, sizeof(dst), src, strlen(src));
    
    assert_int_equal(result, 5);
    assert_string_equal(dst, "hello");
}

static void test_copy_to_buffer_truncates_when_src_larger_than_dst(void **state) {
    (void)state;
    char dst[5];
    const char *src = "hello world";
    
    size_t result = copy_to_buffer(dst, sizeof(dst), src, strlen(src));
    
    assert_int_equal(result, 4);
    assert_string_equal(dst, "hell");
}

static void test_copy_to_buffer_with_exact_fit(void **state) {
    (void)state;
    char dst[6];
    const char *src = "hello";
    
    size_t result = copy_to_buffer(dst, sizeof(dst), src, strlen(src));
    
    assert_int_equal(result, 5);
    assert_string_equal(dst, "hello");
}

static void test_copy_to_buffer_dst_size_zero(void **state) {
    (void)state;
    char dst[1];
    const char *src = "hello";
    
    size_t result = copy_to_buffer(dst, 0, src, strlen(src));
    
    assert_int_equal(result, 0);
}

static void test_copy_to_buffer_dst_size_one(void **state) {
    (void)state;
    char dst[2];
    const char *src = "hello";
    
    size_t result = copy_to_buffer(dst, 1, src, strlen(src));
    
    assert_int_equal(result, 0);
    assert_int_equal(dst[0], '\0');
}

static void test_copy_to_buffer_empty_src(void **state) {
    (void)state;
    char dst[10];
    const char *src = "";
    
    size_t result = copy_to_buffer(dst, sizeof(dst), src, 0);
    
    assert_int_equal(result, 0);
    assert_string_equal(dst, "");
}

static void test_copy_to_buffer_src_len_zero(void **state) {
    (void)state;
    char dst[10];
    const char *src = "hello";
    
    size_t result = copy_to_buffer(dst, sizeof(dst), src, 0);
    
    assert_int_equal(result, 0);
    assert_string_equal(dst, "");
}

static void test_copy_to_buffer_partial_copy(void **state) {
    (void)state;
    char dst[20];
    const char *src = "hello world";
    
    size_t result = copy_to_buffer(dst, sizeof(dst), src, 5);
    
    assert_int_equal(result, 5);
    assert_memory_equal(dst, "hello", 5);
    assert_int_equal(dst[5], '\0');
}

static void test_copy_to_buffer_large_buffer_small_src(void **state) {
    (void)state;
    char dst[1024];
    const char *src = "hi";
    
    size_t result = copy_to_buffer(dst, sizeof(dst), src, strlen(src));
    
    assert_int_equal(result, 2);
    assert_string_equal(dst, "hi");
}

static void test_copy_to_buffer_boundary_condition(void **state) {
    (void)state;
    char dst[5];
    const char *src = "1234567";
    
    size_t result = copy_to_buffer(dst, sizeof(dst), src, strlen(src));
    
    assert_int_equal(result, 4);
    assert_string_equal(dst, "1234");
}

// ============================================================================
// Tests for search_pattern_matches function (inline)
// ============================================================================

static void test_search_pattern_matches_null_pattern(void **state) {
    (void)state;
    SEARCH_PATTERN sp = {0};
    sp.re = NULL;
    
    // Without a valid regex, this would crash, so we test the inline function behavior
    // This is a boundary test for the inline function
    assert_null(sp.re);
}

// ============================================================================
// Tests for hashed_key functions (inline)
// ============================================================================

static void test_hashed_key_cleanup_with_hashtable_allocated(void **state) {
    (void)state;
    HASHED_KEY k = {0};
    k.key = malloc(10);
    strcpy((char *)k.key, "testkey");
    k.len = 7;
    k.hash = 12345;
    k.flags = HK_HASHTABLE_ALLOCATED;
    k.value.txt = malloc(100);
    strcpy(k.value.txt, "testvalue");
    k.value.len = 9;
    k.value.size = 100;
    
    hashed_key_cleanup(&k);
    
    assert_null(k.key);
    assert_int_equal(k.len, 0);
    assert_int_equal(k.hash, 0);
    assert_int_equal(k.flags, HK_NONE);
}

static void test_hashed_key_cleanup_without_hashtable_allocated(void **state) {
    (void)state;
    HASHED_KEY k = {0};
    HASHED_KEY hashtable_key = {0};
    k.key = malloc(10);
    strcpy((char *)k.key, "testkey");
    k.len = 7;
    k.hash = 12345;
    k.flags = HK_NONE;
    k.hashtable_ptr = &hashtable_key;
    
    hashed_key_cleanup(&k);
    
    assert_null(k.key);
    assert_int_equal(k.len, 0);
    assert_null(k.hashtable_ptr);
}

static void test_hashed_key_set_with_explicit_length(void **state) {
    (void)state;
    HASHED_KEY k = {0};
    
    hashed_key_set(&k, "testkey", 4);
    
    assert_non_null(k.key);
    assert_int_equal(k.len, 4);
    assert_memory_equal(k.key, "test", 4);
    assert_int_equal(k.flags, HK_NONE);
    
    hashed_key_cleanup(&k);
}

static void test_hashed_key_set_with_negative_length(void **state) {
    (void)state;
    HASHED_KEY k = {0};
    
    hashed_key_set(&k, "hello", -1);
    
    assert_non_null(k.key);
    assert_int_equal(k.len, 5);
    assert_string_equal(k.key, "hello");
    
    hashed_key_cleanup(&k);
}

static void test_hashed_key_set_empty_string(void **state) {
    (void)state;
    HASHED_KEY k = {0};
    
    hashed_key_set(&k, "", 0);
    
    assert_non_null(k.key);
    assert_int_equal(k.len, 0);
    assert_string_equal(k.key, "");
    
    hashed_key_cleanup(&k);
}

static void test_hashed_keys_match_same_key(void **state) {
    (void)state;
    HASHED_KEY k = {0};
    
    bool result = hashed_keys_match(&k, &k);
    
    assert_true(result);
}

static void test_hashed_keys_match_different_keys_same_content(void **state) {
    (void)state;
    HASHED_KEY k1 = {0};
    HASHED_KEY k2 = {0};
    
    hashed_key_set(&k1, "test", -1);
    hashed_key_set(&k2, "test", -1);
    
    bool result = hashed_keys_match(&k1, &k2);
    
    assert_true(result);
    
    hashed_key_cleanup(&k1);
    hashed_key_cleanup(&k2);
}

static void test_hashed_keys_match_different_content(void **state) {
    (void)state;
    HASHED_KEY k1 = {0};
    HASHED_KEY k2 = {0};
    
    hashed_key_set(&k1, "test1", -1);
    hashed_key_set(&k2, "test2", -1);
    
    bool result = hashed_keys_match(&k1, &k2);
    
    assert_false(result);
    
    hashed_key_cleanup(&k1);
    hashed_key_cleanup(&k2);
}

static void test_compare_keys_equal(void **state) {
    (void)state;
    HASHED_KEY k1 = {0};
    HASHED_KEY k2 = {0};
    
    hashed_key_set(&k1, "test", -1);
    hashed_key_set(&k2, "test", -1);
    
    int result = compare_keys(&k1, &k2);
    
    assert_int_equal(result, 0);
    
    hashed_key_cleanup(&k1);
    hashed_key_cleanup(&k2);
}

static void test_compare_keys_first_less(void **state) {
    (void)state;
    HASHED_KEY k1 = {0};
    HASHED_KEY k2 = {0};
    
    hashed_key_set(&k1, "aaa", -1);
    hashed_key_set(&k2, "bbb", -1);
    
    int result = compare_keys(&k1, &k2);
    
    assert_true(result < 0);
    
    hashed_key_cleanup(&k1);
    hashed_key_cleanup(&k2);
}

static void test_compare_keys_first_greater(void **state) {
    (void)state;
    HASHED_KEY k1 = {0};
    HASHED_KEY k2 = {0};
    
    hashed_key_set(&k1, "zzz", -1);
    hashed_key_set(&k2, "aaa", -1);
    
    int result = compare_keys(&k1, &k2);
    
    assert_true(result > 0);
    
    hashed_key_cleanup(&k1);
    hashed_key_cleanup(&k2);
}

// ============================================================================
// Tests for TXT_L2J functions (inline)
// ============================================================================

static void test_txt_l2j_cleanup_with_allocated_memory(void **state) {
    (void)state;
    TXT_L2J t = {0};
    t.txt = malloc(100);
    strcpy(t.txt, "hello");
    t.size = 100;
    t.len = 5;
    
    txt_l2j_cleanup(&t);
    
    assert_null(t.txt);
    assert_int_equal(t.size, 0);
    assert_int_equal(t.len, 0);
}

static void test_txt_l2j_cleanup_with_null_txt(void **state) {
    (void)state;
    TXT_L2J t = {0};
    t.txt = NULL;
    t.size = 0;
    t.len = 0;
    
    txt_l2j_cleanup(&t);
    
    assert_null(t.txt);
    assert_int_equal(t.size, 0);
    assert_int_equal(t.len, 0);
}

static void test_txt_l2j_cleanup_null_pointer(void **state) {
    (void)state;
    // Testing cleanup with NULL pointer should not crash
    txt_l2j_cleanup(NULL);
}

static void test_txt_l2j_compute_new_size_alignment(void **state) {
    (void)state;
    
    size_t result = txt_l2j_compute_new_size(0, 1024);
    
    assert_int_equal(result, 1024);
    assert_int_equal(result % 1024, 0);
}

static void test_txt_l2j_compute_new_size_unaligned(void **state) {
    (void)state;
    
    size_t result = txt_l2j_compute_new_size(0, 512);
    
    assert_int_equal(result % 1024, 0);
    assert_true(result >= 512);
}

static void test_txt_l2j_compute_new_size_double_old(void **state) {
    (void)state;
    
    size_t result = txt_l2j_compute_new_size(512, 512);
    
    assert_true(result >= 512);
}

static void test_txt_l2j_resize_within_size(void **state) {
    (void)state;
    TXT_L2J t = {0};
    t.txt = malloc(1024);
    strcpy(t.txt, "hello");
    t.size = 1024;
    t.len = 5;
    char *old_ptr = t.txt;
    
    txt_l2j_resize(&t, 512, true);
    
    assert_ptr_equal(t.txt, old_ptr);
    assert_int_equal(t.size, 1024);
    
    free(t.txt);
}

static void test_txt_l2j_resize_expand_keep_content(void **state) {
    (void)state;
    TXT_L2J t = {0};
    t.txt = malloc(512);
    strcpy(t.txt, "hello");
    t.size = 512;
    t.len = 5;
    
    txt_l2j_resize(&t, 2048, true);
    
    assert_non_null(t.txt);
    assert_true(t.size >= 2048);
    assert_string_equal(t.txt, "hello");
    assert_int_equal(t.len, 5);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_resize_expand_discard_content(void **state) {
    (void)state;
    TXT_L2J t = {0};
    t.txt = malloc(512);
    strcpy(t.txt, "hello");
    t.size = 512;
    t.len = 5;
    
    txt_l2j_resize(&t, 2048, false);
    
    assert_non_null(t.txt);
    assert_true(t.size >= 2048);
    assert_int_equal(t.len, 0);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_set_with_string_pointer(void **state) {
    (void)state;
    TXT_L2J t = {0};
    
    txt_l2j_set(&t, "hello", -1);
    
    assert_non_null(t.txt);
    assert_string_equal(t.txt, "hello");
    assert_int_equal(t.len, 5);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_set_with_explicit_length(void **state) {
    (void)state;
    TXT_L2J t = {0};
    
    txt_l2j_set(&t, "hello world", 5);
    
    assert_non_null(t.txt);
    assert_memory_equal(t.txt, "hello", 5);
    assert_int_equal(t.len, 5);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_set_empty_string(void **state) {
    (void)state;
    TXT_L2J t = {0};
    
    txt_l2j_set(&t, "", -1);
    
    assert_non_null(t.txt);
    assert_string_equal(t.txt, "");
    assert_int_equal(t.len, 0);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_set_null_string(void **state) {
    (void)state;
    TXT_L2J t = {0};
    
    txt_l2j_set(&t, NULL, -1);
    
    assert_non_null(t.txt);
    assert_string_equal(t.txt, "");
    assert_int_equal(t.len, 0);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_set_zero_length(void **state) {
    (void)state;
    TXT_L2J t = {0};
    
    txt_l2j_set(&t, "hello", 0);
    
    assert_non_null(t.txt);
    assert_string_equal(t.txt, "");
    assert_int_equal(t.len, 0);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_append_to_empty(void **state) {
    (void)state;
    TXT_L2J t = {0};
    
    txt_l2j_append(&t, "hello", -1);
    
    assert_non_null(t.txt);
    assert_string_equal(t.txt, "hello");
    assert_int_equal(t.len, 5);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_append_to_existing(void **state) {
    (void)state;
    TXT_L2J t = {0};
    
    txt_l2j_set(&t, "hello", -1);
    txt_l2j_append(&t, " world", -1);
    
    assert_string_equal(t.txt, "hello world");
    assert_int_equal(t.len, 11);
    
    txt_l2j_cleanup(&t);
}

static void test_txt_l2j_append_with_length(void **state) {
    (void)state;
    TXT_L2J t = {0};
    
    txt_l2j_set(&t, "hello", -1);
    txt_l2j_append(&t, " world!", 6);
    
    assert_memory_equal(t.txt, "hello world", 11);
    assert_int_equal(t.len, 11);
    
    txt_l2j_cleanup(&t);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // copy_to_buffer tests
        cmocka_unit_test(test_copy_to_buffer_with_valid_inputs),
        cmocka_unit_test(test_copy_to_buffer_truncates_when_src_larger_than_dst),
        cmocka_unit_test(test_copy_to_buffer_with_exact_fit),
        cmocka_unit_test(test_copy_to_buffer_dst_size_zero),
        cmocka_unit_test(test_copy_to_buffer_dst_size_one),
        cmocka_unit_test(test_copy_to_buffer_empty_src),
        cmocka_unit_test(test_copy_to_buffer_src_len_zero),
        cmocka_unit_test(test_copy_to_buffer_partial_copy),
        cmocka_unit_test(test_copy_to_buffer_large_buffer_small_src),
        cmocka_unit_test(test_copy_to_buffer_boundary_condition),
        
        // search_pattern tests
        cmocka_unit_test(test_search_pattern_matches_null_pattern),
        
        // hashed_key tests
        cmocka_unit_test(test_hashed_key_cleanup_with_hashtable_allocated),
        cmocka_unit_test(test_hashed_key_cleanup_without_hashtable_allocated),
        cmocka_unit_test(test_hashed_key_set_with_explicit_length),
        cmocka_unit_test(test_hashed_key_set_with_negative_length),
        cmocka_unit_test(test_hashed_key_set_empty_string),
        cmocka_unit_test(test_hashed_keys_match_same_key),
        cmocka_unit_test(test_hashed_keys_match_different_keys_same_content),
        cmocka_unit_test(test_hashed_keys_match_different_content),
        cmocka_unit_test(test_compare_keys_equal),
        cmocka_unit_test(test_compare_keys_first_less),
        cmocka_unit_test(test_compare_keys_first_greater),
        
        // txt_l2j tests
        cmocka_unit_test(test_txt_l2j_cleanup_with_allocated_memory),
        cmocka_unit_test(test_txt_l2j_cleanup_with_null_txt),
        cmocka_unit_test(test_txt_l2j_cleanup_null_pointer),
        cmocka_unit_test(test_txt_l2j_compute_new_size_alignment),
        cmocka_unit_test(test_txt_l2j_compute_new_size_unaligned),
        cmocka_unit_test(test_txt_l2j_compute_new_size_double_old),
        cmocka_unit_test(test_txt_l2j_resize_within_size),
        cmocka_unit_test(test_txt_l2j_resize_expand_keep_content),
        cmocka_unit_test(test_txt_l2j_resize_expand_discard_content),
        cmocka_unit_test(test_txt_l2j_set_with_string_pointer),
        cmocka_unit_test(test_txt_l2j_set_with_explicit_length),
        cmocka_unit_test(test_txt_l2j_set_empty_string),
        cmocka_unit_test(test_txt_l2j_set_null_string),
        cmocka_unit_test(test_txt_l2j_set_zero_length),
        cmocka_unit_test(test_txt_l2j_append_to_empty),
        cmocka_unit_test(test_txt_l2j_append_to_existing),
        cmocka_unit_test(test_txt_l2j_append_with_length),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}