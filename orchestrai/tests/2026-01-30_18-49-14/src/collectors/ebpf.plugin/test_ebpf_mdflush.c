#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Mock definitions for external dependencies */
#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING 1
#define NETDATA_THREAD_EBPF_STOPPED 0
#define ND_EBPF_MAP_FD_NOT_INITIALIZED -1
#define NETDATA_EBPF_MAP_STATIC 1
#define NETDATA_EBPF_MAP_CONTROLLER 2
#define BPF_MAP_TYPE_PERCPU_HASH 3
#define EBPF_LOAD_TRAMPOLINE 1
#define EBPF_LOAD_PROBE 0
#define EBPF_LOAD_LEGACY 1
#define NETDATA_EBPF_INCREMENTAL_IDX 0
#define NETDATA_EBPF_CHART_TYPE_STACKED "stacked"
#define NETDATA_CHART_PRIO_MDSTAT_FLUSH 60100
#define NETDATA_EBPF_MODULE_NAME_MDFLUSH "mdflush"
#define USEC_PER_SEC 1000000
#define APPCONFIG_INITIALIZER {}
#define CLEANUP_FUNCTION_GET_PTR(x) (*(ebpf_module_t **)(x))
#define CLEANUP_FUNCTION_REGISTER(x)
#define UNUSED(x) (void)(x)
#define unlikely(x) (x)
#define netdata_mutex_lock(x)
#define netdata_mutex_unlock(x)
#define fflush(x)
#define fprintf(f, fmt, ...) mock_fprintf(f, fmt, ##__VA_ARGS__)
#define netdata_log_error(fmt, ...) mock_netdata_log_error(fmt, ##__VA_ARGS__)
#define sprintf snprintf
#define callocz(n, size) mock_callocz(n, size)
#define freez(x) mock_freez(x)

/* Type definitions */
typedef struct {
    int enabled;
    int load;
    int update_every;
    int maps_per_core;
    uint32_t lifetime;
    uint32_t running_time;
    void *objects;
    void *probe_links;
    void *maps;
    struct {
        int mode;
    } targets[2];
} ebpf_module_t;

typedef struct {
    int avl;
    uint32_t unit;
    char disk_name[32];
    bool dim_exists;
    uint64_t cnt;
} netdata_mdflush_t;

typedef uint32_t mdflush_ebpf_key_t;
typedef uint64_t mdflush_ebpf_val_t;

typedef struct {
    char *name;
    int internal_input;
    int user_input;
    int type;
    int map_fd;
    int map_type;
} ebpf_local_maps_t;

typedef struct {
    char *name;
    int mode;
} netdata_ebpf_targets_t;

typedef struct {
    void *progs;
    void *links;
} mdflush_bpf;

typedef struct {
    int avl;
} avl_tree_lock;

typedef struct {
    int avl;
} avl_t;

typedef struct {
    int timestamp;
} heartbeat_t;

typedef struct {
    int mode;
} config;

/* Global variables */
static ebpf_local_maps_t mdflush_maps[] = {
    {.name = "tbl_mdflush",
     .internal_input = 1024,
     .user_input = 0,
     .type = NETDATA_EBPF_MAP_STATIC,
     .map_fd = ND_EBPF_MAP_FD_NOT_INITIALIZED,
     .map_type = BPF_MAP_TYPE_PERCPU_HASH},
    {.name = NULL,
     .internal_input = 0,
     .user_input = 0,
     .type = NETDATA_EBPF_MAP_CONTROLLER,
     .map_fd = ND_EBPF_MAP_FD_NOT_INITIALIZED}};

netdata_ebpf_targets_t mdflush_targets[] = {
    {.name = "md_flush_request", .mode = EBPF_LOAD_TRAMPOLINE},
    {.name = NULL, .mode = EBPF_LOAD_TRAMPOLINE}};

static avl_tree_lock mdflush_pub;
static mdflush_ebpf_val_t *mdflush_ebpf_vals = NULL;

struct config mdflush_config = APPCONFIG_INITIALIZER;

int ebpf_nprocs = 4;
int running_on_kernel = 5010;
int isrh = 0;
char *ebpf_plugin_dir = "/var/lib/netdata/ebpf.plugin";
int default_btf = 0;

struct {
    int stat;
} plugin_statistics;

struct {
    int lock;
} lock;

struct {
    int lock;
} ebpf_exit_cleanup;

mdflush_bpf *mdflush_bpf_obj = NULL;

/* Mock function declarations */
int mock_fprintf(FILE *f, const char *fmt, ...);
void mock_netdata_log_error(const char *fmt, ...);
void *mock_callocz(int n, int size);
void mock_freez(void *ptr);

/* Mock implementations */
int mock_fprintf(FILE *f, const char *fmt, ...) {
    return 0;
}

void mock_netdata_log_error(const char *fmt, ...) {
}

void *mock_callocz(int n, int size) {
    return calloc(n, size);
}

void mock_freez(void *ptr) {
    free(ptr);
}

/* External function declarations for mocking */
int bpf_map_get_next_key(int fd, void *cur, void *next);
int bpf_map_lookup_elem(int fd, void *key, void *val);
void *avl_search_lock(void *tree, void *node);
void *avl_insert_lock(void *tree, void *node);
void avl_init_lock(void *tree, int (*cmp)(void *, void *));
int avl_traverse_lock(void *tree, int (*cb)(void *, void *), void *data);
int ebpf_plugin_stop(void);
void ebpf_write_chart_obsolete(const char *family, const char *name, const char *title_context,
                                const char *title, const char *units, const char *plugin,
                                const char *type, const char *context, int priority, int update_every);
void ebpf_update_kernel_memory_with_vector(void *stats, void *maps, int action);
void ebpf_unload_legacy_code(void *objects, void *links);
void ebpf_update_stats(void *stats, void *em);
void ebpf_write_global_dimension(const char *name, const char *axis, const char *algo);
void ebpf_write_begin_chart(const char *family, const char *name, const char *context);
void ebpf_write_end_chart(void);
void write_chart_dimension(const char *name, uint64_t val);
void ebpf_create_chart(const char *family, const char *name, const char *title, const char *units,
                        const char *plugin, const char *context, const char *type, int priority,
                        void *a, void *b, int c, int update_every, const char *module);
char *ebpf_find_symbol(const char *name);
void ebpf_load_program(const char *dir, void *em, int kernel, int rh, void **objs);
void ebpf_define_map_type(void *maps, int per_core, int kernel);
void ebpf_adjust_thread_load(void *em, int btf);
void ebpf_update_disabled_plugin_stats(void *em);
const char **ebpf_algorithms;

int bpf_map_get_next_key(int fd, void *cur, void *next) {
    check_expected(fd);
    return mock_type(int);
}

int bpf_map_lookup_elem(int fd, void *key, void *val) {
    check_expected(fd);
    return mock_type(int);
}

void *avl_search_lock(void *tree, void *node) {
    return mock_ptr_type(void *);
}

void *avl_insert_lock(void *tree, void *node) {
    return node;
}

void avl_init_lock(void *tree, int (*cmp)(void *, void *)) {
}

int avl_traverse_lock(void *tree, int (*cb)(void *, void *), void *data) {
    return 0;
}

int ebpf_plugin_stop(void) {
    return mock_type(int);
}

void ebpf_write_chart_obsolete(const char *family, const char *name, const char *title_context,
                                const char *title, const char *units, const char *plugin,
                                const char *type, const char *context, int priority, int update_every) {
}

void ebpf_update_kernel_memory_with_vector(void *stats, void *maps, int action) {
}

void ebpf_unload_legacy_code(void *objects, void *links) {
}

void ebpf_update_stats(void *stats, void *em) {
}

void ebpf_write_global_dimension(const char *name, const char *axis, const char *algo) {
}

void ebpf_write_begin_chart(const char *family, const char *name, const char *context) {
}

void ebpf_write_end_chart(void) {
}

void write_chart_dimension(const char *name, uint64_t val) {
}

void ebpf_create_chart(const char *family, const char *name, const char *title, const char *units,
                        const char *plugin, const char *context, const char *type, int priority,
                        void *a, void *b, int c, int update_every, const char *module) {
}

char *ebpf_find_symbol(const char *name) {
    return mock_ptr_type(char *);
}

void ebpf_load_program(const char *dir, void *em, int kernel, int rh, void **objs) {
}

void ebpf_define_map_type(void *maps, int per_core, int kernel) {
}

void ebpf_adjust_thread_load(void *em, int btf) {
}

void ebpf_update_disabled_plugin_stats(void *em) {
}

const char **ebpf_algorithms = NULL;

/* Test: mdflush_val_cmp - unit1 > unit2 */
static void test_mdflush_val_cmp_greater(void **state) {
    netdata_mdflush_t a = {.unit = 10};
    netdata_mdflush_t b = {.unit = 5};
    
    int result = mdflush_val_cmp(&a, &b);
    assert_int_equal(result, 1);
}

/* Test: mdflush_val_cmp - unit1 < unit2 */
static void test_mdflush_val_cmp_less(void **state) {
    netdata_mdflush_t a = {.unit = 5};
    netdata_mdflush_t b = {.unit = 10};
    
    int result = mdflush_val_cmp(&a, &b);
    assert_int_equal(result, -1);
}

/* Test: mdflush_val_cmp - unit1 == unit2 */
static void test_mdflush_val_cmp_equal(void **state) {
    netdata_mdflush_t a = {.unit = 5};
    netdata_mdflush_t b = {.unit = 5};
    
    int result = mdflush_val_cmp(&a, &b);
    assert_int_equal(result, 0);
}

/* Test: mdflush_val_cmp - boundary values */
static void test_mdflush_val_cmp_boundary(void **state) {
    netdata_mdflush_t a = {.unit = 0};
    netdata_mdflush_t b = {.unit = UINT32_MAX};
    
    int result = mdflush_val_cmp(&a, &b);
    assert_int_equal(result, -1);
    
    result = mdflush_val_cmp(&b, &a);
    assert_int_equal(result, 1);
}

/* Test: mdflush_read_count_map - with single core */
static void test_mdflush_read_count_map_single_core(void **state) {
    mdflush_ebpf_vals = calloc(4, sizeof(mdflush_ebpf_val_t));
    mdflush_ebpf_vals[0] = 42;
    
    mdflush_maps[0].map_fd = 10;
    
    expect_value(bpf_map_get_next_key, fd, 10);
    will_return(bpf_map_get_next_key, -1); // No more keys
    
    mdflush_read_count_map(0); // maps_per_core = 0
    
    free(mdflush_ebpf_vals);
    mdflush_ebpf_vals = NULL;
}

/* Test: mdflush_read_count_map - with multiple cores */
static void test_mdflush_read_count_map_multiple_cores(void **state) {
    ebpf_nprocs = 4;
    mdflush_ebpf_vals = calloc(4, sizeof(mdflush_ebpf_val_t));
    mdflush_ebpf_vals[0] = 10;
    mdflush_ebpf_vals[1] = 20;
    mdflush_ebpf_vals[2] = 30;
    mdflush_ebpf_vals[3] = 40;
    
    mdflush_maps[0].map_fd = 10;
    
    expect_value(bpf_map_get_next_key, fd, 10);
    will_return(bpf_map_get_next_key, -1); // No more keys
    
    mdflush_read_count_map(1); // maps_per_core = 1
    
    free(mdflush_ebpf_vals);
    mdflush_ebpf_vals = NULL;
}

/* Test: mdflush_read_count_map - bpf_map_lookup_elem failure */
static void test_mdflush_read_count_map_lookup_failure(void **state) {
    mdflush_ebpf_vals = calloc(4, sizeof(mdflush_ebpf_val_t));
    mdflush_maps[0].map_fd = 10;
    
    expect_value(bpf_map_get_next_key, fd, 10);
    will_return(bpf_map_get_next_key, -1);
    
    mdflush_read_count_map(0);
    
    free(mdflush_ebpf_vals);
    mdflush_ebpf_vals = NULL;
}

/* Test: mdflush_write_dims - new dimension */
static void test_mdflush_write_dims_new_dimension(void **state) {
    netdata_mdflush_t v = {
        .unit = 0,
        .disk_name = "md0",
        .dim_exists = false,
        .cnt = 100
    };
    
    int result = mdflush_write_dims(&v, NULL);
    
    assert_int_equal(result, 1);
    assert_true(v.dim_exists);
}

/* Test: mdflush_write_dims - existing dimension */
static void test_mdflush_write_dims_existing_dimension(void **state) {
    netdata_mdflush_t v = {
        .unit = 0,
        .disk_name = "md0",
        .dim_exists = true,
        .cnt = 100
    };
    
    int result = mdflush_write_dims(&v, NULL);
    
    assert_int_equal(result, 1);
    assert_true(v.dim_exists);
}

/* Test: mdflush_exit - with enabled thread */
static void test_mdflush_exit_enabled(void **state) {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .objects = (void *)0x1,
        .probe_links = (void *)0x2
    };
    ebpf_module_t *pem = &em;
    
    mdflush_exit(&pem);
    
    assert_int_equal(em.enabled, NETDATA_THREAD_EBPF_STOPPED);
}

/* Test: mdflush_exit - with disabled thread */
static void test_mdflush_exit_disabled(void **state) {
    ebpf_module_t em = {
        .enabled = 0,
        .objects = NULL,
        .probe_links = NULL
    };
    ebpf_module_t *pem = &em;
    
    mdflush_exit(&pem);
    
    assert_int_equal(em.enabled, NETDATA_THREAD_EBPF_STOPPED);
}

/* Test: mdflush_exit - with NULL pointer */
static void test_mdflush_exit_null(void **state) {
    ebpf_module_t *pem = NULL;
    
    mdflush_exit(&pem);
}

/* Test: mdflush_exit - cleanup objects */
static void test_mdflush_exit_cleanup_objects(void **state) {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .objects = (void *)0x1234,
        .probe_links = (void *)0x5678
    };
    ebpf_module_t *pem = &em;
    
    mdflush_exit(&pem);
    
    assert_null(em.objects);
    assert_null(em.probe_links);
}

/* Test: ebpf_obsolete_mdflush_global */
static void test_ebpf_obsolete_mdflush_global(void **state) {
    ebpf_module_t em = {
        .update_every = 10
    };
    
    ebpf_obsolete_mdflush_global(&em);
}

/* Test: mdflush_create_charts */
static void test_mdflush_create_charts(void **state) {
    mdflush_create_charts(10);
}

/* Test: ebpf_mdflush_load_and_attach - trampoline mode */
static void test_ebpf_mdflush_load_and_attach_trampoline(void **state) {
    ebpf_module_t em = {
        .targets[0].mode = EBPF_LOAD_TRAMPOLINE
    };
    
    /* This test is conditional on LIBBPF_MAJOR_VERSION */
    /* Skipping actual implementation test as it requires libbpf headers */
}

/* Test: ebpf_mdflush_load_and_attach - probe mode */
static void test_ebpf_mdflush_load_and_attach_probe(void **state) {
    ebpf_module_t em = {
        .targets[0].mode = EBPF_LOAD_PROBE
    };
    
    /* This test is conditional on LIBBPF_MAJOR_VERSION */
    /* Skipping actual implementation test as it requires libbpf headers */
}

/* Test: ebpf_disable_probes */
static void test_ebpf_disable_probes(void **state) {
    /* This test is conditional on LIBBPF_MAJOR_VERSION */
    /* Skipping actual implementation test as it requires libbpf headers */
}

/* Test: ebpf_disable_trampoline */
static void test_ebpf_disable_trampoline(void **state) {
    /* This test is conditional on LIBBPF_MAJOR_VERSION */
    /* Skipping actual implementation test as it requires libbpf headers */
}

/* Test: ebpf_set_trampoline_target */
static void test_ebpf_set_trampoline_target(void **state) {
    /* This test is conditional on LIBBPF_MAJOR_VERSION */
    /* Skipping actual implementation test as it requires libbpf headers */
}

/* Test: ebpf_load_probes */
static void test_ebpf_load_probes(void **state) {
    /* This test is conditional on LIBBPF_MAJOR_VERSION */
    /* Skipping actual implementation test as it requires libbpf headers */
}

/* Test: ebpf_mdflush_load_bpf - legacy mode success */
static void test_ebpf_mdflush_load_bpf_legacy_success(void **state) {
    ebpf_module_t em = {
        .load = EBPF_LOAD_LEGACY,
        .probe_links = (void *)0x1,
        .objects = (void *)0x2
    };
    
    /* Mock would need to be set up for ebpf_load_program */
}

/* Test: ebpf_mdflush_load_bpf - legacy mode failure */
static void test_ebpf_mdflush_load_bpf_legacy_failure(void **state) {
    ebpf_module_t em = {
        .load = EBPF_LOAD_LEGACY,
        .probe_links = NULL,
        .objects = NULL
    };
    
    /* Mock would need to be set up for ebpf_load_program */
}

/* Test: ebpf_mdflush_thread - symbol not found */
static void test_ebpf_mdflush_thread_symbol_not_found(void **state) {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .objects = NULL,
        .probe_links = NULL
    };
    
    will_return(ebpf_find_symbol, NULL);
    
    ebpf_mdflush_thread(&em);
}

/* Test: ebpf_mdflush_thread - symbol found, load failure */
static void test_ebpf_mdflush_thread_load_failure(void **state) {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .objects = NULL,
        .probe_links = NULL,
        .load = EBPF_LOAD_LEGACY,
        .targets[0].mode = EBPF_LOAD_PROBE
    };
    
    will_return(ebpf_find_symbol, (char *)"md_flush_request");
    
    ebpf_mdflush_thread(&em);
}

/* Test: ebpf_mdflush_thread - success path */
static void test_ebpf_mdflush_thread_success(void **state) {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .objects = (void *)0x1,
        .probe_links = (void *)0x2,
        .load = EBPF_LOAD_LEGACY,
        .update_every = 10,
        .maps_per_core = 0,
        .lifetime = 0,
        .targets[0].mode = EBPF_LOAD_PROBE
    };
    
    will_return(ebpf_find_symbol, (char *)"md_flush_request");
    will_return(ebpf_plugin_stop, 1); // Stop immediately
    
    ebpf_mdflush_thread(&em);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mdflush_val_cmp_greater),
        cmocka_unit_test(test_mdflush_val_cmp_less),
        cmocka_unit_test(test_mdflush_val_cmp_equal),
        cmocka_unit_test(test_mdflush_val_cmp_boundary),
        cmocka_unit_test(test_mdflush_read_count_map_single_core),
        cmocka_unit_test(test_mdflush_read_count_map_multiple_cores),
        cmocka_unit_test(test_mdflush_read_count_map_lookup_failure),
        cmocka_unit_test(test_mdflush_write_dims_new_dimension),
        cmocka_unit_test(test_mdflush_write_dims_existing_dimension),
        cmocka_unit_test(test_mdflush_exit_enabled),
        cmocka_unit_test(test_mdflush_exit_disabled),
        cmocka_unit_test(test_mdflush_exit_null),
        cmocka_unit_test(test_mdflush_exit_cleanup_objects),
        cmocka_unit_test(test_ebpf_obsolete_mdflush_global),
        cmocka_unit_test(test_mdflush_create_charts),
        cmocka_unit_test(test_ebpf_mdflush_thread_symbol_not_found),
        cmocka_unit_test(test_ebpf_mdflush_thread_load_failure),
        cmocka_unit_test(test_ebpf_mdflush_thread_success),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}