#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// Mock structures and externs
typedef struct {
    uint32_t pid;
    uint64_t write_call;
    uint64_t writev_call;
    uint64_t read_call;
    uint64_t readv_call;
    uint64_t unlink_call;
    uint64_t fsync_call;
    uint64_t open_call;
    uint64_t create_call;
    uint64_t write_bytes;
    uint64_t writev_bytes;
    uint64_t read_bytes;
    uint64_t readv_bytes;
    uint64_t write_err;
    uint64_t writev_err;
    uint64_t read_err;
    uint64_t readv_err;
    uint64_t unlink_err;
    uint64_t fsync_err;
    uint64_t open_err;
    uint64_t create_err;
    uint64_t ct;
    char name[256];
} netdata_ebpf_vfs_t;

typedef struct {
    uint64_t ncall;
    uint64_t nerr;
    uint64_t bytes;
    char name[256];
} netdata_publish_syscall_t;

typedef struct {
    uint64_t write_call;
    uint64_t writev_call;
    uint64_t read_call;
    uint64_t readv_call;
    uint64_t unlink_call;
    uint64_t fsync_call;
    uint64_t open_call;
    uint64_t create_call;
    uint64_t write_bytes;
    uint64_t writev_bytes;
    uint64_t read_bytes;
    uint64_t readv_bytes;
    uint64_t write_err;
    uint64_t writev_err;
    uint64_t read_err;
    uint64_t readv_err;
    uint64_t unlink_err;
    uint64_t fsync_err;
    uint64_t open_err;
    uint64_t create_err;
    uint64_t ct;
} netdata_publish_vfs_t;

typedef struct {
    uint64_t write_call;
    uint64_t writev_call;
    uint64_t read_call;
    uint64_t readv_call;
    uint64_t unlink_call;
    uint64_t fsync_call;
    uint64_t open_call;
    uint64_t create_call;
    uint64_t write_bytes;
    uint64_t writev_bytes;
    uint64_t read_bytes;
    uint64_t readv_bytes;
    uint64_t write_err;
    uint64_t writev_err;
    uint64_t read_err;
    uint64_t readv_err;
    uint64_t unlink_err;
    uint64_t fsync_err;
    uint64_t open_err;
    uint64_t create_err;
    uint64_t ct;
} netdata_publish_vfs_common_t;

typedef struct {
    long write;
    long read;
} netdata_publish_vfs_common_t_simple;

typedef struct {
    void *next;
} ebpf_pid_on_target;

typedef struct {
    void *next;
    uint32_t pid;
    netdata_publish_vfs_t vfs;
} pid_on_target2;

typedef struct {
    void *next;
    uint32_t pid;
} ebpf_pid_on_target_simple;

typedef struct {
    int mode;
    int update_every;
    int maps_per_core;
    int cgroup_charts;
    int apps_charts;
    uint32_t lifetime;
    uint32_t running_time;
    int enabled;
    void *maps;
    void *hash_table_stats;
    void *objects;
    void *probe_links;
    void *targets;
    int load;
} ebpf_module_t;

typedef struct {
    void *next;
    int systemd;
    int updated;
    int flags;
    char name[256];
    void *pids;
    netdata_publish_vfs_t publish_systemd_vfs;
} ebpf_cgroup_target_t;

typedef struct {
    void *next;
    int exposed;
    int charts_created;
    char clean_name[256];
    char name[256];
    void *root_pid;
    netdata_publish_vfs_t vfs;
} ebpf_target;

// Mock global variables
ebpf_cgroup_target_t *ebpf_cgroup_pids = NULL;
struct ebpf_target *apps_groups_root_target = NULL;
char *ebpf_plugin_dir = "/tmp";
int ebpf_nprocs = 4;
int running_on_kernel = 5000;
int isrh = 0;
int default_btf = 1;
int send_cgroup_chart = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t collect_data_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cgroup_shm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ebpf_exit_cleanup = PTHREAD_MUTEX_INITIALIZER;
uint64_t collect_pids = 0;

struct {
    void *header;
} shm_ebpf_cgroup = {0};

sem_t *shm_mutex_ebpf_integration = NULL;

typedef struct {
    int map_fd;
} ebpf_local_maps_t;

// Helper mock functions
int __wrap_printf(const char *format, ...)
{
    return 0;
}

int __wrap_fprintf(FILE *stream, const char *format, ...)
{
    return 0;
}

void __wrap_fflush(FILE *stream)
{
}

int __wrap_netdata_mutex_lock(pthread_mutex_t *mutex)
{
    return 0;
}

int __wrap_netdata_mutex_unlock(pthread_mutex_t *mutex)
{
    return 0;
}

int __wrap_sem_wait(sem_t *sem)
{
    return 0;
}

int __wrap_sem_post(sem_t *sem)
{
    return 0;
}

int __wrap_kill(pid_t pid, int sig)
{
    return 0;
}

void __wrap_ebpf_write_chart_obsolete(char *type, char *id, char *title, char *units, char *family, char *group, char *charttype, char *context, int order, int update_every)
{
}

void __wrap_ebpf_create_chart(char *type, char *id, char *title, char *units, char *family, char *group, char *charttype, char *context, int order, void *dim_func, void *pub, int nds, int update_every, char *module)
{
}

void __wrap_ebpf_create_chart_labels(char *key, char *value, int source)
{
}

void __wrap_ebpf_commit_label(void)
{
}

void __wrap_write_count_chart(char *name, char *family, void *pub, int n)
{
}

void __wrap_write_err_chart(char *name, char *family, void *pub, int n)
{
}

void __wrap_write_io_chart(char *name, char *family, char *read_name, long long read_val, char *write_name, long long write_val)
{
}

void __wrap_write_chart_dimension(char *name, long long value)
{
}

void __wrap_ebpf_write_begin_chart(char *type, char *id, char *title)
{
}

void __wrap_ebpf_write_end_chart(void)
{
}

void __wrap_ebpf_write_chart_cmd(char *family, char *name, char *id, char *title, char *units, char *family2, char *charttype, char *context, int order, int update_every, char *module)
{
}

void __wrap_ebpf_create_charts_on_systemd(void *args)
{
}

int __wrap_bpf_map_get_next_key(int fd, void *key, void *next_key)
{
    return -1;
}

int __wrap_bpf_map_lookup_elem(int fd, void *key, void *value)
{
    return 0;
}

void __wrap_ebpf_read_global_table_stats(void *out, void *hash, int fd, int maps_per_core, int start, int end)
{
    memset(out, 0, 24 * sizeof(uint64_t));
}

void *__wrap_netdata_ebpf_get_shm_pointer_unsafe(uint32_t pid, int idx)
{
    return NULL;
}

int __wrap_netdata_ebpf_reset_shm_pointer_unsafe(int fd, uint32_t pid, int idx)
{
    return 0;
}

int __wrap_ebpf_plugin_stop(void)
{
    return 0;
}

void __wrap_heartbeat_init(void *hb, int period)
{
}

void __wrap_heartbeat_next(void *hb)
{
}

void __wrap_ebpf_update_map_size(void *map, void *maps, void *em, const char *name)
{
}

void __wrap_ebpf_update_map_type(void *map, void *maps)
{
}

void __wrap_ebpf_adjust_apps_cgroup(void *em, int mode)
{
}

void *__wrap_ebpf_load_program(char *dir, void *em, int kernel, int rh, void **objects)
{
    return (void *)1;
}

int __wrap_ebpf_vfs_load_bpf_internal(void *obj, void *em)
{
    return 0;
}

void __wrap_ebpf_update_pid_table(void *map, void *em)
{
}

void __wrap_ebpf_global_labels(void *data, void *publish, char **dim_names, char **id_names, int *algorithms, int size)
{
}

void __wrap_ebpf_update_stats(void *stats, void *em)
{
}

void __wrap_ebpf_update_kernel_memory_with_vector(void *stats, void *maps, int action)
{
}

void *__wrap_nd_thread_create(char *name, int options, void *func, void *ptr)
{
    return (void *)1;
}

void __wrap_ebpf_unload_legacy_code(void *objects, void *links)
{
}

void __wrap_ebpf_update_disabled_plugin_stats(void *em)
{
}

void __wrap_ebpf_update_controller(int fd, void *em)
{
}

void __wrap_ebpf_define_map_type(void *maps, int per_core, int kernel)
{
}

void __wrap_ebpf_adjust_thread_load(void *em, int btf)
{
}

int __wrap_nd_thread_signal_cancel(void *thread)
{
    return 0;
}

// Test for vfs_aggregate_set_vfs
static void test_vfs_aggregate_set_vfs_basic(void **state)
{
    netdata_publish_vfs_t vfs_out = {0};
    netdata_ebpf_vfs_t vfs_in = {
        .write_call = 100,
        .writev_call = 50,
        .read_call = 200,
        .readv_call = 75,
        .unlink_call = 10,
        .fsync_call = 25,
        .open_call = 150,
        .create_call = 30,
        .write_bytes = 1000,
        .writev_bytes = 500,
        .read_bytes = 2000,
        .readv_bytes = 750,
        .write_err = 5,
        .writev_err = 2,
        .read_err = 10,
        .readv_err = 3,
        .unlink_err = 1,
        .fsync_err = 2,
        .open_err = 4,
        .create_err = 1
    };

    // Call the function
    extern void vfs_aggregate_set_vfs(netdata_publish_vfs_t *vfs, netdata_ebpf_vfs_t *w);
    vfs_aggregate_set_vfs(&vfs_out, &vfs_in);

    assert_int_equal(vfs_out.write_call, 100);
    assert_int_equal(vfs_out.writev_call, 50);
    assert_int_equal(vfs_out.read_call, 200);
    assert_int_equal(vfs_out.readv_call, 75);
    assert_int_equal(vfs_out.unlink_call, 10);
    assert_int_equal(vfs_out.fsync_call, 25);
    assert_int_equal(vfs_out.open_call, 150);
    assert_int_equal(vfs_out.create_call, 30);
    assert_int_equal(vfs_out.write_bytes, 1000);
    assert_int_equal(vfs_out.writev_bytes, 500);
    assert_int_equal(vfs_out.read_bytes, 2000);
    assert_int_equal(vfs_out.readv_bytes, 750);
    assert_int_equal(vfs_out.write_err, 5);
    assert_int_equal(vfs_out.writev_err, 2);
    assert_int_equal(vfs_out.read_err, 10);
    assert_int_equal(vfs_out.readv_err, 3);
    assert_int_equal(vfs_out.unlink_err, 1);
    assert_int_equal(vfs_out.fsync_err, 2);
    assert_int_equal(vfs_out.open_err, 4);
    assert_int_equal(vfs_out.create_err, 1);
}

// Test for vfs_aggregate_set_vfs with zeros
static void test_vfs_aggregate_set_vfs_zeros(void **state)
{
    netdata_publish_vfs_t vfs_out = {0};
    netdata_ebpf_vfs_t vfs_in = {0};

    extern void vfs_aggregate_set_vfs(netdata_publish_vfs_t *vfs, netdata_ebpf_vfs_t *w);
    vfs_aggregate_set_vfs(&vfs_out, &vfs_in);

    assert_int_equal(vfs_out.write_call, 0);
    assert_int_equal(vfs_out.read_call, 0);
    assert_int_equal(vfs_out.write_bytes, 0);
}

// Test for vfs_aggregate_publish_vfs
static void test_vfs_aggregate_publish_vfs_basic(void **state)
{
    netdata_publish_vfs_t vfs_out = {
        .write_call = 100,
        .read_call = 200
    };
    netdata_publish_vfs_t vfs_in = {
        .write_call = 50,
        .read_call = 100
    };

    extern void vfs_aggregate_publish_vfs(netdata_publish_vfs_t *vfs, netdata_publish_vfs_t *w);
    vfs_aggregate_publish_vfs(&vfs_out, &vfs_in);

    assert_int_equal(vfs_out.write_call, 150);
    assert_int_equal(vfs_out.read_call, 300);
}

// Test for vfs_aggregate_publish_vfs with all fields
static void test_vfs_aggregate_publish_vfs_full(void **state)
{
    netdata_publish_vfs_t vfs_out = {
        .write_call = 10,
        .writev_call = 5,
        .read_call = 20,
        .readv_call = 8,
        .unlink_call = 1,
        .fsync_call = 2,
        .open_call = 15,
        .create_call = 3,
        .write_bytes = 100,
        .writev_bytes = 50,
        .read_bytes = 200,
        .readv_bytes = 75,
        .write_err = 1,
        .writev_err = 1,
        .read_err = 2,
        .readv_err = 1,
        .unlink_err = 0,
        .fsync_err = 1,
        .open_err = 1,
        .create_err = 0
    };
    netdata_publish_vfs_t vfs_in = {
        .write_call = 5,
        .writev_call = 2,
        .read_call = 10,
        .readv_call = 4,
        .unlink_call = 1,
        .fsync_call = 1,
        .open_call = 5,
        .create_call = 1,
        .write_bytes = 50,
        .writev_bytes = 20,
        .read_bytes = 100,
        .readv_bytes = 40,
        .write_err = 1,
        .writev_err = 0,
        .read_err = 1,
        .readv_err = 1,
        .unlink_err = 1,
        .fsync_err = 0,
        .open_err = 1,
        .create_err = 1
    };

    extern void vfs_aggregate_publish_vfs(netdata_publish_vfs_t *vfs, netdata_publish_vfs_t *w);
    vfs_aggregate_publish_vfs(&vfs_out, &vfs_in);

    assert_int_equal(vfs_out.write_call, 15);
    assert_int_equal(vfs_out.writev_call, 7);
    assert_int_equal(vfs_out.read_call, 30);
    assert_int_equal(vfs_out.readv_call, 12);
    assert_int_equal(vfs_out.unlink_call, 2);
    assert_int_equal(vfs_out.fsync_call, 3);
    assert_int_equal(vfs_out.open_call, 20);
    assert_int_equal(vfs_out.create_call, 4);
    assert_int_equal(vfs_out.write_bytes, 150);
    assert_int_equal(vfs_out.writev_bytes, 70);
    assert_int_equal(vfs_out.read_bytes, 300);
    assert_int_equal(vfs_out.readv_bytes, 115);
    assert_int_equal(vfs_out.write_err, 2);
    assert_int_equal(vfs_out.writev_err, 1);
    assert_int_equal(vfs_out.read_err, 3);
    assert_int_equal(vfs_out.readv_err, 2);
    assert_int_equal(vfs_out.unlink_err, 1);
    assert_int_equal(vfs_out.fsync_err, 1);
    assert_int_equal(vfs_out.open_err, 2);
    assert_int_equal(vfs_out.create_err, 1);
}

// Test for vfs_aggregate_publish_vfs with zero additions
static void test_vfs_aggregate_publish_vfs_zeros(void **state)
{
    netdata_publish_vfs_t vfs_out = {
        .write_call = 100,
        .read_call = 200
    };
    netdata_publish_vfs_t vfs_in = {0};

    extern void vfs_aggregate_publish_vfs(netdata_publish_vfs_t *vfs, netdata_publish_vfs_t *w);
    vfs_aggregate_publish_vfs(&vfs_out, &vfs_in);

    assert_int_equal(vfs_out.write_call, 100);
    assert_int_equal(vfs_out.read_call, 200);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_vfs_aggregate_set_vfs_basic),
        cmocka_unit_test(test_vfs_aggregate_set_vfs_zeros),
        cmocka_unit_test(test_vfs_aggregate_publish_vfs_basic),
        cmocka_unit_test(test_vfs_aggregate_publish_vfs_full),
        cmocka_unit_test(test_vfs_aggregate_publish_vfs_zeros),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}