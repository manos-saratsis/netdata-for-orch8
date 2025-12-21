#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/libnetdata/sanitizers/utf8-sanitizer.h"

// Mock mapping to use in tests
static unsigned char test_char_map[256] = {
    // Default is 0
    [0] = '\0', [1] = ' ', // Replace control chars with space
    // Most common characters map to themselves
    ['a'] = 'a', ['b'] = 'b', ['c'] = 'c', 
    [' '] = ' '
};

static void test_text_sanitize_normal_ascii(void **state) {
    (void) state; // unused
    unsigned char dst[50] = {0};
    const unsigned char *src = (const unsigned char *)"hello world";
    size_t result = text_sanitize(dst, src, sizeof(dst), test_char_map, false, "[empty]", NULL);
    
    assert_int_equal(result, strlen((char *)src));
    assert_string_equal((char *)dst, "hello world");
}

static void test_text_sanitize_multiple_spaces(void **state) {
    (void) state; // unused
    unsigned char dst[50] = {0};
    const unsigned char *src = (const unsigned char *)"hello   world";
    size_t result = text_sanitize(dst, src, sizeof(dst), test_char_map, false, "[empty]", NULL);
    
    assert_int_equal(result, 11);
    assert_string_equal((char *)dst, "hello world");
}

static void test_text_sanitize_leading_trailing_spaces(void **state) {
    (void) state; // unused
    unsigned char dst[50] = {0};
    const unsigned char *src = (const unsigned char *)"   hello world   ");
    size_t result = text_sanitize(dst, src, sizeof(dst), test_char_map, false, "[empty]", NULL);
    
    assert_int_equal(result, 11);
    assert_string_equal((char *)dst, "hello world");
}

static void test_text_sanitize_only_spaces(void **state) {
    (void) state; // unused
    unsigned char dst[50] = {0};
    const unsigned char *src = (const unsigned char *)"   ");
    size_t result = text_sanitize(dst, src, sizeof(dst), test_char_map, false, "[empty]", NULL);
    
    assert_int_equal(result, 7);
    assert_string_equal((char *)dst, "[empty]");
}

static void test_text_sanitize_utf8_valid(void **state) {
    (void) state; // unused
    unsigned char dst[50] = {0};
    const unsigned char *src = (const unsigned char *)"café";
    size_t result = text_sanitize(dst, src, sizeof(dst), test_char_map, true, "[empty]", NULL);
    
    // In UTF-8 mode, valid UTF-8 characters should pass through
    assert_int_equal(result, 4);
    assert_memory_equal(dst, src, 4);
}

static void test_text_sanitize_utf8_invalid(void **state) {
    (void) state; // unused
    unsigned char dst[50] = {0};
    const unsigned char *src = (const unsigned char *)"\xC3\x28"; // Invalid 2-byte sequence
    size_t result = text_sanitize(dst, src, sizeof(dst), test_char_map, true, "[empty]", NULL);
    
    // In UTF-8 mode with invalid sequence, it should convert to hex
    assert_string_equal((char *)dst, "c328");
}

static void test_text_sanitize_buffer_overflow(void **state) {
    (void) state; // unused
    unsigned char dst[5] = {0};
    const unsigned char *src = (const unsigned char *)"very long string";
    size_t result = text_sanitize(dst, src, sizeof(dst), test_char_map, false, "[empty]", NULL);
    
    assert_int_equal(result, 4);
    assert_string_equal((char *)dst, "very");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_text_sanitize_normal_ascii),
        cmocka_unit_test(test_text_sanitize_multiple_spaces),
        cmocka_unit_test(test_text_sanitize_leading_trailing_spaces),
        cmocka_unit_test(test_text_sanitize_only_spaces),
        cmocka_unit_test(test_text_sanitize_utf8_valid),
        cmocka_unit_test(test_text_sanitize_utf8_invalid),
        cmocka_unit_test(test_text_sanitize_buffer_overflow)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}