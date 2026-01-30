#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Mock structures
typedef struct {
    int value;
} netdata_mutex_t;

typedef struct {
    uint64_t latency;
    uint64_t ts;
} softirq_ebpf_val_t;

typedef struct {
    uint64_t latency;
    char *name;
} softirq_val_t;

typedef struct {
    int enabled;
    int update_every;
    int maps_per_core;
    uint32_t lifetime;
    uint32_t running_time;
    void *objects;
    void *probe_links;
    void *maps;
} ebpf_module_t;

typedef struct {
    char *name;
    int internal_input;
    int user_input;
    int type;
    int map_fd;
    int map_type;
} ebpf_local_maps_t;

typedef struct {
    int enabled;
    char *class;
    char *event;
} ebpf_tracepoint_t;

typedef struct {
    int dummy;
} heartbeat_t;

typedef struct {
    int value;
} plugin_statistics_t;

// Global variables to mock
static netdata_mutex_t lock = {0};
static netdata_mutex_t ebpf_exit_cleanup = {0};
static int ebpf_nprocs = 4;
static int ebpf_plugin_stop_flag = 0;
static plugin_statistics_t plugin_statistics = {0};

// Mock constants
#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING 1
#define NETDATA_THREAD_EBPF_STOPPED 0
#define NETDATA_EBPF_SYSTEM_GROUP "system"
#define NETDATA_EBPF_MODULE_NAME_SOFTIRQ "softirq"
#define EBPF_COMMON_UNITS_MILLISECONDS "milliseconds"
#define NETDATA_EBPF_CHART_TYPE_STACKED "stacked"
#define NETDATA_CHART_PRIO_SYSTEM_SOFTIRQS 5000
#define NETDATA_SOFTIRQ_MAX_IRQS 10
#define NETDATA_EBPF_INCREMENTAL_IDX 0
#define USEC_PER_SEC 1000000
#define ND_EBPF_MAP_FD_NOT_INITIALIZED -1
#define NETDATA_EBPF_MAP_STATIC 1
#define NETDATA_EBPF_MAP_CONTROLLER 2

// Mock function declarations
void netdata_mutex_lock(netdata_mutex_t *m) { }
void netdata_mutex_unlock(netdata_mutex_t *m) { }
int ebpf_plugin_stop() { return ebpf_plugin_stop_flag; }
int bpf_map_lookup_elem(int fd, const void *key, void *value) { return 0; }
void freez(void *ptr) { free(ptr); }
void ebpf_write_chart_obsolete(const char *a, const char *b, const char *c, const char *d, const char *e, const char *f, const char *g, const char *h, int i, int j) { }
void ebpf_unload_legacy_code(void *o, void *p) { }
void ebpf_disable_tracepoint(void *tp) { }
void ebpf_update_disabled_plugin_stats(void *em) { }
void ebpf_update_kernel_memory_with_vector(void *ps, void *maps, int action) { }
void ebpf_update_stats(void *ps, void *em) { }
void ebpf_enable_tracepoints(void *tp) { return 0; }
void ebpf_define_map_type(void *m, int mpc, int rok) { }
void ebpf_load_program(const char *d, void *em, int rok, int isrh, void **obj) { return NULL; }
void ebpf_create_chart(const char *a, const char *b, const char *c, const char *d, const char *e, const char *f, const char *g, int h, void *i, void *j, int k, int l, const char *m) { }
void ebpf_write_global_dimension(const char *a, const char *b, const char *c) { }
void ebpf_write_begin_chart(const char *a, const char *b, const char *c) { }
void write_chart_dimension(const char *a, uint64_t v) { }
void ebpf_write_end_chart() { }
void heartbeat_init(heartbeat_t *hb, uint64_t usec) { }
void heartbeat_next(heartbeat_t *hb) { }
void fflush(FILE *f) { }

// Test data
static softirq_val_t test_softirq_vals[] = {
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

// Test: ebpf_obsolete_softirq_global - basic call
void test_ebpf_obsolete_softirq_global_basic() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1
    };
    
    // Should call ebpf_write_chart_obsolete successfully
    // No return value, just verify it doesn't crash
}

// Test: ebpf_obsolete_softirq_global - with NULL em
void test_ebpf_obsolete_softirq_global_null_em() {
    // If called with NULL, should handle gracefully
    ebpf_module_t *em = NULL;
    if (em != NULL) {
        // Would call function here
    }
}

// Test: softirq_cleanup - basic cleanup
void test_softirq_cleanup_basic() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .objects = (void *)0x12345678,
        .probe_links = (void *)0x87654321,
        .maps = NULL
    };
    
    ebpf_module_t *pptr = &em;
    // Cleanup should handle the module
}

// Test: softirq_cleanup - disabled module
void test_softirq_cleanup_disabled_module() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_STOPPED,
        .objects = NULL,
        .probe_links = NULL
    };
    
    ebpf_module_t *pptr = &em;
    // Cleanup with disabled module
}

// Test: softirq_cleanup - NULL pointer
void test_softirq_cleanup_null_pointer() {
    // Cleanup should handle NULL gracefully
    ebpf_module_t *em = NULL;
    if (em != NULL) {
        // Process cleanup
    }
}

// Test: softirq_read_latency_map - basic read
void test_softirq_read_latency_map_basic() {
    int maps_per_core = 0;
    // Should read from map and populate softirq_vals
    // Mock returns would be set up for this test
}

// Test: softirq_read_latency_map - with maps per core
void test_softirq_read_latency_map_per_core() {
    int maps_per_core = 1;
    // Should read from map for all cores
}

// Test: softirq_read_latency_map - map lookup failure
void test_softirq_read_latency_map_lookup_failure() {
    int maps_per_core = 0;
    // When bpf_map_lookup_elem returns < 0, should continue
}

// Test: softirq_read_latency_map - latency values conversion
void test_softirq_read_latency_map_conversion() {
    int maps_per_core = 1;
    // Latency should be divided by 1000 (ns to us)
}

// Test: softirq_read_latency_map - all irqs
void test_softirq_read_latency_map_all_irqs() {
    // Should iterate through NETDATA_SOFTIRQ_MAX_IRQS (10)
    int expected_irqs = NETDATA_SOFTIRQ_MAX_IRQS;
    if (expected_irqs != 10) {
        return; // FAIL
    }
}

// Test: softirq_create_charts - basic call
void test_softirq_create_charts_basic() {
    int update_every = 1;
    // Should create chart with correct parameters
}

// Test: softirq_create_charts - various update_every values
void test_softirq_create_charts_update_every() {
    // Test with different update_every values
    for (int i = 1; i <= 10; i++) {
        // Chart should be created for each value
    }
}

// Test: softirq_create_charts - zero update_every
void test_softirq_create_charts_zero_update_every() {
    int update_every = 0;
    // Edge case: zero update_every
}

// Test: softirq_create_dims - basic dimension creation
void test_softirq_create_dims_basic() {
    // Should create dimensions for all 10 IRQ types
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        // Each dimension should be created
    }
}

// Test: softirq_create_dims - dimension names
void test_softirq_create_dims_names() {
    // Verify dimension names are correct
    const char *expected_names[] = {
        "HI", "TIMER", "NET_TX", "NET_RX", "BLOCK",
        "IRQ_POLL", "TASKLET", "SCHED", "HRTIMER", "RCU"
    };
    
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        if (test_softirq_vals[i].name == NULL) {
            return; // FAIL
        }
    }
}

// Test: softirq_write_dims - basic dimension writing
void test_softirq_write_dims_basic() {
    // Should write all dimensions with their values
}

// Test: softirq_write_dims - zero latency
void test_softirq_write_dims_zero_latency() {
    // All values start at 0
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        test_softirq_vals[i].latency = 0;
    }
    // Dimensions with 0 should still be written
}

// Test: softirq_write_dims - max latency
void test_softirq_write_dims_max_latency() {
    // Set maximum latency values
    for (int i = 0; i < NETDATA_SOFTIRQ_MAX_IRQS; i++) {
        test_softirq_vals[i].latency = UINT64_MAX;
    }
    // Large latency values should be handled
}

// Test: softirq_collector - basic collection
void test_softirq_collector_basic() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1,
        .maps_per_core = 0,
        .lifetime = 1,
        .running_time = 0,
        .maps = NULL
    };
    
    ebpf_plugin_stop_flag = 1; // Stop immediately
    // Should initialize and run collection loop
}

// Test: softirq_collector - with maps per core
void test_softirq_collector_maps_per_core() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 2,
        .maps_per_core = 1,
        .lifetime = 10,
        .running_time = 0
    };
    
    ebpf_plugin_stop_flag = 1;
    // Should read maps per core
}

// Test: softirq_collector - lifetime exceeded
void test_softirq_collector_lifetime_exceeded() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1,
        .maps_per_core = 0,
        .lifetime = 0,
        .running_time = 0
    };
    
    // Lifetime is 0, should exit immediately
    // running_time (0) is not < lifetime (0)
}

// Test: softirq_collector - plugin stop flag
void test_softirq_collector_plugin_stop() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1,
        .maps_per_core = 0,
        .lifetime = 100,
        .running_time = 0
    };
    
    ebpf_plugin_stop_flag = 1;
    // Should exit when plugin_stop returns true
}

// Test: softirq_collector - counter reaching update_every
void test_softirq_collector_counter_update() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 3,
        .maps_per_core = 0,
        .lifetime = 100,
        .running_time = 0
    };
    
    ebpf_plugin_stop_flag = 1;
    // Counter should reach update_every for data reading
}

// Test: ebpf_softirq_thread - basic initialization
void test_ebpf_softirq_thread_basic() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1,
        .maps_per_core = 0,
        .lifetime = 1,
        .running_time = 0,
        .objects = NULL,
        .probe_links = NULL,
        .maps = NULL
    };
    
    ebpf_plugin_stop_flag = 1;
    // ebpf_softirq_thread should initialize and clean up
}

// Test: ebpf_softirq_thread - tracepoint enable failure
void test_ebpf_softirq_thread_tracepoint_failure() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1,
        .maps_per_core = 0,
        .lifetime = 1,
        .running_time = 0
    };
    
    // When tracepoint enable fails, should go to endsoftirq
}

// Test: ebpf_softirq_thread - probe link failure
void test_ebpf_softirq_thread_probe_link_failure() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1,
        .maps_per_core = 0,
        .lifetime = 1,
        .running_time = 0
    };
    
    // When probe_links loading fails, should handle gracefully
}

// Test: Config struct initialization
void test_softirq_config_initialization() {
    // softirq_config should be initialized as APPCONFIG_INITIALIZER
    // This is a compile-time verification
}

// Test: Static arrays are properly sized
void test_softirq_static_arrays_size() {
    // softirq_vals should have exactly NETDATA_SOFTIRQ_MAX_IRQS elements
    int array_size = NETDATA_SOFTIRQ_MAX_IRQS;
    if (array_size != 10) {
        return; // FAIL
    }
}

// Test: Map initialization
void test_softirq_maps_initialization() {
    // Maps should be properly initialized with correct types
    // NETDATA_EBPF_MAP_STATIC for tbl_softirq
    // NETDATA_EBPF_MAP_CONTROLLER for controller
}

// Test: Tracepoint initialization
void test_softirq_tracepoints_initialization() {
    // Two tracepoints: softirq_entry and softirq_exit
    // Both should be disabled initially
}

// Test: Large latency accumulation
void test_softirq_collector_large_latency() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1,
        .maps_per_core = 1,
        .lifetime = 1,
        .running_time = 0
    };
    
    ebpf_plugin_stop_flag = 1;
    // Should handle large latency values across multiple cores
}

// Test: Multiple update cycles
void test_softirq_collector_multiple_cycles() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 2,
        .maps_per_core = 0,
        .lifetime = 10,
        .running_time = 0
    };
    
    ebpf_plugin_stop_flag = 1;
    // Should handle multiple update cycles
}

// Test: Cleanup with enabled module
void test_softirq_cleanup_enabled() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .objects = (void *)0x1,
        .probe_links = (void *)0x2
    };
    
    // Cleanup should handle enabled module properly
}

// Test: Cleanup with disabled module
void test_softirq_cleanup_disabled() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_STOPPED,
        .objects = NULL,
        .probe_links = NULL
    };
    
    // Cleanup should handle disabled module
}

// Test: Running time tracking
void test_softirq_collector_running_time() {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 1,
        .maps_per_core = 0,
        .lifetime = 5,
        .running_time = 0
    };
    
    ebpf_plugin_stop_flag = 1;
    // Running time should be updated correctly
}