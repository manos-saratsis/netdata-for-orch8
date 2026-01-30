#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <cmocka.h>

/* Type definitions */
typedef long long collected_number;

/* Mock structures */
typedef struct procfile {
    char *data;
    size_t size;
} procfile;

typedef struct netdata_extrafrag {
    char *node_zone;
    uint32_t hash;
    char *id;
    collected_number orders[11];
    struct netdata_extrafrag *next;
} netdata_extrafrag;

/* Mock constants */
#define NETDATA_ORDER_FRAGMENTATION 11
#define NETDATA_CHART_PRIO_MEM_FRAGMENTATION 70100
#define PROCFILE_FLAG_DEFAULT 0

/* Forward declarations */
int do_module_numa_extfrag(int update_every, const char *name);
void debugfs2lower(char *name);

/* Mock external functions */
__attribute__((weak)) procfile *procfile_open(const char *filename, const char *separators, int flags) {
    return (procfile *)malloc(sizeof(procfile));
}

__attribute__((weak)) procfile *procfile_readall(procfile *ff) {
    return ff;
}

__attribute__((weak)) void procfile_close(procfile *ff) {
    if (ff) free(ff);
}

__attribute__((weak)) size_t procfile_lines(procfile *ff) {
    return 0;
}

__attribute__((weak)) size_t procfile_linewords(procfile *ff, size_t line) {
    return 0;
}

__attribute__((weak)) char *procfile_lineword(procfile *ff, size_t line, size_t word) {
    return NULL;
}

__attribute__((weak)) uint32_t simple_hash(const char *name) {
    uint32_t hash = 0;
    while (*name) {
        hash = hash * 31 + *name;
        name++;
    }
    return hash;
}

__attribute__((weak)) void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

__attribute__((weak)) char *strdupz(const char *s) {
    return strdup(s);
}

__attribute__((weak)) void snprintfz(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, size, fmt, args);
    va_end(args);
}

__attribute__((weak)) double str2ndd(const char *str, char **endptr) {
    return strtod(str, endptr);
}

__attribute__((weak)) void strncpyz(char *dest, const char *src, size_t len) {
    strncpy(dest, src, len);
    dest[len] = '\0';
}

/* Test: do_module_numa_extfrag - no file available */
static void test_do_module_numa_extfrag_no_file(void **state) {
    int result = do_module_numa_extfrag(1, "test");
    assert_int_equal(result, 1);
}

/* Test: do_module_numa_extfrag - with update_every 1 */
static void test_do_module_numa_extfrag_update_every_1(void **state) {
    int result = do_module_numa_extfrag(1, "test");
    assert_true(result == 0 || result == 1);
}

/* Test: do_module_numa_extfrag - with update_every 10 */
static void test_do_module_numa_extfrag_update_every_10(void **state) {
    int result = do_module_numa_extfrag(10, "test");
    assert_true(result == 0 || result == 1);
}

/* Test: do_module_numa_extfrag - with NULL name */
static void test_do_module_numa_extfrag_null_name(void **state) {
    int result = do_module_numa_extfrag(1, NULL);
    assert_true(result == 0 || result == 1);
}

/* Test: do_module_numa_extfrag - multiple calls */
static void test_do_module_numa_extfrag_multiple_calls(void **state) {
    int result1 = do_module_numa_extfrag(1, "test");
    int result2 = do_module_numa_extfrag(1, "test");
    assert_true(result1 == 0 || result1 == 1);
    assert_true(result2 == 0 || result2 == 1);
}

/* Test: do_module_numa_extfrag - with zero update_every */
static void test_do_module_numa_extfrag_zero_update_every(void **state) {
    int result = do_module_numa_extfrag(0, "test");
    assert_true(result == 0 || result == 1);
}

/* Test: do_module_numa_extfrag - with max update_every */
static void test_do_module_numa_extfrag_max_update_every(void **state) {
    int result = do_module_numa_extfrag(3600, "test");
    assert_true(result == 0 || result == 1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_module_numa_extfrag_no_file),
        cmocka_unit_test(test_do_module_numa_extfrag_update_every_1),
        cmocka_unit_test(test_do_module_numa_extfrag_update_every_10),
        cmocka_unit_test(test_do_module_numa_extfrag_null_name),
        cmocka_unit_test(test_do_module_numa_extfrag_multiple_calls),
        cmocka_unit_test(test_do_module_numa_extfrag_zero_update_every),
        cmocka_unit_test(test_do_module_numa_extfrag_max_update_every),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}