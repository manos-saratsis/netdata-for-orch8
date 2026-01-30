#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <cmocka.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

/* Mock structures and functions */
typedef struct {
    int enabled;
} netdata_mutex_t;

typedef struct {
    int type;
} heartbeat_t;

typedef enum {
    RRDSET_TYPE_LINE,
    RRDSET_TYPE_AREA,
    RRDSET_TYPE_STACKED
} RRDSET_TYPE;

typedef enum {
    RRD_ALGORITHM_ABSOLUTE,
    RRD_ALGORITHM_INCREMENTAL,
    RRD_ALGORITHM_PCENT_OVER_ROW_TOTAL,
    RRD_ALGORITHM_PCENT_OVER_DIFF_TOTAL
} RRD_ALGORITHM;

#define RRDSET_TYPE_LINE_NAME "line"
#define RRDSET_TYPE_AREA_NAME "area"
#define RRDSET_TYPE_STACKED_NAME "stacked"
#define RRD_ALGORITHM_ABSOLUTE_NAME "absolute"
#define RRD_ALGORITHM_INCREMENTAL_NAME "incremental"
#define RRD_ALGORITHM_PCENT_OVER_ROW_TOTAL_NAME "percentage-of-row-total"
#define RRD_ALGORITHM_PCENT_OVER_DIFF_TOTAL_NAME "percentage-of-incremental-row-total"

/* Forward declarations */
void debugfs2lower(char *name);
const char *debugfs_rrdset_type_name(RRDSET_TYPE chart_type);
const char *debugfs_rrd_algorithm_name(RRD_ALGORITHM algorithm);
int debugfs_check_sys_permission(void);
int debugfs_am_i_running_as_root(void);
int debugfs_check_capabilities(void);

netdata_mutex_t stdout_mutex;

/* Test: debugfs2lower - normal string */
static void test_debugfs2lower_lowercase(void **state) {
    char name[] = "HELLO";
    debugfs2lower(name);
    assert_string_equal(name, "hello");
}

/* Test: debugfs2lower - mixed case */
static void test_debugfs2lower_mixed(void **state) {
    char name[] = "HeLLo";
    debugfs2lower(name);
    assert_string_equal(name, "hello");
}

/* Test: debugfs2lower - already lowercase */
static void test_debugfs2lower_already_lowercase(void **state) {
    char name[] = "hello";
    debugfs2lower(name);
    assert_string_equal(name, "hello");
}

/* Test: debugfs2lower - empty string */
static void test_debugfs2lower_empty(void **state) {
    char name[] = "";
    debugfs2lower(name);
    assert_string_equal(name, "");
}

/* Test: debugfs2lower - single character uppercase */
static void test_debugfs2lower_single_char(void **state) {
    char name[] = "A";
    debugfs2lower(name);
    assert_string_equal(name, "a");
}

/* Test: debugfs2lower - with numbers and special characters */
static void test_debugfs2lower_with_numbers(void **state) {
    char name[] = "TEST123!@#";
    debugfs2lower(name);
    assert_string_equal(name, "test123!@#");
}

/* Test: debugfs_rrdset_type_name - RRDSET_TYPE_LINE */
static void test_debugfs_rrdset_type_name_line(void **state) {
    const char *result = debugfs_rrdset_type_name(RRDSET_TYPE_LINE);
    assert_string_equal(result, RRDSET_TYPE_LINE_NAME);
}

/* Test: debugfs_rrdset_type_name - RRDSET_TYPE_AREA */
static void test_debugfs_rrdset_type_name_area(void **state) {
    const char *result = debugfs_rrdset_type_name(RRDSET_TYPE_AREA);
    assert_string_equal(result, RRDSET_TYPE_AREA_NAME);
}

/* Test: debugfs_rrdset_type_name - RRDSET_TYPE_STACKED */
static void test_debugfs_rrdset_type_name_stacked(void **state) {
    const char *result = debugfs_rrdset_type_name(RRDSET_TYPE_STACKED);
    assert_string_equal(result, RRDSET_TYPE_STACKED_NAME);
}

/* Test: debugfs_rrdset_type_name - default case */
static void test_debugfs_rrdset_type_name_default(void **state) {
    const char *result = debugfs_rrdset_type_name((RRDSET_TYPE)999);
    assert_string_equal(result, RRDSET_TYPE_LINE_NAME);
}

/* Test: debugfs_rrd_algorithm_name - RRD_ALGORITHM_ABSOLUTE */
static void test_debugfs_rrd_algorithm_name_absolute(void **state) {
    const char *result = debugfs_rrd_algorithm_name(RRD_ALGORITHM_ABSOLUTE);
    assert_string_equal(result, RRD_ALGORITHM_ABSOLUTE_NAME);
}

/* Test: debugfs_rrd_algorithm_name - RRD_ALGORITHM_INCREMENTAL */
static void test_debugfs_rrd_algorithm_name_incremental(void **state) {
    const char *result = debugfs_rrd_algorithm_name(RRD_ALGORITHM_INCREMENTAL);
    assert_string_equal(result, RRD_ALGORITHM_INCREMENTAL_NAME);
}

/* Test: debugfs_rrd_algorithm_name - RRD_ALGORITHM_PCENT_OVER_ROW_TOTAL */
static void test_debugfs_rrd_algorithm_name_pcent_row(void **state) {
    const char *result = debugfs_rrd_algorithm_name(RRD_ALGORITHM_PCENT_OVER_ROW_TOTAL);
    assert_string_equal(result, RRD_ALGORITHM_PCENT_OVER_ROW_TOTAL_NAME);
}

/* Test: debugfs_rrd_algorithm_name - RRD_ALGORITHM_PCENT_OVER_DIFF_TOTAL */
static void test_debugfs_rrd_algorithm_name_pcent_diff(void **state) {
    const char *result = debugfs_rrd_algorithm_name(RRD_ALGORITHM_PCENT_OVER_DIFF_TOTAL);
    assert_string_equal(result, RRD_ALGORITHM_PCENT_OVER_DIFF_TOTAL_NAME);
}

/* Test: debugfs_rrd_algorithm_name - default case */
static void test_debugfs_rrd_algorithm_name_default(void **state) {
    const char *result = debugfs_rrd_algorithm_name((RRD_ALGORITHM)999);
    assert_string_equal(result, RRD_ALGORITHM_ABSOLUTE_NAME);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_debugfs2lower_lowercase),
        cmocka_unit_test(test_debugfs2lower_mixed),
        cmocka_unit_test(test_debugfs2lower_already_lowercase),
        cmocka_unit_test(test_debugfs2lower_empty),
        cmocka_unit_test(test_debugfs2lower_single_char),
        cmocka_unit_test(test_debugfs2lower_with_numbers),
        cmocka_unit_test(test_debugfs_rrdset_type_name_line),
        cmocka_unit_test(test_debugfs_rrdset_type_name_area),
        cmocka_unit_test(test_debugfs_rrdset_type_name_stacked),
        cmocka_unit_test(test_debugfs_rrdset_type_name_default),
        cmocka_unit_test(test_debugfs_rrd_algorithm_name_absolute),
        cmocka_unit_test(test_debugfs_rrd_algorithm_name_incremental),
        cmocka_unit_test(test_debugfs_rrd_algorithm_name_pcent_row),
        cmocka_unit_test(test_debugfs_rrd_algorithm_name_pcent_diff),
        cmocka_unit_test(test_debugfs_rrd_algorithm_name_default),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}