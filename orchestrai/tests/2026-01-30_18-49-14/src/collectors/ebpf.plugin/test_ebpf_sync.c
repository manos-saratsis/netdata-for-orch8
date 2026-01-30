#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Mock defines to prevent compilation issues */
#define NETDATA_EBPF_MODULE_NAME_SYNC "sync"
#define NETDATA_SYNC_CONFIG_FILE "sync.conf"
#define NETDATA_SYNC_CONFIG_NAME "syscalls"

#define NETDATA_EBPF_SYNC_SLEEP_MS 800000ULL
#define NETDATA_EBPF_SYNC_CHART "sync"
#define NETDATA_EBPF_MSYNC_CHART "memory_map"
#define NETDATA_EBPF_FILE_SYNC_CHART "file_sync"
#define NETDATA_EBPF_FILE_SEGMENT_CHART "file_segment"
#define NETDATA_EBPF_SYNC_SUBMENU "synchronization (eBPF)"

#define NETDATA_SYSCALLS_SYNC "sync"
#define NETDATA_SYSCALLS_SYNCFS "syncfs"
#define NETDATA_SYSCALLS_MSYNC "msync"
#define NETDATA_SYSCALLS_FSYNC "fsync"
#define NETDATA_SYSCALLS_FDATASYNC "fdatasync"
#define NETDATA_SYSCALLS_SYNC_FILE_RANGE "sync_file_range"

#define NETDATA_EBPF_MEMORY_GROUP "mem"
#define NETDATA_EBPF_CHART_TYPE_LINE "line"
#define EBPF_COMMON_UNITS_CALLS_PER_SEC "calls/s"
#define EBPF_COMMON_UNITS_CALL "calls"

#define NETDATA_EBPF_MAX_SYSCALL_LENGTH 256
#define NETDATA_MAX_PROCESSOR 256
#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING 1
#define NETDATA_THREAD_EBPF_STOPPED 0
#define NETDATA_EBPF_INCREMENTAL_IDX 2

#define ND_EBPF_MAP_FD_NOT_INITIALIZED -1
#define NETDATA_EBPF_MAP_STATIC 1
#define NETDATA_EBPF_MAP_CONTROLLER 2
#define BPF_MAP_TYPE_PERCPU_ARRAY 3

#define EBPF_LOAD_LEGACY 1
#define EBPF_LOAD_TRAMPOLINE 1
#define EBPF_LOAD_PROBE 2
#define EBPF_LOAD_RETPROBE 3

#define USEC_PER_SEC 1000000
#define CONFIG_BOOLEAN_YES 1

#define LIBBPF_MAJOR_VERSION 1

#define CLEANUP_FUNCTION_REGISTER(x)
#define CLEANUP_FUNCTION_GET_PTR(x) (x)

/* Type definitions */
typedef int netdata_ebpf_program_loaded_t;
typedef unsigned long netdata_idx_t;
typedef int netdata_syscall_stat_t;
typedef int netdata_publish_syscall_t;
typedef int ebpf_local_maps_t;
typedef int netdata_ebpf_targets_t;
typedef int ebpf_module_t;
typedef int heartbeat_t;
typedef int netdata_mutex_t;

typedef enum sync_syscalls_index {
    NETDATA_SYNC_SYNC_IDX,
    NETDATA_SYNC_SYNCFS_IDX,
    NETDATA_SYNC_MSYNC_IDX,
    NETDATA_SYNC_FSYNC_IDX,
    NETDATA_SYNC_FDATASYNC_IDX,
    NETDATA_SYNC_SYNC_FILE_RANGE_IDX,
    NETDATA_SYNC_IDX_END
} sync_syscalls_index_t;

enum netdata_sync_charts {
    NETDATA_SYNC_CALL,
    NETDATA_SYNC_END
};

typedef struct {
    char *syscall;
    int enabled;
    ebpf_local_maps_t *sync_maps;
    void *sync_obj;
    void *objects;
    void *probe_links;
} ebpf_sync_syscalls_t;

typedef struct {
    int thread_name;
    int x;
} ebpf_info_t;

typedef struct {
    int enabled;
    ebpf_info_t info;
    int update_every;
    int maps_per_core;
    int lifetime;
    unsigned int running_time;
    int load;
    ebpf_local_maps_t *maps;
    netdata_ebpf_targets_t *targets;
} ebpf_module_mock_t;

typedef struct {
    char *name;
    char *dimension;
    int algorithm;
    void *next;
} netdata_publish_syscall_mock_t;

/* Global variables */
static netdata_idx_t sync_hash_values[6];
static netdata_syscall_stat_t sync_counter_aggregated_data[6];
static netdata_publish_syscall_t sync_counter_publish_aggregated[6];
static char *sync_counter_dimension_name[6] = {"sync", "syncfs", "msync", "fsync", "fdatasync", "sync_file_range"};

ebpf_local_maps_t sync_maps[] = {};
ebpf_local_maps_t syncfs_maps[] = {};
ebpf_local_maps_t msync_maps[] = {};
ebpf_local_maps_t fsync_maps[] = {};
ebpf_local_maps_t fdatasync_maps[] = {};
ebpf_local_maps_t sync_file_range_maps[] = {};

ebpf_sync_syscalls_t local_syscalls[7] = {
    {.syscall = "sync", .enabled = 0, .sync_maps = NULL, .sync_obj = NULL, .objects = NULL, .probe_links = NULL},
    {.syscall = "syncfs", .enabled = 0, .sync_maps = NULL, .sync_obj = NULL, .objects = NULL, .probe_links = NULL},
    {.syscall = "msync", .enabled = 0, .sync_maps = NULL, .sync_obj = NULL, .objects = NULL, .probe_links = NULL},
    {.syscall = "fsync", .enabled = 0, .sync_maps = NULL, .sync_obj = NULL, .objects = NULL, .probe_links = NULL},
    {.syscall = "fdatasync", .enabled = 0, .sync_maps = NULL, .sync_obj = NULL, .objects = NULL, .probe_links = NULL},
    {.syscall = "sync_file_range", .enabled = 0, .sync_maps = NULL, .sync_obj = NULL, .objects = NULL, .probe_links = NULL},
    {.syscall = NULL, .enabled = 0, .sync_maps = NULL, .sync_obj = NULL, .objects = NULL, .probe_links = NULL}
};

int ebpf_nprocs = 1;
int running_on_kernel = 1;
int isrh = 0;
int default_btf = 0;
char *ebpf_plugin_dir = "/etc/netdata";

netdata_mutex_t lock = 0;
netdata_mutex_t ebpf_exit_cleanup = 0;

struct config sync_config;
netdata_ebpf_targets_t sync_targets[7];

/* Mock functions */
void netdata_mutex_lock(netdata_mutex_t *m) { (void)m; }
void netdata_mutex_unlock(netdata_mutex_t *m) { (void)m; }
int ebpf_plugin_stop() { return 0; }

void ebpf_write_chart_cmd(const char *group, const char *id, const char *family, 
                         const char *title, const char *units, const char *submenu,
                         const char *type, const char *context, int order, int update_every, 
                         const char *name) {
    (void)group; (void)id; (void)family; (void)title; (void)units; (void)submenu;
    (void)type; (void)context; (void)order; (void)update_every; (void)name;
}

void ebpf_write_chart_obsolete(const char *group, const char *id, const char *family,
                               const char *title, const char *units, const char *submenu,
                               const char *type, const char *context, int order, int update_every) {
    (void)group; (void)id; (void)family; (void)title; (void)units; (void)submenu;
    (void)type; (void)context; (void)order; (void)update_every;
}

void ebpf_write_begin_chart(const char *group, const char *id, const char *family) {
    (void)group; (void)id; (void)family;
}

void ebpf_write_end_chart() {}

void write_chart_dimension(const char *name, long long value) {
    (void)name; (void)value;
}

void ebpf_one_dimension_write_charts(const char *group, const char *id, const char *dim, long long value) {
    (void)group; (void)id; (void)dim; (void)value;
}

void ebpf_write_global_dimension(const char *name, const char *dim, int algorithm) {
    (void)name; (void)dim; (void)algorithm;
}

void ebpf_global_labels(netdata_syscall_stat_t *agg, netdata_publish_syscall_t *pub, 
                       char **name, char **dimension, int *algorithms, int count) {
    (void)agg; (void)pub; (void)name; (void)dimension; (void)algorithms; (void)count;
}

int ebpf_load_program(const char *dir, ebpf_module_t *em, int kernel, int rh, void **objs) {
    (void)dir; (void)em; (void)kernel; (void)rh; (void)objs;
    return 0;
}

void ebpf_unload_legacy_code(void *objs, void *links) {
    (void)objs; (void)links;
}

void ebpf_update_stats(void *stats, ebpf_module_t *em) {
    (void)stats; (void)em;
}

void ebpf_update_disabled_plugin_stats(ebpf_module_t *em) {
    (void)em;
}

void heartbeat_init(heartbeat_t *hb, uint64_t usec) {
    (void)hb; (void)usec;
}

void heartbeat_next(heartbeat_t *hb) {
    (void)hb;
}

int inicfg_get_boolean(struct config *cfg, const char *section, const char *name, int def) {
    (void)cfg; (void)section; (void)name;
    return def;
}

void ebpf_define_map_type(ebpf_local_maps_t *maps, int per_core, int kernel) {
    (void)maps; (void)per_core; (void)kernel;
}

void ebpf_select_host_prefix(char *dst, int len, const char *src, int kernel) {
    (void)len; (void)kernel;
    if (dst && src) {
        strncpy(dst, src, len - 1);
        dst[len - 1] = '\0';
    }
}

int ebpf_is_function_inside_btf(int btf, const char *name) {
    (void)btf; (void)name;
    return 1;
}

void ebpf_adjust_thread_load(ebpf_module_t *em, int btf) {
    (void)em; (void)btf;
}

int bpf_map_lookup_elem(int fd, const void *key, void *value) {
    (void)fd; (void)key;
    if (value)
        memset(value, 0, sizeof(netdata_idx_t) * NETDATA_MAX_PROCESSOR);
    return 0;
}

void ebpf_update_map_type(void *map, ebpf_local_maps_t *local_map) {
    (void)map; (void)local_map;
}

int plugin_statistics = 0;

/* Test functions */

/**
 * Test: ebpf_sync_cleanup_objects - no syscalls enabled
 */
static void test_ebpf_sync_cleanup_objects_no_enabled(void **state)
{
    (void)state;
    
    /* Setup: all syscalls disabled */
    for (int i = 0; local_syscalls[i].syscall; i++) {
        local_syscalls[i].enabled = 0;
        local_syscalls[i].sync_obj = NULL;
        local_syscalls[i].objects = NULL;
        local_syscalls[i].probe_links = NULL;
    }
    
    /* This should complete without errors */
    extern void ebpf_sync_cleanup_objects();
    ebpf_sync_cleanup_objects();
    
    assert_true(1);
}

/**
 * Test: ebpf_sync_cleanup_objects - with libbpf objects
 */
static void test_ebpf_sync_cleanup_objects_with_libbpf(void **state)
{
    (void)state;
    
    /* Setup: some syscalls with objects */
    for (int i = 0; local_syscalls[i].syscall; i++) {
        if (i < 2) {
            local_syscalls[i].enabled = 1;
            local_syscalls[i].sync_obj = (void *)1; /* Non-null pointer */
        } else {
            local_syscalls[i].enabled = 0;
            local_syscalls[i].sync_obj = NULL;
        }
        local_syscalls[i].objects = NULL;
        local_syscalls[i].probe_links = NULL;
    }
    
    extern void ebpf_sync_cleanup_objects();
    ebpf_sync_cleanup_objects();
    
    assert_true(1);
}

/**
 * Test: ebpf_sync_cleanup_objects - with legacy probe links
 */
static void test_ebpf_sync_cleanup_objects_with_legacy(void **state)
{
    (void)state;
    
    /* Setup: syscalls with legacy probe links */
    for (int i = 0; local_syscalls[i].syscall; i++) {
        local_syscalls[i].enabled = 1;
        local_syscalls[i].sync_obj = NULL;
        if (i == 0 || i == 2) {
            local_syscalls[i].objects = (void *)1;
            local_syscalls[i].probe_links = (void *)1;
        } else {
            local_syscalls[i].objects = NULL;
            local_syscalls[i].probe_links = NULL;
        }
    }
    
    extern void ebpf_sync_cleanup_objects();
    ebpf_sync_cleanup_objects();
    
    assert_true(1);
}

/**
 * Test: ebpf_obsolete_sync_global - both fsync and fdatasync enabled
 */
static void test_ebpf_obsolete_sync_global_file_sync(void **state)
{
    (void)state;
    
    ebpf_module_mock_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 10
    };
    
    local_syscalls[NETDATA_SYNC_FSYNC_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_MSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNCFS_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].enabled = 0;
    
    extern void ebpf_obsolete_sync_global(ebpf_module_t *em);
    ebpf_obsolete_sync_global((ebpf_module_t *)&em);
    
    assert_true(1);
}

/**
 * Test: ebpf_obsolete_sync_global - msync enabled
 */
static void test_ebpf_obsolete_sync_global_msync(void **state)
{
    (void)state;
    
    ebpf_module_mock_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 10
    };
    
    local_syscalls[NETDATA_SYNC_FSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_MSYNC_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_SYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNCFS_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].enabled = 0;
    
    extern void ebpf_obsolete_sync_global(ebpf_module_t *em);
    ebpf_obsolete_sync_global((ebpf_module_t *)&em);
    
    assert_true(1);
}

/**
 * Test: ebpf_obsolete_sync_global - sync and syncfs enabled
 */
static void test_ebpf_obsolete_sync_global_sync(void **state)
{
    (void)state;
    
    ebpf_module_mock_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 10
    };
    
    local_syscalls[NETDATA_SYNC_FSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_MSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_SYNCFS_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].enabled = 0;
    
    extern void ebpf_obsolete_sync_global(ebpf_module_t *em);
    ebpf_obsolete_sync_global((ebpf_module_t *)&em);
    
    assert_true(1);
}

/**
 * Test: ebpf_obsolete_sync_global - sync_file_range enabled
 */
static void test_ebpf_obsolete_sync_global_sync_file_range(void **state)
{
    (void)state;
    
    ebpf_module_mock_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .update_every = 10
    };
    
    local_syscalls[NETDATA_SYNC_FSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_MSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNCFS_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].enabled = 1;
    
    extern void ebpf_obsolete_sync_global(ebpf_module_t *em);
    ebpf_obsolete_sync_global((ebpf_module_t *)&em);
    
    assert_true(1);
}

/**
 * Test: ebpf_sync_exit - with running module
 */
static void test_ebpf_sync_exit_running(void **state)
{
    (void)state;
    
    ebpf_module_mock_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING
    };
    
    void *ptr = &em;
    
    extern void ebpf_sync_exit(void *pptr);
    ebpf_sync_exit(&ptr);
    
    /* Verify module state changed */
    assert_int_equal(em.enabled, NETDATA_THREAD_EBPF_STOPPED);
}

/**
 * Test: ebpf_sync_exit - with null pointer
 */
static void test_ebpf_sync_exit_null(void **state)
{
    (void)state;
    
    void *ptr = NULL;
    
    extern void ebpf_sync_exit(void *pptr);
    ebpf_sync_exit(&ptr);
    
    assert_true(1);
}

/**
 * Test: ebpf_sync_read_global_table - maps per core true
 */
static void test_ebpf_sync_read_global_table_per_core(void **state)
{
    (void)state;
    
    for (int i = 0; local_syscalls[i].syscall; i++) {
        local_syscalls[i].enabled = 1;
        local_syscalls[i].sync_maps = (ebpf_local_maps_t *)malloc(sizeof(ebpf_local_maps_t));
    }
    
    extern void ebpf_sync_read_global_table(int maps_per_core);
    ebpf_sync_read_global_table(1);
    
    for (int i = 0; local_syscalls[i].syscall; i++) {
        if (local_syscalls[i].sync_maps)
            free(local_syscalls[i].sync_maps);
    }
    
    assert_true(1);
}

/**
 * Test: ebpf_sync_read_global_table - maps per core false
 */
static void test_ebpf_sync_read_global_table_single_core(void **state)
{
    (void)state;
    
    for (int i = 0; local_syscalls[i].syscall; i++) {
        local_syscalls[i].enabled = 1;
        local_syscalls[i].sync_maps = (ebpf_local_maps_t *)malloc(sizeof(ebpf_local_maps_t));
    }
    
    extern void ebpf_sync_read_global_table(int maps_per_core);
    ebpf_sync_read_global_table(0);
    
    for (int i = 0; local_syscalls[i].syscall; i++) {
        if (local_syscalls[i].sync_maps)
            free(local_syscalls[i].sync_maps);
    }
    
    assert_true(1);
}

/**
 * Test: ebpf_sync_read_global_table - no syscalls enabled
 */
static void test_ebpf_sync_read_global_table_no_enabled(void **state)
{
    (void)state;
    
    for (int i = 0; local_syscalls[i].syscall; i++) {
        local_syscalls[i].enabled = 0;
    }
    
    extern void ebpf_sync_read_global_table(int maps_per_core);
    ebpf_sync_read_global_table(0);
    
    assert_true(1);
}

/**
 * Test: ebpf_sync_parse_syscalls
 */
static void test_ebpf_sync_parse_syscalls(void **state)
{
    (void)state;
    
    extern void ebpf_sync_parse_syscalls();
    ebpf_sync_parse_syscalls();
    
    assert_true(1);
}

/**
 * Test: ebpf_set_sync_maps
 */
static void test_ebpf_set_sync_maps(void **state)
{
    (void)state;
    
    extern void ebpf_set_sync_maps();
    ebpf_set_sync_maps();
    
    /* Verify maps are set */
    assert_ptr_equal(local_syscalls[NETDATA_SYNC_SYNC_IDX].sync_maps, sync_maps);
    assert_ptr_equal(local_syscalls[NETDATA_SYNC_SYNCFS_IDX].sync_maps, syncfs_maps);
    assert_ptr_equal(local_syscalls[NETDATA_SYNC_MSYNC_IDX].sync_maps, msync_maps);
    assert_ptr_equal(local_syscalls[NETDATA_SYNC_FSYNC_IDX].sync_maps, fsync_maps);
    assert_ptr_equal(local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].sync_maps, fdatasync_maps);
    assert_ptr_equal(local_syscalls[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].sync_maps, sync_file_range_maps);
}

/**
 * Test: sync_send_data - all syscalls disabled
 */
static void test_sync_send_data_all_disabled(void **state)
{
    (void)state;
    
    for (int i = 0; local_syscalls[i].syscall; i++) {
        local_syscalls[i].enabled = 0;
    }
    
    extern void sync_send_data();
    sync_send_data();
    
    assert_true(1);
}

/**
 * Test: sync_send_data - fsync and fdatasync enabled
 */
static void test_sync_send_data_file_sync(void **state)
{
    (void)state;
    
    local_syscalls[NETDATA_SYNC_FSYNC_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_MSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNCFS_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].enabled = 0;
    
    extern void sync_send_data();
    sync_send_data();
    
    assert_true(1);
}

/**
 * Test: sync_send_data - msync enabled
 */
static void test_sync_send_data_msync(void **state)
{
    (void)state;
    
    local_syscalls[NETDATA_SYNC_FSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_MSYNC_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_SYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNCFS_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].enabled = 0;
    
    extern void sync_send_data();
    sync_send_data();
    
    assert_true(1);
}

/**
 * Test: sync_send_data - sync and syncfs enabled
 */
static void test_sync_send_data_sync(void **state)
{
    (void)state;
    
    local_syscalls[NETDATA_SYNC_FSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_MSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_SYNCFS_IDX].enabled = 1;
    local_syscalls[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].enabled = 0;
    
    extern void sync_send_data();
    sync_send_data();
    
    assert_true(1);
}

/**
 * Test: sync_send_data - sync_file_range enabled
 */
static void test_sync_send_data_file_segment(void **state)
{
    (void)state;
    
    local_syscalls[NETDATA_SYNC_FSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_FDATASYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_MSYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNC_IDX].enabled = 0;
    local_syscalls[NETDATA_SYNC_SYNCFS_IDX].enabled = 0;
    local_sysc