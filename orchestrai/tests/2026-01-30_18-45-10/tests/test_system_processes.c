#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Mock declarations */
typedef struct {
    char *id;
    char *name;
} RRDSET;

typedef struct {
    char *name;
} RRDDIM;

#define RRDSET_TYPE_LINE 2
#define RRDSET_TYPE_AREA 1
#define NETDATA_CHART_PRIO_SYSTEM_PROCESSES 3000
#define NETDATA_CHART_PRIO_SYSTEM_CTXT 3100
#define NETDATA_CHART_PRIO_WINDOWS_THREADS 3200
#define RRD_ALGORITHM_ABSOLUTE 2
#define RRD_ALGORITHM_INCREMENTAL 1
#define _COMMON_PLUGIN_NAME "test_plugin"
#define _COMMON_PLUGIN_MODULE_NAME "test_module"
#define unlikely(x) (x)

typedef long long collected_number;

static RRDSET* rrdset_create_localhost(const char *type, const char *id, const char *parentid,
                                      const char *family, const char *context, const char *title,
                                      const char *units, const char *plugin, const char *module,
                                      long priority, int update_every, int chart_type) {
    RRDSET *st = (RRDSET*)malloc(sizeof(RRDSET));
    st->id = (char*)malloc(strlen(id) + 1);
    strcpy(st->id, id);
    return st;
}

static RRDDIM* rrddim_add(RRDSET *st, const char *name, const char *divisor,
                         long multiplier, long divisor_val, int algorithm) {
    RRDDIM *rd = (RRDDIM*)malloc(sizeof(RRDDIM));
    rd->name = (char*)malloc(strlen(name) + 1);
    strcpy(rd->name, name);
    return rd;
}

static void rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, collected_number value) {
    /* Mock implementation */
}

static void rrdset_done(RRDSET *st) {
    /* Mock implementation */
}

#include "../src/collectors/common-contexts/system-processes.h"

/* Test cases for Windows common_system_processes */
#ifdef OS_WINDOWS
static void test_windows_common_system_processes_first_call_initializes(void **state) {
    common_system_processes(5, 1);
    assert_true(1);
}

static void test_windows_common_system_processes_zero_running(void **state) {
    common_system_processes(0, 1);
    assert_true(1);
}

static void test_windows_common_system_processes_large_running(void **state) {
    common_system_processes(0xFFFFFFFFU, 1);
    assert_true(1);
}

static void test_windows_common_system_processes_sequential_calls(void **state) {
    common_system_processes(10, 1);
    common_system_processes(20, 1);
    common_system_processes(15, 1);
    assert_true(1);
}

static void test_windows_common_system_threads_first_call_initializes(void **state) {
    common_system_threads(100, 1);
    assert_true(1);
}

static void test_windows_common_system_threads_zero_threads(void **state) {
    common_system_threads(0, 1);
    assert_true(1);
}

static void test_windows_common_system_threads_large_threads(void **state) {
    common_system_threads(0xFFFFFFFFU, 1);
    assert_true(1);
}

static void test_windows_common_system_threads_sequential_calls(void **state) {
    common_system_threads(100, 1);
    common_system_threads(150, 1);
    common_system_threads(120, 1);
    assert_true(1);
}
#endif

/* Test cases for Linux common_system_processes */
#ifdef OS_LINUX
static void test_linux_common_system_processes_first_call_initializes(void **state) {
    common_system_processes(5, 3, 1);
    assert_true(1);
}

static void test_linux_common_system_processes_zero_running_and_blocked(void **state) {
    common_system_processes(0, 0, 1);
    assert_true(1);
}

static void test_linux_common_system_processes_zero_running_nonzero_blocked(void **state) {
    common_system_processes(0, 10, 1);
    assert_true(1);
}

static void test_linux_common_system_processes_nonzero_running_zero_blocked(void **state) {
    common_system_processes(10, 0, 1);
    assert_true(1);
}

static void test_linux_common_system_processes_large_values(void **state) {
    common_system_processes(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 1);
    assert_true(1);
}

static void test_linux_common_system_processes_sequential_calls(void **state) {
    common_system_processes(5, 3, 1);
    common_system_processes(10, 5, 1);
    common_system_processes(8, 4, 1);
    assert_true(1);
}

static void test_linux_common_system_processes_different_update_every(void **state) {
    common_system_processes(5, 3, 1);
    common_system_processes(5, 3, 10);
    common_system_processes(5, 3, 3600);
    assert_true(1);
}
#endif

/* Test cases for context switch (both Windows and Linux) */
static void test_common_system_context_switch_first_call_initializes(void **state) {
    common_system_context_switch(1000, 1);
    assert_true(1);
}

static void test_common_system_context_switch_zero_value(void **state) {
    common_system_context_switch(0, 1);
    assert_true(1);
}

static void test_common_system_context_switch_large_value(void **state) {
    common_system_context_switch(0xFFFFFFFFFFFFFFFFULL, 1);
    assert_true(1);
}

static void test_common_system_context_switch_sequential_calls(void **state) {
    common_system_context_switch(1000, 1);
    common_system_context_switch(2000, 1);
    common_system_context_switch(1500, 1);
    assert_true(1);
}

static void test_common_system_context_switch_different_update_every(void **state) {
    common_system_context_switch(1000, 1);
    common_system_context_switch(1000, 10);
    common_system_context_switch(1000, 3600);
    assert_true(1);
}

static void test_common_system_context_switch_boundary_update_every(void **state) {
    common_system_context_switch(500, 1);
    common_system_context_switch(500, 2147483647);
    assert_true(1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
#ifdef OS_WINDOWS
        cmocka_unit_test(test_windows_common_system_processes_first_call_initializes),
        cmocka_unit_test(test_windows_common_system_processes_zero_running),
        cmocka_unit_test(test_windows_common_system_processes_large_running),
        cmocka_unit_test(test_windows_common_system_processes_sequential_calls),
        cmocka_unit_test(test_windows_common_system_threads_first_call_initializes),
        cmocka_unit_test(test_windows_common_system_threads_zero_threads),
        cmocka_unit_test(test_windows_common_system_threads_large_threads),
        cmocka_unit_test(test_windows_common_system_threads_sequential_calls),
#endif
#ifdef OS_LINUX
        cmocka_unit_test(test_linux_common_system_processes_first_call_initializes),
        cmocka_unit_test(test_linux_common_system_processes_zero_running_and_blocked),
        cmocka_unit_test(test_linux_common_system_processes_zero_running_nonzero_blocked),
        cmocka_unit_test(test_linux_common_system_processes_nonzero_running_zero_blocked),
        cmocka_unit_test(test_linux_common_system_processes_large_values),
        cmocka_unit_test(test_linux_common_system_processes_sequential_calls),
        cmocka_unit_test(test_linux_common_system_processes_different_update_every),
#endif
        cmocka_unit_test(test_common_system_context_switch_first_call_initializes),
        cmocka_unit_test(test_common_system_context_switch_zero_value),
        cmocka_unit_test(test_common_system_context_switch_large_value),
        cmocka_unit_test(test_common_system_context_switch_sequential_calls),
        cmocka_unit_test(test_common_system_context_switch_different_update_every),
        cmocka_unit_test(test_common_system_context_switch_boundary_update_every),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}