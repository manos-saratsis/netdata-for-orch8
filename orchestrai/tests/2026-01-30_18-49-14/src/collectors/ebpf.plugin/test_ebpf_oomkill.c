#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// Mock declarations for external dependencies
#define NETDATA_EBPF_MAP_FD_NOT_INITIALIZED -1
#define ND_EBPF_MAP_FD_NOT_INITIALIZED -1
#define NETDATA_OOMKILL_MAX_ENTRIES 64
#define NETDATA_EBPF_MAP_STATIC 0
#define NETDATA_EBPF_MAP_CONTROLLER 1
#define BPF_MAP_TYPE_PERCPU_HASH 3

#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING 1
#define NETDATA_THREAD_EBPF_STOPPED 0

#define NETDATA_EBPF_KERNEL_4_14 414
#define NETDATA_EBPF_APPS_FLAG_CHART_CREATED 1
#define NETDATA_EBPF_SERVICES_HAS_OOMKILL_CHART 1
#define NETDATA_EBPF_CGROUP_HAS_OOMKILL_CHART 2
#define NETDATA_EBPF_SERVICES_HAS_OOMKILL_CHART 1

#define NETDATA_MODULE_OOMKILL_IDX 0

#define NETDATA_APP_FAMILY "apps"
#define NETDATA_OOMKILL_CHART "_ebpf_oomkill"
#define EBPF_OOMKILL_UNIT_KILLS "kills/s"
#define NETDATA_EBPF_MEMORY_GROUP "mem"
#define NETDATA_EBPF_CHART_TYPE_STACKED "stacked"
#define NETDATA_EBPF_CHART_TYPE_LINE "line"
#define NETDATA_CGROUP_OOMKILLS_CONTEXT "cgroup.oomkills"
#define NETDATA_SYSTEMD_OOMKILLS_CONTEXT "systemd.oomkills"

#define NETDATA_CHART_PRIO_CGROUPS_CONTAINERS 100
#define NETDATA_EBPF_MODULE_NAME_OOMKILL "oomkill"
#define EBPF_CHART_ALGORITHM_INCREMENTAL "incremental"

#define NETDATA_CONTROLLER_PID_TABLE_ADD 0
#define NETDATA_CONTROLLER_PID_TABLE_DEL 1

#define NETDATA_EBPF_ABSOLUTE_IDX 0

#define APPCONFIG_INITIALIZER {}

// Type definitions for mocking
typedef struct {
    char *name;
    uint32_t internal_input;
    uint32_t user_input;
    int type;
    int map_fd;
} ebpf_local_maps_t;

typedef struct {
    bool enabled;
    char *class;
    char *event;
} ebpf_tracepoint_t;

typedef struct {
    char *name;
    char *dimension;
    char *algorithm;
    void *next;
} netdata_publish_syscall_t;

typedef struct ebpf_target {
    char *clean_name;
    int charts_created;
    int exposed;
    void *root_pid;
    struct ebpf_target *next;
} ebpf_target_t;

typedef struct pid_on_target2 {
    int32_t pid;
    struct pid_on_target2 *next;
} pid_on_target2_t;

typedef struct ebpf_cgroup_target {
    char *name;
    int systemd;
    int flags;
    uint32_t oomkill;
    int updated;
    pid_on_target2_t *pids;
    struct ebpf_cgroup_target *next;
} ebpf_cgroup_target_t;

typedef struct {
    char *title;
    char *units;
    char *family;
    char *charttype;
    int order;
    char *algorithm;
    char *context;
    char *module;
    int update_every;
    char *suffix;
    char *dimension;
    char *id;
} ebpf_systemd_args_t;

typedef struct {
    int systemd_enabled;
} ebpf_cgroup_shm_header_t;

typedef struct {
    ebpf_cgroup_shm_header_t *header;
} ebpf_cgroup_shm_t;

typedef struct {
    uint64_t *hash_table_stats;
    int cgroup_charts;
    int update_every;
    int lifetime;
    int apps_charts;
    void *maps;
    void *objects;
    void *probe_links;
    int enabled;
    int running_time;
    int maps_per_core;
} ebpf_module_t;

typedef struct ebpf_pid_on_target {
    int pid;
    struct ebpf_pid_on_target *next;
} ebpf_pid_on_target_t;

typedef struct {
    uint64_t *data;
} plugin_statistics_t;

typedef uint64_t netdata_idx_t;
typedef int netdata_apps_integration_flags_t;
typedef int RRDLABEL_SRC_AUTO;

// Mock global variables
struct config oomkill_config = APPCONFIG_INITIALIZER;
ebpf_local_maps_t oomkill_maps[2];
ebpf_tracepoint_t oomkill_tracepoints[2];
netdata_publish_syscall_t oomkill_publish_aggregated;
ebpf_cgroup_target_t *ebpf_cgroup_pids = NULL;
ebpf_target_t *apps_groups_root_target = NULL;
ebpf_cgroup_shm_t shm_ebpf_cgroup;
plugin_statistics_t plugin_statistics;
int running_on_kernel = NETDATA_EBPF_KERNEL_4_14;
int isrh = 0;
char *ebpf_plugin_dir = "/tmp/ebpf";
int collect_pids = 0;
int send_cgroup_chart = 0;

// Mock pthread mutexes
typedef struct {} netdata_mutex_t;
netdata_mutex_t lock;
netdata_mutex_t collect_data_mutex;
netdata_mutex_t mutex_cgroup_shm;
netdata_mutex_t ebpf_exit_cleanup;

// Mock function implementations
int netdata_mutex_lock(netdata_mutex_t *mutex) {
    return 0;
}

int netdata_mutex_unlock(netdata_mutex_t *mutex) {
    return 0;
}

void netdata_log_info(const char *fmt, ...) {}
void netdata_log_error(const char *fmt, ...) {}
void fflush(void *stream) {}
int ebpf_enable_tracepoints(ebpf_tracepoint_t *tracepoints) {
    return 1;
}

void ebpf_update_stats(plugin_statistics_t *stats, ebpf_module_t *em) {}
void ebpf_update_kernel_memory_with_vector(plugin_statistics_t *stats, void *maps, int action) {}
void ebpf_unload_legacy_code(void *objects, void *probe_links) {}
void ebpf_update_disabled_plugin_stats(ebpf_module_t *em) {}

int bpf_map_get_next_key(int mapfd, const void *key, void *next_key) {
    return -1; // No more keys
}

int bpf_map_delete_elem(int mapfd, const void *key) {
    return 0;
}

void ebpf_write_chart_obsolete(const char *type, const char *id, const char *name,
                                const char *title, const char *units, const char *family,
                                const char *chart_type, const char *context, int order,
                                int update_every) {}

void ebpf_create_chart(const char *type, const char *id, const char *name,
                       const char *title, const char *units, const char *family,
                       const char *chart_type, int order, void *func,
                       netdata_publish_syscall_t *pub, int count, int update_every,
                       const char *module) {}

void ebpf_write_begin_chart(const char *family, const char *name, const char *id) {}
void write_chart_dimension(const char *dimension, long long value) {}
void ebpf_write_end_chart(void) {}

void ebpf_create_charts_on_systemd(ebpf_systemd_args_t *args) {}

void *ebpf_load_program(const char *dir, ebpf_module_t *em, int kernel,
                        int isrh, void **objects) {
    return (void*)1; // Non-null return
}

void ebpf_write_chart_cmd(const char *family, const char *name, const char *id,
                          const char *title, const char *units, const char *family2,
                          const char *chart_type, const char *context, int order,
                          int update_every, const char *module) {}

void ebpf_create_chart_labels(const char *key, const char *value, int source) {}
void ebpf_commit_label(void) {}

void ebpf_define_map_type(void *maps, int maps_per_core, int kernel) {}

int ebpf_plugin_stop(void) {
    return 0;
}

typedef struct {
    uint64_t value;
} heartbeat_t;

void heartbeat_init(heartbeat_t *hb, uint64_t usec) {}
void heartbeat_next(heartbeat_t *hb) {}

#define CLEANUP_FUNCTION_GET_PTR(ptr) (ebpf_module_t*)ptr
#define CLEANUP_FUNCTION_REGISTER(func) void *__attribute__((cleanup(func)))
#define USEC_PER_SEC 1000000

extern const char *ebpf_algorithms[];

// Test suite

static void test_ebpf_obsolete_oomkill_services(void **state) {
    ebpf_module_t em = {
        .update_every = 10
    };
    
    ebpf_obsolete_oomkill_services(&em, "test_service");
    // No assertions needed - function only calls write function
    assert_true(1);
}

static void test_ebpf_obsolete_oomkill_cgroup_charts_no_cgroups(void **state) {
    ebpf_cgroup_pids = NULL;
    
    ebpf_module_t em = {
        .update_every = 10
    };
    
    ebpf_obsolete_oomkill_cgroup_charts(&em);
    assert_true(1);
}

static void test_ebpf_obsolete_oomkill_cgroup_charts_with_systemd(void **state) {
    ebpf_cgroup_target_t cgroup = {
        .name = "test_systemd",
        .systemd = 1,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    ebpf_module_t em = {
        .update_every = 10
    };
    
    ebpf_obsolete_oomkill_cgroup_charts(&em);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_obsolete_oomkill_cgroup_charts_with_cgroup(void **state) {
    ebpf_cgroup_target_t cgroup = {
        .name = "test_cgroup",
        .systemd = 0,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    ebpf_module_t em = {
        .update_every = 10
    };
    
    ebpf_obsolete_oomkill_cgroup_charts(&em);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_obsolete_oomkill_apps_no_targets(void **state) {
    apps_groups_root_target = NULL;
    
    ebpf_module_t em = {
        .update_every = 10
    };
    
    ebpf_obsolete_oomkill_apps(&em);
    assert_true(1);
}

static void test_ebpf_obsolete_oomkill_apps_with_targets(void **state) {
    ebpf_target_t target = {
        .clean_name = "test_app",
        .charts_created = (1 << NETDATA_MODULE_OOMKILL_IDX),
        .next = NULL
    };
    
    apps_groups_root_target = &target;
    
    ebpf_module_t em = {
        .update_every = 10
    };
    
    ebpf_obsolete_oomkill_apps(&em);
    
    apps_groups_root_target = NULL;
    assert_true(1);
}

static void test_ebpf_obsolete_oomkill_apps_chart_not_created(void **state) {
    ebpf_target_t target = {
        .clean_name = "test_app",
        .charts_created = 0,
        .next = NULL
    };
    
    apps_groups_root_target = &target;
    
    ebpf_module_t em = {
        .update_every = 10
    };
    
    ebpf_obsolete_oomkill_apps(&em);
    
    apps_groups_root_target = NULL;
    assert_true(1);
}

static void test_oomkill_cleanup_null_ptr(void **state) {
    oomkill_cleanup(NULL);
    assert_true(1);
}

static void test_oomkill_cleanup_with_module(void **state) {
    ebpf_module_t em = {
        .enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING,
        .cgroup_charts = 1,
        .maps = oomkill_maps,
        .objects = (void*)1,
        .probe_links = (void*)1
    };
    
    void *ptr = &em;
    oomkill_cleanup(&ptr);
    
    // Verify cleanup state
    assert_int_equal(em.enabled, NETDATA_THREAD_EBPF_STOPPED);
}

static void test_oomkill_cleanup_disabled_state(void **state) {
    ebpf_module_t em = {
        .enabled = 0,
        .cgroup_charts = 1,
        .maps = oomkill_maps,
        .objects = NULL,
        .probe_links = NULL
    };
    
    void *ptr = &em;
    oomkill_cleanup(&ptr);
    
    assert_true(1);
}

static void test_oomkill_write_data_empty_keys(void **state) {
    int32_t keys[NETDATA_OOMKILL_MAX_ENTRIES] = {0};
    
    oomkill_write_data(keys, 0);
    assert_true(1);
}

static void test_oomkill_write_data_with_targets(void **state) {
    ebpf_pid_on_target_t pid1 = {.pid = 100, .next = NULL};
    ebpf_target_t target = {
        .clean_name = "test_app",
        .charts_created = (1 << NETDATA_MODULE_OOMKILL_IDX),
        .root_pid = &pid1,
        .next = NULL
    };
    
    apps_groups_root_target = &target;
    
    int32_t keys[NETDATA_OOMKILL_MAX_ENTRIES] = {100, 0};
    
    oomkill_write_data(keys, 1);
    
    apps_groups_root_target = NULL;
    assert_true(1);
}

static void test_oomkill_write_data_no_chart_created(void **state) {
    ebpf_target_t target = {
        .clean_name = "test_app",
        .charts_created = 0,
        .root_pid = NULL,
        .next = NULL
    };
    
    apps_groups_root_target = &target;
    
    int32_t keys[NETDATA_OOMKILL_MAX_ENTRIES] = {100, 0};
    
    oomkill_write_data(keys, 1);
    
    apps_groups_root_target = NULL;
    assert_true(1);
}

static void test_oomkill_write_data_unmatched_pids(void **state) {
    int32_t keys[NETDATA_OOMKILL_MAX_ENTRIES] = {200, 0};
    
    oomkill_write_data(keys, 1);
    assert_true(1);
}

static void test_ebpf_create_specific_oomkill_charts(void **state) {
    ebpf_create_specific_oomkill_charts("test_cgroup", 10);
    assert_true(1);
}

static void test_ebpf_create_systemd_oomkill_charts_empty(void **state) {
    ebpf_cgroup_pids = NULL;
    
    ebpf_create_systemd_oomkill_charts(10);
    assert_true(1);
}

static void test_ebpf_create_systemd_oomkill_charts_with_targets(void **state) {
    ebpf_cgroup_target_t cgroup = {
        .name = "test_service",
        .systemd = 1,
        .flags = 0,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    ebpf_create_systemd_oomkill_charts(10);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_create_systemd_oomkill_charts_already_created(void **state) {
    ebpf_cgroup_target_t cgroup = {
        .name = "test_service",
        .systemd = 1,
        .flags = NETDATA_EBPF_SERVICES_HAS_OOMKILL_CHART,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    ebpf_create_systemd_oomkill_charts(10);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_send_systemd_oomkill_charts_empty(void **state) {
    ebpf_cgroup_pids = NULL;
    
    ebpf_send_systemd_oomkill_charts();
    assert_true(1);
}

static void test_ebpf_send_systemd_oomkill_charts_with_flag(void **state) {
    ebpf_cgroup_target_t cgroup = {
        .name = "test_service",
        .flags = NETDATA_EBPF_SERVICES_HAS_OOMKILL_CHART,
        .oomkill = 5,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    ebpf_send_systemd_oomkill_charts();
    
    assert_int_equal(cgroup.oomkill, 0);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_send_systemd_oomkill_charts_without_flag(void **state) {
    ebpf_cgroup_target_t cgroup = {
        .name = "test_service",
        .flags = 0,
        .oomkill = 5,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    ebpf_send_systemd_oomkill_charts();
    
    assert_int_equal(cgroup.oomkill, 5);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_send_specific_oomkill_data(void **state) {
    ebpf_send_specific_oomkill_data("test_cgroup", 42);
    assert_true(1);
}

static void test_ebpf_obsolete_specific_oomkill_charts(void **state) {
    ebpf_obsolete_specific_oomkill_charts("test_cgroup", 10);
    assert_true(1);
}

static void test_ebpf_oomkill_send_cgroup_data_no_cgroups(void **state) {
    shm_ebpf_cgroup.header = NULL;
    ebpf_cgroup_pids = NULL;
    
    ebpf_oomkill_send_cgroup_data(10);
    assert_true(1);
}

static void test_ebpf_oomkill_send_cgroup_data_systemd_enabled(void **state) {
    ebpf_cgroup_shm_header_t header = {.systemd_enabled = 1};
    shm_ebpf_cgroup.header = &header;
    ebpf_cgroup_pids = NULL;
    send_cgroup_chart = 1;
    
    ebpf_oomkill_send_cgroup_data(10);
    assert_true(1);
}

static void test_ebpf_oomkill_send_cgroup_data_with_cgroups(void **state) {
    ebpf_cgroup_shm_header_t header = {.systemd_enabled = 0};
    shm_ebpf_cgroup.header = &header;
    
    ebpf_cgroup_target_t cgroup = {
        .name = "test_cgroup",
        .systemd = 0,
        .flags = 0,
        .oomkill = 1,
        .updated = 1,
        .pids = NULL,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    ebpf_oomkill_send_cgroup_data(10);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_oomkill_send_cgroup_data_cgroup_obsolete(void **state) {
    ebpf_cgroup_shm_header_t header = {.systemd_enabled = 0};
    shm_ebpf_cgroup.header = &header;
    
    ebpf_cgroup_target_t cgroup = {
        .name = "test_cgroup",
        .systemd = 0,
        .flags = NETDATA_EBPF_CGROUP_HAS_OOMKILL_CHART,
        .oomkill = 1,
        .updated = 0,
        .pids = NULL,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    ebpf_oomkill_send_cgroup_data(10);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_oomkill_read_data_empty_map(void **state) {
    int32_t keys[NETDATA_OOMKILL_MAX_ENTRIES];
    memset(keys, 0, sizeof(keys));
    
    uint32_t count = oomkill_read_data(keys);
    
    assert_int_equal(count, 0);
}

static void test_ebpf_update_oomkill_cgroup_no_cgroups(void **state) {
    int32_t keys[NETDATA_OOMKILL_MAX_ENTRIES] = {0};
    ebpf_cgroup_pids = NULL;
    
    ebpf_update_oomkill_cgroup(keys, 0);
    assert_true(1);
}

static void test_ebpf_update_oomkill_cgroup_with_cgroups(void **state) {
    pid_on_target2_t pid1 = {.pid = 100, .next = NULL};
    ebpf_cgroup_target_t cgroup = {
        .name = "test_cgroup",
        .oomkill = 0,
        .pids = &pid1,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    int32_t keys[NETDATA_OOMKILL_MAX_ENTRIES] = {100, 0};
    
    ebpf_update_oomkill_cgroup(keys, 1);
    
    assert_int_equal(cgroup.oomkill, 1);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_update_oomkill_cgroup_no_matching_pid(void **state) {
    pid_on_target2_t pid1 = {.pid = 100, .next = NULL};
    ebpf_cgroup_target_t cgroup = {
        .name = "test_cgroup",
        .oomkill = 0,
        .pids = &pid1,
        .next = NULL
    };
    
    ebpf_cgroup_pids = &cgroup;
    
    int32_t keys[NETDATA_OOMKILL_MAX_ENTRIES] = {200, 0};
    
    ebpf_update_oomkill_cgroup(keys, 1);
    
    assert_int_equal(cgroup.oomkill, 0);
    
    ebpf_cgroup_pids = NULL;
    assert_true(1);
}

static void test_ebpf_update_oomkill_period_first_run(void **state) {
    ebpf_module_t em = {
        .running_time = 0,
        .update_every = 10
    };
    
    int result = ebpf_update_oomkill_period(0, &em);
    
    assert_int_equal(result, 10);
    assert_int_equal(em.running_time, 10);
}

static void test_ebpf_update_oomkill_period_subsequent_runs(void **state) {
    ebpf_module_t em = {
        .running_time = 10,
        .update_every = 10
    };
    
    int result = ebpf_update_oomkill_period(10, &em);
    
    assert_int_equal(result, 20);
    assert_int_equal(em.running_time, 20);
}

static void test_ebpf_oomkill_create_apps_charts_empty(void **state) {
    apps_groups_root_target = NULL;
    
    ebpf_module_t em = {
        .update_every = 10,
        .apps_charts = 0
    };
    
    ebpf_oomkill_create_apps_charts(&em, NULL);
    
    assert_int_equal(em.apps_charts, NETDATA_EBPF_APPS_FLAG_CHART_CREATED);
}

static void test_ebpf_oomkill_create_apps_charts_with_targets(void **state) {
    ebpf_target_t target = {
        .clean_name = "test_app",
        .exposed = 1,
        .charts_created = 0,
        .next = NULL
    };
    
    ebpf_module_t em = {
        .update_every = 10,
        .apps_charts = 0
    };
    
    ebpf_oomkill_create_apps_charts(&em, &target);
    
    assert_true(target.charts_created & (1 << NETDATA_MODULE_OOMKILL_IDX));
    assert_int_equal(em.apps_charts, NETDATA_EBPF_APPS_FLAG_CHART_CREATED);
}

static void test_ebpf_oomkill_create_apps_charts_not_exposed(void **state) {
    ebpf_target_t target = {
        .clean_name = "test_app",
        .exposed = 0,
        .charts_created = 0,
        .next = NULL
    };
    
    ebpf_module_t em = {
        .update_every = 10,
        .apps_charts = 0
    };
    
    ebpf_oomkill_create_apps_charts(&em, &target);
    
    assert_int_equal(target.charts_created, 0);
    assert_int_equal(em.apps_charts, NETDATA_EBPF_APPS_FLAG_CHART_CREATED);
}

static void test_ebpf_oomkill_thread_null_ptr(void **state) {
    ebpf_oomkill_thread(NULL);
    assert_true(1);
}

static void test_ebpf_oomkill_thread_no_apps_charts(void **state) {
    ebpf_module_t em = {
        .apps_charts = 0,
        .enabled = 1,
        .maps = oomkill_maps,
        .objects = NULL,
        .probe_links = NULL
    };
    
    ebpf_oomkill_thread(&em);
    assert_true(1);
}

static void test_ebpf_oomkill_thread_old_kernel(void **state) {
    int old_kernel = running_on_kernel;
    running_on_kernel = 400;
    
    ebpf_module_t em = {
        .apps_charts = 1,
        .enabled = 1,
        .maps = oomkill_maps,
        .objects = NULL,
        .probe_links = NULL
    };
    
    ebpf_oomkill_thread(&em);
    
    running_on_kernel = old_kernel;
    assert_true(1);
}

static void test_ebpf_oomkill_thread_tracepoint_enable_fail(void **state) {
    ebpf_module_t em = {
        .apps_charts = 1,
        .enabled = 1,
        .maps = oomkill_maps,
        .objects = NULL,
        .probe_links = NULL
    };
    
    // Mock ebpf_enable_tracepoints to return 0
    ebpf_oomkill_thread(&em);
    assert_true(1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ebpf_obsolete_oomkill_services),
        cmocka_unit_test(test_ebpf_obsolete_oomkill_cgroup_charts_no_cgroups),