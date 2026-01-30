// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Mock structures and external dependencies
typedef unsigned int uint32_t;
typedef unsigned long long ull;

#define FILENAME_MAX 4096
#define PROCFILE_FLAG_DEFAULT 0
#define O_RDONLY 0
#define O_CLOEXEC 0
#define O_NOFOLLOW 0

// Mock types
typedef struct procfile {
    int dummy;
} procfile;

typedef struct {
    int dummy;
} SPAWN_SERVER;

typedef struct {
    int dummy;
} SPAWN_REQUEST;

typedef struct {
    int dummy;
} SPAWN_INSTANCE;

// Global mocks
procfile_open_flags = 0;
const char *netdata_configured_host_prefix = "";
const char *program_name = "cgroup-network-test";
int is_inside_k8s = 0;

// Mock functions
void nd_log(int ls, int lp, const char *format, ...) {
    // Mock implementation
}

void errno_clear(void) {
    // Mock implementation
}

int read_single_number_file(const char *filename, unsigned long long *value) {
    mock();
    *value = mock_type(unsigned long long);
    return mock_type(int);
}

uint32_t simple_hash(const char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void *mallocz(size_t size) {
    return malloc(size);
}

void freez(void *ptr) {
    free(ptr);
}

void *strdupz(const char *str) {
    return strdup(str);
}

char *strsep(char **stringp, const char *delim) {
    char *start = *stringp;
    if (!start) return NULL;
    
    while (*stringp && !strchr(delim, **stringp))
        (*stringp)++;
    
    if (*stringp && **stringp) {
        **stringp = '\0';
        (*stringp)++;
    } else {
        *stringp = NULL;
    }
    
    return start;
}

procfile *procfile_open(const char *filename, const char *separators, int flags) {
    mock();
    return mock_type(procfile *);
}

procfile *procfile_readall(procfile *ff) {
    mock();
    return mock_type(procfile *);
}

void procfile_close(procfile *ff) {
    mock();
}

size_t procfile_lines(procfile *ff) {
    mock();
    return mock_type(size_t);
}

size_t procfile_linewords(procfile *ff, size_t line) {
    mock();
    return mock_type(size_t);
}

const char *procfile_lineword(procfile *ff, size_t line, size_t word) {
    mock();
    return mock_type(const char *);
}

void gettid_uncached(void) {
    mock();
}

void tinysleep(void) {
    // Mock sleep
}

int setns(int fd, int nstype) {
    mock();
    return mock_type(int);
}

int setgroups(int ngroups, const gid_t *groups) {
    mock();
    return mock_type(int);
}

int fchdir(int fd) {
    mock();
    return mock_type(int);
}

int chroot(const char *path) {
    mock();
    return mock_type(int);
}

int close(int fd) {
    return 0;
}

pid_t fork(void) {
    mock();
    return mock_type(pid_t);
}

pid_t waitpid(pid_t pid, int *status, int options) {
    mock();
    *status = mock_type(int);
    return mock_type(pid_t);
}

pid_t getpid(void) {
    return 1;
}

int kill(pid_t pid, int sig) {
    return 0;
}

void exit(int status) {
    mock();
}

int open(const char *path, int flags, ...) {
    mock();
    return mock_type(int);
}

FILE *fdopen(int fd, const char *mode) {
    mock();
    return mock_type(FILE *);
}

char *fgets(char *s, int size, FILE *stream) {
    mock();
    return mock_type(char *);
}

int fclose(FILE *fp) {
    return 0;
}

int atoi(const char *nptr) {
    return (int)strtol(nptr, NULL, 10);
}

DIR *opendir(const char *name) {
    mock();
    return mock_type(DIR *);
}

struct dirent *readdir(DIR *dirp) {
    mock();
    return mock_type(struct dirent *);
}

int closedir(DIR *dirp) {
    return 0;
}

int stat(const char *path, struct stat *statbuf) {
    mock();
    return mock_type(int);
}

int snprintfz(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(str, size, format, args);
    va_end(args);
    return ret;
}

void *memset(void *s, int c, size_t n) {
    return memset(s, c, n);
}

char *trim(char *str) {
    if (!str) return str;
    while (*str && isspace(*str)) str++;
    if (!*str) return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) *(end--) = '\0';
    return str;
}

int verify_netdata_host_prefix(bool) {
    mock();
    return mock_type(int);
}

const char *nd_log_method_for_external_plugins(const char *s) {
    mock();
    return mock_type(const char *);
}

int nd_log_collectors_fd(void) {
    return 1;
}

void nd_log_initialize_for_external_plugins(const char *name) {
    mock();
}

SPAWN_SERVER *spawn_server_create(int opts, void *x, int (*cb)(SPAWN_REQUEST *), int argc, const char **argv) {
    mock();
    return mock_type(SPAWN_SERVER *);
}

void spawn_server_destroy(SPAWN_SERVER *s) {
    mock();
}

void nd_log_register_fatal_final_cb(void (*cb)(void)) {
    mock();
}

SPAWN_INSTANCE *spawn_server_exec(SPAWN_SERVER *s, int fd, int a, const char **argv, void *data, size_t len, int type) {
    mock();
    return mock_type(SPAWN_INSTANCE *);
}

int spawn_server_instance_read_fd(SPAWN_INSTANCE *si) {
    mock();
    return mock_type(int);
}

void spawn_server_instance_read_fd_unset(SPAWN_INSTANCE *si) {
    mock();
}

void spawn_server_exec_kill(SPAWN_SERVER *s, SPAWN_INSTANCE *si, int a) {
    mock();
}

// Tests

static void test_calc_num_ifaces_with_single_interface(void **state) {
    struct iface i = {
        .device = "eth0",
        .hash = 0,
        .ifindex = 1,
        .iflink = 2,
        .next = NULL
    };
    
    unsigned int count = calc_num_ifaces(&i);
    assert_int_equal(count, 1);
}

static void test_calc_num_ifaces_with_multiple_interfaces(void **state) {
    struct iface i2 = {
        .device = "eth1",
        .hash = 0,
        .ifindex = 2,
        .iflink = 3,
        .next = NULL
    };
    
    struct iface i1 = {
        .device = "eth0",
        .hash = 0,
        .ifindex = 1,
        .iflink = 2,
        .next = &i2
    };
    
    unsigned int count = calc_num_ifaces(&i1);
    assert_int_equal(count, 2);
}

static void test_calc_num_ifaces_with_null_root(void **state) {
    unsigned int count = calc_num_ifaces(NULL);
    assert_int_equal(count, 0);
}

static void test_read_iface_iflink_with_null_prefix(void **state) {
    expect_function_call(read_single_number_file);
    will_return(read_single_number_file, 42ULL);
    will_return(read_single_number_file, 0);
    
    unsigned int result = read_iface_iflink(NULL, "eth0");
    assert_int_equal(result, 42);
}

static void test_read_iface_iflink_with_valid_prefix(void **state) {
    expect_function_call(read_single_number_file);
    will_return(read_single_number_file, 100ULL);
    will_return(read_single_number_file, 0);
    
    unsigned int result = read_iface_iflink("/host", "eth0");
    assert_int_equal(result, 100);
}

static void test_read_iface_iflink_file_read_error(void **state) {
    expect_function_call(read_single_number_file);
    will_return(read_single_number_file, 0ULL);
    will_return(read_single_number_file, -1);
    
    unsigned int result = read_iface_iflink(NULL, "eth0");
    assert_int_equal(result, 0);
}

static void test_read_iface_ifindex_with_null_prefix(void **state) {
    expect_function_call(read_single_number_file);
    will_return(read_single_number_file, 1ULL);
    will_return(read_single_number_file, 0);
    
    unsigned int result = read_iface_ifindex(NULL, "eth0");
    assert_int_equal(result, 1);
}

static void test_read_iface_ifindex_with_valid_prefix(void **state) {
    expect_function_call(read_single_number_file);
    will_return(read_single_number_file, 10ULL);
    will_return(read_single_number_file, 0);
    
    unsigned int result = read_iface_ifindex("/host", "eth0");
    assert_int_equal(result, 10);
}

static void test_read_iface_ifindex_file_read_error(void **state) {
    expect_function_call(read_single_number_file);
    will_return(read_single_number_file, 0ULL);
    will_return(read_single_number_file, -1);
    
    unsigned int result = read_iface_ifindex(NULL, "eth0");
    assert_int_equal(result, 0);
}

static void test_read_proc_net_dev_null_prefix(void **state) {
    procfile ff_mock;
    expect_function_call(procfile_open);
    will_return(procfile_open, &ff_mock);
    
    expect_function_call(procfile_readall);
    will_return(procfile_readall, &ff_mock);
    
    expect_function_call(procfile_lines);
    will_return(procfile_lines, 2);
    
    expect_function_call(procfile_close);
    
    struct iface *result = read_proc_net_dev("host", NULL);
    assert_null(result);
}

static void test_read_proc_net_dev_cannot_open_file(void **state) {
    expect_function_call(procfile_open);
    will_return(procfile_open, NULL);
    
    struct iface *result = read_proc_net_dev("host", "");
    assert_null(result);
}

static void test_read_proc_net_dev_cannot_read_file(void **state) {
    procfile ff_mock;
    expect_function_call(procfile_open);
    will_return(procfile_open, &ff_mock);
    
    expect_function_call(procfile_readall);
    will_return(procfile_readall, NULL);
    
    struct iface *result = read_proc_net_dev("host", "");
    assert_null(result);
}

static void test_read_proc_net_dev_with_valid_interfaces(void **state) {
    procfile ff_mock;
    expect_function_call(procfile_open);
    will_return(procfile_open, &ff_mock);
    
    expect_function_call(procfile_readall);
    will_return(procfile_readall, &ff_mock);
    
    expect_function_call(procfile_lines);
    will_return(procfile_lines, 3);
    
    expect_function_call(procfile_linewords);
    will_return(procfile_linewords, 1);
    
    expect_function_call(procfile_lineword);
    will_return(procfile_lineword, "eth0");
    
    expect_function_call(read_single_number_file);
    will_return(read_single_number_file, 1ULL);
    will_return(read_single_number_file, 0);
    
    expect_function_call(read_single_number_file);
    will_return(read_single_number_file, 2ULL);
    will_return(read_single_number_file, 0);
    
    expect_function_call(procfile_linewords);
    will_return(procfile_linewords, 0);
    
    expect_function_call(procfile_close);
    
    struct iface *result = read_proc_net_dev("host", "");
    assert_non_null(result);
    assert_string_equal(result->device, "eth0");
    assert_int_equal(result->ifindex, 1);
    assert_int_equal(result->iflink, 2);
    free_host_ifaces(result);
}

static void test_free_iface(void **state) {
    struct iface *iface = callocz(1, sizeof(struct iface));
    iface->device = strdupz("eth0");
    
    free_iface(iface);
    // Memory is freed, just verify it doesn't crash
}

static void test_free_host_ifaces_single(void **state) {
    struct iface *iface = callocz(1, sizeof(struct iface));
    iface->device = strdupz("eth0");
    iface->next = NULL;
    
    free_host_ifaces(iface);
    // Memory is freed
}

static void test_free_host_ifaces_multiple(void **state) {
    struct iface *i2 = callocz(1, sizeof(struct iface));
    i2->device = strdupz("eth1");
    i2->next = NULL;
    
    struct iface *i1 = callocz(1, sizeof(struct iface));
    i1->device = strdupz("eth0");
    i1->next = i2;
    
    free_host_ifaces(i1);
    // Memory is freed
}

static void test_iface_is_eligible_when_iflink_differs(void **state) {
    struct iface iface = {
        .device = "eth0",
        .ifindex = 1,
        .iflink = 2
    };
    
    int result = iface_is_eligible(&iface);
    assert_int_equal(result, 1);
}

static void test_iface_is_eligible_when_iflink_equals(void **state) {
    struct iface iface = {
        .device = "eth0",
        .ifindex = 1,
        .iflink = 1
    };
    
    int result = iface_is_eligible(&iface);
    assert_int_equal(result, 0);
}

static void test_eligible_ifaces_single_eligible(void **state) {
    struct iface i = {
        .device = "eth0",
        .ifindex = 1,
        .iflink = 2,
        .next = NULL
    };
    
    int count = eligible_ifaces(&i);
    assert_int_equal(count, 1);
}

static void test_eligible_ifaces_none_eligible(void **state) {
    struct iface i = {
        .device = "eth0",
        .ifindex = 1,
        .iflink = 1,
        .next = NULL
    };
    
    int count = eligible_ifaces(&i);
    assert_int_equal(count, 0);
}

static void test_eligible_ifaces_mixed(void **state) {
    struct iface i2 = {
        .device = "eth1",
        .ifindex = 2,
        .iflink = 2,
        .next = NULL
    };
    
    struct iface i1 = {
        .device = "eth0",
        .ifindex = 1,
        .iflink = 2,
        .next = &i2
    };
    
    int count = eligible_ifaces(&i1);
    assert_int_equal(count, 1);
}

static void test_eligible_ifaces_null_root(void **state) {
    int count = eligible_ifaces(NULL);
    assert_int_equal(count, 0);
}

static void test_proc_pid_fd_with_null_prefix(void **state) {
    expect_function_call(open);
    will_return(open, 5);
    
    int fd = proc_pid_fd(NULL, "root", 100);
    assert_int_equal(fd, 5);
}

static void test_proc_pid_fd_with_valid_prefix(void **state) {
    expect_function_call(open);
    will_return(open, 10);
    
    int fd = proc_pid_fd("/host", "ns/net", 100);
    assert_int_equal(fd, 10);
}

static void test_proc_pid_fd_open_failure(void **state) {
    expect_function_call(open);
    will_return(open, -1);
    
    int fd = proc_pid_fd("/host", "ns/net", 100);
    assert_int_equal(fd, -1);
}

static void test_read_pid_from_cgroup_file_cannot_open(void **state) {
    expect_function_call(open);
    will_return(open, -1);
    
    pid_t pid = read_pid_from_cgroup_file("/path/to/file");
    assert_int_equal(pid, 0);
}

static void test_read_pid_from_cgroup_file_cannot_fdopen(void **state) {
    expect_function_call(open);
    will_return(open, 3);
    
    expect_function_call(fdopen);
    will_return(fdopen, NULL);
    
    pid_t pid = read_pid_from_cgroup_file("/path/to/file");
    assert_int_equal(pid, 0);
}

static void test_read_pid_from_cgroup_file_valid_pid(void **state) {
    FILE fp_mock;
    expect_function_call(open);
    will_return(open, 3);
    
    expect_function_call(fdopen);
    will_return(fdopen, &fp_mock);
    
    expect_function_call(fgets);
    will_return(fgets, "100\n");
    
    pid_t pid = read_pid_from_cgroup_file("/path/to/file");
    assert_int_equal(pid, 100);
}

static void test_read_pid_from_cgroup_file_no_valid_pid(void **state) {
    FILE fp_mock;
    expect_function_call(open);
    will_return(open, 3);
    
    expect_function_call(fdopen);
    will_return(fdopen, &fp_mock);
    
    expect_function_call(fgets);
    will_return(fgets, NULL);
    
    pid_t pid = read_pid_from_cgroup_file("/path/to/file");
    assert_int_equal(pid, 0);
}

static void test_read_pid_from_cgroup_files_cgroup_procs_success(void **state) {
    FILE fp_mock;
    expect_function_call(open);
    will_return(open, 3);
    
    expect_function_call(fdopen);
    will_return(fdopen, &fp_mock);
    
    expect_function_call(fgets);
    will_return(fgets, "200\n");
    
    pid_t pid = read_pid_from_cgroup_files("/path");
    assert_int_equal(pid, 200);
}

static void test_ishex_valid_digits(void **state) {
    assert_int_equal(ishex('0'), 1);
    assert_int_equal(ishex('9'), 1);
}

static void test_ishex_valid_lowercase(void **state) {
    assert_int_equal(ishex('a'), 1);
    assert_int_equal(ishex('f'), 1);
}

static void test_ishex_valid_uppercase(void **state) {
    assert_int_equal(ishex('A'), 1);
    assert_int_equal(ishex('F'), 1);
}

static void test_ishex_invalid(void **state) {
    assert_int_equal(ishex('g'), 0);
    assert_int_equal(ishex('z'), 0);
    assert_int_equal(ishex(' '), 0);
}

static void test_is_valid_hex_escape_valid(void **state) {
    assert_int_equal(is_valid_hex_escape("\\x41"), 1);
    assert_int_equal(is_valid_hex_escape("\\xff"), 1);
    assert_int_equal(is_valid_hex_escape("\\x00"), 1);
}

static void test_is_valid_hex_escape_invalid(void **state) {
    assert_int_equal(is_valid_hex_escape("x41"), 0);
    assert_int_equal(is_valid_hex_escape("\\X41"), 0);
    assert_int_equal(is_valid_hex_escape("\\xgg"), 0);
    assert_int_equal(is_valid_hex_escape("\\x4"), 0);
}

static void test_is_valid_path_symbol_slash(void **state) {
    assert_int_equal(is_valid_path_symbol('/'), 1);
}

static void test_is_valid_path_symbol_space(void **state) {
    assert_int_equal(is_valid_path_symbol(' '), 1);
}

static void test_is_valid_path_symbol_hyphen(void **state) {
    assert_int_equal(is_valid_path_symbol('-'), 1);
}

static void test_is_valid_path_symbol_underscore(void **state) {
    assert_int_equal(is_valid_path_symbol('_'), 1);
}

static void test_is_valid_path_symbol_dot(void **state) {
    assert_int_equal(is_valid_path_symbol('.'), 1);
}

static void test_is_valid_path_symbol_comma(void **state) {
    assert_int_equal(is_valid_path_symbol(','), 1);
}

static void test_is_valid_path_symbol_at(void **state) {
    assert_int_equal(is_valid_path_symbol('@'), 1);
}

static void test_is_valid_path_symbol_invalid(void **state) {
    assert_int_equal(is_valid_path_symbol('$'), 0);
    assert_int_equal(is_valid_path_symbol('&'), 0);
    assert_int_equal(is_valid_path_symbol(';'), 0);
    assert_int_equal(is_valid_path_symbol('|'), 0);
}

static void test_verify_path_relative_path(void **state) {
    int result = verify_path("relative/path");
    assert_int_equal(result, -1);
}

static void test_verify_path_parent_traversal(void **state) {
    int result = verify_path("/../path");
    assert_int_equal(result, 1);
}

static void test_verify_path_parent_traversal_middle(void **state) {
    int result = verify_path("/path/../other");
    assert_int_equal(result, 1);
}

static void test_verify_path_invalid_character(void **state) {
    int result = verify_path("/path$invalid");
    assert_int_equal(result, -1);
}

static void test_verify_path_stat_failure(void **state) {
    expect_function_call(stat);
    will_return(stat, -1);
    
    int result = verify_path("/valid/path");
    assert_int_equal(result, -1);
}

static void test_verify_path_not_directory(void **state) {
    struct stat sb;
    memset(&sb, 0, sizeof(sb));
    sb.st_mode = S_IFREG;  // Regular file
    
    expect_function_call(stat);
    will_return(stat, 0);
    
    int result = verify_path("/path/to/file");
    assert_int_equal(result, -1);
}

static void test_verify_path_valid_directory(void **state) {
    struct stat sb;
    memset(&sb, 0, sizeof(sb));
    sb.st_mode = S_IFDIR;  // Directory
    
    expect_function_call(stat);
    will_return(stat, 0);
    
    int result = verify_path("/valid/path");
    assert_int_equal(result, 0);
}

static void test_verify_path_with_hex_escape(void **state) {
    struct stat sb;
    memset(&sb, 0, sizeof(sb));
    sb.st_mode = S_IFDIR;
    
    expect_function_call(stat);
    will_return(stat, 0);
    
    int result = verify_path("/path\\x20with\\x20spaces");
    assert_int_equal(result, 0);
}

static void test_add_device_new_device(void **state) {
    detected_devices = NULL;
    add_device("eth0", "veth123");
    
    assert_non_null(detected_devices);
    assert_string_equal(detected_devices->host_device, "eth0");
    assert_string_equal(detected_devices->guest_device, "veth123");
    
    freez((void *)detected_devices->host_device);
    freez((void *)detected_devices->guest_device);
    freez(detected_devices);
    detected_devices = NULL;
}

static void test_add_device_empty_guest(void **state) {
    detected_devices = NULL;
    add_device("eth0", "");
    
    assert_non_null(detected_devices);
    assert_string_equal(detected_devices->host_device, "eth0");
    assert_null(detected_devices->guest_device);
    
    freez((void *)detected_devices->host_device);
    freez(detected_devices);
    detected_devices = NULL;
}

static void test_add_device_same_host_guest(void **state) {
    detected_devices = NULL;
    add_device("eth0", "eth0");
    
    assert_non_null(detected_devices);
    assert_string_equal(detected_devices->host_device, "eth0");
    assert_null(detected_devices->guest_device);
    
    freez((void *)detected_devices->host_device);
    freez(detected_devices);
    detected_devices = NULL;
}

static void test_add_device_null_guest(void **state) {
    detected_devices = NULL;
    add_device("eth0", NULL);
    
    assert_non_null(detected_devices);
    assert_string_equal(detected_devices->host_device, "eth0");
    assert_null(detected_devices->guest_device);
    
    freez((void *)detected_devices->host_device);
    freez(detected_devices);
    detected_devices = NULL;
}

static void test_add_device_duplicate_update_guest(void **state) {
    detected_devices = NULL;
    add_device("eth0", "veth1");
    add_device("eth0", "veth2");
    
    assert_non_null(detected_devices);
    assert_string_equal(detected_devices->host_device, "eth0");
    assert_string_equal(detected_devices->guest_device, "veth2");
    
    freez((void *)detected_devices->host_device);
    freez((void *)detected_devices->guest_device);
    freez(detected_devices);
    detected_devices = NULL;
}

static void test_send_devices_empty_list(void **state) {
    detected_devices = NULL;
    int count = send_devices();
    assert_int_equal(count, 0);
}

static void test_send_devices_single_device(void **state) {
    detected_devices = NULL;
    add_device("eth0", "veth123");
    
    int count = send_devices();
    assert_int_equal(count, 1);
    
    freez((void *)detected_devices->host_device);
    freez((void *)detected_devices->guest_device);
    freez(detected_devices);
    detected_devices = NULL;
}

static void test_send_devices_multiple_devices(void **state) {
    detected_devices = NULL;
    add_device("eth0", "veth0");
    add_device("eth1", "veth1");
    
    int count = send_devices();
    assert_int_equal(count, 2);
    
    // Cleanup
    struct found_device *f = detected_devices;
    while (f) {
        struct found_device *next = f->next;
        freez((void *)f->host_device);
        if (f->guest_device)
            freez((void *)f->guest_device);
        freez(f);
        f = next;
    }
    detected_devices = NULL;
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // calc_num_ifaces tests
        cmocka_unit_test(test_calc_num_ifaces_with_single_interface),
        cmocka_unit_test(test_calc_num_ifaces_with_multiple_interfaces),
        cmocka_unit_test(test_calc_num_ifaces_with_null_root),
        
        // read_iface