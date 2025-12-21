#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/libnetdata/sanitizers/sanitizers-functions.h"

static void test_rrd_functions_sanitize_normal_input(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "valid_function_name";
    size_t result = rrd_functions_sanitize(dst, src, sizeof(dst));
    
    assert_int_equal(result, strlen(src));
    assert_string_equal(dst, "valid_function_name");
}

static void test_rrd_functions_sanitize_with_symbols(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "function!@#$%^&*name";
    size_t result = rrd_functions_sanitize(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "function'name");
}

static void test_rrd_functions_sanitize_control_chars(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "function\twith\ncontrol\rchars";
    size_t result = rrd_functions_sanitize(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "function with control chars");
}

static void test_rrd_functions_sanitize_buffer_overflow(void **state) {
    (void) state; // unused
    char dst[5];
    const char *src = "very_long_function_name";
    size_t result = rrd_functions_sanitize(dst, src, sizeof(dst));
    
    assert_int_equal(result, 4);
    assert_string_equal(dst, "very");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rrd_functions_sanitize_normal_input),
        cmocka_unit_test(test_rrd_functions_sanitize_with_symbols),
        cmocka_unit_test(test_rrd_functions_sanitize_control_chars),
        cmocka_unit_test(test_rrd_functions_sanitize_buffer_overflow)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}