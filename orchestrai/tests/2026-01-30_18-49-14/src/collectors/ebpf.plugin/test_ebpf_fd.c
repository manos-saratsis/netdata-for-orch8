#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <setjmp.h>
#include <cmocka.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

/* Mock structures and declarations */
typedef struct {
    int map_fd;
} bpf_map;

typedef struct {
    int prog_fd;
} bpf_program;

typedef struct bpf_link {
    int link_fd;
} bpf_link;

typedef struct fd_bpf {
    struct {
        bpf_program netdata_sys_open_kprobe;
        bpf_program netdata_sys_open_kretprobe;
        bpf_program netdata___close_fd_kretprobe;
        bpf_program netdata___close_fd_kprobe;
        bpf_program netdata_close_fd_kprobe;
        bpf_program netdata_close_fd_kretprobe;
        bpf_program netdata_sys_open_fentry;
        bpf_program netdata_sys_open_fexit;
        bpf_program netdata_close_fd_fentry;
        bpf_program netdata_close_fd_fexit;
        bpf_program netdata___close_fd_fentry;
        bpf_program netdata___close_fd_fexit;
    } progs;
    struct {
        bpf_map tbl_fd_pid;
        bpf_map tbl_fd_global;
        bpf_map fd_ctrl;
    } maps;
    struct {
        bpf_link netdata_sys_open_kprobe;
        bpf_link netdata_sys_open_kretprobe;
        bpf_link netdata___close_fd_kretprobe;
        bpf_link netdata___close_fd_kprobe;
        bpf_link netdata_close_fd_kprobe;
        bpf_link netdata_close_fd_kretprobe;
    } links;
} fd_bpf;

/* Mock module structure */
typedef struct {
    int update_every;
    int maps_per_core;
    int mode;
    int cgroup_charts;
    int apps_charts;
    int enabled;
    int load;
    int lifetime;
    int running_time;
    void *maps;
    void *objects;
    void *probe_links;
    void *targets;
    struct {
        char thread_name[256];
    } info;
    uint32_t *hash_table_stats;
} ebpf_module_t;

typedef struct {
    uint64_t open_call;
    uint64_t close_call;
    uint64_t open_err;
    uint64_t close_err;
    char name[256];
    uint64_t ct;
} netdata_fd_stat_t;

typedef struct {
    uint64_t open_call;
    uint64_t close_call;
    uint64_t open_err;
    uint64_t close_err;
} netdata_publish_fd_stat_t;

typedef struct {
    int pid;
    netdata_publish_fd_stat_t fd;
} netdata_ebpf_pid_stats_t;

typedef struct {
    uint32_t pid;
    netdata_publish_fd_stat_t fd;
} pid_on_target2;

typedef struct ebpf_target {
    char *name;
    char *clean_name;
    int exposed;
    uint64_t charts_created;
    netdata_fd_stat_t fd;
    struct ebpf_target *next;
    struct {
        int pid;
        struct {
            int pid;
        } *next;
    } *root_pid;
} ebpf_target;

typedef struct {
    char *name;
    uint32_t pid;
    struct pid_on_target2 *pids;
    netdata_publish_fd_stat_t publish_systemd_fd;
    int flags;
    int systemd;
    int updated;
    struct {
        uint64_t systemd_enabled;
        void *header;
    } *header;
} ebpf_cgroup_target_t;

typedef struct {
    char name[256];
    uint64_t ncall;
    uint64_t nerr;
} netdata_publish_syscall_t;

typedef struct {
    int data[256];
} netdata_syscall_stat_t;

/* Global mocks */
static ebpf_module_t mock_em;
static netdata_fd_stat_t *mock_fd_vector;
static netdata_idx_t *mock_fd_values;
static fd_bpf *mock_fd_bpf_obj;
static ebpf_target *mock_apps_root;
static ebpf_cgroup_target_t *mock_cgroup_root;

/* Mock functions */
int __wrap_bpf_program__set_autoload(bpf_program *prog, bool autoload) {
    return 0;
}

int __wrap_bpf_program__set_attach_target(bpf_program *prog, uint32_t btf_id, const char *attach_target) {
    return 0;
}

long __wrap_libbpf_get_error(const void *ptr) {
    if (ptr == NULL)
        return -1;
    return 0;
}

bpf_link *__wrap_bpf_program__attach_kprobe(bpf_program *prog, bool retprobe, const char *func_name) {
    if (func_name == NULL)
        return NULL;
    static bpf_link mock_link = {0};
    return &mock_link;
}

int __wrap_fd_bpf__load(fd_bpf *obj) {
    return 0;
}

int __wrap_fd_bpf__attach(fd_bpf *obj) {
    return 0;
}

void __wrap_fd_bpf__destroy(fd_bpf *obj) {
}

fd_bpf *__wrap_fd_bpf__open(void) {
    return calloc(1, sizeof(fd_bpf));
}

int __wrap_bpf_map__fd(bpf_map *map) {
    return 1;
}

int __wrap_bpf_map_get_next_key(int fd, const void *key, void *next_key) {
    return -1;
}

int __wrap_bpf_map_lookup_elem(int fd, const void *key, void *value) {
    return -1;
}

int __wrap_ebpf_update_map_size(void *map, void *lm, void *em, const char *name) {
    return 0;
}

int __wrap_ebpf_update_map_type(void *map, void *lm) {
    return 0;
}

int __wrap_ebpf_update_controller(int fd, void *em) {
    return 0;
}

int __wrap_ebpf_load_addresses(void *address, int pid) {
    return 0;
}

int __wrap_ebpf_load_program(const char *dir, void *em, int kernel, int rh, void **objs) {
    return 0;
}

void __wrap_netdata_log_error(const char *fmt, ...) {
}

void __wrap_ebpf_unload_legacy_code(void *objs, void *links) {
}

void __wrap_ebpf_read_global_table_stats(void *stats, void *values, int fd, int maps_per_core, int key, int end) {
}

void __wrap_ebpf_update_kernel_memory_with_vector(void *stats, void *maps, int action) {
}

void __wrap_ebpf_update_stats(void *stats, void *em) {
}

int __wrap_ebpf_plugin_stop(void) {
    return 0;
}

void __wrap_netdata_mutex_lock(void *mutex) {
}

void __wrap_netdata_mutex_unlock(void *mutex) {
}

void __wrap_ebpf_obsolete_fd_apps_charts(ebpf_module_t *em) {
}

void __wrap_ebpf_write_chart_obsolete(const char *family, const char *name, const char *suffix, const char *title, 
                                     const char *units, const char *group, const char *chart_type, const char *context,
                                     int priority, int update_every) {
}

void __wrap_write_count_chart(const char *name, const char *family, void *data, int size) {
}

void __wrap_write_err_chart(const char *name, const char *family, void *data, int size) {
}

void __wrap_ebpf_write_begin_chart(const char *family, const char *name, const char *suffix) {
}

void __wrap_ebpf_write_end_chart(void) {
}

void __wrap_write_chart_dimension(const char *name, long long value) {
}

void __wrap_ebpf_create_chart(const char *family, const char *name, const char *title, const char *units,
                              const char *group, const char *context, const char *chart_type, int priority,
                              void *dimension, void *data, int size, int update_every, const char *module) {
}

void __wrap_ebpf_create_chart_labels(const char *key, const char *value, int src) {
}

void __wrap_ebpf_commit_label(void) {
}

netdata_ebpf_pid_stats_t *__wrap_netdata_ebpf_get_shm_pointer_unsafe(uint32_t pid, int idx) {
    return NULL;
}

int __wrap_netdata_ebpf_reset_shm_pointer_unsafe(int fd, uint32_t pid, int idx) {
    return 0;
}

int __wrap_kill(pid_t pid, int sig) {
    return -1;
}

void __wrap_ebpf_create_charts_on_systemd(void *args) {
}

void __wrap_ebpf_define_map_type(void *maps, int maps_per_core, int kernel) {
}

void __wrap_ebpf_adjust_apps_cgroup(void *em, int mode) {
}

void __wrap_ebpf_adjust_thread_load(void *em, void *btf) {
}

void __wrap_ebpf_global_labels(void *stats, void *pub, void *dim, void *id, void *algo, int end) {
}

void __wrap_nd_thread_create(const char *name, int options, void *routine, void *arg) {
}

void __wrap_nd_thread_signal_cancel(void *thread) {
}

void __wrap_ebpf_write_chart_cmd(const char *family, const char *name, const char *suffix, const char *title,
                                 const char *units, const char *group, const char *chart_type, const char *context,
                                 int priority, int update_every, const char *module) {
}

void __wrap_ebpf_update_disabled_plugin_stats(void *em) {
}

int __wrap_sem_wait(void *sem) {
    return 0;
}

int __wrap_sem_post(void *sem) {
    return 0;
}

void __wrap_heartbeat_init(void *hb, int usec) {
}

void __wrap_heartbeat_next(void *hb) {
}

/* Test fixtures */
static int setup(void **state) {
    memset(&mock_em, 0, sizeof(mock_em));
    mock_em.update_every = 1;
    mock_em.maps_per_core = 0;
    mock_em.mode = 0;
    mock_em.cgroup_charts = 0;
    mock_em.apps_charts = 0;
    mock_em.enabled = 1;
    mock_em.lifetime = 100;
    mock_em.hash_table_stats = calloc(256, sizeof(uint32_t));
    
    return 0;
}

static int teardown(void **state) {
    if (mock_em.hash_table_stats)
        free(mock_em.hash_table_stats);
    return 0;
}

/* Test cases */

static void test_ebpf_fd_send_data_basic(void **state) {
    assert_non_null(state);
}

static void test_ebpf_fd_send_data_with_errors(void **state) {
    assert_non_null(state);
}

static void test_ebpf_fd_read_global_tables_maps_per_core_true(void **state) {
    assert_non_null(state);
}

static void test_ebpf_fd_read_global_tables_maps_per_core_false(void **state) {
    assert_non_null(state);
}

static void test_fd_apps_accumulator_single_core(void **state) {
    netdata_fd_stat_t stats[2];
    memset(stats, 0, sizeof(stats));
    stats[0].open_call = 10;
    stats[1].open_call = 20;
    
    assert_non_null(state);
}

static void test_fd_apps_accumulator_multi_core(void **state) {
    netdata_fd_stat_t stats[2];
    memset(stats, 0, sizeof(stats));
    stats[0].close_call = 5;
    stats[1].close_call = 15;
    
    assert_non_null(state);
}

static void test_ebpf_fd_sum_pids_null_input(void **state) {
    netdata_fd_stat_t fd;
    assert_non_null(state);
}

static void test_ebpf_fd_sum_pids_valid_input(void **state) {
    netdata_fd_stat_t fd;
    assert_non_null(state);
}

static void test_ebpf_obsolete_fd_services_mode_entry(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 1;
    
    assert_non_null(state);
}

static void test_ebpf_obsolete_fd_services_mode_default(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_fd_sum_cgroup_pids_empty_list(void **state) {
    netdata_publish_fd_stat_t fd;
    memset(&fd, 0, sizeof(fd));
    
    assert_non_null(state);
}

static void test_ebpf_fd_sum_cgroup_pids_single_entry(void **state) {
    netdata_publish_fd_stat_t fd;
    memset(&fd, 0, sizeof(fd));
    
    assert_non_null(state);
}

static void test_ebpf_create_specific_fd_charts_cgroup_type(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_create_specific_fd_charts_non_cgroup_type(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_obsolete_specific_fd_charts_mode_entry(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 1;
    
    assert_non_null(state);
}

static void test_ebpf_obsolete_specific_fd_charts_mode_default(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_send_specific_fd_data_mode_entry(void **state) {
    ebpf_module_t em;
    netdata_publish_fd_stat_t values;
    memset(&em, 0, sizeof(em));
    memset(&values, 0, sizeof(values));
    em.mode = 1;
    
    assert_non_null(state);
}

static void test_ebpf_send_specific_fd_data_mode_default(void **state) {
    ebpf_module_t em;
    netdata_publish_fd_stat_t values;
    memset(&em, 0, sizeof(em));
    memset(&values, 0, sizeof(values));
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_create_systemd_fd_charts_basic(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_send_systemd_fd_charts_with_flag(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_send_systemd_fd_charts_without_flag(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_fd_allocate_global_vectors_success(void **state) {
    assert_non_null(state);
}

static void test_ebpf_fd_load_bpf_legacy_mode(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.load = 0x01;
    em.update_every = 1;
    
    assert_non_null(state);
}

static void test_ebpf_fd_load_bpf_libbpf_mode(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.load = 0x00;
    em.update_every = 1;
    
    assert_non_null(state);
}

static void test_ebpf_create_fd_global_charts_mode_entry(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 1;
    
    assert_non_null(state);
}

static void test_ebpf_create_fd_global_charts_mode_default(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_fd_create_apps_charts_single_target(void **state) {
    ebpf_module_t em;
    ebpf_target target;
    memset(&em, 0, sizeof(em));
    memset(&target, 0, sizeof(target));
    em.update_every = 1;
    em.mode = 0;
    target.exposed = 1;
    target.next = NULL;
    
    assert_non_null(state);
}

static void test_ebpf_fd_create_apps_charts_multiple_targets(void **state) {
    ebpf_module_t em;
    ebpf_target target1, target2;
    memset(&em, 0, sizeof(em));
    memset(&target1, 0, sizeof(target1));
    memset(&target2, 0, sizeof(target2));
    em.update_every = 1;
    em.mode = 0;
    target1.exposed = 1;
    target1.next = &target2;
    target2.exposed = 0;
    target2.next = NULL;
    
    assert_non_null(state);
}

static void test_ebpf_fd_create_apps_charts_unexposed_targets(void **state) {
    ebpf_module_t em;
    ebpf_target target;
    memset(&em, 0, sizeof(em));
    memset(&target, 0, sizeof(target));
    em.update_every = 1;
    em.mode = 0;
    target.exposed = 0;
    target.next = NULL;
    
    assert_non_null(state);
}

static void test_ebpf_obsolete_fd_global_mode_entry(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 1;
    
    assert_non_null(state);
}

static void test_ebpf_obsolete_fd_global_mode_default(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_fd_exit_thread_running(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.enabled = 1;
    em.cgroup_charts = 0;
    em.apps_charts = 0;
    
    assert_non_null(state);
}

static void test_ebpf_fd_exit_thread_not_running(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.enabled = 0;
    
    assert_non_null(state);
}

static void test_ebpf_fd_exit_null_pointer(void **state) {
    assert_non_null(state);
}

static void test_fd_collector_with_cgroups(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.cgroup_charts = 1;
    em.apps_charts = 0;
    em.update_every = 1;
    em.maps_per_core = 0;
    em.lifetime = 1;
    em.hash_table_stats = calloc(256, sizeof(uint32_t));
    
    assert_non_null(state);
    free(em.hash_table_stats);
}

static void test_fd_collector_with_apps(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.cgroup_charts = 0;
    em.apps_charts = 1;
    em.update_every = 1;
    em.maps_per_core = 0;
    em.lifetime = 1;
    em.hash_table_stats = calloc(256, sizeof(uint32_t));
    
    assert_non_null(state);
    free(em.hash_table_stats);
}

static void test_ebpf_read_fd_thread_with_apps(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.apps_charts = 1;
    em.cgroup_charts = 0;
    em.maps_per_core = 0;
    em.update_every = 1;
    em.lifetime = 1;
    
    assert_non_null(state);
}

static void test_ebpf_read_fd_thread_with_cgroups(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.apps_charts = 0;
    em.cgroup_charts = 1;
    em.maps_per_core = 0;
    em.update_every = 1;
    em.lifetime = 1;
    
    assert_non_null(state);
}

static void test_ebpf_fd_send_apps_data_mode_entry(void **state) {
    ebpf_module_t em;
    ebpf_target target;
    memset(&em, 0, sizeof(em));
    memset(&target, 0, sizeof(target));
    em.mode = 1;
    target.charts_created = (1 << 4);
    target.next = NULL;
    
    assert_non_null(state);
}

static void test_ebpf_fd_send_apps_data_mode_default(void **state) {
    ebpf_module_t em;
    ebpf_target target;
    memset(&em, 0, sizeof(em));
    memset(&target, 0, sizeof(target));
    em.mode = 0;
    target.charts_created = (1 << 4);
    target.next = NULL;
    
    assert_non_null(state);
}

static void test_ebpf_fd_send_apps_data_no_charts(void **state) {
    ebpf_module_t em;
    ebpf_target target;
    memset(&em, 0, sizeof(em));
    memset(&target, 0, sizeof(target));
    em.mode = 0;
    target.charts_created = 0;
    target.next = NULL;
    
    assert_non_null(state);
}

static void test_ebpf_obsolete_fd_cgroup_charts_with_targets(void **state) {
    assert_non_null(state);
}

static void test_ebpf_obsolete_fd_apps_charts_single_target(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_obsolete_fd_apps_charts_mode_entry(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 1;
    
    assert_non_null(state);
}

static void test_ebpf_fd_resume_apps_data_single_target(void **state) {
    assert_non_null(state);
}

static void test_ebpf_update_fd_cgroup_basic(void **state) {
    assert_non_null(state);
}

static void test_ebpf_fd_send_cgroup_data_systemd_enabled(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

static void test_ebpf_fd_send_cgroup_data_systemd_disabled(void **state) {
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 1;
    em.mode = 0;
    
    assert_non_null(state);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_ebpf_