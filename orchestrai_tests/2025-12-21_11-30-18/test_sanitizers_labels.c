#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/libnetdata/sanitizers/sanitizers-labels.h"

static void test_rrdlabels_sanitize_name_normal(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "valid_label-name123";
    size_t result = rrdlabels_sanitize_name(dst, src, sizeof(dst));
    
    assert_int_equal(result, strlen(src));
    assert_string_equal(dst, "valid_label-name123");
}

static void test_rrdlabels_sanitize_name_symbols(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "label:name;with=symbols@123";
    size_t result = rrdlabels_sanitize_name(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "label_name_with_symbols_123");
}

static void test_rrdlabels_sanitize_name_space(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "label name with spaces";
    size_t result = rrdlabels_sanitize_name(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "label_name_with_spaces");
}

static void test_rrdlabels_sanitize_value_normal(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "valid_label_value-123";
    size_t result = rrdlabels_sanitize_value(dst, src, sizeof(dst));
    
    assert_int_equal(result, strlen(src));
    assert_string_equal(dst, "valid_label_value-123");
}

static void test_rrdlabels_sanitize_value_symbols(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "value!@#$%^&*()";
    size_t result = rrdlabels_sanitize_value(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "value[none]");
}

static void test_prometheus_rrdlabels_sanitize_name(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "Prometheus_label-name123";
    size_t result = prometheus_rrdlabels_sanitize_name(dst, src, sizeof(dst));
    
    assert_int_not_equal(result, 0);
    assert_string_equal(dst, "_rometheus_label_name123");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rrdlabels_sanitize_name_normal),
        cmocka_unit_test(test_rrdlabels_sanitize_name_symbols),
        cmocka_unit_test(test_rrdlabels_sanitize_name_space),
        cmocka_unit_test(test_rrdlabels_sanitize_value_normal),
        cmocka_unit_test(test_rrdlabels_sanitize_value_symbols),
        cmocka_unit_test(test_prometheus_rrdlabels_sanitize_name)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}