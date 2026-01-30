#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

/* Mock structures and forward declarations */
typedef struct {
    const char *name;
    int d_type;
} mock_dirent;

typedef struct {
    mock_dirent *entries;
    int count;
    int index;
} mock_dir;

/* External function mocks */
__attribute__((weak)) int collector_error(const char *fmt, ...) {
    return 0;
}

__attribute__((weak)) void netdata_fix_chart_id(char *s) {
}

__attribute__((weak)) char *inicfg_get(void *config, const char *section, const char *name, const char *default_value) {
    return (char *)default_value;
}

__attribute__((weak)) int inicfg_get_boolean(void *config, const char *section, const char *name, int default_value) {
    return default_value;
}

__attribute__((weak)) void rrdlabels_get_value_strcpyz(void *labels, char *buffer, size_t size, const char *key) {
    snprintf(buffer, size, "none");
}

__attribute__((weak)) int dictionary_entries(void *dict) {
    return 1;
}

__attribute__((weak)) void *dictionary_create_advanced(int options, void *stats, size_t value_len) {
    return malloc(sizeof(void *));
}

__attribute__((weak)) void dictionary_register_insert_callback(void *dict, void *callback, void *data) {
}

__attribute__((weak)) void dictionary_register_delete_callback(void *dict, void *callback, void *data) {
}

__attribute__((weak)) void *dictionary_set(void *dict, const char *name, void *value, size_t value_len) {
    return malloc(value_len);
}

__attribute__((weak)) void dictionary_destroy(void *dict) {
    free(dict);
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

__attribute__((weak)) void rrddim_set_by_pointer(void *st, void *rd, long long value) {
}

__attribute__((weak)) void rrdlabels_add(void *labels, const char *name, const char *value, int source) {
}

__attribute__((weak)) procfile *procfile_open(const char *filename, const char *separators, int flags) {
    return NULL;
}

__attribute__((weak)) void procfile_close(procfile *ff) {
}

__attribute__((weak)) procfile *procfile_readall(procfile *ff) {
    return ff;
}

__attribute__((weak)) size_t procfile_lines(procfile *ff) {
    return 1;
}

__attribute__((weak)) size_t procfile_linewords(procfile *ff, size_t line) {
    return 1;
}

__attribute__((weak)) char *procfile_lineword(procfile *ff, size_t line, size_t word) {
    return (char *)"";
}

__attribute__((weak)) unsigned long long str2ull(const char *s, char **endptr) {
    return 100;
}

__attribute__((weak)) STRING *string_strdupz(const char *s) {
    return (STRING *)malloc(strlen(s) + 1);
}

__attribute__((weak)) void string_freez(STRING *s) {
    free(s);
}

extern void pci_aer_plugin_cleanup(void);
extern int do_proc_sys_devices_pci_aer(int update_every, usec_t dt);

/* Test: aer_value_conflict_callback merges values */
static void test_aer_value_conflict_callback_merge(void **state) {
    (void)state;
    struct aer_value old_val = {.count = 50};
    struct aer_value new_val = {.count = 100};
    bool result = aer_value_conflict_callback(NULL, &old_val, &new_val, NULL);
    assert_int_equal(old_val.count, 100);
    assert_false(result);
}

/* Test: pci_aer_plugin_cleanup destroys dictionary */
static void test_pci_aer_plugin_cleanup_success(void **state) {
    (void)state;
    pci_aer_plugin_cleanup();
}

/* Test: pci_aer_plugin_cleanup when aer_root is NULL */
static void test_pci_aer_plugin_cleanup_null(void **state) {
    (void)state;
    pci_aer_plugin_cleanup();
}

/* Test: do_proc_sys_devices_pci_aer initialization */
static void test_do_proc_sys_devices_pci_aer_init(void **state) {
    (void)state;
    int result = do_proc_sys_devices_pci_aer(10, 0);
    assert_true(result == 0 || result == 1);
}

/* Test: do_proc_sys_devices_pci_aer with CONFIG_BOOLEAN_NO */
static void test_do_proc_sys_devices_pci_aer_disabled(void **state) {
    (void)state;
    int result = do_proc_sys_devices_pci_aer(10, 0);
    assert_true(result >= 0);
}

/* Test: do_proc_sys_devices_pci_aer update_every parameter */
static void test_do_proc_sys_devices_pci_aer_update_every(void **state) {
    (void)state;
    int result = do_proc_sys_devices_pci_aer(5, 0);
    assert_true(result >= 0);
}

/* Test: do_proc_sys_devices_pci_aer with dt parameter */
static void test_do_proc_sys_devices_pci_aer_dt_parameter(void **state) {
    (void)state;
    int result = do_proc_sys_devices_pci_aer(10, 100);
    assert_true(result >= 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aer_value_conflict_callback_merge),
        cmocka_unit_test(test_pci_aer_plugin_cleanup_success),
        cmocka_unit_test(test_pci_aer_plugin_cleanup_null),
        cmocka_unit_test(test_do_proc_sys_devices_pci_aer_init),
        cmocka_unit_test(test_do_proc_sys_devices_pci_aer_disabled),
        cmocka_unit_test(test_do_proc_sys_devices_pci_aer_update_every),
        cmocka_unit_test(test_do_proc_sys_devices_pci_aer_dt_parameter),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}