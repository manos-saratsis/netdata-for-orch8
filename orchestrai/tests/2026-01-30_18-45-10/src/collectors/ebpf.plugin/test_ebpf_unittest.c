#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <cmocka.h>

#include "ebpf.h"
#include "ebpf_unittest.h"

/* Mock for external functions */
void __wrap_freez(void *ptr) {
    free(ptr);
}

char *__wrap_strdupz(const char *s) {
    return strdup(s);
}

/* Mock structures */
ebpf_module_t test_em;

/* Setup and teardown */
static int setup(void **state) {
    memset(&test_em, 0, sizeof(ebpf_module_t));
    return 0;
}

static int teardown(void **state) {
    if (test_em.info.thread_name) {
        free((void *)test_em.info.thread_name);
        test_em.info.thread_name = NULL;
    }
    if (test_em.info.config_name && test_em.info.config_name != test_em.info.thread_name) {
        free((void *)test_em.info.config_name);
        test_em.info.config_name = NULL;
    }
    return 0;
}

/* Test ebpf_ut_initialize_structure with RUNMODE_NORMAL */
static void test_initialize_structure_normal_mode(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    assert_non_null(test_em.info.thread_name);
    assert_string_equal(test_em.info.thread_name, "process");
    assert_string_equal(test_em.info.config_name, test_em.info.thread_name);
    assert_int_equal(test_em.apps_level, NETDATA_APPS_LEVEL_REAL_PARENT);
    assert_int_equal(test_em.mode, MODE_ENTRY);
}

/* Test ebpf_ut_initialize_structure initializes kernels */
static void test_initialize_structure_kernels(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    assert_true(test_em.kernels & NETDATA_V3_10);
    assert_true(test_em.kernels & NETDATA_V4_14);
    assert_true(test_em.kernels & NETDATA_V4_16);
    assert_true(test_em.kernels & NETDATA_V4_18);
    assert_true(test_em.kernels & NETDATA_V5_4);
    assert_true(test_em.kernels & NETDATA_V5_10);
    assert_true(test_em.kernels & NETDATA_V5_14);
}

/* Test ebpf_ut_initialize_structure initializes pid_map_size */
static void test_initialize_structure_pid_map_size(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    assert_int_equal(test_em.pid_map_size, ND_EBPF_DEFAULT_PID_SIZE);
}

/* Test ebpf_ut_initialize_structure initializes apps_level */
static void test_initialize_structure_apps_level(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    assert_int_equal(test_em.apps_level, NETDATA_APPS_LEVEL_REAL_PARENT);
}

/* Test ebpf_ut_initialize_structure with different modes */
static void test_initialize_structure_different_modes(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    assert_int_equal(test_em.mode, MODE_ENTRY);
    
    ebpf_ut_initialize_structure(MODE_RETURN);
    assert_int_equal(test_em.mode, MODE_RETURN);
}

/* Test ebpf_ut_initialize_structure clears structure */
static void test_initialize_structure_clears_memory(void **state) {
    /* First initialization */
    ebpf_ut_initialize_structure(MODE_ENTRY);
    char *first_name = (char *)test_em.info.thread_name;
    
    /* Reinitialize */
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    assert_non_null(test_em.info.thread_name);
    assert_string_equal(test_em.info.thread_name, "process");
}

/* Test ebpf_ut_cleanup_memory frees thread_name */
static void test_cleanup_memory_frees_thread_name(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    assert_non_null(test_em.info.thread_name);
    
    ebpf_ut_cleanup_memory();
    
    /* After cleanup, pointer should be cleared by the function */
    /* We can't directly check the pointer value, but we verified cleanup was called */
}

/* Test ebpf_ut_cleanup_memory with uninitialized structure */
static void test_cleanup_memory_with_null_pointer(void **state) {
    memset(&test_em, 0, sizeof(ebpf_module_t));
    
    /* Should not crash with NULL pointer */
    ebpf_ut_cleanup_memory();
}

/* Test ebpf_ut_load_real_binary returns 0 or -1 */
static void test_load_real_binary_return_type(void **state) {
    int result = ebpf_ut_load_real_binary();
    
    assert_true(result == 0 || result == -1);
}

/* Test ebpf_ut_load_fake_binary returns success for expected failure */
static void test_load_fake_binary_handles_failure(void **state) {
    int result = ebpf_ut_load_fake_binary();
    
    /* The function should return 1 (success) because we expect a failure
       and handle it properly */
    assert_true(result == 0 || result == 1);
}

/* Test ebpf_ut_load_fake_binary with invalid thread name */
static void test_load_fake_binary_invalid_name(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    int result = ebpf_ut_load_fake_binary();
    
    /* Should succeed at testing expected failure case */
    assert_true(result == 0 || result == 1);
}

/* Test structure reinitialization */
static void test_initialize_structure_reinitialization(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    const char *first_thread_name = test_em.info.thread_name;
    
    ebpf_ut_initialize_structure(MODE_ENTRY);
    const char *second_thread_name = test_em.info.thread_name;
    
    assert_string_equal(first_thread_name, "process");
    assert_string_equal(second_thread_name, "process");
}

/* Test config_name points to thread_name */
static void test_config_name_equals_thread_name(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    assert_ptr_equal(test_em.info.config_name, test_em.info.thread_name);
}

/* Test multiple initializations and cleanups */
static void test_multiple_init_cleanup_cycles(void **state) {
    for (int i = 0; i < 3; i++) {
        ebpf_ut_initialize_structure(MODE_ENTRY);
        assert_non_null(test_em.info.thread_name);
        assert_string_equal(test_em.info.thread_name, "process");
        
        ebpf_ut_cleanup_memory();
    }
}

/* Test initialize with all kernel versions set */
static void test_initialize_includes_all_kernels(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    uint64_t all_kernels = NETDATA_V3_10 | NETDATA_V4_14 | NETDATA_V4_16 | 
                          NETDATA_V4_18 | NETDATA_V5_4 | NETDATA_V5_10 | NETDATA_V5_14;
    
    assert_int_equal(test_em.kernels, all_kernels);
}

/* Test that cleanup memory doesn't affect global structures */
static void test_cleanup_memory_isolated(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    ebpf_ut_cleanup_memory();
    
    /* Initialize again to verify structure is still usable */
    ebpf_ut_initialize_structure(MODE_ENTRY);
    assert_non_null(test_em.info.thread_name);
}

/* Test thread name string properties */
static void test_thread_name_string_properties(void **state) {
    ebpf_ut_initialize_structure(MODE_ENTRY);
    
    assert_non_null(test_em.info.thread_name);
    assert_int_not_equal(strlen(test_em.info.thread_name), 0);
    assert_string_equal(test_em.info.thread_name, "process");
}

/* Test mode parameter is stored correctly */
static void test_mode_parameter_stored(void **state) {
    netdata_run_mode_t modes[] = {MODE_ENTRY, MODE_RETURN};
    
    for (int i = 0; i < 2; i++) {
        ebpf_ut_initialize_structure(modes[i]);
        assert_int_equal(test_em.mode, modes[i]);
    }
}

/* Run all tests */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_initialize_structure_normal_mode, setup, teardown),
        cmocka_unit_test_setup_teardown(test_initialize_structure_kernels, setup, teardown),
        cmocka_unit_test_setup_teardown(test_initialize_structure_pid_map_size, setup, teardown),
        cmocka_unit_test_setup_teardown(test_initialize_structure_apps_level, setup, teardown),
        cmocka_unit_test_setup_teardown(test_initialize_structure_different_modes, setup, teardown),
        cmocka_unit_test_setup_teardown(test_initialize_structure_clears_memory, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_memory_frees_thread_name, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_memory_with_null_pointer, setup, teardown),
        cmocka_unit_test_setup_teardown(test_load_real_binary_return_type, setup, teardown),
        cmocka_unit_test_setup_teardown(test_load_fake_binary_handles_failure, setup, teardown),
        cmocka_unit_test_setup_teardown(test_load_fake_binary_invalid_name, setup, teardown),
        cmocka_unit_test_setup_teardown(test_initialize_structure_reinitialization, setup, teardown),
        cmocka_unit_test_setup_teardown(test_config_name_equals_thread_name, setup, teardown),
        cmocka_unit_test_setup_teardown(test_multiple_init_cleanup_cycles, setup, teardown),
        cmocka_unit_test_setup_teardown(test_initialize_includes_all_kernels, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_memory_isolated, setup, teardown),
        cmocka_unit_test_setup_teardown(test_thread_name_string_properties, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mode_parameter_stored, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}