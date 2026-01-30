#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <cmocka.h>

#include "log2journal.h"

// Mock definitions for external dependencies
char journal_key_characters_map[256];

// Test fixture setup
static int setup(void **state) {
    // Initialize journal_key_characters_map
    for (int i = 0; i < 256; i++) {
        if ((i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z') || 
            (i >= '0' && i <= '9') || i == '_') {
            journal_key_characters_map[i] = i;
        } else {
            journal_key_characters_map[i] = '_';
        }
    }
    return 0;
}

static int teardown(void **state) {
    return 0;
}

// Test: pcre2_parser_create with NULL jb
static void test_pcre2_parser_create_null_jb(void **state) {
    PCRE2_STATE *pcre2 = pcre2_parser_create(NULL);
    assert_non_null(pcre2);
    assert_null(pcre2->re);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parser_create with valid pattern and no prefix
static void test_pcre2_parser_create_valid_pattern_no_prefix(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<name>\\w+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    assert_non_null(pcre2->re);
    assert_non_null(pcre2->match_data);
    assert_int_equal(pcre2->key_start, 0);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parser_create with valid pattern and prefix
static void test_pcre2_parser_create_valid_pattern_with_prefix(void **state) {
    LOG_JOB jb = {
        .prefix = "TEST_",
        .pattern = "(?<field>\\w+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    assert_non_null(pcre2->re);
    assert_non_null(pcre2->match_data);
    assert_int_equal(pcre2->key_start, 5);
    assert_string_equal(pcre2->key, "TEST_");
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parser_create with empty prefix
static void test_pcre2_parser_create_empty_prefix(void **state) {
    LOG_JOB jb = {
        .prefix = "",
        .pattern = "(?<id>\\d+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    assert_non_null(pcre2->re);
    assert_int_equal(pcre2->key_start, 0);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parser_create with invalid regex pattern
static void test_pcre2_parser_create_invalid_pattern(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "[invalid(",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    assert_null(pcre2->re);
    assert_non_null(pcre2->msg);
    assert_true(strlen(pcre2->msg) > 0);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_has_error with valid state (no error)
static void test_pcre2_has_error_no_error(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<name>\\w+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    assert_false(pcre2_has_error(pcre2));
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_has_error with error state (no regex)
static void test_pcre2_has_error_with_error(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "[invalid(",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    assert_true(pcre2_has_error(pcre2));
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_has_error with error in message
static void test_pcre2_has_error_with_error_msg(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<name>\\w+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    strcpy(pcre2->msg, "Some error");
    assert_true(pcre2_has_error(pcre2));
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parser_destroy with NULL state
static void test_pcre2_parser_destroy_null(void **state) {
    pcre2_parser_destroy(NULL);
    // Should not crash
    assert_true(true);
}

// Test: pcre2_parser_destroy with valid state
static void test_pcre2_parser_destroy_valid(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<field>\\w+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    pcre2_parser_destroy(pcre2);
    // Should not crash, memory should be freed
    assert_true(true);
}

// Test: pcre2_parser_error with no error
static void test_pcre2_parser_error_no_error(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<name>\\w+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    const char *error = pcre2_parser_error(pcre2);
    assert_non_null(error);
    assert_int_equal(strlen(error), 0);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parser_error with error
static void test_pcre2_parser_error_with_error(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "[invalid(",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    const char *error = pcre2_parser_error(pcre2);
    assert_non_null(error);
    assert_true(strlen(error) > 0);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parse_document with matching pattern
static void test_pcre2_parse_document_match(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<name>\\w+)",
        .hashtable = {0},
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    
    bool result = pcre2_parse_document(pcre2, "test_value", 0);
    assert_true(result);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parse_document with non-matching pattern
static void test_pcre2_parse_document_no_match(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<number>\\d+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    
    bool result = pcre2_parse_document(pcre2, "no_digits_here", 0);
    assert_false(result);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parse_document with zero length specified
static void test_pcre2_parse_document_zero_len(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<name>\\w+)",
        .hashtable = {0},
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    
    bool result = pcre2_parse_document(pcre2, "value", 0);
    assert_true(result);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parse_document with specified length
static void test_pcre2_parse_document_with_len(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<name>\\w+)",
        .hashtable = {0},
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    
    bool result = pcre2_parse_document(pcre2, "value_extra_text", 5);
    assert_true(result);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parse_document clears error state
static void test_pcre2_parse_document_clears_error(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<name>\\w+)",
        .hashtable = {0},
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    
    // Set error manually
    strcpy(pcre2->msg, "previous error");
    assert_true(strlen(pcre2->msg) > 0);
    
    // Parse should clear the error if successful
    bool result = pcre2_parse_document(pcre2, "value", 0);
    assert_true(result);
    assert_int_equal(pcre2->msg[0], '\0');
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_get_error_in_buffer with positive position
static void test_pcre2_get_error_in_buffer_with_pos(void **state) {
    char msg[1024];
    pcre2_get_error_in_buffer(msg, sizeof(msg), 101, 42);
    assert_non_null(msg);
    assert_true(strlen(msg) > 0);
    assert_true(strstr(msg, "101") != NULL);
    assert_true(strstr(msg, "42") != NULL);
}

// Test: pcre2_get_error_in_buffer with negative position
static void test_pcre2_get_error_in_buffer_no_pos(void **state) {
    char msg[1024];
    pcre2_get_error_in_buffer(msg, sizeof(msg), 101, -1);
    assert_non_null(msg);
    assert_true(strlen(msg) > 0);
    assert_true(strstr(msg, "101") != NULL);
}

// Test: pcre2_get_error_in_buffer with small buffer
static void test_pcre2_get_error_in_buffer_small_buffer(void **state) {
    char msg[10];
    pcre2_get_error_in_buffer(msg, sizeof(msg), 101, 42);
    assert_non_null(msg);
    // Should not crash, just truncate
    assert_true(strlen(msg) < 10);
}

// Test: pcre2_get_error_in_buffer with zero-length buffer
static void test_pcre2_get_error_in_buffer_zero_buffer(void **state) {
    char msg[1];
    pcre2_get_error_in_buffer(msg, 0, 101, 42);
    // Should handle gracefully
    assert_true(true);
}

// Test: copy_and_convert_key basic functionality
static void test_copy_and_convert_key_basic(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<Field_Name>\\w+)",
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    
    // Test by parsing
    bool result = pcre2_parse_document(pcre2, "test", 0);
    assert_true(result);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parse_document with multiple named groups
static void test_pcre2_parse_document_multiple_groups(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<first>\\w+) (?<second>\\w+)",
        .hashtable = {0},
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    
    bool result = pcre2_parse_document(pcre2, "hello world", 0);
    assert_true(result);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_parse_document with empty named groups
static void test_pcre2_parse_document_empty_groups(void **state) {
    LOG_JOB jb = {
        .prefix = NULL,
        .pattern = "(?<optional>\\w*)",
        .hashtable = {0},
    };
    
    PCRE2_STATE *pcre2 = pcre2_parser_create(&jb);
    assert_non_null(pcre2);
    
    bool result = pcre2_parse_document(pcre2, "", 0);
    assert_true(result);
    pcre2_parser_destroy(pcre2);
}

// Test: pcre2_test function
static void test_pcre2_test(void **state) {
    // Just ensure it doesn't crash
    pcre2_test();
    assert_true(true);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pcre2_parser_create_null_jb),
        cmocka_unit_test(test_pcre2_parser_create_valid_pattern_no_prefix),
        cmocka_unit_test(test_pcre2_parser_create_valid_pattern_with_prefix),
        cmocka_unit_test(test_pcre2_parser_create_empty_prefix),
        cmocka_unit_test(test_pcre2_parser_create_invalid_pattern),
        cmocka_unit_test(test_pcre2_has_error_no_error),
        cmocka_unit_test(test_pcre2_has_error_with_error),
        cmocka_unit_test(test_pcre2_has_error_with_error_msg),
        cmocka_unit_test(test_pcre2_parser_destroy_null),
        cmocka_unit_test(test_pcre2_parser_destroy_valid),
        cmocka_unit_test(test_pcre2_parser_error_no_error),
        cmocka_unit_test(test_pcre2_parser_error_with_error),
        cmocka_unit_test(test_pcre2_parse_document_match),
        cmocka_unit_test(test_pcre2_parse_document_no_match),
        cmocka_unit_test(test_pcre2_parse_document_zero_len),
        cmocka_unit_test(test_pcre2_parse_document_with_len),
        cmocka_unit_test(test_pcre2_parse_document_clears_error),
        cmocka_unit_test(test_pcre2_get_error_in_buffer_with_pos),
        cmocka_unit_test(test_pcre2_get_error_in_buffer_no_pos),
        cmocka_unit_test(test_pcre2_get_error_in_buffer_small_buffer),
        cmocka_unit_test(test_pcre2_get_error_in_buffer_zero_buffer),
        cmocka_unit_test(test_copy_and_convert_key_basic),
        cmocka_unit_test(test_pcre2_parse_document_multiple_groups),
        cmocka_unit_test(test_pcre2_parse_document_empty_groups),
        cmocka_unit_test(test_pcre2_test),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}