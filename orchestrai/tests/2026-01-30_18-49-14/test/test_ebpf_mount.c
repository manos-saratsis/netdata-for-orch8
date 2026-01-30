#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Include the source file we're testing */
#include "../src/collectors/ebpf.plugin/ebpf_mount.c"

/* Mock function declarations */
void __wrap_bpf_program__set_autoload(struct bpf_program *prog, bool autoload);
void __wrap_bpf_program__set_attach_target(struct bpf_program *prog, uint32_t btf_id, const char *target_name);
struct bpf_link *__wrap_bpf_program__attach_kprobe(struct bpf_program *prog, bool retprobe, const char *func_name);
long __wrap_libbpf_get_error(const void *ptr);
int __wrap_bpf_map_lookup_elem(int fd, const void *key, void *value);
int __wrap_bpf_map__fd(struct bpf_map *map);
int __wrap_mount_bpf__load(struct mount_bpf *obj);
int __wrap_mount_bpf__attach(struct mount_bpf *obj);
int __wrap_mount_bpf__destroy(struct mount_bpf *obj);
struct mount_bpf *__wrap_mount_bpf__open(void);
void __wrap_ebpf_select_host_prefix(char *syscall, size_t size, const char *name, int kernel);
void __wrap_ebpf_update_map_type(struct bpf_map *map, ebpf_local_maps_t *local_map);
void __wrap_ebpf_write_chart_obsolete(const char *family, const char *id, const char *dimension, 
                                      const char *title, const char *units, const char *plugin, 
                                      const char *type, const char *category, int prio, int update_every);
void __wrap_ebpf_create_chart(const char *family, const char *id, const char *title, 
                             const char *units, const char *plugin, const char *category, 
                             const char *type, int prio, void *callback, void *data, 
                             size_t size, int update_every, const char *module);
void __wrap_write_count_chart(const char *id, const char *family, void *data, size_t size);
void __wrap_write_err_chart(const char *id, const char *family, void *data, size_t size);
void __wrap_ebpf_unload_legacy_code(void *objects, void *probe_links);
void __wrap_ebpf_update_kernel_memory_with_vector(void *stats, void *maps, int action);
void __wrap_ebpf_update_stats(void *stats, void *em);
void __wrap_ebpf_update_disabled_plugin_stats(void *em);
void __wrap_fflush(FILE *stream);
int __wrap_ebpf_plugin_stop(void);
void __wrap_heartbeat_init(void *hb, uint64_t freq);
void __wrap_heartbeat_next(void *hb);
void *__wrap_ebpf_load_program(const char *dir, void *em, int kernel, int isrh, void **objects);
void __wrap_ebpf_global_labels(void *agg, void *pub, void *dim_name1, void *dim_name2, void *algorithms, size_t size);
void __wrap_ebpf_adjust_thread_load(void *em, void *btf);
void __wrap_ebpf_define_map_type(void *maps, int per_core, int kernel);
int __wrap_netdata_mutex_lock(void *mutex);
int __wrap_netdata_mutex_unlock(void *mutex);

/* Forward declarations for internal static functions */
static void test_ebpf_mount_disable_probe(void **state);
static void test_ebpf_mount_disable_tracepoint(void **state);
static void test_ebpf_mount_disable_trampoline(void **state);
static void test_netdata_set_trampoline_target(void **state);
static void test_ebpf_mount_attach_probe_success(void **state);
static void test_ebpf_mount_attach_probe_first_kprobe_failure(void **state);
static void test_ebpf_mount_attach_probe_second_kprobe_failure(void **state);
static void test_ebpf_mount_attach_probe_third_kprobe_failure(void **state);
static void test_ebpf_mount_attach_probe_fourth_kprobe_failure(void **state);
static void test_ebpf_mount_set_hash_tables(void **state);
static void test_ebpf_mount_load_and_attach_trampoline(void **state);
static void test_ebpf_mount_load_and_attach_probe(void **state);
static void test_ebpf_mount_load_and_attach_retprobe(void **state);
static void test_ebpf_mount_load_and_attach_tracepoint(void **state);
static void test_ebpf_mount_load_and_attach_load_failure(void **state);
static void test_ebpf_obsolete_mount_global(void **state);
static void test_ebpf_mount_exit_not_running(void **state);
static void test_ebpf_mount_exit_running_with_cleanup(void **state);
static void test_ebpf_mount_exit_with_legacy_objects(void **state);
static void test_ebpf_mount_read_global_table_single_core(void **state);
static void test_ebpf_mount_read_global_table_multi_core(void **state);
static void test_ebpf_mount_read_global_table_lookup_failure(void **state);
static void test_ebpf_mount_send_data(void **state);
static void test_mount_collector_lifecycle(void **state);
static void test_mount_collector_early_stop(void **state);
static void test_mount_collector_skip_update(void **state);
static void test_ebpf_create_mount_charts(void **state);
static void test_ebpf_mount_load_bpf_legacy(void **state);
static void test_ebpf_mount_load_bpf_legacy_failure(void **state);
static void test_ebpf_mount_load_bpf_modern(void **state);
static void test_ebpf_mount_load_bpf_modern_open_failure(void **state);
static void test_ebpf_mount_load_bpf_modern_load_failure(void **state);
static void test_ebpf_mount_thread_success(void **state);
static void test_ebpf_mount_thread_load_failure(void **state);

/* ============================================================================
 * Mock Implementations
 * ============================================================================ */

void __wrap_bpf_program__set_autoload(struct bpf_program *prog, bool autoload) {
    /* Track calls for verification */
}

void __wrap_bpf_program__set_attach_target(struct bpf_program *prog, uint32_t btf_id, const char *target_name) {
    /* Track calls for verification */
}

struct bpf_link *__wrap_bpf_program__attach_kprobe(struct bpf_program *prog, bool retprobe, const char *func_name) {
    struct bpf_link *link = (struct bpf_link *)test_malloc(sizeof(struct bpf_link));
    return link;
}

long __wrap_libbpf_get_error(const void *ptr) {
    return (long)mock();
}

int __wrap_bpf_map_lookup_elem(int fd, const void *key, void *value) {
    return (int)mock();
}

int __wrap_bpf_map__fd(struct bpf_map *map) {
    return (int)mock();
}

int __wrap_mount_bpf__load(struct mount_bpf *obj) {
    return (int)mock();
}

int __wrap_mount_bpf__attach(struct mount_bpf *obj) {
    return (int)mock();
}

int __wrap_mount_bpf__destroy(struct mount_bpf *obj) {
    return 0;
}

struct mount_bpf *__wrap_mount_bpf__open(void) {
    return (struct mount_bpf *)test_malloc(sizeof(struct mount_bpf));
}

void __wrap_ebpf_select_host_prefix(char *syscall, size_t size, const char *name, int kernel) {
    strncpy(syscall, name ? name : "mount", size - 1);
    syscall[size - 1] = '\0';
}

void __wrap_ebpf_update_map_type(struct bpf_map *map, ebpf_local_maps_t *local_map) {
    /* No-op for testing */
}

void __wrap_ebpf_write_chart_obsolete(const char *family, const char *id, const char *dimension,
                                      const char *title, const char *units, const char *plugin,
                                      const char *type, const char *category, int prio, int update_every) {
    check_expected(family);
    check_expected(id);
}

void __wrap_ebpf_create_chart(const char *family, const char *id, const char *title,
                             const char *units, const char *plugin, const char *category,
                             const char *type, int prio, void *callback, void *data,
                             size_t size, int update_every, const char *module) {
    check_expected(family);
    check_expected(id);
}

void __wrap_write_count_chart(const char *id, const char *family, void *data, size_t size) {
    check_expected(id);
    check_expected(family);
}

void __wrap_write_err_chart(const char *id, const char *family, void *data, size_t size) {
    check_expected(id);
    check_expected(family);
}

void __wrap_ebpf_unload_legacy_code(void *objects, void *probe_links) {
    /* No-op */
}

void __wrap_ebpf_update_kernel_memory_with_vector(void *stats, void *maps, int action) {
    /* No-op */
}

void __wrap_ebpf_update_stats(void *stats, void *em) {
    /* No-op */
}

void __wrap_ebpf_update_disabled_plugin_stats(void *em) {
    /* No-op */
}

void __wrap_fflush(FILE *stream) {
    /* No-op */
}

int __wrap_ebpf_plugin_stop(void) {
    return (int)mock();
}

void __wrap_heartbeat_init(void *hb, uint64_t freq) {
    /* No-op */
}

void __wrap_heartbeat_next(void *hb) {
    /* No-op */
}

void *__wrap_ebpf_load_program(const char *dir, void *em, int kernel, int isrh, void **objects) {
    int ret = (int)mock();
    if (ret)
        return NULL;
    if (objects)
        *objects = test_malloc(1);
    return test_malloc(1);
}

void __wrap_ebpf_global_labels(void *agg, void *pub, void *dim_name1, void *dim_name2, void *algorithms, size_t size) {
    /* No-op */
}

void __wrap_ebpf_adjust_thread_load(void *em, void *btf) {
    /* No-op */
}

void __wrap_ebpf_define_map_type(void *maps, int per_core, int kernel) {
    /* No-op */
}

int __wrap_netdata_mutex_lock(void *mutex) {
    return 0;
}

int __wrap_netdata_mutex_unlock(void *mutex) {
    return 0;
}

/* ============================================================================
 * Test Cases - Disable Functions
 * ============================================================================ */

static void test_ebpf_mount_disable_probe(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_prog = {0};
    struct bpf_program umount_prog = {0};
    struct bpf_program mount_ret_prog = {0};
    struct bpf_program umount_ret_prog = {0};

    obj.progs.netdata_mount_probe = &mount_prog;
    obj.progs.netdata_umount_probe = &umount_prog;
    obj.progs.netdata_mount_retprobe = &mount_ret_prog;
    obj.progs.netdata_umount_retprobe = &umount_ret_prog;

    expect_any_always(__wrap_bpf_program__set_autoload, prog);
    expect_any_always(__wrap_bpf_program__set_autoload, autoload);

    ebpf_mount_disable_probe(&obj);
    /* Function should complete without errors */
}

static void test_ebpf_mount_disable_tracepoint(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_exit = {0};
    struct bpf_program umount_exit = {0};

    obj.progs.netdata_mount_exit = &mount_exit;
    obj.progs.netdata_umount_exit = &umount_exit;

    expect_any_always(__wrap_bpf_program__set_autoload, prog);
    expect_any_always(__wrap_bpf_program__set_autoload, autoload);

    ebpf_mount_disable_tracepoint(&obj);
    /* Function should complete without errors */
}

static void test_ebpf_mount_disable_trampoline(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_fentry = {0};
    struct bpf_program umount_fentry = {0};
    struct bpf_program mount_fexit = {0};
    struct bpf_program umount_fexit = {0};

    obj.progs.netdata_mount_fentry = &mount_fentry;
    obj.progs.netdata_umount_fentry = &umount_fentry;
    obj.progs.netdata_mount_fexit = &mount_fexit;
    obj.progs.netdata_umount_fexit = &umount_fexit;

    expect_any_always(__wrap_bpf_program__set_autoload, prog);
    expect_any_always(__wrap_bpf_program__set_autoload, autoload);

    ebpf_mount_disable_trampoline(&obj);
    /* Function should complete without errors */
}

static void test_netdata_set_trampoline_target(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_fentry = {0};
    struct bpf_program umount_fentry = {0};
    struct bpf_program mount_fexit = {0};
    struct bpf_program umount_fexit = {0};

    obj.progs.netdata_mount_fentry = &mount_fentry;
    obj.progs.netdata_umount_fentry = &umount_fentry;
    obj.progs.netdata_mount_fexit = &mount_fexit;
    obj.progs.netdata_umount_fexit = &umount_fexit;

    expect_any_always(__wrap_bpf_program__set_attach_target, prog);
    expect_any_always(__wrap_bpf_program__set_attach_target, btf_id);
    expect_any_always(__wrap_bpf_program__set_attach_target, target_name);

    netdata_set_trampoline_target(&obj);
    /* Function should complete without errors */
}

/* ============================================================================
 * Test Cases - Attach Probe
 * ============================================================================ */

static void test_ebpf_mount_attach_probe_success(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_probe = {0};
    struct bpf_program mount_ret_probe = {0};
    struct bpf_program umount_probe = {0};
    struct bpf_program umount_ret_probe = {0};

    obj.progs.netdata_mount_probe = &mount_probe;
    obj.progs.netdata_mount_retprobe = &mount_ret_probe;
    obj.progs.netdata_umount_probe = &umount_probe;
    obj.progs.netdata_umount_retprobe = &umount_ret_probe;

    /* Mock all 4 kprobe attachments to succeed */
    will_return_count(__wrap_libbpf_get_error, 0, 4);

    int result = ebpf_mount_attach_probe(&obj);
    assert_int_equal(result, 0);
}

static void test_ebpf_mount_attach_probe_first_kprobe_failure(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_probe = {0};
    struct bpf_program mount_ret_probe = {0};
    struct bpf_program umount_probe = {0};
    struct bpf_program umount_ret_probe = {0};

    obj.progs.netdata_mount_probe = &mount_probe;
    obj.progs.netdata_mount_retprobe = &mount_ret_probe;
    obj.progs.netdata_umount_probe = &umount_probe;
    obj.progs.netdata_umount_retprobe = &umount_ret_probe;

    /* First kprobe attachment fails */
    will_return(__wrap_libbpf_get_error, -1);

    int result = ebpf_mount_attach_probe(&obj);
    assert_int_equal(result, -1);
}

static void test_ebpf_mount_attach_probe_second_kprobe_failure(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_probe = {0};
    struct bpf_program mount_ret_probe = {0};
    struct bpf_program umount_probe = {0};
    struct bpf_program umount_ret_probe = {0};

    obj.progs.netdata_mount_probe = &mount_probe;
    obj.progs.netdata_mount_retprobe = &mount_ret_probe;
    obj.progs.netdata_umount_probe = &umount_probe;
    obj.progs.netdata_umount_retprobe = &umount_ret_probe;

    /* First succeeds, second fails */
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, -1);

    int result = ebpf_mount_attach_probe(&obj);
    assert_int_equal(result, -1);
}

static void test_ebpf_mount_attach_probe_third_kprobe_failure(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_probe = {0};
    struct bpf_program mount_ret_probe = {0};
    struct bpf_program umount_probe = {0};
    struct bpf_program umount_ret_probe = {0};

    obj.progs.netdata_mount_probe = &mount_probe;
    obj.progs.netdata_mount_retprobe = &mount_ret_probe;
    obj.progs.netdata_umount_probe = &umount_probe;
    obj.progs.netdata_umount_retprobe = &umount_ret_probe;

    /* First two succeed, third fails */
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, -1);

    int result = ebpf_mount_attach_probe(&obj);
    assert_int_equal(result, -1);
}

static void test_ebpf_mount_attach_probe_fourth_kprobe_failure(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_probe = {0};
    struct bpf_program mount_ret_probe = {0};
    struct bpf_program umount_probe = {0};
    struct bpf_program umount_ret_probe = {0};

    obj.progs.netdata_mount_probe = &mount_probe;
    obj.progs.netdata_mount_retprobe = &mount_ret_probe;
    obj.progs.netdata_umount_probe = &umount_probe;
    obj.progs.netdata_umount_retprobe = &umount_ret_probe;

    /* First three succeed, fourth fails */
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, -1);

    int result = ebpf_mount_attach_probe(&obj);
    assert_int_equal(result, -1);
}

/* ============================================================================
 * Test Cases - Set Hash Tables
 * ============================================================================ */

static void test_ebpf_mount_set_hash_tables(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_map tbl_mount = {0};
    obj.maps.tbl_mount = &tbl_mount;

    will_return(__wrap_bpf_map__fd, 42);

    ebpf_mount_set_hash_tables(&obj);

    assert_int_equal(mount_maps[NETDATA_KEY_MOUNT_TABLE].map_fd, 42);
}

/* ============================================================================
 * Test Cases - Load and Attach
 * ============================================================================ */

static void test_ebpf_mount_load_and_attach_trampoline(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_fentry = {0};
    struct bpf_program umount_fentry = {0};
    struct bpf_program mount_fexit = {0};
    struct bpf_program umount_fexit = {0};
    struct bpf_program mount_probe = {0};
    struct bpf_program umount_probe = {0};
    struct bpf_program mount_ret_probe = {0};
    struct bpf_program umount_ret_probe = {0};
    struct bpf_program mount_exit = {0};
    struct bpf_program umount_exit = {0};
    struct bpf_map tbl_mount = {0};

    obj.progs.netdata_mount_fentry = &mount_fentry;
    obj.progs.netdata_umount_fentry = &umount_fentry;
    obj.progs.netdata_mount_fexit = &mount_fexit;
    obj.progs.netdata_umount_fexit = &umount_fexit;
    obj.progs.netdata_mount_probe = &mount_probe;
    obj.progs.netdata_umount_probe = &umount_probe;
    obj.progs.netdata_mount_retprobe = &mount_ret_probe;
    obj.progs.netdata_umount_retprobe = &umount_ret_probe;
    obj.progs.netdata_mount_exit = &mount_exit;
    obj.progs.netdata_umount_exit = &umount_exit;
    obj.maps.tbl_mount = &tbl_mount;

    ebpf_module_t em = {0};
    netdata_ebpf_targets_t targets[] = {
        {.name = "mount", .mode = EBPF_LOAD_TRAMPOLINE},
        {.name = "umount", .mode = EBPF_LOAD_TRAMPOLINE},
        {.name = NULL, .mode = EBPF_LOAD_TRAMPOLINE}
    };
    em.targets = targets;

    expect_any_always(__wrap_bpf_program__set_autoload, prog);
    expect_any_always(__wrap_bpf_program__set_autoload, autoload);
    expect_any_always(__wrap_bpf_program__set_attach_target, prog);
    expect_any_always(__wrap_bpf_program__set_attach_target, btf_id);
    expect_any_always(__wrap_bpf_program__set_attach_target, target_name);

    will_return(__wrap_mount_bpf__load, 0);
    will_return(__wrap_mount_bpf__attach, 0);
    will_return(__wrap_bpf_map__fd, 50);

    int result = ebpf_mount_load_and_attach(&obj, &em);
    assert_int_equal(result, 0);
}

static void test_ebpf_mount_load_and_attach_probe(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_fentry = {0};
    struct bpf_program umount_fentry = {0};
    struct bpf_program mount_fexit = {0};
    struct bpf_program umount_fexit = {0};
    struct bpf_program mount_probe = {0};
    struct bpf_program umount_probe = {0};
    struct bpf_program mount_ret_probe = {0};
    struct bpf_program umount_ret_probe = {0};
    struct bpf_program mount_exit = {0};
    struct bpf_program umount_exit = {0};
    struct bpf_map tbl_mount = {0};

    obj.progs.netdata_mount_fentry = &mount_fentry;
    obj.progs.netdata_umount_fentry = &umount_fentry;
    obj.progs.netdata_mount_fexit = &mount_fexit;
    obj.progs.netdata_umount_fexit = &umount_fexit;
    obj.progs.netdata_mount_probe = &mount_probe;
    obj.progs.netdata_umount_probe = &umount_probe;
    obj.progs.netdata_mount_retprobe = &mount_ret_probe;
    obj.progs.netdata_umount_retprobe = &umount_ret_probe;
    obj.progs.netdata_mount_exit = &mount_exit;
    obj.progs.netdata_umount_exit = &umount_exit;
    obj.maps.tbl_mount = &tbl_mount;

    ebpf_module_t em = {0};
    netdata_ebpf_targets_t targets[] = {
        {.name = "mount", .mode = EBPF_LOAD_PROBE},
        {.name = "umount", .mode = EBPF_LOAD_PROBE},
        {.name = NULL, .mode = EBPF_LOAD_PROBE}
    };
    em.targets = targets;

    expect_any_always(__wrap_bpf_program__set_autoload, prog);
    expect_any_always(__wrap_bpf_program__set_autoload, autoload);

    will_return(__wrap_mount_bpf__load, 0);
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_libbpf_get_error, 0);
    will_return(__wrap_bpf_map__fd, 50);

    int result = ebpf_mount_load_and_attach(&obj, &em);
    assert_int_equal(result, 0);
}

static void test_ebpf_mount_load_and_attach_retprobe(void **state) {
    (void)state;

    struct mount_bpf obj = {0};
    struct bpf_program mount_fentry = {0};
    struct bpf_program umount_fentry = {0};
    struct bpf_program mount_fexit = {0};
    struct bpf_program umount_fexit = {0};
    struct bpf_program mount_probe = {0};
    struct bpf_program umount_probe = {0};
    struct bpf_program mount_ret_probe = {0};
    struct bpf_program umount_ret_probe = {0};
    struct bpf_program mount_exit = {0};
    struct bpf_program