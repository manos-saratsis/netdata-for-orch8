#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/libnetdata/sanitizers/chart_id_and_name.h"

static void test_netdata_fix_chart_name_normal(void **state) {
    (void) state; // unused
    char input[] = "normal chart name";
    netdata_fix_chart_name(input);
    assert_string_equal(input, "normal_chart_name");
}

static void test_netdata_fix_chart_name_symbols(void **state) {
    (void) state; // unused
    char input[] = "chart!@#$%^&*()name";
    netdata_fix_chart_name(input);
    assert_string_equal(input, "chart_________name");
}

static void test_netdata_fix_chart_name_starting_with_exclamation(void **state) {
    (void) state; // unused
    char input[] = "!invalidchartname";
    netdata_fix_chart_name(input);
    assert_string_equal(input, "_invalidchartname");
}

static void test_netdata_fix_chart_id_preserves_valid_chars(void **state) {
    (void) state; // unused
    char input[] = "valid-chart-123_id";
    netdata_fix_chart_id(input);
    assert_string_equal(input, "valid-chart-123_id");
}

static void test_rrdset_strncpyz_name(void **state) {
    (void) state; // unused
    char dst[50];
    const char *src = "type.long chart name with spaces";
    char *result = rrdset_strncpyz_name(dst, src, sizeof(dst) - 1);
    
    assert_non_null(result);
    assert_string_equal(result, "type_long_chart_name_with_spaces");
}

static void test_rrdvar_fix_name(void **state) {
    (void) state; // unused
    char input[] = "variable!@name";
    bool changed = rrdvar_fix_name(input);
    
    assert_true(changed);
    assert_string_equal(input, "variable__name");
}

static void test_rrdvar_fix_name_no_change(void **state) {
    (void) state; // unused
    char input[] = "valid_variable_name";
    bool changed = rrdvar_fix_name(input);
    
    assert_false(changed);
    assert_string_equal(input, "valid_variable_name");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_netdata_fix_chart_name_normal),
        cmocka_unit_test(test_netdata_fix_chart_name_symbols),
        cmocka_unit_test(test_netdata_fix_chart_name_starting_with_exclamation),
        cmocka_unit_test(test_netdata_fix_chart_id_preserves_valid_chars),
        cmocka_unit_test(test_rrdset_strncpyz_name),
        cmocka_unit_test(test_rrdvar_fix_name),
        cmocka_unit_test(test_rrdvar_fix_name_no_change)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}