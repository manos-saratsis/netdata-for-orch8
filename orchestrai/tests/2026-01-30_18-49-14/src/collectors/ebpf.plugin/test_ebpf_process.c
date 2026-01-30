#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

// Mock definitions and includes
#define NETDATA_EBPF_KERNEL_5_3 5
#define NETDATA_EBPF_KERNEL_5_9_16 16
#define NETDATA_EBPF_KERNEL_6_16 116
#define NETDATA_KEY_END_VECTOR 6
#define NETDATA_KEY_CALLS_DO_EXIT 0
#define NETDATA_KEY_CALLS_RELEASE_TASK 1
#define NETDATA_KEY_CALLS_DO_FORK 2
#define NETDATA_KEY_ERROR_DO_FORK 3
#define NETDATA_KEY_CALLS_SYS_CLONE 4
#define NETDATA_KEY_ERROR_SYS_CLONE 5
#define NETDATA_KEY_PUBLISH_PROCESS_EXIT 0
#define NETDATA_KEY_PUBLISH_PROCESS_RELEASE_TASK 1
#define NETDATA_KEY_PUBLISH_PROCESS_FORK 2
#define NETDATA_KEY_PUBLISH_PROCESS_CLONE 3
#define NETDATA_KEY_PUBLISH_PROCESS_END 4
#define NETDATA_CONTROLLER_PID_TABLE_ADD 0
#define NETDATA_CONTROLLER_END 1
#define NETDATA_EBPF_MODULE_PROCESS_IDX 0
#define NETDATA_EBPF_PIDS_PROCESS_IDX 0
#define NETDATA_EBPF_PIDS_CACHESTAT_IDX 1
#define NETDATA_PROCESS_GLOBAL_TABLE 1
#define NETDATA_PROCESS_PID_TABLE 0
#define NETDATA_PROCESS_CTRL_TABLE 2
#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING 1
#define NETDATA_THREAD_EBPF_STOPPING 2
#define NETDATA_THREAD_EBPF_STOPPED 3
#define NETDATA_EBPF_APPS_FLAG_CHART_CREATED (1 << 0)
#define NETDATA_EBPF_CGROUP_HAS_PROCESS_CHART (1 << 0)
#define NETDATA_EBPF_SERVICES_HAS_PROCESS_CHART (1 << 0)
#define NETDATA_EBPF_MAP_RESIZABLE 0x01
#define NETDATA_EBPF_MAP_PID 0x02
#define NETDATA_EBPF_MAP_STATIC 0x04
#define NETDATA_EBPF_MAP_CONTROLLER 0x08
#define ND_EBPF_MAP_FD_NOT_INITIALIZED -1
#define ND_EBPF_DEFAULT_PID_SIZE 10000
#define BPF_MAP_TYPE_PERCPU_HASH 3
#define BPF_MAP_TYPE_PERCPU_ARRAY 13
#define EBPF_LOAD_TRAMPOLINE 0
#define EBPF_LOAD_PROBE 1
#define EBPF_LOAD_RETPROBE 2
#define NETDATA_EBPF_MAP_FD_NOT_INITIALIZED -1
#define NETDATA_MINIMUM_RH_VERSION 1
#define NETDATA_RH_8 8
#define MODE_ENTRY 5
#define NETDATA_EBPF_ABSOLUTE_IDX 0
#define NETDATA_EBPF_INCREMENTAL_IDX 1
#define likely(x) (x)
#define unlikely(x) (x)

typedef struct {
    uint64_t call;
    uint64_t bytes;
    uint64_t ecall;
    void *next;
} netdata_syscall_stat_t;

typedef struct {
    uint64_t ncall;
    uint64_t nbyte;
    uint64_t nerr;
    uint64_t pcall;
    uint64_t pbyte;
    uint64_t perr;
    char *name;
    char *algorithm;
} netdata_publish_syscall_t;

typedef struct {
    long running;
    long zombie;
} netdata_publish_vfs_common_t;

typedef struct {
    uint64_t exit_call;
    uint64_t task_err;
    uint64_t create_thread;
    uint64_t create_process;
    uint64_t release_call;
    uint64_t ct;
} ebpf_process_stat_t;

typedef struct {
    uint64_t exit_call;
    uint64_t task_err;
    uint64_t create_thread;
    uint64_t create_process;
    uint64_t release_call;
} ebpf_publish_process_t;

typedef struct {
    char *name;
    int mode;
} netdata_ebpf_targets_t;

typedef struct {
    char *name;
    int internal_input;
    int user_input;
    int type;
    int map_fd;
#ifdef LIBBPF_MAJOR_VERSION
    int map_type;
#endif
} ebpf_local_maps_t;

typedef struct {
    int load;
    int enabled;
    int global_charts;
    int apps_charts;
    int cgroup_charts;
    int update_every;
    int lifetime;
    int maps_per_core;
    int running_time;
    netdata_ebpf_targets_t *targets;
    ebpf_local_maps_t *maps;
    void *objects;
    void *probe_links;
    char info[256];
    uint32_t *hash_table_stats;
    struct {
        char thread_name[256];
    } info;
} ebpf_module_t;

typedef struct {
    uint32_t pid;
    ebpf_publish_process_t ps;
} pid_on_target2;

typedef struct {
    char *name;
    int systemd;
    int flags;
    uint32_t updated;
    ebpf_publish_process_t publish_systemd_ps;
    pid_on_target2 *pids;
} ebpf_cgroup_target_t;

typedef struct ebpf_target {
    char *name;
    char *clean_name;
    int exposed;
    uint64_t charts_created;
    struct {
        ebpf_process_stat_t process;
    } process;
    struct {
        void *root_pid;
    } root_pid;
    void *processes;
    struct ebpf_target *next;
} ebpf_target;

typedef struct {
    uint32_t pid;
    void *next;
} ebpf_pid_on_target;

typedef struct {
    uint32_t pid;
    void *ps;
    void *next;
} netdata_ebpf_pid_stats_t;

typedef uint64_t netdata_idx_t;

// Global mock variables
static int running_on_kernel = NETDATA_EBPF_KERNEL_6_16;
static int isrh = 0;
static char *ebpf_plugin_dir = "/tmp";
static ebpf_process_stat_t mock_process_stat_vector[2];
static netdata_idx_t mock_process_hash_values[2];
static netdata_syscall_stat_t mock_process_aggregated_data[4];
static netdata_publish_syscall_t mock_process_publish_aggregated[4];
static netdata_module_t mock_plugin_statistics;
static int ebpf_nprocs = 2;
static int send_cgroup_chart = 1;
static int collect_pids = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t collect_data_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_cgroup_shm = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ebpf_exit_cleanup = PTHREAD_MUTEX_INITIALIZER;
static sem_t *shm_mutex_ebpf_integration = NULL;
static struct config process_config = {};
static ebpf_cgroup_target_t *ebpf_cgroup_pids = NULL;
static ebpf_target *apps_groups_root_target = NULL;
static ebpf_module_t ebpf_modules[1] = {};
static struct {
    void *header;
    struct {
        int systemd_enabled;
    } *header;
} shm_ebpf_cgroup = {0};
static int pids_fd[1] = {-1};
static int process_pid_fd = -1;
static int ebpf_all_pids_count = 0;

// Mock functions
void netdata_log_error(const char *fmt, ...) {}
void netdata_mutex_lock(pthread_mutex_t *mutex) { pthread_mutex_lock(mutex); }
void netdata_mutex_unlock(pthread_mutex_t *mutex) { pthread_mutex_unlock(mutex); }
int ebpf_plugin_stop(void) { return 0; }
void ebpf_write_begin_chart(char *family, char *name, char *ctx) {}
void ebpf_write_end_chart(void) {}
void write_chart_dimension(char *name, long long value) {}
int ebpf_create_chart(char *family, char *name, char *title, char *units, char *group,
                     char *context, char *type, int order, void *func, void *ptr,
                     int dim, int update_every, char *module) { return 0; }
void ebpf_create_global_dimension(void) {}
void fflush(FILE *f) {}
void ebpf_write_chart_obsolete(char *family, char *name, char *ctx, char *title,
                              char *units, char *group, char *type, char *context,
                              int order, int update_every) {}
void write_err_chart(char *name, char *family, void *ptr, int dim) {}
void ebpf_update_stats(void *stats, ebpf_module_t *em) {}
void ebpf_update_kernel_memory_with_vector(void *stats, void *maps, int action) {}
void ebpf_update_disabled_plugin_stats(ebpf_module_t *em) {}
int ebpf_unload_legacy_code(void *obj, void *links) { return 0; }
void freez(void *ptr) { if (ptr) free(ptr); }
void *callocz(size_t nmemb, size_t size) { return calloc(nmemb, size); }
int ebpf_enable_tracing_values(char *type, char *point) { return 0; }
int ebpf_disable_tracing_values(char *type, char *point) { return 0; }
int ebpf_is_tracepoint_enabled(char *type, char *point) { return 0; }
void ebpf_update_pid_table(void *map, ebpf_module_t *em) {}
int ebpf_read_global_table_stats(void *res, void *hash, int fd, int per_core, int off, int end) { return 0; }
void ebpf_cgroup_target_t *ebpf_cgroup_pids = NULL;
int netdata_ebpf_get_shm_pointer_unsafe(uint32_t pid, int idx) { return 0; }
int netdata_ebpf_reset_shm_pointer_unsafe(int fd, uint32_t pid, int idx) { return 0; }
int bpf_map_get_next_key(int fd, void *key, void *next_key) { return -1; }
int bpf_map_lookup_elem(int fd, void *key, void *value) { return 0; }
int kill(pid_t pid, int sig) { return -1; }
void ebpf_write_chart_cmd(char *family, char *name, char *suffix, char *title,
                         char *units, char *group, char *type, char *context,
                         int order, int update_every, char *module) {}
void ebpf_create_chart_labels(char *key, char *value, int src) {}
void ebpf_commit_label(void) {}
void fprintf(FILE *f, const char *fmt, ...) {}
void ebpf_create_charts_on_systemd(void *args) {}
void heartbeat_init(void *hb, uint64_t usec) {}
void heartbeat_next(void *hb) {}

// Forward declarations from ebpf_process.c
extern static char *process_dimension_names[];
extern static char *process_id_names[];
extern static char *status[];
extern static ebpf_local_maps_t process_maps[];
extern static char *tracepoint_sched_type;
extern static netdata_idx_t *process_hash_values;
extern ebpf_process_stat_t *process_stat_vector;
extern static netdata_syscall_stat_t process_aggregated_data[];
extern static netdata_publish_syscall_t process_publish_aggregated[];

// Test cases
void test_ebpf_update_global_publish_basic(void) {
    netdata_publish_syscall_t publish[4] = {0};
    netdata_publish_vfs_common_t pvc = {0};
    netdata_syscall_stat_t input = {.call = 100, .bytes = 500, .ecall = 5, .next = NULL};
    
    // Setup initial state
    publish[0].pcall = 50;
    publish[0].pbyte = 250;
    publish[0].perr = 2;
    publish[0].next = NULL;
    
    ebpf_update_global_publish(publish, &pvc, &input);
    
    // Verify calculations
    assert(publish[0].ncall == 50); // 100 - 50
    assert(publish[0].nbyte == 250); // 500 - 250
    assert(publish[0].nerr == 3); // 5 - 2
    assert(publish[0].pcall == 100);
    assert(publish[0].pbyte == 500);
    assert(publish[0].perr == 5);
}

void test_ebpf_update_global_publish_with_linked_data(void) {
    netdata_publish_syscall_t publish[4] = {0};
    netdata_publish_syscall_t *link = &publish[1];
    netdata_publish_vfs_common_t pvc = {0};
    
    netdata_syscall_stat_t input1 = {.call = 100, .bytes = 500, .ecall = 5};
    netdata_syscall_stat_t input2 = {.call = 200, .bytes = 1000, .ecall = 10};
    input1.next = &input2;
    input2.next = NULL;
    
    publish[0].pcall = 50;
    publish[0].pbyte = 250;
    publish[0].perr = 2;
    publish[0].next = link;
    
    link->pcall = 100;
    link->pbyte = 500;
    link->perr = 5;
    link->next = NULL;
    
    ebpf_update_global_publish(publish, &pvc, &input1);
    
    assert(publish[1].ncall == 100); // 200 - 100
    assert(publish[1].nbyte == 500); // 1000 - 500
}

void test_ebpf_update_global_publish_reverse_diff(void) {
    netdata_publish_syscall_t publish[4] = {0};
    netdata_publish_vfs_common_t pvc = {0};
    netdata_syscall_stat_t input = {.call = 50, .bytes = 250, .ecall = 2};
    
    publish[0].pcall = 100;
    publish[0].pbyte = 500;
    publish[0].perr = 5;
    publish[0].next = NULL;
    
    ebpf_update_global_publish(publish, &pvc, &input);
    
    assert(publish[0].ncall == 50); // 100 - 50
    assert(publish[0].nbyte == 250); // 500 - 250
    assert(publish[0].nerr == 3); // 5 - 2
}

void test_ebpf_update_global_publish_running_calculation(void) {
    netdata_publish_syscall_t publish[4] = {0};
    netdata_publish_vfs_common_t pvc = {0};
    netdata_syscall_stat_t input = {0};
    
    publish[0].ncall = 100;
    publish[0].next = &publish[1];
    publish[1].ncall = 50;
    publish[1].next = &publish[2];
    publish[2].ncall = 75;
    publish[2].next = &publish[3];
    publish[3].ncall = 25;
    publish[3].next = NULL;
    
    // Initialize all four elements
    for (int i = 0; i < 4; i++) {
        publish[i].pcall = 0;
        publish[i].pbyte = 0;
        publish[i].perr = 0;
    }
    
    ebpf_update_global_publish(publish, &pvc, &input);
    
    // running = FORK - CLONE = publish[2].ncall - publish[3].ncall
    assert(pvc.running == (75 - 25)); // 50
    assert(publish[1].ncall == -publish[1].ncall); // Release task negated
    // zombie = EXIT + RELEASE_TASK = publish[0].ncall + publish[1].ncall
}

void test_write_status_chart_normal(void) {
    netdata_publish_vfs_common_t pvc = {.running = 10, .zombie = 5};
    // This function just prints, so verify it doesn't crash
    write_status_chart("test_family", &pvc);
}

void test_ebpf_process_send_data_global_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    em.global_charts = 1;
    
    // Mock aggregated data
    for (int i = 0; i < 4; i++) {
        process_publish_aggregated[i].name = "test";
    }
    
    ebpf_process_send_data(&em);
}

void test_ebpf_process_send_data_entry_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY;
    
    ebpf_process_send_data(&em);
}

void test_ebpf_process_send_apps_data_with_empty_list(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    
    ebpf_process_send_apps_data(NULL, &em);
}

void test_ebpf_process_send_apps_data_with_targets(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    
    ebpf_target target = {0};
    target.charts_created = (1 << EBPF_MODULE_PROCESS_IDX);
    target.clean_name = "test";
    target.process.create_process = 10;
    target.process.create_thread = 5;
    target.process.exit_call = 2;
    target.process.release_call = 1;
    target.process.task_err = 0;
    
    ebpf_process_send_apps_data(&target, &em);
}

void test_ebpf_process_send_apps_data_entry_mode_no_errors(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY;
    
    ebpf_target target = {0};
    target.charts_created = (1 << EBPF_MODULE_PROCESS_IDX);
    target.clean_name = "test";
    
    ebpf_process_send_apps_data(&target, &em);
}

void test_ebpf_read_process_hash_global_tables_per_core(void) {
    netdata_idx_t stats[6] = {0};
    
    ebpf_read_process_hash_global_tables(stats, 1);
}

void test_ebpf_read_process_hash_global_tables_no_per_core(void) {
    netdata_idx_t stats[6] = {0};
    
    ebpf_read_process_hash_global_tables(stats, 0);
}

void test_ebpf_update_process_cgroup_empty(void) {
    ebpf_cgroup_pids = NULL;
    
    ebpf_update_process_cgroup();
}

void test_ebpf_process_status_chart_creation(void) {
    ebpf_process_status_chart("family", "name", "axis", "web", "algorithm", 1, 10);
}

void test_ebpf_create_global_charts_full_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    em.update_every = 10;
    
    ebpf_create_global_charts(&em);
}

void test_ebpf_create_global_charts_entry_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY;
    em.update_every = 10;
    
    ebpf_create_global_charts(&em);
}

void test_ebpf_process_create_apps_charts_exposed(void) {
    ebpf_module_t em = {0};
    em.update_every = 10;
    em.mode = MODE_ENTRY + 1;
    
    ebpf_target target = {0};
    target.exposed = 1;
    target.clean_name = "test";
    target.next = NULL;
    
    ebpf_process_create_apps_charts(&em, &target);
}

void test_ebpf_process_create_apps_charts_not_exposed(void) {
    ebpf_module_t em = {0};
    em.update_every = 10;
    em.mode = MODE_ENTRY + 1;
    
    ebpf_target target = {0};
    target.exposed = 0;
    
    ebpf_process_create_apps_charts(&em, &target);
}

void test_ebpf_process_create_apps_charts_null(void) {
    ebpf_module_t em = {0};
    em.update_every = 10;
    em.mode = MODE_ENTRY + 1;
    
    ebpf_process_create_apps_charts(&em, NULL);
}

void test_ebpf_obsolete_process_services_full_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    em.update_every = 10;
    
    ebpf_obsolete_process_services(&em, "test_id");
}

void test_ebpf_obsolete_process_services_entry_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY;
    em.update_every = 10;
    
    ebpf_obsolete_process_services(&em, "test_id");
}

void test_ebpf_obsolete_process_cgroup_charts_empty(void) {
    ebpf_cgroup_pids = NULL;
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    em.update_every = 10;
    
    ebpf_obsolete_process_cgroup_charts(&em);
}

void test_ebpf_obsolete_process_apps_charts_no_charts(void) {
    ebpf_module_t em = {0};
    em.update_every = 10;
    
    ebpf_target target = {0};
    target.charts_created = 0;
    target.next = NULL;
    
    apps_groups_root_target = &target;
    
    ebpf_obsolete_process_apps_charts(&em);
}

void test_ebpf_obsolete_process_apps_charts_with_charts(void) {
    ebpf_module_t em = {0};
    em.update_every = 10;
    em.mode = MODE_ENTRY + 1;
    
    ebpf_target target = {0};
    target.charts_created = (1 << EBPF_MODULE_PROCESS_IDX);
    target.clean_name = "test";
    target.next = NULL;
    
    apps_groups_root_target = &target;
    
    ebpf_obsolete_process_apps_charts(&em);
}

void test_ebpf_obsolete_process_global_full_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    em.update_every = 10;
    
    ebpf_obsolete_process_global(&em);
}

void test_ebpf_obsolete_process_global_entry_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY;
    em.update_every = 10;
    
    ebpf_obsolete_process_global(&em);
}

void test_ebpf_process_disable_tracepoints_all_disabled(void) {
    // Mock the was_sched_process_* variables
    static int was_sched_process_exit_enabled = 0;
    static int was_sched_process_exec_enabled = 0;
    static int was_sched_process_fork_enabled = 0;
    
    ebpf_process_disable_tracepoints();
}

void test_ebpf_process_exit_normal(void) {
    ebpf_module_t em = {0};
    em.enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING;
    em.cgroup_charts = 1;
    em.apps_charts = NETDATA_EBPF_APPS_FLAG_CHART_CREATED;
    em.maps = NULL;
    em.objects = NULL;
    em.probe_links = NULL;
    
    pids_fd[NETDATA_EBPF_PIDS_PROCESS_IDX] = -1;
    
    ebpf_process_exit(&em);
}

void test_ebpf_process_exit_disabled(void) {
    ebpf_module_t em = {0};
    em.enabled = NETDATA_THREAD_EBPF_STOPPED;
    
    ebpf_process_exit(&em);
}

void test_ebpf_process_exit_no_module(void) {
    ebpf_process_exit(NULL);
}

void test_ebpf_process_sum_cgroup_pids_single_pid(void) {
    ebpf_publish_process_t ps = {0};
    pid_on_target2 pid = {0};
    pid.ps.exit_call = 10;
    pid.ps.release_call = 5;
    pid.ps.create_process = 20;
    pid.ps.create_thread = 8;
    pid.ps.task_err = 1;
    pid.next = NULL;
    
    ebpf_process_sum_cgroup_pids(&ps, &pid);
    
    assert(ps.exit_call == 10);
    assert(ps.release_call == 5);
    assert(ps.create_process == 20);
    assert(ps.create_thread == 8);
    assert(ps.task_err == 1);
}

void test_ebpf_process_sum_cgroup_pids_multiple_pids(void) {
    ebpf_publish_process_t ps = {0};
    pid_on_target2 pid1 = {0};
    pid_on_target2 pid2 = {0};
    
    pid1.ps.exit_call = 10;
    pid1.ps.release_call = 5;
    pid1.ps.create_process = 20;
    pid1.ps.create_thread = 8;
    pid1.ps.task_err = 1;
    pid1.next = &pid2;
    
    pid2.ps.exit_call = 5;
    pid2.ps.release_call = 3;
    pid2.ps.create_process = 10;
    pid2.ps.create_thread = 4;
    pid2.ps.task_err = 1;
    pid2.next = NULL;
    
    ebpf_process_sum_cgroup_pids(&ps, &pid1);
    
    // Should use max values
    assert(ps.exit_call == 10);
    assert(ps.create_process == 20);
}

void test_ebpf_process_sum_cgroup_pids_with_initial_values(void) {
    ebpf_publish_process_t ps = {
        .exit_call = 15,
        .release_call = 10,
        .create_process = 25,
        .create_thread = 12,
        .task_err = 2
    };
    
    pid_on_target2 pid = {0};
    pid.ps.exit_call = 20;
    pid.ps.release_call = 5;
    pid.ps.create_process = 30;
    pid.ps.create_thread = 8;
    pid.ps.task_err = 1;
    pid.next = NULL;
    
    ebpf_process_sum_cgroup_pids(&ps, &pid);
    
    // Should keep max
    assert(ps.exit_call == 20);
    assert(ps.create_process == 30);
}

void test_ebpf_send_specific_process_data_full_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    
    ebpf_publish_process_t values = {
        .create_process = 100,
        .create_thread = 50,
        .exit_call = 10,
        .release_call = 5,
        .task_err = 2
    };
    
    ebpf_send_specific_process_data("test_type", &values, &em);
}

void test_ebpf_send_specific_process_data_entry_mode(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY;
    
    ebpf_publish_process_t values = {0};
    
    ebpf_send_specific_process_data("test_type", &values, &em);
}

void test_ebpf_create_specific_process_charts_cgroup_type(void) {
    ebpf_module_t em = {0};
    em.mode = MODE_ENTRY + 1;
    em.update_every = 10;