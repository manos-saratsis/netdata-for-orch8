#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

// Mock the libnetdata functions and structures used in log2journal.c
#define LIBNETDATA_LIBRARY_HEADER 1
#include "log2journal.h"

// ============================================================================
// Test for copy_to_buffer inline function
// ============================================================================

static void test_copy_to_buffer_empty_destination(void **state) {
    (void)state;
    char dst[0];
    size_t result = copy_to_buffer(dst, 0, "source", 6);
    assert_int_equal(result, 0);
}

static void test_copy_to_buffer_single_byte_destination(void **state) {
    (void)state;
    char dst[1];
    size_t result = copy_to_buffer(dst, 1, "source", 6);
    assert_int_equal(result, 0);
    assert_int_equal(dst[0], '\0');
}

static void test_copy_to_buffer_fits_exactly(void **state) {
    (void)state;
    char dst[7];
    size_t result = copy_to_buffer(dst, 7, "source", 6);
    assert_int_equal(result, 6);
    assert_string_equal(dst, "source");
}

static void test_copy_to_buffer_source_shorter(void **state) {
    (void)state;
    char dst[20];
    size_t result = copy_to_buffer(dst, 20, "src", 3);
    assert_int_equal(result, 3);
    assert_string_equal(dst, "src");
}

static void test_copy_to_buffer_truncated(void **state) {
    (void)state;
    char dst[5];
    size_t result = copy_to_buffer(dst, 5, "this is too long", 16);
    assert_int_equal(result, 4);
    assert_memory_equal(dst, "this", 4);
    assert_int_equal(dst[4], '\0');
}

static void test_copy_to_buffer_zero_length_source(void **state) {
    (void)state;
    char dst[10];
    memset(dst, 'X', 10);
    size_t result = copy_to_buffer(dst, 10, "source", 0);
    assert_int_equal(result, 0);
    assert_int_equal(dst[0], '\0');
}

// ============================================================================
// Test for journal_key_characters_map
// ============================================================================

static void test_journal_key_characters_map_uppercase(void **state) {
    (void)state;
    for(char c = 'A'; c <= 'Z'; c++) {
        assert_int_equal(journal_key_characters_map[(unsigned char)c], c);
    }
}

static void test_journal_key_characters_map_digits(void **state) {
    (void)state;
    for(char c = '0'; c <= '9'; c++) {
        assert_int_equal(journal_key_characters_map[(unsigned char)c], c);
    }
}

static void test_journal_key_characters_map_lowercase_to_uppercase(void **state) {
    (void)state;
    for(char c = 'a'; c <= 'z'; c++) {
        char expected = c - 32;  // Convert to uppercase
        assert_int_equal(journal_key_characters_map[(unsigned char)c], expected);
    }
}

static void test_journal_key_characters_map_underscore(void **state) {
    (void)state;
    assert_int_equal(journal_key_characters_map['_'], '_');
}

static void test_journal_key_characters_map_space_to_underscore(void **state) {
    (void)state;
    assert_int_equal(journal_key_characters_map[' '], '_');
}

static void test_journal_key_characters_map_special_chars_to_underscore(void **state) {
    (void)state;
    const char special[] = "!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~";
    for(const char *c = special; *c; c++) {
        assert_int_equal(journal_key_characters_map[(unsigned char)*c], '_');
    }
}

static void test_journal_key_characters_map_control_chars(void **state) {
    (void)state;
    // Control characters 1-31 should map to underscore (0 is null terminator)
    for(unsigned char c = 1; c < 32; c++) {
        assert_int_equal(journal_key_characters_map[c], '_');
    }
}

static void test_journal_key_characters_map_extended_ascii(void **state) {
    (void)state;
    // Extended ASCII (128-255) should map to underscore
    for(unsigned char c = 128; c < 255; c++) {
        assert_int_equal(journal_key_characters_map[c], '_');
    }
}

static void test_journal_key_characters_map_null_terminator(void **state) {
    (void)state;
    assert_int_equal(journal_key_characters_map[0], '\0');
}

// ============================================================================
// Return test suite
// ============================================================================

int main(void) {
    const struct CMUnitTest tests[] = {
        // copy_to_buffer tests
        cmocka_unit_test(test_copy_to_buffer_empty_destination),
        cmocka_unit_test(test_copy_to_buffer_single_byte_destination),
        cmocka_unit_test(test_copy_to_buffer_fits_exactly),
        cmocka_unit_test(test_copy_to_buffer_source_shorter),
        cmocka_unit_test(test_copy_to_buffer_truncated),
        cmocka_unit_test(test_copy_to_buffer_zero_length_source),
        
        // journal_key_characters_map tests
        cmocka_unit_test(test_journal_key_characters_map_uppercase),
        cmocka_unit_test(test_journal_key_characters_map_digits),
        cmocka_unit_test(test_journal_key_characters_map_lowercase_to_uppercase),
        cmocka_unit_test(test_journal_key_characters_map_underscore),
        cmocka_unit_test(test_journal_key_characters_map_space_to_underscore),
        cmocka_unit_test(test_journal_key_characters_map_special_chars_to_underscore),
        cmocka_unit_test(test_journal_key_characters_map_control_chars),
        cmocka_unit_test(test_journal_key_characters_map_extended_ascii),
        cmocka_unit_test(test_journal_key_characters_map_null_terminator),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}