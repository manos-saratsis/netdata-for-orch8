// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "log2journal.h"

// Mock implementations
void *mallocz(size_t size) {
    return malloc(size);
}

void *reallocz(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void freez(void *ptr) {
    free(ptr);
}

char *strndupz(const char *s, size_t n) {
    char *result = malloc(n + 1);
    if (result) {
        strncpy(result, s, n);
        result[n] = '\0';
    }
    return result;
}

char *strdupz(const char *s) {
    return strdup(s);
}

// Mock for pcre2 functions
pcre2_code* pcre2_compile(PCRE2_SPTR pattern, PCRE2_SIZE length,
                          uint32_t options, int *errorcode,
                          PCRE2_SIZE *erroroffset, pcre2_compile_context *ccontext) {
    if (pattern == NULL) {
        *errorcode = 1;
        *erroroffset = 0;
        return NULL;
    }

    // Check if pattern contains "invalid" to simulate compilation error
    if (strstr((const char*)pattern, "invalid_pattern") != NULL) {
        *errorcode = 1002;
        *erroroffset = 5;
        return NULL;
    }

    // Allocate a mock pcre2_code object
    pcre2_code *code = malloc(sizeof(pcre2_code));
    return code;
}

void pcre2_code_free(pcre2_code *code) {
    if (code) free(code);
}

pcre2_match_data* pcre2_match_data_create_from_pattern(const pcre2_code *code,
                                                       pcre2_general_context *gcontext) {
    if (code == NULL) return NULL;
    pcre2_match_data *match_data = malloc(sizeof(pcre2_match_data));
    return match_data;
}

void pcre2_match_data_free(pcre2_match_data *match_data) {
    if (match_data) free(match_data);
}

int pcre2_match(const pcre2_code *code, PCRE2_SPTR subject, PCRE2_SIZE length,
                PCRE2_SIZE startoffset, uint32_t options, pcre2_match_data *match_data,
                pcre2_match_context *mcontext) {
    return 0; // Simulate successful match
}

int pcre2_get_error_message(int errorcode, PCRE2_UCHAR *buffer, PCRE2_SIZE bufflen) {
    const char *msg = "Test error message";
    size_t msg_len = strlen(msg);
    if (bufflen > 0) {
        size_t copy_len = (bufflen - 1 < msg_len) ? bufflen - 1 : msg_len;
        strncpy((char *)buffer, msg, copy_len);
        buffer[copy_len] = '\0';
    }
    return 0;
}

// This function is declared in log2journal-pcre2.c
void pcre2_get_error_in_buffer(char *msg, size_t msg_len, int rc, int pos);

// Tests for search_pattern_cleanup
static void test_search_pattern_cleanup_with_all_fields(void **state) {
    SEARCH_PATTERN sp;
    sp.pattern = strdup("test_pattern");
    sp.re = malloc(sizeof(pcre2_code));
    sp.match_data = malloc(sizeof(pcre2_match_data));
    sp.error.txt = malloc(100);
    sp.error.size = 100;
    sp.error.len = 10;

    search_pattern_cleanup(&sp);

    assert_null(sp.pattern);
    assert_null(sp.re);
    assert_null(sp.match_data);
}

static void test_search_pattern_cleanup_with_null_fields(void **state) {
    SEARCH_PATTERN sp;
    sp.pattern = NULL;
    sp.re = NULL;
    sp.match_data = NULL;
    sp.error.txt = NULL;
    sp.error.size = 0;
    sp.error.len = 0;

    search_pattern_cleanup(&sp);

    assert_null(sp.pattern);
    assert_null(sp.re);
    assert_null(sp.match_data);
}

static void test_search_pattern_cleanup_with_partial_fields(void **state) {
    SEARCH_PATTERN sp;
    sp.pattern = strdup("test_pattern");
    sp.re = NULL;
    sp.match_data = malloc(sizeof(pcre2_match_data));
    sp.error.txt = NULL;
    sp.error.size = 0;
    sp.error.len = 0;

    search_pattern_cleanup(&sp);

    assert_null(sp.pattern);
    assert_null(sp.re);
    assert_null(sp.match_data);
}

// Tests for pcre2_error_message (static function)
// This is tested indirectly through search_pattern_set

// Tests for compile_pcre2 (static function)
// This is tested indirectly through search_pattern_set

// Tests for search_pattern_set
static void test_search_pattern_set_valid_pattern(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    bool result = search_pattern_set(&sp, "test_pattern", 12);

    assert_true(result);
    assert_non_null(sp.pattern);
    assert_non_null(sp.re);
    assert_non_null(sp.match_data);
    assert_string_equal(sp.pattern, "test_pattern");

    search_pattern_cleanup(&sp);
}

static void test_search_pattern_set_invalid_pattern(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    bool result = search_pattern_set(&sp, "invalid_pattern_syntax", 23);

    assert_false(result);
    assert_non_null(sp.pattern);
    assert_null(sp.re);
    assert_null(sp.match_data);
    assert_true(sp.error.len > 0);

    search_pattern_cleanup(&sp);
}

static void test_search_pattern_set_empty_pattern(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    bool result = search_pattern_set(&sp, "", 0);

    assert_true(result);
    assert_non_null(sp.pattern);
    assert_string_equal(sp.pattern, "");

    search_pattern_cleanup(&sp);
}

static void test_search_pattern_set_overwrites_previous(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    search_pattern_set(&sp, "pattern1", 8);
    assert_string_equal(sp.pattern, "pattern1");
    pcre2_code *old_re = sp.re;

    search_pattern_set(&sp, "pattern2", 8);
    assert_string_equal(sp.pattern, "pattern2");
    assert_ptr_not_equal(sp.re, old_re);

    search_pattern_cleanup(&sp);
}

static void test_search_pattern_set_with_special_regex_chars(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    bool result = search_pattern_set(&sp, "^test.*pattern$", 15);

    assert_true(result);
    assert_non_null(sp.re);
    assert_non_null(sp.match_data);

    search_pattern_cleanup(&sp);
}

static void test_search_pattern_set_with_null_pattern(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    // This test checks behavior with NULL input - implementation should handle gracefully
    // The function will call strndupz which should handle NULL
    bool result = search_pattern_set(&sp, NULL, 0);

    // Depending on implementation, this might fail
    if (result) {
        search_pattern_cleanup(&sp);
    }
}

static void test_search_pattern_set_long_pattern(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    char long_pattern[512];
    memset(long_pattern, 'a', 511);
    long_pattern[511] = '\0';

    bool result = search_pattern_set(&sp, long_pattern, 511);

    assert_true(result);
    assert_non_null(sp.pattern);
    assert_string_equal(sp.pattern, long_pattern);

    search_pattern_cleanup(&sp);
}

static void test_search_pattern_set_pattern_with_null_bytes(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    char pattern_with_nulls[20] = "test\0pattern\0";
    bool result = search_pattern_set(&sp, pattern_with_nulls, 20);

    assert_true(result);
    assert_non_null(sp.pattern);

    search_pattern_cleanup(&sp);
}

static void test_search_pattern_cleanup_called_multiple_times(void **state) {
    SEARCH_PATTERN sp;
    sp.pattern = strdup("test");
    sp.re = malloc(sizeof(pcre2_code));
    sp.match_data = malloc(sizeof(pcre2_match_data));
    sp.error.txt = NULL;
    sp.error.size = 0;
    sp.error.len = 0;

    search_pattern_cleanup(&sp);
    // Call again to ensure no double-free
    search_pattern_cleanup(&sp);

    assert_null(sp.pattern);
    assert_null(sp.re);
    assert_null(sp.match_data);
}

static void test_search_pattern_set_after_error(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    // First call with invalid pattern
    search_pattern_set(&sp, "invalid_pattern_syntax", 23);
    assert_true(sp.error.len > 0);

    // Reset by setting valid pattern
    bool result = search_pattern_set(&sp, "valid_pattern", 13);

    assert_true(result);
    assert_non_null(sp.re);

    search_pattern_cleanup(&sp);
}

static void test_search_pattern_set_zero_length_valid(void **state) {
    SEARCH_PATTERN sp;
    memset(&sp, 0, sizeof(SEARCH_PATTERN));

    bool result = search_pattern_set(&sp, "pattern", 0);

    assert_true(result);
    assert_non_null(sp.pattern);

    search_pattern_cleanup(&sp);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_search_pattern_cleanup_with_all_fields),
        cmocka_unit_test(test_search_pattern_cleanup_with_null_fields),
        cmocka_unit_test(test_search_pattern_cleanup_with_partial_fields),
        cmocka_unit_test(test_search_pattern_set_valid_pattern),
        cmocka_unit_test(test_search_pattern_set_invalid_pattern),
        cmocka_unit_test(test_search_pattern_set_empty_pattern),
        cmocka_unit_test(test_search_pattern_set_overwrites_previous),
        cmocka_unit_test(test_search_pattern_set_with_special_regex_chars),
        cmocka_unit_test(test_search_pattern_set_long_pattern),
        cmocka_unit_test(test_search_pattern_set_pattern_with_null_bytes),
        cmocka_unit_test(test_search_pattern_cleanup_called_multiple_times),
        cmocka_unit_test(test_search_pattern_set_after_error),
        cmocka_unit_test(test_search_pattern_set_zero_length_valid),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}