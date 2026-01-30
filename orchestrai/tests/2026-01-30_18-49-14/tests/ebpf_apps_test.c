#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Mock definitions */
#define MOCK_MAX_PIDS 10

/* Mock structures and declarations */
typedef struct {
    void *ptr;
    size_t size;
} mock_alloc_t;

/* Mock implementations */
static int mock_bpf_map_lookup_elem_called = 0;
static int mock_bpf_map_lookup_elem_result = 0;

int bpf_map_lookup_elem(int fd, void *key, void *value) {
    mock_bpf_map_lookup_elem_called++;
    return mock_bpf_map_lookup_elem_result;
}

void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void freez(void *ptr) {
    free(ptr);
}

void *mallocz(size_t size) {
    return malloc(size);
}

void netdata_log_error(const char *fmt, ...) {
}

void netdata_log_info(const char *fmt, ...) {
}

void netdata_fix_chart_name(char *s) {
}

uint32_t simple_hash(const char *ptr) {
    uint32_t hash = 0;
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    return hash;
}

size_t str2pid_t(const char *str) {
    return strtol(str, NULL, 10);
}

void strncpyz(char *dest, const char *src, size_t maxlen) {
    strncpy(dest, src, maxlen - 1);
    dest[maxlen - 1] = '\0';
}

void internal_fatal(int condition, const char *fmt, ...) {
    if (condition) {
        abort();
    }
}

void fatal(const char *fmt, ...) {
    abort();
}

typedef void ARAL;

/* Judy array mock */
typedef void* Pvoid_t;
typedef unsigned long Word_t;

#define PJE0 ((PJE_t)-1)
#define PJERR ((PJE_t)-2)
typedef long PJE_t;

Pvoid_t JudyLGet(Pvoid_t PArray, Word_t Index, PJE_t *PJError) {
    if (PJError) *PJError = PJE0;
    return NULL;
}

Pvoid_t *JudyLIns(Pvoid_t *PPArray, Word_t Index, PJE_t *PJError) {
    if (PJError) *PJError = PJE0;
    static Pvoid_t result = NULL;
    return &result;
}

void JudyLDel(Pvoid_t *PPArray, Word_t Index, PJE_t *PJError) {
    if (PJError) *PJError = PJE0;
}

Pvoid_t *JudyLFirstThenNext(Pvoid_t PArray, Word_t *PIndex, bool *PFirstPass) {
    return NULL;
}

void JudyLFreeArray(Pvoid_t *PPArray, PJE_t *PJError) {
    if (PJError) *PJError = PJE0;
}

int aral_freez(void *aral, void *ptr) {
    free(ptr);
    return 0;
}

typedef struct {
    Pvoid_t JudyLArray;
} netdata_ebpf_judy_pid_socket_stats_t;

typedef struct {
    netdata_ebpf_judy_pid_socket_stats_t socket_stats;
} netdata_ebpf_judy_pid_stats_t;

typedef struct {
    struct {
        Pvoid_t JudyLArray;
    } index;
} mock_ebpf_judy_pid_t;

mock_ebpf_judy_pid_t ebpf_judy_pid = {0};

typedef struct {
    int rw_spinlock;
} rw_spinlock_t;

int rw_spinlock_write_lock(rw_spinlock_t *lock) {
    return 0;
}

int rw_spinlock_write_unlock(rw_spinlock_t *lock) {
    return 0;
}

netdata_ebpf_judy_pid_stats_t *ebpf_get_pid_from_judy_unsafe(Pvoid_t *ptr, uint32_t pid) {
    return NULL;
}

typedef struct {
    int rw_spinlock;
} ebpf_judy_pid_t;

void *aral_socket_table = NULL;
const char *netdata_configured_host_prefix = "";
int procfile_open_flags = 0;

typedef struct {
    char lines;
} procfile;

procfile *procfile_open_no_log(const char *filename, const char *separators, int flags) {
    return NULL;
}

procfile *procfile_open(const char *filename, const char *separators, int flags) {
    return NULL;
}

void procfile_set_quotes(procfile *ff, const char *quotes) {
}

void procfile_set_open_close(procfile *ff, const char *open, const char *close) {
}

procfile *procfile_readall(procfile *ff) {
    return NULL;
}

size_t procfile_lines(procfile *ff) {
    return 0;
}

size_t procfile_linewords(procfile *ff, size_t line) {
    return 0;
}

char *procfile_lineword(procfile *ff, size_t line, size_t word) {
    return NULL;
}

void procfile_close(procfile *ff) {
}

int pid_max = 32768;

typedef struct {
    int x;
} SPINLOCK;

#define SPINLOCK_INITIALIZER {0}

void spinlock_lock(SPINLOCK *lock) {
}

void spinlock_unlock(SPINLOCK *lock) {
}

/* Global variables */
extern Pvoid_t ebpf_pid_judyL;
extern SPINLOCK ebpf_pid_spinlock;
extern ebpf_pid_data_t *ebpf_pids_link_list;
extern struct ebpf_target *apps_groups_default_target;
extern struct ebpf_target *apps_groups_root_target;

/* Include the code under test */
#include "../src/collectors/ebpf.plugin/ebpf_apps.c"

/* Test functions */

/* Tests for ebpf_read_hash_table */
static void test_ebpf_read_hash_table_null_ep(void **state) {
    (void)state;
    mock_bpf_map_lookup_elem_called = 0;
    
    int result = ebpf_read_hash_table(NULL, 1, 0);
    
    assert_int_equal(result, -1);
    assert_int_equal(mock_bpf_map_lookup_elem_called, 0);
}

static void test_ebpf_read_hash_table_success(void **state) {
    (void)state;
    char buffer[256];
    mock_bpf_map_lookup_elem_called = 0;
    mock_bpf_map_lookup_elem_result = 1;
    
    int result = ebpf_read_hash_table(buffer, 1, 0);
    
    assert_int_equal(result, -1);
    assert_int_equal(mock_bpf_map_lookup_elem_called, 1);
}

static void test_ebpf_read_hash_table_lookup_fails(void **state) {
    (void)state;
    char buffer[256];
    mock_bpf_map_lookup_elem_called = 0;
    mock_bpf_map_lookup_elem_result = 0;
    
    int result = ebpf_read_hash_table(buffer, 1, 0);
    
    assert_int_equal(result, 0);
    assert_int_equal(mock_bpf_map_lookup_elem_called, 1);
}

/* Tests for zero_all_targets */
static void test_zero_all_targets_null_root(void **state) {
    (void)state;
    
    size_t count = zero_all_targets(NULL);
    
    assert_int_equal(count, 0);
}

static void test_zero_all_targets_single_target(void **state) {
    (void)state;
    struct ebpf_target target = {0};
    
    size_t count = zero_all_targets(&target);
    
    assert_int_equal(count, 1);
    assert_null(target.root_pid);
}

static void test_zero_all_targets_with_pids(void **state) {
    (void)state;
    struct ebpf_pid_on_target pid1 = {.pid = 1, .next = NULL};
    struct ebpf_pid_on_target pid2 = {.pid = 2, .next = &pid1};
    struct ebpf_target target = {.root_pid = &pid2};
    
    size_t count = zero_all_targets(&target);
    
    assert_int_equal(count, 1);
    assert_null(target.root_pid);
}

static void test_zero_all_targets_multiple_targets(void **state) {
    (void)state;
    struct ebpf_target t1 = {0};
    struct ebpf_target t2 = {.next = &t1};
    
    size_t count = zero_all_targets(&t2);
    
    assert_int_equal(count, 2);
    assert_null(t2.root_pid);
    assert_null(t1.root_pid);
}

/* Tests for clean_apps_groups_target */
static void test_clean_apps_groups_target_null(void **state) {
    (void)state;
    
    clean_apps_groups_target(NULL);
    
    /* Should not crash */
}

static void test_clean_apps_groups_target_single(void **state) {
    (void)state;
    struct ebpf_target *target = callocz(1, sizeof(struct ebpf_target));
    
    clean_apps_groups_target(target);
    
    /* Should not crash, memory freed */
}

static void test_clean_apps_groups_target_chain(void **state) {
    (void)state;
    struct ebpf_target *t1 = callocz(1, sizeof(struct ebpf_target));
    struct ebpf_target *t2 = callocz(1, sizeof(struct ebpf_target));
    struct ebpf_target *t3 = callocz(1, sizeof(struct ebpf_target));
    
    t3->target = t2;
    t2->target = t1;
    t1->target = NULL;
    
    clean_apps_groups_target(t3);
    
    /* Should not crash, all memory freed */
}

/* Tests for get_apps_groups_target */
static void test_get_apps_groups_target_simple_id(void **state) {
    (void)state;
    struct ebpf_target *root = NULL;
    
    struct ebpf_target *result = get_apps_groups_target(&root, "test", NULL, "test_name");
    
    assert_non_null(result);
    assert_string_equal(result->id, "test");
    assert_string_equal(result->name, "test_name");
    assert_string_equal(result->compare, "test");
    assert_int_equal(result->hidden, 0);
}

static void test_get_apps_groups_target_hidden_flag(void **state) {
    (void)state;
    struct ebpf_target *root = NULL;
    
    struct ebpf_target *result = get_apps_groups_target(&root, "-test", NULL, "name");
    
    assert_non_null(result);
    assert_int_equal(result->hidden, 1);
}

static void test_get_apps_groups_target_debug_flag(void **state) {
    (void)state;
    struct ebpf_target *root = NULL;
    
    struct ebpf_target *result = get_apps_groups_target(&root, "+test", NULL, "name");
    
    assert_non_null(result);
}

static void test_get_apps_groups_target_wildcard_start(void **state) {
    (void)state;
    struct ebpf_target *root = NULL;
    
    struct ebpf_target *result = get_apps_groups_target(&root, "test*", NULL, "name");
    
    assert_non_null(result);
    assert_int_equal(result->starts_with, 1);
}

static void test_get_apps_groups_target_wildcard_end(void **state) {
    (void)state;
    struct ebpf_target *root = NULL;
    
    struct ebpf_target *result = get_apps_groups_target(&root, "*test", NULL, "name");
    
    assert_non_null(result);
    assert_int_equal(result->ends_with, 1);
}

static void test_get_apps_groups_target_wildcard_both(void **state) {
    (void)state;
    struct ebpf_target *root = NULL;
    
    struct ebpf_target *result = get_apps_groups_target(&root, "*test*", NULL, "name");
    
    assert_non_null(result);
    assert_int_equal(result->starts_with, 1);
    assert_int_equal(result->ends_with, 1);
}

static void test_get_apps_groups_target_existing(void **state) {
    (void)state;
    struct ebpf_target *root = NULL;
    
    struct ebpf_target *t1 = get_apps_groups_target(&root, "test", NULL, "name");
    struct ebpf_target *t2 = get_apps_groups_target(&root, "test", NULL, "name2");
    
    assert_ptr_equal(t1, t2);
}

static void test_get_apps_groups_target_with_parent(void **state) {
    (void)state;
    struct ebpf_target *root = NULL;
    struct ebpf_target *parent = get_apps_groups_target(&root, "parent", NULL, "parent");
    
    struct ebpf_target *child = get_apps_groups_target(&root, "child", parent, "parent");
    
    assert_non_null(child);
    assert_ptr_equal(child->target, parent);
    assert_string_equal(child->name, "parent");
}

/* Tests for ebpf_find_pid_data */
static void test_ebpf_find_pid_data_not_found(void **state) {
    (void)state;
    
    ebpf_pid_data_t *result = ebpf_find_pid_data(999);
    
    assert_null(result);
}

/* Tests for ebpf_find_or_create_pid_data */
static void test_ebpf_find_or_create_pid_data_new(void **state) {
    (void)state;
    
    ebpf_pid_data_t *result = ebpf_find_or_create_pid_data(12345);
    
    assert_non_null(result);
    assert_int_equal(result->pid, 0); /* Initially not set */
}

/* Tests for ebpf_pid_del */
static void test_ebpf_pid_del(void **state) {
    (void)state;
    
    ebpf_pid_del(123);
    
    /* Should not crash */
}

/* Tests for assign_target_to_pid */
static void test_assign_target_to_pid_exact_match(void **state) {
    (void)state;
    struct ebpf_target root = {0};
    strcpy(root.compare, "bash");
    root.comparehash = simple_hash("bash");
    root.comparelen = strlen("bash");
    root.id[0] = '\0';
    
    apps_groups_root_target = &root;
    apps_groups_default_target = callocz(1, sizeof(struct ebpf_target));
    
    ebpf_pid_data_t pid_data = {0};
    strcpy(pid_data.comm, "bash");
    pid_data.target = NULL;
    
    assign_target_to_pid(&pid_data);
    
    assert_ptr_equal(pid_data.target, &root);
    
    freez(apps_groups_default_target);
}

static void test_assign_target_to_pid_default(void **state) {
    (void)state;
    struct ebpf_target root = {0};
    strcpy(root.compare, "xyz");
    root.comparehash = simple_hash("xyz");
    
    apps_groups_root_target = &root;
    apps_groups_default_target = callocz(1, sizeof(struct ebpf_target));
    
    ebpf_pid_data_t pid_data = {0};
    strcpy(pid_data.comm, "bash");
    pid_data.target = NULL;
    
    assign_target_to_pid(&pid_data);
    
    assert_ptr_equal(pid_data.target, apps_groups_default_target);
    
    freez(apps_groups_default_target);
}

static void test_assign_target_to_pid_prefix_match(void **state) {
    (void)state;
    struct ebpf_target root = {0};
    strcpy(root.compare, "bash");
    root.comparehash = simple_hash("bash");
    root.comparelen = 4;
    root.starts_with = 1;
    root.ends_with = 0;
    
    apps_groups_root_target = &root;
    apps_groups_default_target = callocz(1, sizeof(struct ebpf_target));
    
    ebpf_pid_data_t pid_data = {0};
    strcpy(pid_data.comm, "bash_subshell");
    pid_data.target = NULL;
    
    assign_target_to_pid(&pid_data);
    
    assert_ptr_equal(pid_data.target, &root);
    
    freez(apps_groups_default_target);
}

static void test_assign_target_to_pid_suffix_match(void **state) {
    (void)state;
    struct ebpf_target root = {0};
    strcpy(root.compare, "log");
    root.comparehash = simple_hash("log");
    root.comparelen = 3;
    root.starts_with = 0;
    root.ends_with = 1;
    
    apps_groups_root_target = &root;
    apps_groups_default_target = callocz(1, sizeof(struct ebpf_target));
    
    ebpf_pid_data_t pid_data = {0};
    strcpy(pid_data.comm, "syslog");
    pid_data.target = NULL;
    
    assign_target_to_pid(&pid_data);
    
    assert_ptr_equal(pid_data.target, &root);
    
    freez(apps_groups_default_target);
}

/* Tests for ebpf_aral_init */
static void test_ebpf_aral_init(void **state) {
    (void)state;
    
    ebpf_aral_init();
    
    /* Should not crash */
}

/* Tests for read_proc_pid_cmdline */
static void test_read_proc_pid_cmdline_file_not_found(void **state) {
    (void)state;
    ebpf_pid_data_t pid_data = {.pid = 99999};
    char cmdline[16384];
    
    int result = read_proc_pid_cmdline(&pid_data, cmdline);
    
    assert_int_equal(result, 0);
    assert_int_equal(pid_data.cmdline[0], '\0');
}

/* Tests for read_proc_pid_stat */
static void test_read_proc_pid_stat_not_found(void **state) {
    (void)state;
    ebpf_pid_data_t pid_data = {.pid = 99999};
    
    apps_groups_root_target = NULL;
    apps_groups_default_target = NULL;
    
    int result = read_proc_pid_stat(&pid_data);
    
    assert_int_equal(result, 0);
    assert_int_equal(pid_data.has_proc_file, 0);
}

/* Tests for ebpf_collect_data_for_pid */
static void test_ebpf_collect_data_for_pid_invalid_pid(void **state) {
    (void)state;
    
    int result = ebpf_collect_data_for_pid(-1);
    
    assert_int_equal(result, 0);
}

static void test_ebpf_collect_data_for_pid_pid_too_large(void **state) {
    (void)state;
    pid_max = 100;
    
    int result = ebpf_collect_data_for_pid(200);
    
    assert_int_equal(result, 0);
}

/* Tests for link_all_processes_to_their_parents */
static void test_link_all_processes_to_their_parents_empty_list(void **state) {
    (void)state;
    ebpf_pids_link_list = NULL;
    
    link_all_processes_to_their_parents();
    
    assert_null(ebpf_pids_link_list);
}

/* Tests for apply_apps_groups_targets_inheritance */
static void test_apply_apps_groups_targets_inheritance_empty(void **state) {
    (void)state;
    ebpf_pids_link_list = NULL;
    apps_groups_root_target = NULL;
    apps_groups_default_target = callocz(1, sizeof(struct ebpf_target));
    
    apply_apps_groups_targets_inheritance();
    
    freez(apps_groups_default_target);
}

/* Tests for post_aggregate_targets */
static void test_post_aggregate_targets_null_root(void **state) {
    (void)state;
    
    post_aggregate_targets(NULL);
    
    /* Should not crash */
}

static void test_post_aggregate_targets_with_starttime(void **state) {
    (void)state;
    struct ebpf_target target = {.collected_starttime = 100};
    
    post_aggregate_targets(&target);
    
    assert_int_equal(target.starttime, 100);
}

/* Tests for ebpf_del_pid_entry */
static void test_ebpf_del_pid_entry_null_pid(void **state) {
    (void)state;
    
    ebpf_del_pid_entry(0);
    
    /* Should not crash */
}

/* Tests for ebpf_cleanup_exited_pids */
static void test_ebpf_cleanup_exited_pids_empty_list(void **state) {
    (void)state;
    ebpf_pids_link_list = NULL;
    
    ebpf_cleanup_exited_pids();
    
    /* Should not crash */
}

/* Tests for ebpf_read_proc_filesystem */
static void test_ebpf_read_proc_filesystem_no_dir(void **state) {
    (void)state;
    
    int result = ebpf_read_proc_filesystem();
    
    assert_int_equal(result, -1);
}

/* Tests for aggregate_pid_on_target */
static void test_aggregate_pid_on_target_no_proc_file(void **state) {
    (void)state;
    struct ebpf_target target = {0};
    ebpf_pid_data_t pid_data = {.has_proc_file = 0};
    
    aggregate_pid_on_target(&target, &pid_data, NULL);
    
    assert_int_equal(target.processes, 0);
}

static void test_aggregate_pid_on_target_valid(void **state) {
    (void)state;
    struct ebpf_target target = {0};
    ebpf_pid_data_t pid_data = {.pid = 123, .has_proc_file = 1};
    
    aggregate_pid_on_target(&target, &pid_data, NULL);
    
    assert_int_equal(target.processes, 1);
}

/* Tests for ebpf_parse_proc_files */
static void test_ebpf_parse_proc_files_empty_list(void **state) {
    (void)state;
    ebpf_pids_link_list = NULL;
    apps_groups_root_target = NULL;
    apps_groups_default_target = NULL;
    
    ebpf_parse_proc_files();
    
    /* Should not crash */
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ebpf_read_hash_table_null_ep),
        cmocka_unit_test(test_ebpf_read_hash_table_success),
        cmocka_unit_test(test_ebpf_read_hash_table_lookup_fails),
        cmocka_unit_test(test_zero_all_targets_null_root),
        cmocka_unit_test(test_zero_all_targets_single_target),
        cmocka_unit_test(test_zero_all_targets_with_pids),
        cmocka_unit_test(test_zero_all_targets_multiple_targets),
        cmocka_unit_test(test_clean_apps_groups_target_null),
        cmocka_unit_test(test_clean_apps_groups_target_single),
        cmocka_unit_test(test_clean_apps_groups_target_chain),
        cmocka_unit_test(test_get_apps_groups_target_simple_id),
        cmocka_unit_test(test_get_apps_groups_target_hidden_flag),
        cmocka_unit_test(test_get_apps_groups_target_debug_flag),
        cmocka_unit_test(test_get_apps_groups_target_wildcard_start),
        cmocka_unit_test(test_get_apps_groups_target_wildcard_end),
        cmocka_unit_test(test_get_apps_groups_target_wildcard_both),
        cmocka_unit_test(test_get_apps_groups_target_existing),
        cmocka_unit_test(test_get_apps_groups_target_with_parent),
        cmocka_unit_test(test_ebpf_find_pid_data_not_found),
        cmocka_unit_test(test_ebpf_find_or_create_pid_data_new),
        cmocka_unit_test(test_ebpf_pid_del),
        cmocka_unit_test(test_assign_target_to_pid_exact_match),
        cmocka_unit_test(test_assign_target_to_pid_default),
        cmocka_unit_test(test_assign_target_to_pid_prefix_match),
        cmocka_unit_test(test_assign_target_to_pid_suffix_match),
        cmocka_unit_test(test_ebpf_aral_init),
        cmocka_unit_test(test_read_proc_pid_cmdline_file_not_found),
        cmocka_unit_test(test_read_proc_pid_stat_not_found),
        cmocka_unit_test(test_ebpf_collect_data_for_pid_invalid_pid),
        cmocka_unit_test(test_ebpf_collect_data_for_pid_pid_too_large),
        cmocka_unit_test(test_link_all_processes_to_their_parents_empty_list),
        cmocka_unit_test(test_apply_apps_groups_targets_inheritance_empty),
        cmocka_unit_test(test_post_aggregate_targets_null_root),
        cmocka_unit_test(test_post_aggregate_targets_with_starttime),
        cmocka_unit_test(test_ebpf_del_pid_entry_null_pid),
        cmocka_unit_test(test_ebpf_cleanup_exited_pids_empty_list),
        cmocka_unit_test(test_ebpf_read_proc_filesystem_no_dir),
        cmocka_unit_test(test_aggregate_pid_on_target_no_proc_file),
        cmocka_unit_test(test_aggregate_pid_on_target_valid),
        cmocka_unit_test(test_ebpf_parse_proc_files_empty_list),
    };

    return cmocka_run_tests(tests);
}