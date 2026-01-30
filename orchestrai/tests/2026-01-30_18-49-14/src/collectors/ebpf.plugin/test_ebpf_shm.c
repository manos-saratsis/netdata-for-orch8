#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <cmocka.h>

/* Mock includes and definitions */
#include "../ebpf.h"
#include "ebpf_shm.h"

/* Global mocks */
static jmp_buf test_jump_buffer;

/* Forward declarations */
extern netdata_ebpf_shm_t *shm_vector;
extern netdata_idx_t shm_hash_values[];
extern netdata_idx_t *shm_values;
extern struct config shm_config;
extern ebpf_local_maps_t shm_maps[];
extern netdata_ebpf_targets_t shm_targets[];
extern struct netdata_static_thread ebpf_read_shm;
extern netdata_syscall_stat_t shm_aggregated_data[];
extern netdata_publish_syscall_t shm_publish_aggregated[];

/* Mock variables */
static int mock_bpf_program_set_autoload_called = 0;
static int mock_bpf_program_set_attach_target_called = 0;
static int mock_bpf_program_attach_kprobe_called = 0;
static int mock_bpf_program_attach_kprobe_return_value = 0;
static int mock_bpf_map_get_next_key_return = 0;
static int mock_bpf_map_lookup_elem_return = 0;
static int mock_bpf_map_update_elem_return = 0;

/* Mock structures for testing */
struct mock_bpf_program {
    int autoload;
    char attach_target[256];
};

struct mock_shm_bpf {
    struct {
        struct mock_bpf_program netdata_syscall_shmget;
        struct mock_bpf_program netdata_syscall_shmat;
        struct mock_bpf_program netdata_syscall_shmdt;
        struct mock_bpf_program netdata_syscall_shmctl;
        struct mock_bpf_program netdata_shmget_probe;
        struct mock_bpf_program netdata_shmat_probe;
        struct mock_bpf_program netdata_shmdt_probe;
        struct mock_bpf_program netdata_shmctl_probe;
        struct mock_bpf_program netdata_shmget_fentry;
        struct mock_bpf_program netdata_shmat_fentry;
        struct mock_bpf_program netdata_shmdt_fentry;
        struct mock_bpf_program netdata_shmctl_fentry;
    } progs;
    struct {
        int tbl_pid_shm;
        int shm_ctrl;
        int tbl_shm;
    } maps;
    struct {
        void *netdata_shmget_probe;
        void *netdata_shmat_probe;
        void *netdata_shmdt_probe;
        void *netdata_shmctl_probe;
    } links;
};

/* Mock functions */
void bpf_program__set_autoload(void *prog, bool autoload) {
    mock_bpf_program_set_autoload_called++;
}

void bpf_program__set_attach_target(void *prog, int btf_id, const char *target_name) {
    mock_bpf_program_set_attach_target_called++;
}

void *bpf_program__attach_kprobe(void *prog, bool retprobe, const char *func_name) {
    mock_bpf_program_attach_kprobe_called++;
    if (mock_bpf_program_attach_kprobe_return_value != 0) {
        return (void *)(intptr_t)mock_bpf_program_attach_kprobe_return_value;
    }
    return (void *)1;
}

long libbpf_get_error(const void *ptr) {
    if (ptr == NULL)
        return -EINVAL;
    if ((intptr_t)ptr < 0)
        return (intptr_t)ptr;
    return 0;
}

int bpf_map_get_next_key(int fd, const void *key, void *next_key) {
    return mock_bpf_map_get_next_key_return;
}

int bpf_map_lookup_elem(int fd, const void *key, void *value) {
    return mock_bpf_map_lookup_elem_return;
}

int bpf_map_update_elem(int fd, const void *key, const void *value, __u64 flags) {
    return mock_bpf_map_update_elem_return;
}

int shm_bpf__load(struct shm_bpf *obj) {
    return 0;
}

int shm_bpf__attach(struct shm_bpf *obj) {
    return 0;
}

void shm_bpf__destroy(struct shm_bpf *obj) {
}

struct shm_bpf *shm_bpf__open(void) {
    return (struct shm_bpf *)calloc(1, sizeof(struct shm_bpf));
}

int bpf_map__fd(void *map) {
    return 42;
}

const char *bpf_map__name(void *map) {
    return "test_map";
}

void ebpf_select_host_prefix(char *output, size_t size, const char *syscall, int is_64bit) {
    snprintf(output, size, "%s", syscall);
}

int kill(pid_t pid, int sig) {
    return 0;
}

void ebpf_update_map_size(void *map, ebpf_local_maps_t *local_map,
                          ebpf_module_t *em, const char *map_name) {
}

void ebpf_update_map_type(void *map, ebpf_local_maps_t *local_map) {
}

void ebpf_write_chart_obsolete(const char *id, const char *chart, const char *suffix,
                              const char *title, const char *units, const char *family,
                              const char *charttype, const char *context, int priority,
                              int update_every) {
}

void ebpf_write_begin_chart(const char *family, const char *name, const char *axis_label) {
}

void write_chart_dimension(const char *name, long long value) {
}

void ebpf_write_end_chart(void) {
}

void ebpf_create_chart(const char *family, const char *name, const char *title,
                      const char *units, const char *family_submenu, const char *context,
                      const char *charttype, int priority,
                      void (*callback)(void *, void *), void *data, int num_dimensions,
                      int update_every, const char *module) {
}

void ebpf_create_chart_labels(const char *key, const char *value, int src) {
}

void ebpf_commit_label(void) {
}

void ebpf_cgroup_target_t_initialize(ebpf_cgroup_target_t *ect) {
    if (ect) {
        ect->name = NULL;
        ect->pids = NULL;
        ect->next = NULL;
        ect->systemd = 0;
        ect->flags = 0;
        ect->updated = 0;
        memset(&ect->publish_shm, 0, sizeof(ect->publish_shm));
    }
}

ebpf_cgroup_target_t *ebpf_cgroup_pids = NULL;
netdata_mutex_t mutex_cgroup_shm = NETDATA_MUTEX_INITIALIZER;
netdata_ebpf_cgroup_shm_t shm_ebpf_cgroup = {0};

int send_cgroup_chart = 0;

int ebpf_plugin_stop(void) {
    return 0;
}

void ebpf_read_global_table_stats(netdata_idx_t *hash_values, netdata_idx_t *values,
                                 int map_fd, int maps_per_core, int start_idx, int end_idx) {
}

netdata_ebpf_pid_stats_t *netdata_ebpf_get_shm_pointer_unsafe(uint32_t pid, int index) {
    return NULL;
}

int netdata_ebpf_reset_shm_pointer_unsafe(int fd, uint32_t pid, int index) {
    return 0;
}

struct ebpf_target *apps_groups_root_target = NULL;
netdata_mutex_t collect_data_mutex = NETDATA_MUTEX_INITIALIZER;
netdata_mutex_t lock = NETDATA_MUTEX_INITIALIZER;
netdata_mutex_t ebpf_exit_cleanup = NETDATA_MUTEX_INITIALIZER;

int ebpf_module_flag = NETDATA_THREAD_EBPF_FUNCTION_RUNNING;
int running_on_kernel = 0;
int isrh = 0;
int ebpf_nprocs = 1;
char *ebpf_plugin_dir = "/tmp";
plugin_statistics_t plugin_statistics = {0};

void ebpf_write_chart_cmd(const char *family, const char *name, const char *suffix,
                         const char *title, const char *units, const char *family_submenu,
                         const char *charttype, const char *context, int priority,
                         int update_every, const char *module) {
}

extern const char *ebpf_algorithms[];

void ebpf_create_chart_labels_for_dimension(const char *key, const char *value, int src) {
}

void ebpf_global_labels(netdata_syscall_stat_t *stats, netdata_publish_syscall_t *publish,
                       char **dim_names, char **names, int *algorithms, int end) {
}

void ebpf_create_global_dimension(void *ptr, void *data) {
}

void ebpf_update_stats(plugin_statistics_t *stats, ebpf_module_t *em) {
}

void ebpf_update_kernel_memory_with_vector(plugin_statistics_t *stats,
                                          ebpf_local_maps_t *maps,
                                          int action) {
}

void nd_thread_signal_cancel(netdata_thread_t thread) {
}

netdata_thread_t nd_thread_create(const char *tag, int options,
                                 void *(*start_routine)(void *), void *arg) {
    return (netdata_thread_t)1;
}

void ebpf_adjust_apps_cgroup(ebpf_module_t *em, int mode) {
}

void *ebpf_load_program(const char *dir, ebpf_module_t *em, int kernel, int isrh,
                       struct bpf_object **objects) {
    return NULL;
}

void ebpf_unload_legacy_code(struct bpf_object *obj, struct bpf_link **links) {
}

void netdata_log_error(const char *fmt, ...) {
}

void ebpf_define_map_type(ebpf_local_maps_t *maps, int maps_per_core, int kernel) {
}

void ebpf_adjust_thread_load(ebpf_module_t *em, int btf) {
}

void ebpf_update_pid_table(ebpf_local_maps_t *maps, ebpf_module_t *em) {
}

void ebpf_update_disabled_plugin_stats(ebpf_module_t *em) {
}

void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void freez(void *ptr) {
    free(ptr);
}

void heartbeat_init(void *hb, useconds_t period) {
}

void heartbeat_next(void *hb) {
}

void sem_wait(void *sem) {
}

void sem_post(void *sem) {
}

void *shm_mutex_ebpf_integration = NULL;
int pids_fd[] = {[NETDATA_EBPF_PIDS_SHM_IDX] = -1};

/* Test utilities */
static void test_setup(void **state) {
    mock_bpf_program_set_autoload_called = 0;
    mock_bpf_program_set_attach_target_called = 0;
    mock_bpf_program_attach_kprobe_called = 0;
    mock_bpf_program_attach_kprobe_return_value = 0;
    mock_bpf_map_get_next_key_return = -1;
    mock_bpf_map_lookup_elem_return = 0;
    mock_bpf_map_update_elem_return = 0;
}

static void test_teardown(void **state) {
}

/* ========================================================================
 * Tests for ebpf_shm_disable_tracepoint
 * ======================================================================== */

static void test_ebpf_shm_disable_tracepoint_called(void **state)
{
    struct shm_bpf obj;
    memset(&obj, 0, sizeof(obj));
    
    ebpf_shm_disable_tracepoint((struct shm_bpf *)&obj);
    
    /* Verify that bpf_program__set_autoload was called 4 times */
    assert_int_equal(mock_bpf_program_set_autoload_called, 4);
}

/* ========================================================================
 * Tests for ebpf_disable_probe
 * ======================================================================== */

static void test_ebpf_disable_probe_called(void **state)
{
    struct shm_bpf obj;
    memset(&obj, 0, sizeof(obj));
    
    mock_bpf_program_set_autoload_called = 0;
    ebpf_disable_probe((struct shm_bpf *)&obj);
    
    assert_int_equal(mock_bpf_program_set_autoload_called, 4);
}

/* ========================================================================
 * Tests for ebpf_disable_trampoline
 * ======================================================================== */

static void test_ebpf_disable_trampoline_called(void **state)
{
    struct shm_bpf obj;
    memset(&obj, 0, sizeof(obj));
    
    mock_bpf_program_set_autoload_called = 0;
    ebpf_disable_trampoline((struct shm_bpf *)&obj);
    
    assert_int_equal(mock_bpf_program_set_autoload_called, 4);
}

/* ========================================================================
 * Tests for ebpf_set_trampoline_target
 * ======================================================================== */

static void test_ebpf_set_trampoline_target_called(void **state)
{
    struct shm_bpf obj;
    memset(&obj, 0, sizeof(obj));
    
    mock_bpf_program_set_attach_target_called = 0;
    ebpf_set_trampoline_target((struct shm_bpf *)&obj);
    
    assert_int_equal(mock_bpf_program_set_attach_target_called, 4);
}

/* ========================================================================
 * Tests for ebpf_shm_attach_probe
 * ======================================================================== */

static void test_ebpf_shm_attach_probe_success(void **state)
{
    struct shm_bpf obj;
    memset(&obj, 0, sizeof(obj));
    
    mock_bpf_program_attach_kprobe_return_value = 0;
    int ret = ebpf_shm_attach_probe((struct shm_bpf *)&obj);
    
    assert_int_equal(ret, 0);
    assert_int_equal(mock_bpf_program_attach_kprobe_called, 4);
}

static void test_ebpf_shm_attach_probe_first_failure(void **state)
{
    struct shm_bpf obj;
    memset(&obj, 0, sizeof(obj));
    
    /* Setup to fail on first kprobe attachment */
    mock_bpf_program_attach_kprobe_return_value = -EINVAL;
    int ret = ebpf_shm_attach_probe((struct shm_bpf *)&obj);
    
    assert_int_equal(ret, -1);
}

/* ========================================================================
 * Tests for ebpf_shm_set_hash_tables
 * ======================================================================== */

static void test_ebpf_shm_set_hash_tables_called(void **state)
{
    struct shm_bpf obj;
    memset(&obj, 0, sizeof(obj));
    
    ebpf_shm_set_hash_tables((struct shm_bpf *)&obj);
    
    /* Verify map file descriptors were set */
    assert_int_equal(shm_maps[NETDATA_PID_SHM_TABLE].map_fd, 42);
    assert_int_equal(shm_maps[NETDATA_SHM_CONTROLLER].map_fd, 42);
    assert_int_equal(shm_maps[NETDATA_SHM_GLOBAL_TABLE].map_fd, 42);
}

/* ========================================================================
 * Tests for ebpf_shm_adjust_map
 * ======================================================================== */

static void test_ebpf_shm_adjust_map_called(void **state)
{
    struct shm_bpf obj;
    ebpf_module_t em;
    
    memset(&obj, 0, sizeof(obj));
    memset(&em, 0, sizeof(em));
    
    ebpf_shm_adjust_map((struct shm_bpf *)&obj, &em);
}

/* ========================================================================
 * Tests for ebpf_shm_load_and_attach
 * ======================================================================== */

static void test_ebpf_shm_load_and_attach_trampoline_mode(void **state)
{
    struct shm_bpf obj;
    ebpf_module_t em;
    
    memset(&obj, 0, sizeof(obj));
    memset(&em, 0, sizeof(em));
    em.targets = shm_targets;
    
    int ret = ebpf_shm_load_and_attach((struct shm_bpf *)&obj, &em);
    
    assert_int_equal(ret, 0);
}

static void test_ebpf_shm_load_and_attach_probe_mode(void **state)
{
    struct shm_bpf obj;
    ebpf_module_t em;
    netdata_ebpf_targets_t targets[5] = {
        {.name = "shmget", .mode = EBPF_LOAD_PROBE},
        {.name = "shmat", .mode = EBPF_LOAD_PROBE},
        {.name = "shmdt", .mode = EBPF_LOAD_PROBE},
        {.name = "shmctl", .mode = EBPF_LOAD_PROBE},
        {.name = NULL}
    };
    
    memset(&obj, 0, sizeof(obj));
    memset(&em, 0, sizeof(em));
    em.targets = targets;
    
    mock_bpf_program_attach_kprobe_return_value = 0;
    int ret = ebpf_shm_load_and_attach((struct shm_bpf *)&obj, &em);
    
    assert_int_equal(ret, 0);
}

static void test_ebpf_shm_load_and_attach_retprobe_mode(void **state)
{
    struct shm_bpf obj;
    ebpf_module_t em;
    netdata_ebpf_targets_t targets[5] = {
        {.name = "shmget", .mode = EBPF_LOAD_RETPROBE},
        {.name = "shmat", .mode = EBPF_LOAD_RETPROBE},
        {.name = "shmdt", .mode = EBPF_LOAD_RETPROBE},
        {.name = "shmctl", .mode = EBPF_LOAD_RETPROBE},
        {.name = NULL}
    };
    
    memset(&obj, 0, sizeof(obj));
    memset(&em, 0, sizeof(em));
    em.targets = targets;
    
    mock_bpf_program_attach_kprobe_return_value = 0;
    int ret = ebpf_shm_load_and_attach((struct shm_bpf *)&obj, &em);
    
    assert_int_equal(ret, 0);
}

static void test_ebpf_shm_load_and_attach_tracepoint_mode(void **state)
{
    struct shm_bpf obj;
    ebpf_module_t em;
    netdata_ebpf_targets_t targets[5] = {
        {.name = "shmget", .mode = EBPF_LOAD_TRACEPOINT},
        {.name = "shmat", .mode = EBPF_LOAD_TRACEPOINT},
        {.name = "shmdt", .mode = EBPF_LOAD_TRACEPOINT},
        {.name = "shmctl", .mode = EBPF_LOAD_TRACEPOINT},
        {.name = NULL}
    };
    
    memset(&obj, 0, sizeof(obj));
    memset(&em, 0, sizeof(em));
    em.targets = targets;
    
    int ret = ebpf_shm_load_and_attach((struct shm_bpf *)&obj, &em);
    
    assert_int_equal(ret, 0);
}

/* ========================================================================
 * Tests for ebpf_obsolete_shm_services
 * ======================================================================== */

static void test_ebpf_obsolete_shm_services_called(void **state)
{
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 10;
    
    ebpf_obsolete_shm_services(&em, "test_id");
}

/* ========================================================================
 * Tests for ebpf_obsolete_shm_cgroup_charts
 * ======================================================================== */

static void test_ebpf_obsolete_shm_cgroup_charts_empty(void **state)
{
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 10;
    
    ebpf_obsolete_shm_cgroup_charts(&em);
}

static void test_ebpf_obsolete_shm_cgroup_charts_with_targets(void **state)
{
    ebpf_module_t em;
    ebpf_cgroup_target_t ect;
    
    memset(&em, 0, sizeof(em));
    memset(&ect, 0, sizeof(ect));
    em.update_every = 10;
    ect.name = "test_cgroup";
    ect.systemd = 0;
    ebpf_cgroup_pids = &ect;
    
    ebpf_obsolete_shm_cgroup_charts(&em);
    
    ebpf_cgroup_pids = NULL;
}

/* ========================================================================
 * Tests for ebpf_obsolete_shm_apps_charts
 * ======================================================================== */

static void test_ebpf_obsolete_shm_apps_charts_no_targets(void **state)
{
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.update_every = 10;
    
    ebpf_obsolete_shm_apps_charts(&em);
}

/* ========================================================================
 * Tests for shm_apps_accumulator
 * ======================================================================== */

static void test_shm_apps_accumulator_single_core(void **state)
{
    netdata_ebpf_shm_t data[2];
    memset(data, 0, sizeof(data));
    
    data[0].get = 10;
    data[0].at = 20;
    data[0].dt = 30;
    data[0].ctl = 40;
    data[0].ct = 1;
    strcpy(data[0].name, "test");
    
    data[1].get = 5;
    data[1].at = 10;
    data[1].dt = 15;
    data[1].ctl = 20;
    data[1].ct = 2;
    strcpy(data[1].name, "");
    
    shm_apps_accumulator(data, 1);
    
    assert_int_equal(data[0].get, 10);
    assert_int_equal(data[0].at, 20);
    assert_int_equal(data[0].dt, 30);
    assert_int_equal(data[0].ctl, 40);
    assert_int_equal(data[0].ct, 1);
}

static void test_shm_apps_accumulator_multi_core(void **state)
{
    netdata_ebpf_shm_t data[2];
    memset(data, 0, sizeof(data));
    
    data[0].get = 10;
    data[0].at = 20;
    data[0].dt = 30;
    data[0].ctl = 40;
    data[0].ct = 1;
    strcpy(data[0].name, "test");
    
    data[1].get = 5;
    data[1].at = 10;
    data[1].dt = 15;
    data[1].ctl = 20;
    data[1].ct = 2;
    strcpy(data[1].name, "other");
    
    ebpf_nprocs = 2;
    shm_apps_accumulator(data, 1);
    ebpf_nprocs = 1;
    
    assert_int_equal(data[0].get, 15);
    assert_int_equal(data[0].at, 30);
    assert_int_equal(data[0].dt, 45);
    assert_int_equal(data[0].ctl, 60);
    assert_int_equal(data[0].ct, 2);
}

/* ========================================================================
 * Tests for ebpf_update_shm_cgroup
 * ======================================================================== */

static void test_ebpf_update_shm_cgroup_empty(void **state)
{
    ebpf_update_shm_cgroup();
}

/* ========================================================================
 * Tests for ebpf_read_shm_apps_table
 * ======================================================================== */

static void test_ebpf_read_shm_apps_table_empty(void **state)
{
    shm_maps[NETDATA_PID_SHM_TABLE].map_fd = 1;
    mock_bpf_map_get_next_key_return = -1;
    
    ebpf_read_shm_apps_table(0);
}

static void test_ebpf_read_shm_apps_table_with_key(void **state)
{
    shm_maps[NETDATA_PID_SHM_TABLE].map_fd = 1;
    mock_bpf_map_get_next_key_return = -1;
    mock_bpf_map_lookup_elem_return = 0;
    
    ebpf_read_shm_apps_table(0);
}

/* ========================================================================
 * Tests for shm_send_global
 * ======================================================================== */

static void test_shm_send_global_called(void **state)
{
    shm_send_global();
}

/* ========================================================================
 * Tests for ebpf_shm_read_global_table
 * ======================================================================== */

static void test_ebpf_shm_rea