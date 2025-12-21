#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/libnetdata/sanitizers/sanitizers-pluginsd.h"

static void test_external_plugins_sanitize_normal(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "valid_plugin_name-123";
    size_t result = external_plugins_sanitize(dst, src, sizeof(dst));
    
    assert_int_equal(result, strlen(src));
    assert_string_equal(dst, "valid_plugin_name-123");
}

static void test_external_plugins_sanitize_with_control_chars(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "plugin\twith\ncontrol\rchars";
    size_t result = external_plugins_sanitize(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "plugin with control chars");
}

static void test_external_plugins_sanitize_special_symbols(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "plugin!\"'`=|name";
    size_t result = external_plugins_sanitize(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "plugin_______name");
}

static void test_external_plugins_sanitize_backslash(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "plugin\\name";
    size_t result = external_plugins_sanitize(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "plugin/name");
}

static void test_external_plugins_sanitize_buffer_overflow(void **state) {
    (void) state; // unused
    char dst[5];
    const char *src = "very_long_plugin_name";
    size_t result = external_plugins_sanitize(dst, src, sizeof(dst));
    
    assert_int_equal(result, 4);
    assert_string_equal(dst, "very");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_external_plugins_sanitize_normal),
        cmocka_unit_test(test_external_plugins_sanitize_with_control_chars),
        cmocka_unit_test(test_external_plugins_sanitize_special_symbols),
        cmocka_unit_test(test_external_plugins_sanitize_backslash),
        cmocka_unit_test(test_external_plugins_sanitize_buffer_overflow)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}