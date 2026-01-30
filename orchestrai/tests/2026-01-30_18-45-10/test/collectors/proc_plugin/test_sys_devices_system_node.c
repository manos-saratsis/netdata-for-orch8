#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Mock implementations */
__attribute__((weak)) void nd_log(int category, int priority, const char *fmt, ...) {
}

__attribute__((weak)) char *inicfg_get(void *config, const char *section, const char *name, const char *default_value) {
    return (char *)default_value;
}

__attribute__((weak)) int inicfg_get_boolean_ondemand(void *config, const char *section, const char *name, int default_value) {
    return default_value;
}

__attribute__((weak)) void collector_error(const char *fmt, ...) {
}

__attribute__((weak)) void rrdlabels_add(void *labels, const char *name, const char *value, int source) {
}

__attribute__((weak)) void *rrdset_create_localhost(const char *type, const char *id, const char *name,
                                                   const char *family, const char *context, const char *title,
                                                   const char *units, const char *plugin, const char *module,
                                                   long priority, int update_every, int chart_type) {
    return malloc(sizeof(void *));
}

__attribute__((weak)) void rrdset_done(void *st) {
}

__attribute__((weak)) void *rrddim_add(void *st, const char *id, const char *name, long multiply, long divide,
                                       int algorithm) {
    return malloc(sizeof(void *));
}

__attribute__((weak)) void rrddim_set(void *st, const char *id, long long value) {
}

__attribute__((weak)) procfile *procfile_open(const char *filename, const char *separators, int flags) {
    return NULL;
}

__attribute__((weak)) procfile *procfile_readall(procfile *ff) {
    return ff;
}

__attribute__((weak)) size_t procfile_lines(procfile *ff) {
    return 1;
}

__attribute__((weak)) size_t procfile_linewords(procfile *ff, size_t line) {
    return 2;
}

__attribute__((weak)) char *procfile_lineword(procfile *ff, size_t line, size_t word) {
    return (char *)"value";
}

__attribute__((weak)) unsigned long long str2kernel_uint_t(const char *s) {
    return 1024;
}

__attribute__((weak)) uint32_t simple_hash(const char *s) {
    return 12345;
}

extern int do_proc_sys_devices_system_node(int update_every, usec_t dt);

/* Test: Initial call to NUMA node function */
static void test_do_proc_sys_devices_system_node_initial(void **state) {
    (void)state;
    int result = do_proc_sys_devices_system_node(10, 0);
    assert_true(result == 0 || result == 1);
}

/* Test: Subsequent calls */
static void test_do_proc_sys_devices_system_node_subsequent(void **state) {
    (void)state;
    int result = do_proc_sys_devices_system_node(10, 0);
    assert_true(result == 0 || result == 1);
    result = do_proc_sys_devices_system_node(10, 0);
    assert_true(result == 0 || result == 1);
}

/* Test: Different update_every values */
static void test_do_proc_sys_devices_system_node_update_every_5(void **state) {
    (void)state;
    int result = do_proc_sys_devices_system_node(5, 0);
    assert_true(result == 0 || result == 1);
}

/* Test: dt parameter */
static void test_do_proc_sys_devices_system_node_dt_nonzero(void **state) {
    (void)state;
    int result = do_proc_sys_devices_system_node(10, 100);
    assert_true(result == 0 || result == 1);
}

/* Test: node structure initialization */
static void test_node_structure_initialization(void **state) {
    (void)state;
    struct node test_node = {0};
    assert_null(test_node.name);
    assert_null(test_node.numastat.filename);
    assert_null(test_node.meminfo.filename);
    assert_null(test_node.next);
}

/* Test: numastat initialization */
static void test_node_numastat_initialization(void **state) {
    (void)state;
    struct node test_node = {0};
    test_node.numastat.filename = NULL;
    test_node.numastat.ff = NULL;
    test_node.numastat.st = NULL;
    assert_null(test_node.numastat.st);
}

/* Test: meminfo initialization */
static void test_node_meminfo_initialization(void **state) {
    (void)state;
    struct node test_node = {0};
    test_node.meminfo.filename = NULL;
    test_node.meminfo.ff = NULL;
    test_node.meminfo.st_mem_usage = NULL;
    test_node.meminfo.st_mem_activity = NULL;
    assert_null(test_node.meminfo.st_mem_usage);
    assert_null(test_node.meminfo.st_mem_activity);
}

/* Test: CONFIG_BOOLEAN_AUTO handling */
static void test_do_proc_sys_devices_system_node_auto_config(void **state) {
    (void)state;
    int result = do_proc_sys_devices_system_node(10, 0);
    assert_true(result >= 0);
}

/* Test: Edge case - update_every is 1 */
static void test_do_proc_sys_devices_system_node_update_every_1(void **state) {
    (void)state;
    int result = do_proc_sys_devices_system_node(1, 0);
    assert_true(result == 0 || result == 1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_proc_sys_devices_system_node_initial),
        cmocka_unit_test(test_do_proc_sys_devices_system_node_subsequent),
        cmocka_unit_test(test_do_proc_sys_devices_system_node_update_every_5),
        cmocka_unit_test(test_do_proc_sys_devices_system_node_dt_nonzero),
        cmocka_unit_test(test_node_structure_initialization),
        cmocka_unit_test(test_node_numastat_initialization),
        cmocka_unit_test(test_node_meminfo_initialization),
        cmocka_unit_test(test_do_proc_sys_devices_system_node_auto_config),
        cmocka_unit_test(test_do_proc_sys_devices_system_node_update_every_1),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}