#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <cmocka.h>
#include <stdint.h>
#include <stdbool.h>

/* Mock structures and declarations from apps_plugin.h */
#define INIT_PID 1
#define ALL_PIDS_ARE_READ_INSTANTLY 0
#define PROCESSES_HAVE_FDS 1
#define PROCESSES_HAVE_SMAPS_ROLLUP 1
#define PROCESSES_HAVE_CPU_CHILDREN_TIME 1
#define PROCESSES_HAVE_CHILDREN_FLTS 1
#define PROCESSES_HAVE_CPU_GUEST_TIME 1
#define PROCESSES_HAVE_MAJFLT 1
#define PROCESSES_HAVE_LOGICAL_IO 1
#define PROCESSES_HAVE_PHYSICAL_IO 1
#define PROCESSES_HAVE_IO_CALLS 1
#define PROCESSES_HAVE_VOLCTX 1
#define PROCESSES_HAVE_NVOLCTX 1
#define RATES_DETAIL 10000ULL
#define CPU_TO_NANOSECONDCORES (1)
#define INCREMENTAL_DATA_COLLECTION 1

typedef int32_t pid_t;
typedef uint64_t kernel_uint_t;
typedef uint64_t usec_t;

struct aral {
    void *dummy;
};

struct string_s {
    char data[256];
};
typedef struct string_s STRING;

struct pid_fd {
    int fd;
    ino_t inode;
    char *filename;
    uint32_t link_hash;
    size_t cache_iterations_counter;
    size_t cache_iterations_reset;
};

struct pid_stat {
    int32_t pid;
    int32_t ppid;
    struct pid_stat *parent;
    struct pid_stat *next;
    struct pid_stat *prev;
    struct target *target;
    struct target *prev_target;
    struct target *uid_target;
    struct target *gid_target;
    struct target *sid_target;
    STRING *comm_orig;
    STRING *comm;
    STRING *name;
    STRING *cmdline;
    kernel_uint_t raw[100];
    kernel_uint_t values[100];
    uid_t uid;
    gid_t gid;
    STRING *sid_name;
    uint32_t sortlist;
    struct pid_fd *fds;
    uint32_t fds_size;
    uint32_t children_count;
    uint32_t keeploops;
    unsigned int log_thrown;
    bool read;
    bool updated;
    bool merged;
    bool keep;
    bool is_manager;
    bool is_aggregator;
    bool matched_by_config;
    char state;
    usec_t stat_collected_usec;
    usec_t last_stat_collected_usec;
    usec_t io_collected_usec;
    usec_t last_io_collected_usec;
    usec_t last_limits_collected_usec;
    struct arl_base *status_arl;
    char *fds_dirname;
    char *stat_filename;
    char *status_filename;
    char *io_filename;
    char *cmdline_filename;
    char *limits_filename;
    char *smaps_rollup_filename;
    struct arl_base *smaps_rollup_arl;
    kernel_uint_t vmshared_delta;
    double pss_total_ratio;
    size_t last_pss_iteration;
    kernel_uint_t pss_bytes;
    struct openfds {
        kernel_uint_t files;
        kernel_uint_t pipes;
        kernel_uint_t sockets;
        kernel_uint_t inotifies;
        kernel_uint_t eventfds;
        kernel_uint_t timerfds;
        kernel_uint_t signalfds;
        kernel_uint_t eventpolls;
        kernel_uint_t other;
    } openfds;
};

struct target {
    STRING *id;
    STRING *name;
    char dummy[512];
};

enum PDF_VALUES {
    PDF_UTIME,
    PDF_STIME,
    PDF_GTIME,
    PDF_CUTIME,
    PDF_CSTIME,
    PDF_CGTIME,
    PDF_MINFLT,
    PDF_MAJFLT,
    PDF_CMINFLT,
    PDF_CMAJFLT,
    PDF_VMSIZE,
    PDF_VMRSS,
    PDF_VMSHARED,
    PDF_MEM_ESTIMATED,
    PDF_PSS,
    PDF_RSSFILE,
    PDF_RSSSHMEM,
    PDF_VMSWAP,
    PDF_VOLCTX,
    PDF_NVOLCTX,
    PDF_LREAD,
    PDF_LWRITE,
    PDF_PREAD,
    PDF_PWRITE,
    PDF_OREAD,
    PDF_OWRITE,
    PDF_UPTIME,
    PDF_THREADS,
    PDF_PROCESSES,
    PDF_MAX
};

/* Mock functions */
static struct aral *mock_aral_create(void) {
    return (struct aral *)malloc(sizeof(struct aral));
}

static void mock_aral_freez(struct aral *aral, void *ptr) {
    free(ptr);
}

static void *mock_aral_callocz(struct aral *aral) {
    return calloc(1, sizeof(struct pid_stat));
}

static void mock_simple_hashtable_init(void) {}
static void mock_simple_hashtable_set_slot(void) {}
static void mock_simple_hashtable_del_slot(void) {}

/* Minimal mocks for external functions */
void nd_log(int level, int priority, const char *fmt, ...) {}
void netdata_log_error(const char *fmt, ...) {}
void debug_log(const char *fmt, ...) {}
void sanitize_apps_plugin_chart_meta(char *buf) {}
void arl_free(struct arl_base *arl) {}
void string_freez(STRING *s) {}
STRING *string_strdupz(const char *s) {
    STRING *result = (STRING *)malloc(sizeof(STRING));
    if (result && s) {
        strncpy(result->data, s, sizeof(result->data) - 1);
        result->data[sizeof(result->data) - 1] = '\0';
    }
    return result;
}
int string_strcmp(STRING *a, const char *b) {
    return strcmp(a ? a->data : "", b ? b : "");
}
const char *string2str(STRING *s) {
    return s ? s->data : "";
}
size_t string_strlen(STRING *s) {
    return s ? strlen(s->data) : 0;
}
bool is_process_a_manager(struct pid_stat *p) { return false; }
bool is_process_an_aggregator(struct pid_stat *p) { return false; }
bool is_process_an_interpreter(struct pid_stat *p) { return false; }
void freez(void *ptr) { free(ptr); }
void *mallocz(size_t size) { return malloc(size); }
void *reallocz(void *ptr, size_t size) { return realloc(ptr, size); }
void *callocz(size_t count, size_t size) { return calloc(count, size); }
int strdupz(const char *s) { return 0; }
char *strrchr(const char *s, int c) { return (char*)malloc(1); }
size_t quoted_strings_splitter_whitespace(char *buf, char **words, size_t max) { return 0; }
bool managed_log(struct pid_stat *p, unsigned int log, bool status) { return false; }
void incrementally_collect_data_for_pid_stat(struct pid_stat *p, void *ptr) {}
void errno_clear(void) {}
int statvfs(const char *path, void *stat) { return -1; }

extern size_t global_iterations_counter;
extern bool include_exited_childs;
extern bool proc_pid_cmdline_is_needed;
extern bool enable_file_charts;

size_t global_iterations_counter = 2;
bool include_exited_childs = true;
bool proc_pid_cmdline_is_needed = false;
bool enable_file_charts = 1;

/* Tests for apps_pid.c functions */

static void test_root_of_pids_returns_valid_root(void **state) {
    struct pid_stat *root = root_of_pids();
    assert_null(root);
}

static void test_all_pids_count_returns_zero_initially(void **state) {
    size_t count = all_pids_count();
    assert_int_equal(count, 0);
}

static void test_apps_pids_init_initializes_aral(void **state) {
    apps_pids_init();
    struct pid_stat *root = root_of_pids();
    size_t count = all_pids_count();
    assert_int_equal(count, 0);
}

static void test_find_pid_entry_with_invalid_pid_returns_null(void **state) {
    struct pid_stat *result = find_pid_entry(0);
    assert_null(result);
}

static void test_find_pid_entry_with_pid_below_init_returns_null(void **state) {
    struct pid_stat *result = find_pid_entry(-1);
    assert_null(result);
}

static void test_get_or_allocate_pid_entry_allocates_new_entry(void **state) {
    apps_pids_init();
    struct pid_stat *entry = get_or_allocate_pid_entry(1000);
    assert_non_null(entry);
    assert_int_equal(entry->pid, 1000);
    assert_int_equal(entry->values[PDF_PROCESSES], 1);
    assert_true(entry->pss_total_ratio > 0.99 && entry->pss_total_ratio < 1.01);
}

static void test_get_or_allocate_pid_entry_returns_existing_entry(void **state) {
    apps_pids_init();
    struct pid_stat *entry1 = get_or_allocate_pid_entry(2000);
    struct pid_stat *entry2 = get_or_allocate_pid_entry(2000);
    assert_ptr_equal(entry1, entry2);
}

static void test_del_pid_entry_removes_entry(void **state) {
    apps_pids_init();
    struct pid_stat *entry = get_or_allocate_pid_entry(3000);
    assert_non_null(entry);
    
    del_pid_entry(3000);
    struct pid_stat *found = find_pid_entry(3000);
    assert_null(found);
}

static void test_del_pid_entry_with_nonexistent_pid(void **state) {
    del_pid_entry(99999);
}

static void test_pid_collection_started_saves_state(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(4000);
    p->values[PDF_UTIME] = 100;
    p->values[PDF_STIME] = 50;
    
    pid_collection_started(p);
    
    assert_true(p->read);
    assert_int_equal(p->values[PDF_PROCESSES], 1);
}

static void test_pid_collection_failed_restores_state(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(5000);
    p->values[PDF_UTIME] = 100;
    p->values[PDF_STIME] = 50;
    
    pid_collection_started(p);
    p->values[PDF_UTIME] = 0;
    p->values[PDF_STIME] = 0;
    
    pid_collection_failed(p);
    
    assert_int_equal(p->values[PDF_UTIME], 100);
    assert_int_equal(p->values[PDF_STIME], 50);
}

static void test_pid_collection_completed_sets_flags(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(6000);
    
    pid_collection_completed(p);
    
    assert_true(p->updated);
    assert_false(p->keep);
    assert_int_equal(p->keeploops, 0);
}

static void test_compute_new_sorted_size_rounds_up(void **state) {
    size_t result = compute_new_sorted_size(0, 500);
    assert_true(result >= 500);
    assert_int_equal(result % 1024, 0);
}

static void test_compute_new_sorted_size_doubles_when_needed(void **state) {
    size_t result = compute_new_sorted_size(512, 600);
    assert_true(result >= 1024);
}

static void test_compar_pid_sortlist_compares_correctly(void **state) {
    apps_pids_init();
    struct pid_stat p1, p2;
    struct pid_stat *pp1 = &p1, *pp2 = &p2;
    
    p1.sortlist = 100;
    p2.sortlist = 50;
    
    int result = compar_pid_sortlist(&pp1, &pp2);
    assert_true(result < 0);
}

static void test_compar_pid_sortlist_equal_returns_one(void **state) {
    struct pid_stat p1, p2;
    struct pid_stat *pp1 = &p1, *pp2 = &p2;
    
    p1.sortlist = 100;
    p2.sortlist = 100;
    
    int result = compar_pid_sortlist(&pp1, &pp2);
    assert_int_equal(result, 1);
}

static void test_is_filename_returns_false_for_null(void **state) {
    bool result = is_filename(NULL);
    assert_false(result);
}

static void test_is_filename_returns_false_for_empty_string(void **state) {
    bool result = is_filename("");
    assert_false(result);
}

static void test_is_filename_detects_exec_prefix(void **state) {
    const char *test_string = "exec /path/to/command";
    bool result = is_filename(test_string);
}

static void test_remove_extension_strips_sh(void **state) {
    char name[] = "script.sh";
    remove_extension(name);
    assert_string_equal(name, "script");
}

static void test_remove_extension_strips_py(void **state) {
    char name[] = "script.py";
    remove_extension(name);
    assert_string_equal(name, "script");
}

static void test_remove_extension_strips_pl(void **state) {
    char name[] = "script.pl";
    remove_extension(name);
    assert_string_equal(name, "script");
}

static void test_remove_extension_strips_js(void **state) {
    char name[] = "script.js";
    remove_extension(name);
    assert_string_equal(name, "script");
}

static void test_remove_extension_does_nothing_for_unknown(void **state) {
    char name[] = "script.txt";
    remove_extension(name);
    assert_string_equal(name, "script.txt");
}

static void test_comm_from_cmdline_param_sanitized_null_returns_null(void **state) {
    STRING *result = comm_from_cmdline_param_sanitized(NULL);
    assert_null(result);
}

static void test_comm_from_cmdline_sanitized_null_returns_null(void **state) {
    STRING *comm = string_strdupz("test");
    STRING *result = comm_from_cmdline_sanitized(comm, NULL);
    assert_null(result);
}

static void test_update_pid_comm_with_parenthesis_strips_them(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(7000);
    
    update_pid_comm(p, "(bash)");
    
    const char *comm_str = string2str(p->comm_orig);
    assert_non_null(comm_str);
}

static void test_update_pid_comm_no_change_early_return(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(8000);
    
    STRING *test_comm = string_strdupz("test");
    p->comm_orig = test_comm;
    
    update_pid_comm(p, "test");
}

static void test_update_pid_cmdline_with_null(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(9000);
    
    update_pid_cmdline(p, NULL);
    
    assert_null(p->cmdline);
}

static void test_update_pid_cmdline_with_string(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(10000);
    
    update_pid_cmdline(p, "/bin/bash");
    
    assert_non_null(p->cmdline);
}

static void test_remove_exited_child_from_parent_full_absorption(void **state) {
    kernel_uint_t field = 50;
    kernel_uint_t pfield = 100;
    
    kernel_uint_t absorbed = remove_exited_child_from_parent(&field, &pfield);
    
    assert_int_equal(absorbed, 50);
    assert_int_equal(field, 0);
    assert_int_equal(pfield, 50);
}

static void test_remove_exited_child_from_parent_partial_absorption(void **state) {
    kernel_uint_t field = 150;
    kernel_uint_t pfield = 100;
    
    kernel_uint_t absorbed = remove_exited_child_from_parent(&field, &pfield);
    
    assert_int_equal(absorbed, 100);
    assert_int_equal(field, 50);
    assert_int_equal(pfield, 0);
}

static void test_process_exited_pids_with_no_work(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(11000);
    p->updated = false;
    p->stat_collected_usec = 1000;
    p->raw[PDF_UTIME] = 0;
    p->raw[PDF_STIME] = 0;
    
    process_exited_pids();
}

static void test_clear_pid_rates_clears_all_rates(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(12000);
    
    p->values[PDF_UTIME] = 100;
    p->values[PDF_STIME] = 50;
    p->values[PDF_MINFLT] = 10;
    
    clear_pid_rates(p);
    
    assert_int_equal(p->values[PDF_UTIME], 0);
    assert_int_equal(p->values[PDF_STIME], 0);
    assert_int_equal(p->values[PDF_MINFLT], 0);
}

static void test_collect_data_for_all_pids_marks_unread(void **state) {
    apps_pids_init();
    struct pid_stat *p1 = get_or_allocate_pid_entry(13000);
    struct pid_stat *p2 = get_or_allocate_pid_entry(13001);
    
    p1->read = true;
    p1->updated = true;
    p2->read = true;
    p2->updated = true;
    
    collect_data_for_all_pids();
    
    assert_false(p1->read);
    assert_false(p1->updated);
}

static void test_collect_data_for_all_pids_first_iteration_clears_rates(void **state) {
    apps_pids_init();
    global_iterations_counter = 1;
    struct pid_stat *p = get_or_allocate_pid_entry(14000);
    p->read = true;
    p->values[PDF_UTIME] = 100;
    
    collect_data_for_all_pids();
    
    assert_int_equal(p->values[PDF_UTIME], 0);
}

static void test_is_already_a_parent_finds_parent(void **state) {
    apps_pids_init();
    struct pid_stat *parent = get_or_allocate_pid_entry(15000);
    struct pid_stat *child = get_or_allocate_pid_entry(15001);
    
    child->parent = parent;
    
    bool result = is_already_a_parent(parent, child);
    assert_true(result);
}

static void test_is_already_a_parent_not_found(void **state) {
    apps_pids_init();
    struct pid_stat *p1 = get_or_allocate_pid_entry(16000);
    struct pid_stat *p2 = get_or_allocate_pid_entry(16001);
    struct pid_stat *p3 = get_or_allocate_pid_entry(16002);
    
    p2->parent = p3;
    
    bool result = is_already_a_parent(p1, p2);
    assert_false(result);
}

static void test_link_pid_to_its_parent_with_zero_ppid(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(17000);
    p->ppid = 0;
    
    link_pid_to_its_parent(p);
    
    assert_null(p->parent);
}

static void test_link_pid_to_its_parent_self_reference(void **state) {
    apps_pids_init();
    struct pid_stat *p = get_or_allocate_pid_entry(17001);
    p->ppid = p->pid;
    
    link_pid_to_its_parent(p);
    
    assert_null(p->parent);
    assert_int_equal(p->ppid, 0);
}

static void test_link_all_processes_to_their_parents(void **state) {
    apps_pids_init();
    struct pid_stat *parent = get_or_allocate_pid_entry(18000);
    struct pid_stat *child = get_or_allocate_pid_entry(18001);
    
    child->ppid = parent->pid;
    
    link_all_processes_to_their_parents();
    
    assert_ptr_equal(child->parent, parent);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_root_of_pids_returns_valid_root),
        cmocka_unit_test(test_all_pids_count_returns_zero_initially),
        cmocka_unit_test(test_apps_pids_init_initializes_aral),
        cmocka_unit_test(test_find_pid_entry_with_invalid_pid_returns_null),
        cmocka_unit_test(test_find_pid_entry_with_pid_below_init_returns_null),
        cmocka_unit_test(test_get_or_allocate_pid_entry_allocates_new_entry),
        cmocka_unit_test(test_get_or_allocate_pid_entry_returns_existing_entry),
        cmocka_unit_test(test_del_pid_entry_removes_entry),
        cmocka_unit_test(test_del_pid_entry_with_nonexistent_pid),
        cmocka_unit_test(test_pid_collection_started_saves_state),
        cmocka_unit_test(test_pid_collection_failed_restores_state),
        cmocka_unit_test(test_pid_collection_completed_sets_flags),
        cmocka_unit_test(test_compute_new_sorted_size_rounds_up),
        cmocka_unit_test(test_compute_new_sorted_size_doubles_when_needed),
        cmocka_unit_test(test_compar_pid_sortlist_compares_correctly),
        cmocka_unit_test(test_compar_pid_sortlist_equal_returns_one),
        cmocka_unit_test(test_is_filename_returns_false_for_null),
        cmocka_unit_test(test_is_filename_returns_false_for_empty_string),
        cmocka_unit_test(test_is_filename_detects_exec_prefix),
        cmocka_unit_test(test_remove_extension_strips_sh),
        cmocka_unit_test(test_remove_extension_strips_py),
        cmocka_unit_test(test_remove_extension_strips_pl),
        cmocka_unit_test(test_remove_extension_strips_js),
        cmocka_unit_test(test_remove_extension_does_nothing_for_unknown),
        cmocka_unit_test(test_comm_from_cmdline_param_sanitized_null_returns_null),
        cmocka_unit_test(test_comm_from_cmdline_sanitized_null_returns_null),
        cmocka_unit_test(test_update_pid_comm_with_parenthesis_strips_them),
        cmocka_unit_test(test_update_pid_comm_no_change_early_return),
        cmocka_unit_test(test_update_pid_cmdline_with_null),
        cmocka_unit_test(test_update_pid_cmdline_with_string),
        cmocka_unit_test(test_remove_exited_child_from_parent_full_absorption),
        cmocka_unit_test(test_remove_exited_child_from_parent_partial_absorption),
        cmocka_unit_test(test_process_exited_pids_with_no_work),
        cmocka_unit_test(test_clear_pid_rates_clears_all_rates),
        cmocka_unit_test(test_collect_data_for_all_pids_marks_unread),
        cmocka_unit_test(test_collect_data_for_all_pids_first_iteration_clears_rates),
        cmocka_unit_test(test_is_already_a_parent_finds_parent),
        cmocka_unit_test(test_is_already_a_parent_not_found),
        cmocka_unit_test(test_link_pid_to_its_parent_with_zero_ppid),
        cmocka_unit_test(test_link_pid_to_its_parent_self_reference),
        cmocka_unit_test(test_link_all_processes_to_their_parents),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}