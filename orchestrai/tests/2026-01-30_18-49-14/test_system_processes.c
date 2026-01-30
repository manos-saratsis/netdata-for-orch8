#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Mock definitions for Netdata types and functions
typedef struct {
    char *name;
    void *data;
} RRDDIM;

typedef struct {
    char *type;
    RRDDIM *dimensions;
    int num_dimensions;
} RRDSET;

// Mock function implementations
static RRDSET *__mock_rrdset = NULL;
static RRDDIM *__mock_rrddim = NULL;

RRDSET* __wrap_rrdset_create_localhost(
    const char *type,
    const char *id,
    const char *parent_id,
    const char *family,
    const char *context,
    const char *title,
    const char *units,
    const char *plugin,
    const char *module,
    long priority,
    int update_every,
    int chart_type)
{
    check_expected(type);
    check_expected(id);
    check_expected(family);
    check_expected(units);
    check_expected(plugin);
    check_expected(module);
    check_expected(priority);
    check_expected(update_every);
    check_expected(chart_type);
    
    __mock_rrdset = malloc(sizeof(RRDSET));
    __mock_rrdset->type = malloc(strlen(type) + 1);
    strcpy(__mock_rrdset->type, type);
    __mock_rrdset->num_dimensions = 0;
    
    return __mock_rrdset;
}

RRDDIM* __wrap_rrddim_add(
    RRDSET *st,
    const char *name,
    const char *divisor,
    long multiplier,
    long divisor_val,
    int algorithm)
{
    check_expected(st);
    check_expected(name);
    check_expected(multiplier);
    check_expected(divisor_val);
    check_expected(algorithm);
    
    __mock_rrddim = malloc(sizeof(RRDDIM));
    __mock_rrddim->name = malloc(strlen(name) + 1);
    strcpy(__mock_rrddim->name, name);
    __mock_rrddim->data = NULL;
    
    return __mock_rrddim;
}

void __wrap_rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, int64_t value)
{
    check_expected(st);
    check_expected(rd);
    check_expected(value);
}

void __wrap_rrdset_done(RRDSET *st)
{
    check_expected(st);
}

// Helper macros for test setup
#define SETUP_MOCKS() \
    __mock_rrdset = NULL; \
    __mock_rrddim = NULL;

#define CLEANUP_MOCKS() \
    if (__mock_rrdset) { \
        free(__mock_rrdset->type); \
        free(__mock_rrdset); \
    } \
    if (__mock_rrddim) { \
        free(__mock_rrddim->name); \
        free(__mock_rrddim); \
    }

// Test for Windows common_system_processes - First call (initialization)
static int test_windows_system_processes_init(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "processes");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "processes");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 100);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "processes", NULL, "processes", NULL, "System Processes", "processes",
        "test_plugin", "test_module", 100, 1, 1));
    expect_string(__wrap_rrddim_add, name, "running");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 5);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Windows common_system_processes - Subsequent call (no reinitialization)
static int test_windows_system_processes_subsequent_call(void **state)
{
    SETUP_MOCKS();
    
    // First call - setup
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "processes");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "processes");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 100);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "processes", NULL, "processes", NULL, "System Processes", "processes",
        "test_plugin", "test_module", 100, 1, 1));
    expect_string(__wrap_rrddim_add, name, "running");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 10);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    // Second call - should not reinitialize
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 15);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Windows common_system_processes - Zero value
static int test_windows_system_processes_zero_value(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "processes");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "processes");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 100);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "processes", NULL, "processes", NULL, "System Processes", "processes",
        "test_plugin", "test_module", 100, 1, 1));
    expect_string(__wrap_rrddim_add, name, "running");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Windows common_system_processes - Max uint64_t value
static int test_windows_system_processes_max_value(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "processes");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "processes");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 100);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "processes", NULL, "processes", NULL, "System Processes", "processes",
        "test_plugin", "test_module", 100, 1, 1));
    expect_string(__wrap_rrddim_add, name, "running");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, UINT64_MAX);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Windows common_system_threads - First call (initialization)
static int test_windows_system_threads_init(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "threads");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "threads");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 200);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "threads", NULL, "processes", NULL, "Threads", "threads",
        "test_plugin", "test_module", 200, 1, 1));
    expect_string(__wrap_rrddim_add, name, "threads");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 42);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Windows common_system_threads - Subsequent call
static int test_windows_system_threads_subsequent_call(void **state)
{
    SETUP_MOCKS();
    
    // First call - setup
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "threads");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "threads");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 200);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "threads", NULL, "processes", NULL, "Threads", "threads",
        "test_plugin", "test_module", 200, 1, 1));
    expect_string(__wrap_rrddim_add, name, "threads");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 50);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    // Second call
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 55);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Windows common_system_threads - Zero value
static int test_windows_system_threads_zero_value(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "threads");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "threads");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 200);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "threads", NULL, "processes", NULL, "Threads", "threads",
        "test_plugin", "test_module", 200, 1, 1));
    expect_string(__wrap_rrddim_add, name, "threads");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Linux common_system_processes - First call (initialization)
static int test_linux_system_processes_init(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "processes");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "processes");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 100);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "processes", NULL, "processes", NULL, "System Processes", "processes",
        "test_plugin", "test_module", 100, 1, 1));
    expect_string(__wrap_rrddim_add, name, "running");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_add, st, __mock_rrdset);
    expect_string(__wrap_rrddim_add, name, "blocked");
    expect_value(__wrap_rrddim_add, multiplier, -1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 5);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 2);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Linux common_system_processes - Multiple calls
static int test_linux_system_processes_multiple_calls(void **state)
{
    SETUP_MOCKS();
    
    // Setup expectations for first call
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "processes");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "processes");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 100);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "processes", NULL, "processes", NULL, "System Processes", "processes",
        "test_plugin", "test_module", 100, 1, 1));
    expect_string(__wrap_rrddim_add, name, "running");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_add, st, __mock_rrdset);
    expect_string(__wrap_rrddim_add, name, "blocked");
    expect_value(__wrap_rrddim_add, multiplier, -1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 10);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 3);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    // Second call - no reinitialization
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 15);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 4);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Linux common_system_processes - Zero values
static int test_linux_system_processes_zero_values(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "processes");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "processes");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 100);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "processes", NULL, "processes", NULL, "System Processes", "processes",
        "test_plugin", "test_module", 100, 1, 1));
    expect_string(__wrap_rrddim_add, name, "running");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_add, st, __mock_rrdset);
    expect_string(__wrap_rrddim_add, name, "blocked");
    expect_value(__wrap_rrddim_add, multiplier, -1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, 0);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for Linux common_system_processes - Max values
static int test_linux_system_processes_max_values(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "processes");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "processes");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 100);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_rrdset_create_localhost, chart_type, 1);
    
    expect_value(__wrap_rrddim_add, st, __wrap_rrdset_create_localhost(
        "system", "processes", NULL, "processes", NULL, "System Processes", "processes",
        "test_plugin", "test_module", 100, 1, 1));
    expect_string(__wrap_rrddim_add, name, "running");
    expect_value(__wrap_rrddim_add, multiplier, 1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_add, st, __mock_rrdset);
    expect_string(__wrap_rrddim_add, name, "blocked");
    expect_value(__wrap_rrddim_add, multiplier, -1);
    expect_value(__wrap_rrddim_add, divisor_val, 1);
    expect_value(__wrap_rrddim_add, algorithm, 0);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, UINT64_MAX);
    
    expect_value(__wrap_rrddim_set_by_pointer, st, __mock_rrdset);
    expect_value(__wrap_rrddim_set_by_pointer, rd, __mock_rrddim);
    expect_value(__wrap_rrddim_set_by_pointer, value, UINT64_MAX);
    
    expect_value(__wrap_rrdset_done, st, __mock_rrdset);
    
    CLEANUP_MOCKS();
    return 0;
}

// Test for common_system_context_switch - First call (initialization)
static int test_system_context_switch_init(void **state)
{
    SETUP_MOCKS();
    
    expect_string(__wrap_rrdset_create_localhost, type, "system");
    expect_string(__wrap_rrdset_create_localhost, id, "ctxt");
    expect_string(__wrap_rrdset_create_localhost, family, "processes");
    expect_string(__wrap_rrdset_create_localhost, units, "context switches/s");
    expect_string(__wrap_rrdset_create_localhost, plugin, "test_plugin");
    expect_string(__wrap_rrdset_create_localhost, module, "test_module");
    expect_value(__wrap_rrdset_create_localhost, priority, 300);
    expect_value(__wrap_rrdset_create_localhost, update_every, 1);
    expect_value(__wrap_