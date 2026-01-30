#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <mach/mach.h>

// Mock declarations
int do_macos_mach_smi(int update_every, usec_t dt);
extern int rrdset_find_active_bytype_localhost(const char *type, const char *id);
extern int rrdset_create_localhost(const char *type, const char *id, const char *name,
    const char *family, const char *context, const char *title, const char *units,
    const char *plugin, const char *module, int priority, int update_every, int type_enum);
extern void rrddim_add(void *st, const char *name, const char *divisor, 
    long multiplier, long divisor_val, int algorithm);
extern void rrddim_set(void *st, const char *name, long long value);
extern void rrddim_hide(void *st, const char *name);
extern void rrdset_done(void *st);
extern int inicfg_get_boolean(void *config, const char *section, const char *key, int default_val);
extern const char *mach_error_string(int kr);
extern void collector_error(const char *fmt, ...);

// Mock implementations
typedef struct {
    int error;
    natural_t cpu_data[4];
    vm_size_t page_size;
} mock_mach_state;

static mock_mach_state g_mach_state;

// Mock functions
kern_return_t __wrap_host_page_size(host_t host, vm_size_t *page_size) {
    if (g_mach_state.error) {
        return g_mach_state.error;
    }
    *page_size = g_mach_state.page_size;
    return KERN_SUCCESS;
}

kern_return_t __wrap_host_statistics(host_t host, host_flavor_t flavor,
    host_info_t host_info, mach_msg_type_number_t *count) {
    if (g_mach_state.error) {
        return g_mach_state.error;
    }
    if (flavor == HOST_CPU_LOAD_INFO) {
        natural_t *cp_time = (natural_t *)host_info;
        memcpy(cp_time, g_mach_state.cpu_data, sizeof(g_mach_state.cpu_data));
        *count = 4;
    }
    return KERN_SUCCESS;
}

kern_return_t __wrap_host_statistics64(host_t host, host_flavor_t flavor,
    host_info64_t host_info, mach_msg_type_number_t *count) {
    return KERN_SUCCESS;
}

host_t __wrap_mach_host_self(void) {
    return 1;
}

// Mock rrdset functions
void *__wrap_rrdset_find_active_bytype_localhost(const char *type, const char *id) {
    if (mock_is_eq_string(type, "system") && mock_is_eq_string(id, "cpu")) {
        static char mock_rrdset[] = "mock_rrdset";
        return mock_rrdset;
    }
    return NULL;
}

void *__wrap_rrdset_create_localhost(const char *type, const char *id, const char *name,
    const char *family, const char *context, const char *title, const char *units,
    const char *plugin, const char *module, int priority, int update_every, int type_enum) {
    static char mock_rrdset[] = "mock_rrdset";
    check_expected(type);
    check_expected(id);
    return mock_rrdset;
}

void __wrap_rrddim_add(void *st, const char *name, const char *divisor,
    long multiplier, long divisor_val, int algorithm) {
    check_expected(name);
}

void __wrap_rrddim_set(void *st, const char *name, long long value) {
    check_expected(name);
}

void __wrap_rrddim_hide(void *st, const char *name) {
    check_expected(name);
}

void __wrap_rrdset_done(void *st) {
}

int __wrap_inicfg_get_boolean(void *config, const char *section, const char *key, int default_val) {
    return default_val;
}

const char *__wrap_mach_error_string(int kr) {
    return "Mock error";
}

void __wrap_collector_error(const char *fmt, ...) {
    // Suppress errors during testing
}

// Test cases

static void test_do_macos_mach_smi_success_with_cpu_enabled(void **state) {
    g_mach_state.error = KERN_SUCCESS;
    g_mach_state.page_size = 4096;
    g_mach_state.cpu_data[0] = 100;
    g_mach_state.cpu_data[1] = 50;
    g_mach_state.cpu_data[2] = 75;
    g_mach_state.cpu_data[3] = 1000;

    int result = do_macos_mach_smi(10, 1000000);
    assert_int_equal(result, 0);
}

static void test_do_macos_mach_smi_host_page_size_fails(void **state) {
    g_mach_state.error = KERN_FAILURE;
    g_mach_state.page_size = 4096;

    int result = do_macos_mach_smi(10, 1000000);
    assert_int_equal(result, -1);
}

static void test_do_macos_mach_smi_multiple_calls_reuses_config(void **state) {
    g_mach_state.error = KERN_SUCCESS;
    g_mach_state.page_size = 4096;
    g_mach_state.cpu_data[0] = 100;
    g_mach_state.cpu_data[1] = 50;
    g_mach_state.cpu_data[2] = 75;
    g_mach_state.cpu_data[3] = 1000;

    int result1 = do_macos_mach_smi(10, 1000000);
    int result2 = do_macos_mach_smi(10, 1000000);
    
    assert_int_equal(result1, 0);
    assert_int_equal(result2, 0);
}

static void test_do_macos_mach_smi_cpu_stats_path(void **state) {
    g_mach_state.error = KERN_SUCCESS;
    g_mach_state.page_size = 4096;
    g_mach_state.cpu_data[0] = 100;
    g_mach_state.cpu_data[1] = 50;
    g_mach_state.cpu_data[2] = 75;
    g_mach_state.cpu_data[3] = 1000;

    int result = do_macos_mach_smi(10, 1000000);
    assert_int_equal(result, 0);
}

static void test_do_macos_mach_smi_dt_parameter_ignored(void **state) {
    g_mach_state.error = KERN_SUCCESS;
    g_mach_state.page_size = 4096;
    g_mach_state.cpu_data[0] = 100;
    g_mach_state.cpu_data[1] = 50;
    g_mach_state.cpu_data[2] = 75;
    g_mach_state.cpu_data[3] = 1000;

    int result1 = do_macos_mach_smi(10, 0);
    int result2 = do_macos_mach_smi(10, 1000);
    int result3 = do_macos_mach_smi(10, 999999999);
    
    assert_int_equal(result1, 0);
    assert_int_equal(result2, 0);
    assert_int_equal(result3, 0);
}

static void test_do_macos_mach_smi_various_page_sizes(void **state) {
    g_mach_state.error = KERN_SUCCESS;
    
    // Test with small page size
    g_mach_state.page_size = 512;
    g_mach_state.cpu_data[0] = 100;
    g_mach_state.cpu_data[1] = 50;
    g_mach_state.cpu_data[2] = 75;
    g_mach_state.cpu_data[3] = 1000;
    int result1 = do_macos_mach_smi(10, 1000000);
    assert_int_equal(result1, 0);
    
    // Test with large page size
    g_mach_state.page_size = 65536;
    int result2 = do_macos_mach_smi(10, 1000000);
    assert_int_equal(result2, 0);
}

static void test_do_macos_mach_smi_zero_update_every(void **state) {
    g_mach_state.error = KERN_SUCCESS;
    g_mach_state.page_size = 4096;
    g_mach_state.cpu_data[0] = 100;
    g_mach_state.cpu_data[1] = 50;
    g_mach_state.cpu_data[2] = 75;
    g_mach_state.cpu_data[3] = 1000;

    int result = do_macos_mach_smi(0, 1000000);
    assert_int_equal(result, 0);
}

static void test_do_macos_mach_smi_large_update_every(void **state) {
    g_mach_state.error = KERN_SUCCESS;
    g_mach_state.page_size = 4096;
    g_mach_state.cpu_data[0] = 100;
    g_mach_state.cpu_data[1] = 50;
    g_mach_state.cpu_data[2] = 75;
    g_mach_state.cpu_data[3] = 1000;

    int result = do_macos_mach_smi(3600, 1000000);
    assert_int_equal(result, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_macos_mach_smi_success_with_cpu_enabled),
        cmocka_unit_test(test_do_macos_mach_smi_host_page_size_fails),
        cmocka_unit_test(test_do_macos_mach_smi_multiple_calls_reuses_config),
        cmocka_unit_test(test_do_macos_mach_smi_cpu_stats_path),
        cmocka_unit_test(test_do_macos_mach_smi_dt_parameter_ignored),
        cmocka_unit_test(test_do_macos_mach_smi_various_page_sizes),
        cmocka_unit_test(test_do_macos_mach_smi_zero_update_every),
        cmocka_unit_test(test_do_macos_mach_smi_large_update_every),
    };

    return cmocka_run_tests(tests);
}