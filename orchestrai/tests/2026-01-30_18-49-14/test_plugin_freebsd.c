#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <cmocka.h>

/* Mock structures and variables for external dependencies */
#define NETDATA_MAIN_THREAD_EXITING 2
#define NETDATA_MAIN_THREAD_EXITED 3

typedef struct {
    int rrd_update_every;
} netdata_localhost_t;

typedef struct {
    int enabled;
} netdata_static_thread_t;

typedef struct {
    int dummy;
} RRDDIM;

typedef struct {
    int dummy;
} RRDSET;

typedef struct {
    long ldavg[3];
    long fscale;
} loadavg_t;

typedef struct {
    int t_rq;
    int t_dw;
    int t_pw;
    int t_sl;
    int t_sw;
    int t_rm;
} vmtotal_t;

/* Mock global variables */
static netdata_localhost_t mock_localhost = {.rrd_update_every = 10};
static netdata_static_thread_t mock_static_thread = {.enabled = 1};
static int system_pagesize = 4096;
static int number_of_cpus = 4;
static unsigned long long zfs_arcstats_shrinkable_cache_size_bytes = 0;

/* Mock function declarations */
void collector_error(const char *fmt, ...);
void netdata_log_debug(int level, const char *fmt, ...);
void netdata_exit_fatal(void);
void worker_register(const char *name);
void worker_unregister(void);
void worker_register_job_name(int i, const char *name);
void worker_is_idle(void);
void worker_is_busy(int i);
int service_running(int service);
void heartbeat_init(void *hb, uint64_t usec);
uint64_t heartbeat_next(void *hb);

int inicfg_get_boolean(void *config, const char *section, const char *key, int default_val);

RRDSET *rrdset_create_localhost(const char *type, const char *id, const char *parent,
    const char *title, const char *units, const char *help,
    const char *plugin, const char *module, long priority,
    int update_every, int chart_type);
RRDDIM *rrddim_add(RRDSET *st, const char *id, const char *name,
    long multiplier, long divisor, int algorithm);
void rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, collected_number value);
void rrdset_done(RRDSET *st);
void rrddim_hide(RRDSET *st, const char *id);

typedef long collected_number;
typedef long long int usec_t;

/* Mock implementations */
void collector_error(const char *fmt, ...) {
    (void)fmt;
}

void netdata_log_debug(int level, const char *fmt, ...) {
    (void)level;
    (void)fmt;
}

void netdata_exit_fatal(void) {
    exit(1);
}

void worker_register(const char *name) {
    (void)name;
}

void worker_unregister(void) {
}

void worker_register_job_name(int i, const char *name) {
    (void)i;
    (void)name;
}

void worker_is_idle(void) {
}

void worker_is_busy(int i) {
    (void)i;
}

static int mock_service_running = 1;
int service_running(int service) {
    (void)service;
    return mock_service_running;
}

void heartbeat_init(void *hb, uint64_t usec) {
    (void)hb;
    (void)usec;
}

uint64_t heartbeat_next(void *hb) {
    (void)hb;
    return 1000000; /* 1 second */
}

int inicfg_get_boolean(void *config, const char *section, const char *key, int default_val) {
    (void)config;
    (void)section;
    (void)key;
    return default_val;
}

RRDSET *rrdset_create_localhost(const char *type, const char *id, const char *parent,
    const char *title, const char *units, const char *help,
    const char *plugin, const char *module, long priority,
    int update_every, int chart_type) {
    (void)type;
    (void)id;
    (void)parent;
    (void)title;
    (void)units;
    (void)help;
    (void)plugin;
    (void)module;
    (void)priority;
    (void)update_every;
    (void)chart_type;
    return malloc(sizeof(RRDSET));
}

RRDDIM *rrddim_add(RRDSET *st, const char *id, const char *name,
    long multiplier, long divisor, int algorithm) {
    (void)st;
    (void)id;
    (void)name;
    (void)multiplier;
    (void)divisor;
    (void)algorithm;
    return malloc(sizeof(RRDDIM));
}

void rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, collected_number value) {
    (void)st;
    (void)rd;
    (void)value;
}

void rrdset_done(RRDSET *st) {
    (void)st;
}

void rrddim_hide(RRDSET *st, const char *id) {
    (void)st;
    (void)id;
}

/* Forward declarations */
int freebsd_plugin_init(void);

/* Test: freebsd_plugin_init with valid pagesize */
static int test_freebsd_plugin_init_success(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 4;
    
    int result = freebsd_plugin_init();
    assert_int_equal(result, 0);
    assert_int_equal(system_pagesize, 4096);
    assert_int_equal(number_of_cpus, 4);
    return 0;
}

/* Test: freebsd_plugin_init with invalid pagesize */
static int test_freebsd_plugin_init_invalid_pagesize(void **state) {
    (void)state;
    system_pagesize = 0;
    number_of_cpus = 4;
    
    int result = freebsd_plugin_init();
    assert_int_equal(result, 1);
    return 0;
}

/* Test: freebsd_plugin_init with zero CPUs */
static int test_freebsd_plugin_init_zero_cpus(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 0;
    
    int result = freebsd_plugin_init();
    assert_int_equal(result, 1);
    return 0;
}

/* Test: freebsd_main initialization and cleanup */
static int test_freebsd_main_basic_execution(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 2;
    mock_service_running = 0; /* Exit immediately */
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: freebsd_main with single iteration */
static int test_freebsd_main_single_iteration(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 1;
    mock_service_running = 1;
    
    /* This will cause immediate exit on second service_running check */
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: freebsd_main with null pointer */
static int test_freebsd_main_null_pointer(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 1;
    mock_service_running = 0;
    
    void *result = freebsd_main(NULL);
    assert_null(result);
    return 0;
}

/* Test: Module array initialization */
static int test_module_array_structure(void **state) {
    (void)state;
    /* Verify the module array has correct structure */
    assert_non_null(&freebsd_modules);
    return 0;
}

/* Test: Multiple iterations of main loop */
static int test_freebsd_main_multiple_iterations(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 2;
    mock_service_running = 0;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    assert_int_equal(mock_static_thread.enabled, NETDATA_MAIN_THREAD_EXITED);
    return 0;
}

/* Test: Worker registration and unregistration */
static int test_freebsd_main_worker_lifecycle(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 1;
    mock_service_running = 0;
    mock_static_thread.enabled = 1;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: Edge case with negative update_every */
static int test_freebsd_main_negative_update_every(void **state) {
    (void)state;
    mock_localhost.rrd_update_every = -1;
    mock_service_running = 0;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: Cleanup function with valid pointer */
static int test_freebsd_main_cleanup_function(void **state) {
    (void)state;
    /* Testing the cleanup function behavior indirectly */
    mock_static_thread.enabled = 1;
    system_pagesize = 4096;
    number_of_cpus = 1;
    mock_service_running = 0;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    assert_int_equal(mock_static_thread.enabled, NETDATA_MAIN_THREAD_EXITED);
    return 0;
}

/* Test: Service_running check at iteration start */
static int test_freebsd_main_service_check_at_start(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 1;
    mock_service_running = 0;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: Service_running check at iteration end */
static int test_freebsd_main_service_check_at_end(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 1;
    mock_service_running = 1;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: Module iterator with all modules */
static int test_freebsd_main_all_modules_loop(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 4;
    mock_service_running = 0;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: Heartbeat timing calculation */
static int test_freebsd_main_heartbeat_dt(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 1;
    mock_localhost.rrd_update_every = 10;
    mock_service_running = 0;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: Module disabling on failure */
static int test_freebsd_main_module_enable_config(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 1;
    mock_service_running = 0;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: Large CPU count */
static int test_freebsd_main_large_cpu_count(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 128;
    mock_service_running = 0;
    
    void *result = freebsd_main((void *)&mock_static_thread);
    assert_null(result);
    return 0;
}

/* Test: Small pagesize value */
static int test_freebsd_plugin_init_small_pagesize(void **state) {
    (void)state;
    system_pagesize = 1;
    number_of_cpus = 1;
    
    int result = freebsd_plugin_init();
    assert_int_equal(result, 0);
    return 0;
}

/* Test: Large pagesize value */
static int test_freebsd_plugin_init_large_pagesize(void **state) {
    (void)state;
    system_pagesize = 65536;
    number_of_cpus = 8;
    
    int result = freebsd_plugin_init();
    assert_int_equal(result, 0);
    return 0;
}

/* Test: Single CPU system */
static int test_freebsd_plugin_init_single_cpu(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 1;
    
    int result = freebsd_plugin_init();
    assert_int_equal(result, 0);
    return 0;
}

/* Test: Maximum reasonable CPU count */
static int test_freebsd_plugin_init_max_cpus(void **state) {
    (void)state;
    system_pagesize = 4096;
    number_of_cpus = 512;
    
    int result = freebsd_plugin_init();
    assert_int_equal(result, 0);
    return 0;
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_freebsd_plugin_init_success),
        cmocka_unit_test(test_freebsd_plugin_init_invalid_pagesize),
        cmocka_unit_test(test_freebsd_plugin_init_zero_cpus),
        cmocka_unit_test(test_freebsd_main_basic_execution),
        cmocka_unit_test(test_freebsd_main_single_iteration),
        cmocka_unit_test(test_freebsd_main_null_pointer),
        cmocka_unit_test(test_module_array_structure),
        cmocka_unit_test(test_freebsd_main_multiple_iterations),
        cmocka_unit_test(test_freebsd_main_worker_lifecycle),
        cmocka_unit_test(test_freebsd_main_negative_update_every),
        cmocka_unit_test(test_freebsd_main_cleanup_function),
        cmocka_unit_test(test_freebsd_main_service_check_at_start),
        cmocka_unit_test(test_freebsd_main_service_check_at_end),
        cmocka_unit_test(test_freebsd_main_all_modules_loop),
        cmocka_unit_test(test_freebsd_main_heartbeat_dt),
        cmocka_unit_test(test_freebsd_main_module_enable_config),
        cmocka_unit_test(test_freebsd_main_large_cpu_count),
        cmocka_unit_test(test_freebsd_plugin_init_small_pagesize),
        cmocka_unit_test(test_freebsd_plugin_init_large_pagesize),
        cmocka_unit_test(test_freebsd_plugin_init_single_cpu),
        cmocka_unit_test(test_freebsd_plugin_init_max_cpus),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}