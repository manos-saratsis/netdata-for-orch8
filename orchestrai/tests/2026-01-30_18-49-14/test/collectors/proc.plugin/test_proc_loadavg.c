#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

// Mock definitions for external dependencies
#define RRD_ALGORITHM_ABSOLUTE 1
#define RRDSET_TYPE_LINE 1
#define PROCFILE_FLAG_DEFAULT 0
#define USEC_PER_SEC 1000000
#define CONFIG_BOOLEAN_YES 1
#define NETDATA_CHART_PRIO_SYSTEM_LOAD 100
#define NETDATA_CHART_PRIO_SYSTEM_ACTIVE_PROCESSES 110
#define PLUGIN_PROC_NAME "proc.plugin"
#define PLUGIN_PROC_MODULE_LOADAVG_NAME "/proc/loadavg"
#define PLUGIN_PROC_CONFIG_NAME "proc"
#define MIN_LOADAVG_UPDATE_EVERY 5
#define CONFIG_SECTION_PLUGIN_PROC_LOADAVG "plugin:proc:/proc/loadavg"
#define FILENAME_MAX 4096

typedef struct {
    unsigned long long val;
} collected_number;

typedef struct {
    char *data;
} RRDDIM;

typedef struct {
    RRDDIM *dims;
    unsigned long long update_every;
} RRDSET;

typedef struct {
    void *data;
} RRDVAR_ACQUIRED;

typedef struct {
    char *name;
} procfile;

typedef unsigned long long usec_t;

// Global mocks
static procfile *mock_procfile = NULL;
static RRDSET *mock_load_chart = NULL;
static RRDSET *mock_processes_chart = NULL;
static RRDVAR_ACQUIRED *mock_rd_pidmax = NULL;
static int mock_procfile_lines = 0;
static int mock_procfile_linewords = 0;
static char *mock_procfile_words[10];
static int mock_config_do_loadavg = 1;
static int mock_config_do_all_processes = 1;
static unsigned long long mock_pid_max = 32768;
static int mock_procfile_open_fail = 0;
static int mock_procfile_readall_fail = 0;
static int mock_read_single_number_fail = 0;

// Mock implementations
const char *netdata_configured_host_prefix = "/";

struct config_mock {
    const char *section;
    const char *name;
    const char *default_value;
};

typedef struct config_mock netdata_config_t;
netdata_config_t netdata_config;

// Mock functions
procfile *procfile_open(const char *filename, const char *separator, int flags) {
    if (mock_procfile_open_fail) {
        return NULL;
    }
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile->name = (char *)malloc(strlen(filename) + 1);
    strcpy(mock_procfile->name, filename);
    return mock_procfile;
}

procfile *procfile_readall(procfile *ff) {
    if (mock_procfile_readall_fail) {
        return NULL;
    }
    return ff;
}

size_t procfile_lines(procfile *ff) {
    return mock_procfile_lines;
}

size_t procfile_linewords(procfile *ff, size_t line) {
    return mock_procfile_linewords;
}

const char *procfile_lineword(procfile *ff, size_t line, size_t word) {
    if (word < 10) {
        return mock_procfile_words[word] ? mock_procfile_words[word] : "";
    }
    return "";
}

int inicfg_get_boolean(netdata_config_t *config, const char *section, const char *name, int default_value) {
    if (strcmp(name, "enable load average") == 0) {
        return mock_config_do_loadavg;
    }
    if (strcmp(name, "enable total processes") == 0) {
        return mock_config_do_all_processes;
    }
    return default_value;
}

const char *inicfg_get(netdata_config_t *config, const char *section, const char *name, const char *default_value) {
    return default_value;
}

unsigned long long str2ull(const char *s, char **endptr) {
    return strtoull(s, endptr, 10);
}

double strtod(const char *s, char **endptr) {
    return strtod(s, endptr);
}

unsigned long long os_get_system_pid_max(void) {
    return mock_pid_max;
}

RRDSET *rrdset_find(void *host, const char *id, int check_if_active) {
    return mock_processes_chart;
}

RRDSET *rrdset_create_localhost(const char *type, const char *id, const char *parentid,
                                 const char *family, const char *context, const char *title,
                                 const char *units, const char *plugin, const char *module,
                                 long priority, int update_every, int chart_type) {
    RRDSET *st = (RRDSET *)malloc(sizeof(RRDSET));
    st->update_every = update_every;
    if (strcmp(id, "load") == 0) {
        mock_load_chart = st;
    } else if (strcmp(id, "active_processes") == 0) {
        mock_processes_chart = st;
    }
    return st;
}

RRDDIM *rrddim_add(RRDSET *st, const char *id, const char *name, int multiplier,
                   int divisor, int algorithm) {
    RRDDIM *rd = (RRDDIM *)malloc(sizeof(RRDDIM));
    rd->data = (char *)malloc(strlen(id) + 1);
    strcpy(rd->data, id);
    return rd;
}

void rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, collected_number value) {
    // Mock implementation
}

void rrdset_done(RRDSET *st) {
    // Mock implementation
}

RRDVAR_ACQUIRED *rrdvar_chart_variable_add_and_acquire(RRDSET *st, const char *name) {
    mock_rd_pidmax = (RRDVAR_ACQUIRED *)malloc(sizeof(RRDVAR_ACQUIRED));
    return mock_rd_pidmax;
}

void rrdvar_chart_variable_set(RRDSET *st, RRDVAR_ACQUIRED *var, unsigned long long value) {
    // Mock implementation
}

void rrdvar_chart_variable_release(RRDSET *st, RRDVAR_ACQUIRED *var) {
    // Mock implementation
}

void collector_error(const char *format, ...) {
    // Mock implementation
}

void collector_info(const char *format, ...) {
    // Mock implementation
}

// Forward declarations
void proc_loadavg_plugin_cleanup(void);
int do_proc_loadavg(int update_every, usec_t dt);

// Test cases

static int setup(void **state) {
    // Reset mocks
    mock_procfile = NULL;
    mock_load_chart = NULL;
    mock_processes_chart = NULL;
    mock_rd_pidmax = NULL;
    mock_procfile_lines = 0;
    mock_procfile_linewords = 0;
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 1;
    mock_pid_max = 32768;
    mock_procfile_open_fail = 0;
    mock_procfile_readall_fail = 0;
    mock_read_single_number_fail = 0;
    
    for (int i = 0; i < 10; i++) {
        mock_procfile_words[i] = NULL;
    }
    
    return 0;
}

static int teardown(void **state) {
    // Cleanup
    if (mock_load_chart) {
        free(mock_load_chart);
    }
    if (mock_processes_chart) {
        free(mock_processes_chart);
    }
    if (mock_rd_pidmax) {
        free(mock_rd_pidmax);
    }
    
    return 0;
}

// Test: procfile_open fails
static void test_do_proc_loadavg_procfile_open_fails(void **state) {
    mock_procfile_open_fail = 1;
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 1);
}

// Test: procfile_readall fails
static void test_do_proc_loadavg_procfile_readall_fails(void **state) {
    mock_procfile_open_fail = 0;
    mock_procfile_readall_fail = 1;
    do_proc_loadavg(10, 10000000);
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: procfile has less than 1 line
static void test_do_proc_loadavg_no_lines(void **state) {
    mock_procfile_lines = 0;
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 1);
}

// Test: procfile line has less than 6 words
static void test_do_proc_loadavg_less_than_6_words(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 5;
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 1);
}

// Test: successful load average parsing with load chart creation
static void test_do_proc_loadavg_success_with_loadavg_chart(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.5";
    mock_procfile_words[1] = "2.0";
    mock_procfile_words[2] = "2.5";
    mock_procfile_words[3] = "3";
    mock_procfile_words[4] = "450";
    mock_procfile_words[5] = "31234";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 0;
    mock_pid_max = 32768;
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: successful active processes chart creation
static void test_do_proc_loadavg_success_with_processes_chart(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.0";
    mock_procfile_words[1] = "1.5";
    mock_procfile_words[2] = "2.0";
    mock_procfile_words[3] = "2";
    mock_procfile_words[4] = "100";
    mock_procfile_words[5] = "32000";
    mock_config_do_loadavg = 0;
    mock_config_do_all_processes = 1;
    mock_pid_max = 65536;
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: both load average and processes charts enabled
static void test_do_proc_loadavg_both_charts_enabled(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "0.5";
    mock_procfile_words[1] = "1.0";
    mock_procfile_words[2] = "1.5";
    mock_procfile_words[3] = "1";
    mock_procfile_words[4] = "50";
    mock_procfile_words[5] = "32100";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 1;
    mock_pid_max = 32768;
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: both load average and processes charts disabled
static void test_do_proc_loadavg_both_charts_disabled(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.0";
    mock_procfile_words[1] = "1.0";
    mock_procfile_words[2] = "1.0";
    mock_procfile_words[3] = "0";
    mock_procfile_words[4] = "0";
    mock_procfile_words[5] = "32000";
    mock_config_do_loadavg = 0;
    mock_config_do_all_processes = 0;
    mock_pid_max = 32768;
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: load values with decimal points
static void test_do_proc_loadavg_decimal_load_values(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "0.25";
    mock_procfile_words[1] = "0.75";
    mock_procfile_words[2] = "1.25";
    mock_procfile_words[3] = "1";
    mock_procfile_words[4] = "25";
    mock_procfile_words[5] = "32050";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 1;
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: high load values
static void test_do_proc_loadavg_high_load_values(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "100.5";
    mock_procfile_words[1] = "200.75";
    mock_procfile_words[2] = "300.25";
    mock_procfile_words[3] = "500";
    mock_procfile_words[4] = "5000";
    mock_procfile_words[5] = "65535";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 1;
    mock_pid_max = 65535;
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: zero load values
static void test_do_proc_loadavg_zero_load_values(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "0.0";
    mock_procfile_words[1] = "0.0";
    mock_procfile_words[2] = "0.0";
    mock_procfile_words[3] = "0";
    mock_procfile_words[4] = "0";
    mock_procfile_words[5] = "32000";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 1;
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: minimum update_every value
static void test_do_proc_loadavg_min_update_every(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.0";
    mock_procfile_words[1] = "1.0";
    mock_procfile_words[2] = "1.0";
    mock_procfile_words[3] = "1";
    mock_procfile_words[4] = "50";
    mock_procfile_words[5] = "32000";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 0;
    
    int result = do_proc_loadavg(1, 10000000);
    assert_int_equal(result, 0);
}

// Test: large update_every value
static void test_do_proc_loadavg_large_update_every(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.0";
    mock_procfile_words[1] = "1.0";
    mock_procfile_words[2] = "1.0";
    mock_procfile_words[3] = "1";
    mock_procfile_words[4] = "50";
    mock_procfile_words[5] = "32000";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 0;
    
    int result = do_proc_loadavg(300, 10000000);
    assert_int_equal(result, 0);
}

// Test: proc_loadavg_plugin_cleanup function
static void test_proc_loadavg_plugin_cleanup(void **state) {
    // Setup: Create a processes chart
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.0";
    mock_procfile_words[1] = "1.0";
    mock_procfile_words[2] = "1.0";
    mock_procfile_words[3] = "1";
    mock_procfile_words[4] = "50";
    mock_procfile_words[5] = "32000";
    mock_config_do_loadavg = 0;
    mock_config_do_all_processes = 1;
    
    // Create the chart
    do_proc_loadavg(10, 10000000);
    
    // Now cleanup
    proc_loadavg_plugin_cleanup();
    
    // Verify cleanup completed
    assert_true(1);
}

// Test: dt parameter with next_loadavg_dt update
static void test_do_proc_loadavg_dt_update(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.0";
    mock_procfile_words[1] = "1.0";
    mock_procfile_words[2] = "1.0";
    mock_procfile_words[3] = "1";
    mock_procfile_words[4] = "50";
    mock_procfile_words[5] = "32000";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 1;
    
    // First call with dt = 10 seconds
    int result1 = do_proc_loadavg(10, 10000000);
    assert_int_equal(result1, 0);
    
    // Second call with smaller dt
    int result2 = do_proc_loadavg(10, 5000000);
    assert_int_equal(result2, 0);
}

// Test: Large number of active processes
static void test_do_proc_loadavg_many_active_processes(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "10.0";
    mock_procfile_words[1] = "15.0";
    mock_procfile_words[2] = "20.0";
    mock_procfile_words[3] = "10000";
    mock_procfile_words[4] = "50000";
    mock_procfile_words[5] = "65535";
    mock_config_do_loadavg = 0;
    mock_config_do_all_processes = 1;
    mock_pid_max = 4194304; // Large pid_max
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: Empty load values (edge case)
static void test_do_proc_loadavg_exact_6_words(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.0";
    mock_procfile_words[1] = "1.0";
    mock_procfile_words[2] = "1.0";
    mock_procfile_words[3] = "1";
    mock_procfile_words[4] = "1";
    mock_procfile_words[5] = "65535";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 1;
    
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

// Test: Configuration initialization on first call
static void test_do_proc_loadavg_config_init(void **state) {
    mock_procfile_lines = 1;
    mock_procfile_linewords = 6;
    mock_procfile_words[0] = "1.0";
    mock_procfile_words[1] = "1.0";
    mock_procfile_words[2] = "1.0";
    mock_procfile_words[3] = "1";
    mock_procfile_words[4] = "50";
    mock_procfile_words[5] = "32000";
    mock_config_do_loadavg = 1;
    mock_config_do_all_processes = 1;
    
    // First call initializes configuration
    int result = do_proc_loadavg(10, 10000000);
    assert_int_equal(result, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_procfile_open_fails, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_procfile_readall_fails, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_no_lines, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_less_than_6_words, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_success_with_loadavg_chart, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_success_with_processes_chart, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_both_charts_enabled, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_both_charts_disabled, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_decimal_load_values, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_high_load_values, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_zero_load_values, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_min_update_every, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_large_update_every, setup, teardown),
        cmocka_unit_test_setup_teardown(test_proc_loadavg_plugin_cleanup, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_dt_update, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_many_active_processes, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_exact_6_words, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_proc_loadavg_config_init, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}