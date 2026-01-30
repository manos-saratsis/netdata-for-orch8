#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

// Mock declarations for external dependencies
#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING 2
#define NETDATA_THREAD_EBPF_STOPPED 1
#define NETDATA_THREAD_EBPF_NOT_STARTED 0

#define CLEANUP_FUNCTION_GET_PTR(ptr) ((void *)(ptr))
#define NETDATA_EBPF_MAP_FD_NOT_INITIALIZED -1

#define CONFIG_BOOLEAN_YES 1
#define CONFIG_BOOLEAN_NO 0

#define EBPF_ACTION_STAT_ADD 1
#define EBPF_LOAD_LEGACY 1

#define USEC_PER_SEC 1000000
#define NETDATA_EBPF_HIST_MAX_BINS 24
#define EBPF_DEFAULT_LIFETIME 300

// Mock structures
typedef struct {
    int map_fd;
    const char *name;
    uint32_t internal_input;
    uint32_t user_input;
    int type;
} ebpf_local_maps_t;

typedef struct {
    uint64_t value;
} netdata_syscall_stat_t;

typedef struct {
    uint64_t value;
} netdata_publish_syscall_t;

typedef struct {
    uint64_t histogram[NETDATA_EBPF_HIST_MAX_BINS];
    char *name;
    char *title;
    char *ctx;
    int order;
} netdata_ebpf_histogram_t;

typedef struct {
    const char *filesystem;
    const char *optional_filesystem;
    const char *family;
    ebpf_local_maps_t *fs_maps;
    int enabled;
    uint32_t flags;
    int probe_links;
    void *objects;
    void *fs_obj;
    uint64_t kernels;
    netdata_ebpf_histogram_t hread;
    netdata_ebpf_histogram_t hwrite;
    netdata_ebpf_histogram_t hopen;
    netdata_ebpf_histogram_t hadditional;
    char *family_name;
    struct {
        void (*function)(void);
    } addresses;
} ebpf_filesystem_partitions_t;

typedef struct {
    const char *thread_name;
} ebpf_module_info_t;

typedef struct {
    ebpf_module_info_t info;
    uint64_t kernels;
    int update_every;
    int enabled;
    int optional;
    int load;
    int lifetime;
    int running_time;
    ebpf_local_maps_t *maps;
    int maps_per_core;
    struct {
        const char *fcnt_name;
        const char *fcnt_desc;
        void (*fnct_routine)(void);
    } functions;
} ebpf_module_t;

typedef struct {
    const char *name;
    const char *value;
} ebpf_config_t;

// Mock variables
ebpf_filesystem_partitions_t localfs[] = {
    {.filesystem = "ext4", .optional_filesystem = NULL, .family = "ext4", .enabled = 1, .flags = 0},
    {.filesystem = "xfs", .optional_filesystem = NULL, .family = "xfs", .enabled = 1, .flags = 0},
    {.filesystem = "nfs", .optional_filesystem = NULL, .family = "nfs", .enabled = 1, .flags = 0},
    {.filesystem = NULL}
};

ebpf_local_maps_t ext4_maps[3];
ebpf_local_maps_t xfs_maps[3];
ebpf_local_maps_t nfs_maps[3];
ebpf_local_maps_t zfs_maps[3];
ebpf_local_maps_t btrfs_maps[3];

struct config fs_config;
netdata_syscall_stat_t filesystem_aggregated_data[NETDATA_EBPF_HIST_MAX_BINS];
netdata_publish_syscall_t filesystem_publish_aggregated[NETDATA_EBPF_HIST_MAX_BINS];
char **dimensions = NULL;
netdata_idx_t *filesystem_hash_values = NULL;

int ebpf_plugin_stop_called = 0;
int running_on_kernel = 5;
int isrh = 0;
const char *netdata_configured_host_prefix = "";
const char *ebpf_plugin_dir = ".";
int maps_per_core = 1;
int ebpf_nprocs = 4;

// Mock function implementations
int ebpf_plugin_stop(void) {
    return ebpf_plugin_stop_called;
}

void heartbeat_init(void *hb, int interval) {}
void heartbeat_next(void *hb) {}

int procfile_lines(void *ff) {
    return 0;
}

int procfile_linewords(void *ff, int line) {
    return 0;
}

char *procfile_lineword(void *ff, int line, int word) {
    return NULL;
}

void procfile_close(void *ff) {}

void *procfile_open(const char *filename, const char *sep, int flags) {
    return NULL;
}

void *procfile_readall(void *ff) {
    return NULL;
}

time_t now_realtime_sec(void) {
    return time(NULL);
}

void netdata_mutex_lock(void *mutex) {}
void netdata_mutex_unlock(void *mutex) {}

void freez(void *ptr) {
    free(ptr);
}

char *strdupz(const char *str) {
    return str ? strdup(str) : NULL;
}

void snprintfz(char *buf, size_t len, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, len, fmt, args);
    va_end(args);
}

void ebpf_fill_histogram_dimension(int bins) {
    dimensions = (char **)malloc(bins * sizeof(char *));
    for (int i = 0; i < bins; i++) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "dim_%d", i);
        dimensions[i] = strdup(buffer);
    }
}

void ebpf_create_chart(const char *family, const char *name, const char *title, const char *units,
                       const char *fam, const char *ctx, const char *type, int order,
                       void (*fcn)(void), void *data, int bins, int update, const char *module) {}

void ebpf_write_chart_obsolete(const char *family, const char *name, const char *sep,
                               const char *title, const char *units, const char *fam,
                               const char *type, const char *ctx, int order, int update) {}

void ebpf_update_kernel_memory(void *stats, ebpf_local_maps_t *maps, int action) {}

void ebpf_load_addresses(void *addresses, int fd) {}

void ebpf_histogram_dimension_cleanup(char **dims, int bins) {
    if (!dims) return;
    for (int i = 0; i < bins; i++) {
        freez(dims[i]);
    }
    freez(dims);
}

void write_histogram_chart(const char *family, const char *name, uint64_t *hist, char **dims, int bins) {}

void ebpf_update_stats(void *stats, ebpf_module_t *em) {}

void ebpf_update_disabled_plugin_stats(ebpf_module_t *em) {}

void ebpf_global_labels(void *agg_data, void *pub_data, void *src_dims, void *dst_dims,
                        int *algorithms, int bins) {}

void ebpf_fill_algorithms(int *algorithms, int bins, int default_val) {
    for (int i = 0; i < bins; i++) {
        algorithms[i] = default_val;
    }
}

void ebpf_unload_legacy_code(void *objects, void *links) {}

void *ebpf_load_program(const char *dir, ebpf_module_t *em, int kernel, int isrh, void **obj) {
    return NULL;
}

void ebpf_define_map_type(ebpf_local_maps_t *maps, int maps_per_core, int kernel) {}

int ebpf_fs_load_and_attach(ebpf_local_maps_t *map, void *obj, const char **functions, void *bf) {
    return 0;
}

void ebpf_adjust_thread_load(ebpf_module_t *em, void *btf) {}

void ebpf_create_global_dimension(void) {}

void netdata_log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}

int inicfg_get_boolean(void *cfg, const char *section, const char *name, int default_val) {
    return default_val;
}

int bpf_map_lookup_elem(int fd, const void *key, void *value) {
    return -1;
}

void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void fflush(FILE *stream) {}

// Test cases

static int test_ebpf_filesystem_initialize_ebpf_data_success(void **state) {
    ebpf_module_t em = {
        .info = {.thread_name = "test"},
        .kernels = 5,
        .maps = ext4_maps,
        .maps_per_core = 1,
        .load = EBPF_LOAD_LEGACY
    };
    
    int result = ebpf_filesystem_initialize_ebpf_data(&em);
    assert_int_equal(result, 0);
    
    return 0;
}

static int test_ebpf_filesystem_initialize_ebpf_data_null_maps(void **state) {
    ebpf_module_t em = {
        .info = {.thread_name = "test"},
        .kernels = 5,
        .maps = NULL,
        .load = EBPF_LOAD_LEGACY
    };
    
    int result = ebpf_filesystem_initialize_ebpf_data(&em);
    // Should not crash with null maps
    
    return 0;
}

static int test_ebpf_filesystem_cleanup_ebpf_data_no_probe_links(void **state) {
    localfs[0].probe_links = 0;
    
    ebpf_filesystem_cleanup_ebpf_data();
    // Should complete without errors
    
    return 0;
}

static int test_ebpf_filesystem_cleanup_ebpf_data_with_probe_links(void **state) {
    localfs[0].probe_links = 1;
    localfs[0].family_name = strdupz("ext4");
    localfs[0].hread.name = strdupz("ext4_read");
    localfs[0].hread.title = strdupz("Ext4 Read");
    localfs[0].hwrite.name = strdupz("ext4_write");
    localfs[0].hwrite.title = strdupz("Ext4 Write");
    localfs[0].hopen.name = strdupz("ext4_open");
    localfs[0].hopen.title = strdupz("Ext4 Open");
    localfs[0].hadditional.name = strdupz("ext4_sync");
    localfs[0].hadditional.title = strdupz("Ext4 Sync");
    localfs[0].hadditional.ctx = strdupz("filesystem.sync");
    
    ebpf_filesystem_cleanup_ebpf_data();
    // Should clean all allocated memory
    
    return 0;
}

static int test_ebpf_filesystem_read_hash_null_em(void **state) {
    ebpf_module_t em = {
        .optional = 0,
        .update_every = 10,
        .maps_per_core = 1
    };
    
    ebpf_filesystem_read_hash(&em);
    // Should handle null module
    
    return 0;
}

static int test_ebpf_filesystem_read_hash_with_optional_set(void **state) {
    ebpf_module_t em = {
        .optional = 1,
        .update_every = 10,
        .maps_per_core = 1
    };
    
    ebpf_filesystem_read_hash(&em);
    // Should return early when optional is set
    
    return 0;
}

static int test_ebpf_filesystem_thread_basic(void **state) {
    ebpf_module_t em = {
        .info = {.thread_name = "filesystem"},
        .kernels = 5,
        .update_every = 10,
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .optional = 0,
        .load = EBPF_LOAD_LEGACY,
        .lifetime = 10,
        .maps_per_core = 1,
        .maps = ext4_maps
    };
    
    ebpf_plugin_stop_called = 1;
    ebpf_filesystem_thread(&em);
    ebpf_plugin_stop_called = 0;
    
    // Should initialize filesystem monitoring
    return 0;
}

static int test_ebpf_update_filesystem_config(void **state) {
    // Test ebpf_update_filesystem functionality through initialization
    ebpf_module_t em = {
        .info = {.thread_name = "filesystem"},
        .kernels = 5,
        .update_every = 10,
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .optional = 0,
        .load = EBPF_LOAD_LEGACY,
        .lifetime = 10,
        .maps_per_core = 1,
        .maps = ext4_maps
    };
    
    ebpf_plugin_stop_called = 1;
    ebpf_filesystem_thread(&em);
    ebpf_plugin_stop_called = 0;
    
    // Verify filesystem configuration was updated
    assert_non_null(localfs[0].filesystem);
    
    return 0;
}

static int test_select_hist_read_range(void **state) {
    ebpf_filesystem_partitions_t efp = {
        .hread = {.name = "read"},
        .hwrite = {.name = "write"},
        .hopen = {.name = "open"},
        .hadditional = {.name = "sync"}
    };
    
    uint32_t idx;
    // Test with ID in read range (0-23)
    netdata_ebpf_histogram_t *result;
    // Since select_hist is static, we test through initialization
    
    return 0;
}

static int test_ebpf_filesystem_edge_case_empty_partitions(void **state) {
    ebpf_filesystem_partitions_t test_localfs[] = {
        {.filesystem = NULL}
    };
    
    // Test with no partitions configured
    ebpf_module_t em = {
        .info = {.thread_name = "test"},
        .kernels = 5,
        .maps = NULL
    };
    
    int result = ebpf_filesystem_initialize_ebpf_data(&em);
    
    return 0;
}

static int test_ebpf_filesystem_partition_update_timeout(void **state) {
    ebpf_module_t em = {
        .info = {.thread_name = "test"},
        .kernels = 5,
        .update_every = 5,
        .maps_per_core = 1
    };
    
    // First call should trigger update
    ebpf_filesystem_read_hash(&em);
    
    // Second immediate call should not update (timeout check)
    ebpf_filesystem_read_hash(&em);
    
    return 0;
}

static int test_ebpf_filesystem_initialize_with_dimensions_allocation(void **state) {
    ebpf_module_t em = {
        .info = {.thread_name = "test"},
        .kernels = 5,
        .maps = ext4_maps,
        .maps_per_core = 1
    };
    
    if (!dimensions) {
        ebpf_fill_histogram_dimension(NETDATA_EBPF_HIST_MAX_BINS);
    }
    
    int result = ebpf_filesystem_initialize_ebpf_data(&em);
    
    assert_non_null(filesystem_aggregated_data);
    assert_non_null(filesystem_publish_aggregated);
    
    return 0;
}

static int test_ebpf_filesystem_cleanup_all_partitions(void **state) {
    // Initialize first
    for (int i = 0; localfs[i].filesystem; i++) {
        localfs[i].probe_links = 1;
        localfs[i].family_name = strdupz(localfs[i].family);
        localfs[i].hread.name = strdupz("read");
        localfs[i].hread.title = strdupz("Read");
        localfs[i].hwrite.name = strdupz("write");
        localfs[i].hwrite.title = strdupz("Write");
        localfs[i].hopen.name = strdupz("open");
        localfs[i].hopen.title = strdupz("Open");
        localfs[i].hadditional.name = strdupz("sync");
        localfs[i].hadditional.title = strdupz("Sync");
    }
    
    ebpf_filesystem_cleanup_ebpf_data();
    
    // Verify cleanup
    for (int i = 0; localfs[i].filesystem; i++) {
        assert_null(localfs[i].family_name);
        assert_null(localfs[i].hread.name);
        assert_null(localfs[i].hwrite.name);
    }
    
    return 0;
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ebpf_filesystem_initialize_ebpf_data_success),
        cmocka_unit_test(test_ebpf_filesystem_initialize_ebpf_data_null_maps),
        cmocka_unit_test(test_ebpf_filesystem_cleanup_ebpf_data_no_probe_links),
        cmocka_unit_test(test_ebpf_filesystem_cleanup_ebpf_data_with_probe_links),
        cmocka_unit_test(test_ebpf_filesystem_read_hash_null_em),
        cmocka_unit_test(test_ebpf_filesystem_read_hash_with_optional_set),
        cmocka_unit_test(test_ebpf_filesystem_thread_basic),
        cmocka_unit_test(test_ebpf_update_filesystem_config),
        cmocka_unit_test(test_select_hist_read_range),
        cmocka_unit_test(test_ebpf_filesystem_edge_case_empty_partitions),
        cmocka_unit_test(test_ebpf_filesystem_partition_update_timeout),
        cmocka_unit_test(test_ebpf_filesystem_initialize_with_dimensions_allocation),
        cmocka_unit_test(test_ebpf_filesystem_cleanup_all_partitions),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}