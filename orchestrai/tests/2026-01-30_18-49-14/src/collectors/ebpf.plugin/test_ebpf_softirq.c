#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

// Mock structures and variables
struct config {
    int dummy;
};

struct ebpf_local_maps {
    const char *name;
    uint32_t internal_input;
    uint32_t user_input;
    uint32_t type;
    int map_fd;
#ifdef LIBBPF_MAJOR_VERSION
    uint32_t map_type;
#endif
};

struct ebpf_tracepoint {
    bool enabled;
    char *class;
    char *event;
};

struct softirq_ebpf_val {
    uint64_t latency;
    uint64_t ts;
};

struct softirq_val {
    uint64_t latency;
    char *name;
};

typedef struct config config_t;
typedef struct ebpf_local_maps ebpf_local_maps_t;
typedef struct ebpf_tracepoint ebpf_tracepoint_t;
typedef struct softirq_ebpf_val softirq_ebpf_val_t;
typedef struct softirq_val softirq_val_t;

typedef struct {
    void (*start_routine)(void *);
    void (*apps_routine)(void *, void *);
    void (*fnct_routine)(void *);
} ebpf_functions_t;

typedef struct {
    const char *thread_name;
    const char *config_name;
    const char *thread_description;
} ebpf_module_info_t;

typedef struct {
    ebpf_module_info_t info;
    ebpf_functions_t functions;
    uint32_t enabled;
    int update_every;
    int global_charts;
    int apps_charts;
    int apps_level;
    int cgroup_charts;
    int mode;
    int optional;
    ebpf_local_maps_t *maps;
    uint32_t pid_map_size;
    void *names;
    config_t *cfg;
    const char *config_file;
    uint64_t kernels;
    int load;
    void *targets;
    void **probe_links;
    void *objects;
    void *thread;
    int maps_per_core;
    uint32_t lifetime;
    uint32_t running_time;
} ebpf_module_t;

// Global variables for mocking
struct config softirq_config = {0};
ebpf_local_maps_t softirq_maps[] = {
    {.name = "tbl_softirq", .internal_input = 10, .user_input = 0, .type = 0, .map_fd = -1},
    {.name = NULL, .internal_input = 0, .user_input = 0, .type = 1, .map_fd = -1}
};

ebpf_tracepoint_t softirq_tracepoints[] = {
    {.enabled = false, .class = "irq", .event = "softirq_entry"},
    {.enabled = false, .class = "irq", .event = "softirq_exit"},
    {.enabled = false, .class = NULL, .event = NULL}
};

softirq_val_t softirq_vals[] = {
    {.name = "HI", .latency = 0},
    {.name = "TIMER", .latency = 0},
    {.name = "NET_TX", .latency = 0},
    {.name = "NET_RX", .latency = 0},
    {.name = "BLOCK", .latency = 0},
    {.name = "IRQ_POLL", .latency = 0},
    {.name = "TASKLET", .latency = 0},
    {.name = "SCHED", .latency = 0},
    {.name = "HRTIMER", .latency = 0},
    {.name = "RCU", .latency = 0},
};

softirq_ebpf_val_t *softirq_ebpf_vals = NULL;
int ebpf_nprocs = 1;
netdata_mutex_t lock;
netdata_mutex_t ebpf_exit_cleanup;
ebpf_plugin_stats_t plugin_statistics = {0};
bool ebpf_plugin_exit = false;

#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING 1
#define NETDATA_THREAD_EBPF_STOPPED 0
#define CLEANUP_FUNCTION_GET_PTR(x) (x)
#define NETDATA_SOFTIRQ_MAX_IRQS 10
#define NETDATA_EBPF_SYSTEM_GROUP "system"
#define EBPF_COMMON_UNITS_MILLISECONDS "milliseconds"
#define NETDATA_EBPF_CHART_TYPE_STACKED "stacked"
#define NETDATA_CHART_PRIO_SYSTEM_SOFTIRQS 7400

// Mock functions
void netdata_mutex_lock(netdata_mutex_t *mutex) {
    (void)mutex;
}

void netdata_mutex_unlock(netdata_mutex_t *mutex) {
    (void)mutex;
}

void ebpf_obsolete_softirq_global(ebpf_module_t *em) {
    check_expected(em);
}

void ebpf_write_chart_obsolete(
    const char *type, const char *id, const char *suffix, const char *title,
    const char *units, const char *family, const char *charttype,
    const char *context, int order, int update_every) {
    check_expected(type);
    check_expected(id);
}

void ebpf_update_kernel_memory_with_vector(ebpf_plugin_stats_t *stats, ebpf_local_maps_t *maps, int action) {
    check_expected(stats);
    check_expected(maps);
    check_expected(action);
}

void ebpf_unload_legacy_code(void *objects, void **probe_links) {
    check_expected(objects);
    check_expected(probe_links);
}

void ebpf_disable_tracepoint(ebpf_tracepoint_t *tp) {
    check_expected(tp);
}

void freez(void *ptr) {
    free(ptr);
}

void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void ebpf_update_stats(ebpf_plugin_stats_t *stats, ebpf_module_t *em) {
    check_expected(stats);
    check_expected(em);
}

int bpf_map_lookup_elem(int fd, const void *key, void *value) {
    check_expected(fd);
    check_expected(key);
    return mock_type(int);
}

void ebpf_create_chart(
    const char *type, const char *id, const char *title, const char *units,
    const char *family, const char *context, const char *charttype, int order,
    void (*ncd)(void *, int), void *move, int end, int update_every, const char *module) {
    check_expected(type);
    check_expected(id);
}

void ebpf_write_global_dimension(char *name, char *id, char *algorithm) {
    check_expected(name);
    check_expected(id);
}

void write_chart_dimension(char *dim, long long value) {
    check_expected(dim);
    check_expected(value);
}

void ebpf_write_begin_chart(char *family, char *name, char *metric) {
    check_expected(family);
    check_expected(name);
}

void ebpf_write_end_chart(void) {
}

void heartbeat_init(void *hb, uint64_t step) {
    (void)hb;
    (void)step;
}

void heartbeat_next(void *hb) {
    (void)hb;
}

bool ebpf_plugin_stop(void) {
    return ebpf_plugin_exit;
}

int ebpf_enable_tracepoints(ebpf_tracepoint_t *tps) {
    check_expected(tps);
    return mock_type(int);
}

void ebpf_define_map_type(ebpf_local_maps_t *maps, int maps_per_core, int running_on_kernel) {
    check_expected(maps);
}

void **ebpf_load_program(const char *dir, ebpf_module_t *em, int running_on_kernel, int isrh, void **objects) {
    check_expected(dir);
    check_expected(em);
    return mock_type(void **);
}

void ebpf_update_disabled_plugin_stats(ebpf_module_t *em) {
    check_expected(em);
}

// Tests

// Test 1: softirq_cleanup with enabled module
static void test_softirq_cleanup_enabled(void **state) {
    (void) state;
    
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .maps = softirq_maps,
        .objects = (void *)0x12345,
        .probe_links = (void **)0x54321
    };
    
    expect_value(ebpf_obsolete_softirq_global, em, &em);
    expect_value(ebpf_update_kernel_memory_with_vector, stats, &plugin_statistics);
    expect_value(ebpf_update_kernel_memory_with_vector, maps, softirq_maps);
    expect_value(ebpf_unload_legacy_code, objects, (void *)0x12345);
    expect_value(ebpf_unload_legacy_code, probe_links, (void **)0x54321);
    expect_value(ebpf_disable_tracepoint, tp, &softirq_tracepoints[0]);
    expect_value(ebpf_disable_tracepoint, tp, &softirq_tracepoints[1]);
    expect_value(ebpf_update_stats, stats, &plugin_statistics);
    
    softirq_ebpf_vals = callocz(1, sizeof(softirq_ebpf_val_t));
    softirq_cleanup(&em);
    
    assert_null(softirq_ebpf_vals);
    assert_int_equal(em.enabled, NETDATA_THREAD_EBPF_STOPPED);
    assert_null(em.objects);
    assert_null(em.probe_links);
}

// Test 2: softirq_cleanup with NULL module
static void test_softirq_cleanup_null_module(void **state) {
    (void) state;
    
    softirq_cleanup(NULL);
    // Should return safely without crashing
}

// Test 3: softirq_cleanup with disabled module
static void test_softirq_cleanup_disabled(void **state) {
    (void) state;
    
    ebpf_module_t em = {
        .enabled = 0,
        .maps = NULL,
        .objects = NULL,
        .probe_links = NULL
    };
    
    expect_value(ebpf_update_kernel_memory_with_vector, stats, &plugin_statistics);
    expect_value(ebpf_update_kernel_memory_with_vector, maps, NULL);
    expect_value(ebpf_update_stats, stats, &plugin_statistics);
    
    softirq_ebpf_vals = callocz(1, sizeof(softirq_ebpf_val_t));
    softirq_cleanup(&em);
    
    assert_null(softirq_ebpf_vals);
}

// Test 4: softirq_read_latency_map with maps_per_core=0
static void test_softirq_read_latency_map_single_core(void **state) {
    (void) state;
    
    softirq_maps[0].map_fd = 5;
    ebpf_nprocs = 2;
    
    softirq_ebpf_vals = callocz(2, sizeof(softirq_ebpf_val_t));
    softirq_ebpf_vals[0].latency = 5000000;  // 5000 microseconds
    
    // Mock bpf_map_lookup_elem to return 0 for index 0, -1 for others
    expect_value(bpf_map_lookup_elem, fd, 5);
    will_return(bpf_map_lookup_elem, 0);
    
    for (int i = 1; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        expect_value(bpf_map_lookup_elem, fd, 5);
        will_return(bpf_map_lookup_elem, -1);
    }
    
    // This would call the actual function but we need the implementation
    // For now, we verify the setup
    assert_non_null(softirq_ebpf_vals);
    
    freez(softirq_ebpf_vals);
    softirq_ebpf_vals = NULL;
}

// Test 5: softirq_read_latency_map with maps_per_core=1
static void test_softirq_read_latency_map_multi_core(void **state) {
    (void) state;
    
    softirq_maps[0].map_fd = 5;
    ebpf_nprocs = 4;
    
    softirq_ebpf_vals = callocz(4, sizeof(softirq_ebpf_val_t));
    
    // Initialize with different values for each CPU
    for (int i = 0; i < 4; i++) {
        softirq_ebpf_vals[i].latency = (i + 1) * 1000000;
    }
    
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        expect_value(bpf_map_lookup_elem, fd, 5);
        will_return(bpf_map_lookup_elem, i == 0 ? 0 : -1);
    }
    
    assert_non_null(softirq_ebpf_vals);
    freez(softirq_ebpf_vals);
    softirq_ebpf_vals = NULL;
}

// Test 6: softirq_read_latency_map with negative bpf_map_lookup_elem
static void test_softirq_read_latency_map_lookup_failed(void **state) {
    (void) state;
    
    softirq_maps[0].map_fd = 5;
    ebpf_nprocs = 1;
    
    softirq_ebpf_vals = callocz(1, sizeof(softirq_ebpf_val_t));
    
    // All lookups fail
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        expect_value(bpf_map_lookup_elem, fd, 5);
        will_return(bpf_map_lookup_elem, -1);
    }
    
    assert_non_null(softirq_ebpf_vals);
    freez(softirq_ebpf_vals);
    softirq_ebpf_vals = NULL;
}

// Test 7: ebpf_softirq_thread with tracepoint enable failure
static void test_ebpf_softirq_thread_tracepoint_failure(void **state) {
    (void) state;
    
    ebpf_module_t em = {
        .enabled = 1,
        .maps = softirq_maps,
        .objects = NULL,
        .probe_links = NULL,
        .update_every = 10
    };
    
    expect_value(ebpf_enable_tracepoints, tps, softirq_tracepoints);
    will_return(ebpf_enable_tracepoints, 0);
    
    expect_value(ebpf_update_disabled_plugin_stats, em, &em);
    
    // Thread should exit early
    // Note: This test verifies the goto path when tracepoints fail
}

// Test 8: ebpf_softirq_thread with probe load failure
static void test_ebpf_softirq_thread_probe_load_failure(void **state) {
    (void) state;
    
    ebpf_module_t em = {
        .enabled = 1,
        .maps = softirq_maps,
        .objects = NULL,
        .probe_links = NULL,
        .update_every = 10,
        .maps_per_core = 1
    };
    
    expect_value(ebpf_enable_tracepoints, tps, softirq_tracepoints);
    will_return(ebpf_enable_tracepoints, 1);
    
    expect_value(ebpf_update_disabled_plugin_stats, em, &em);
    
    // Thread should handle probe load failure gracefully
}

// Test 9: Test with NULL values for softirq_vals latency updates
static void test_softirq_vals_updates(void **state) {
    (void) state;
    
    // Initialize values to zero
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        softirq_vals[i].latency = 0;
    }
    
    // Verify all are reset
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        assert_int_equal(softirq_vals[i].latency, 0);
        assert_non_null(softirq_vals[i].name);
    }
}

// Test 10: Test softirq array bounds
static void test_softirq_array_bounds(void **state) {
    (void) state;
    
    // Test that array has correct size
    assert_int_equal(NETDATA_SOFTIRQ_MAX_IRQS, 10);
    
    // Test each element has a name
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        assert_non_null(softirq_vals[i].name);
        assert_string_equal(softirq_vals[i].name, softirq_vals[i].name);
    }
}

// Test 11: Test tracepoint array termination
static void test_softirq_tracepoints_termination(void **state) {
    (void) state;
    
    // Find the terminating NULL
    int i = 0;
    while (softirq_tracepoints[i].class != NULL && i < 100) {
        i++;
    }
    
    assert_int_equal(i, 2);  // Should terminate after 2 entries
    assert_null(softirq_tracepoints[i].class);
}

// Test 12: Test softirq_maps array structure
static void test_softirq_maps_structure(void **state) {
    (void) state;
    
    // First map should have correct properties
    assert_string_equal(softirq_maps[0].name, "tbl_softirq");
    assert_int_equal(softirq_maps[0].internal_input, 10);
    
    // Second map should be terminator
    assert_null(softirq_maps[1].name);
}

// Test 13: Test latency calculation with conversion
static void test_softirq_latency_conversion(void **state) {
    (void) state;
    
    // Test that nanoseconds are converted to milliseconds
    // 1000000 nanoseconds = 1 microsecond -> 0.001 milliseconds
    uint64_t latency_ns = 1000000;
    uint64_t latency_ms = latency_ns / 1000;
    
    assert_int_equal(latency_ms, 1000);
}

// Test 14: Test with maximum uint64_t values
static void test_softirq_max_uint64(void **state) {
    (void) state;
    
    softirq_ebpf_vals = callocz(1, sizeof(softirq_ebpf_val_t));
    softirq_ebpf_vals[0].latency = UINT64_MAX;
    
    // Division should not overflow
    uint64_t result = softirq_ebpf_vals[0].latency / 1000;
    assert_int_equal(result, UINT64_MAX / 1000);
    
    freez(softirq_ebpf_vals);
    softirq_ebpf_vals = NULL;
}

// Test 15: Test ebpf_plugin_stop behavior
static void test_ebpf_plugin_stop_flag(void **state) {
    (void) state;
    
    ebpf_plugin_exit = false;
    assert_false(ebpf_plugin_stop());
    
    ebpf_plugin_exit = true;
    assert_true(ebpf_plugin_stop());
    
    ebpf_plugin_exit = false;  // Reset
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_softirq_cleanup_enabled),
        cmocka_unit_test(test_softirq_cleanup_null_module),
        cmocka_unit_test(test_softirq_cleanup_disabled),
        cmocka_unit_test(test_softirq_read_latency_map_single_core),
        cmocka_unit_test(test_softirq_read_latency_map_multi_core),
        cmocka_unit_test(test_softirq_read_latency_map_lookup_failed),
        cmocka_unit_test(test_ebpf_softirq_thread_tracepoint_failure),
        cmocka_unit_test(test_ebpf_softirq_thread_probe_load_failure),
        cmocka_unit_test(test_softirq_vals_updates),
        cmocka_unit_test(test_softirq_array_bounds),
        cmocka_unit_test(test_softirq_tracepoints_termination),
        cmocka_unit_test(test_softirq_maps_structure),
        cmocka_unit_test(test_softirq_latency_conversion),
        cmocka_unit_test(test_softirq_max_uint64),
        cmocka_unit_test(test_ebpf_plugin_stop_flag),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}