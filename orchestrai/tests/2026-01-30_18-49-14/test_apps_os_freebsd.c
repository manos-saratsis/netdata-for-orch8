#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <sys/user.h>

// Mock declarations
extern long global_block_size;
extern usec_t system_current_time_ut;

// Forward declarations of functions under test
void apps_os_init_freebsd(void);
uint64_t apps_os_get_total_memory_freebsd(void);
bool apps_os_read_pid_fds_freebsd(struct pid_stat *p, void *ptr);
bool apps_os_get_pid_cmdline_freebsd(struct pid_stat *p, char *cmdline, size_t bytes);
bool apps_os_read_pid_io_freebsd(struct pid_stat *p, void *ptr);
bool apps_os_read_pid_limits_freebsd(struct pid_stat *p __maybe_unused, void *ptr __maybe_unused);
bool apps_os_read_pid_status_freebsd(struct pid_stat *p, void *ptr);
bool apps_os_read_pid_stat_freebsd(struct pid_stat *p, void *ptr);
bool apps_os_collect_all_pids_freebsd(void);

// Mock structures
typedef struct {
    uint32_t f_frsize;
    uint32_t f_bsize;
} mock_statvfs;

// Mock functions
static int mock_statvfs_success(const char *path, struct statvfs *buf) {
    (void)path;
    if (buf != NULL) {
        buf->f_frsize = 4096;
        buf->f_bsize = 512;
    }
    return 0;
}

static int mock_statvfs_failure(const char *path, struct statvfs *buf) {
    (void)path;
    (void)buf;
    return -1;
}

static int mock_gettimeofday_success(struct timeval *tv, struct timezone *tz) {
    (void)tz;
    if (tv != NULL) {
        tv->tv_sec = 1000000;
        tv->tv_usec = 500000;
    }
    return 0;
}

// Test: apps_os_init_freebsd - successful statvfs
static void test_apps_os_init_freebsd_statvfs_success(void **state) {
    (void)state;
    
    will_return(__wrap_statvfs, 0);
    
    apps_os_init_freebsd();
    
    assert_int_not_equal(global_block_size, 0);
}

// Test: apps_os_init_freebsd - statvfs failure
static void test_apps_os_init_freebsd_statvfs_failure(void **state) {
    (void)state;
    
    will_return(__wrap_statvfs, -1);
    
    apps_os_init_freebsd();
    
    assert_int_equal(global_block_size, 512);
}

// Test: apps_os_get_total_memory_freebsd - success
static void test_apps_os_get_total_memory_freebsd_success(void **state) {
    (void)state;
    
    will_return(__wrap_sysctl, 0);
    will_set_parameter(__wrap_sysctl, result, 8589934592ULL); // 8GB
    
    uint64_t result = apps_os_get_total_memory_freebsd();
    
    assert_int_equal(result, 8589934592ULL);
}

// Test: apps_os_get_total_memory_freebsd - sysctl failure
static void test_apps_os_get_total_memory_freebsd_sysctl_failure(void **state) {
    (void)state;
    
    will_return(__wrap_sysctl, -1);
    
    uint64_t result = apps_os_get_total_memory_freebsd();
    
    assert_int_equal(result, 0);
}

// Test: apps_os_read_pid_limits_freebsd
static void test_apps_os_read_pid_limits_freebsd(void **state) {
    (void)state;
    
    struct pid_stat p = {0};
    
    bool result = apps_os_read_pid_limits_freebsd(&p, NULL);
    
    assert_false(result);
}

// Test: apps_os_get_pid_cmdline_freebsd - success
static void test_apps_os_get_pid_cmdline_freebsd_success(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    char cmdline[256] = {0};
    
    will_return(__wrap_sysctl, 0);
    will_set_parameter(__wrap_sysctl, cmdline_out, "test\0command\0line");
    
    bool result = apps_os_get_pid_cmdline_freebsd(&p, cmdline, sizeof(cmdline));
    
    assert_true(result);
}

// Test: apps_os_get_pid_cmdline_freebsd - sysctl failure
static void test_apps_os_get_pid_cmdline_freebsd_sysctl_failure(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    char cmdline[256] = {0};
    
    will_return(__wrap_sysctl, -1);
    
    bool result = apps_os_get_pid_cmdline_freebsd(&p, cmdline, sizeof(cmdline));
    
    assert_false(result);
}

// Test: apps_os_read_pid_io_freebsd
static void test_apps_os_read_pid_io_freebsd(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    struct kinfo_proc proc_info = {0};
    proc_info.ki_rusage.ru_inblock = 100;
    proc_info.ki_rusage.ru_oublock = 50;
    
    bool result = apps_os_read_pid_io_freebsd(&p, &proc_info);
    
    assert_true(result);
}

// Test: apps_os_read_pid_status_freebsd
static void test_apps_os_read_pid_status_freebsd(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    struct kinfo_proc proc_info = {0};
    proc_info.ki_uid = 1000;
    proc_info.ki_groups[0] = 1000;
    proc_info.ki_size = 1024000;
    proc_info.ki_rssize = 256;
    
    bool result = apps_os_read_pid_status_freebsd(&p, &proc_info);
    
    assert_true(result);
    assert_int_equal(p.uid, 1000);
    assert_int_equal(p.gid, 1000);
}

// Test: apps_os_read_pid_stat_freebsd - idle thread (should return false)
static void test_apps_os_read_pid_stat_freebsd_idle_thread(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    struct kinfo_proc proc_info = {0};
    proc_info.ki_tdflags = TDF_IDLETD;
    
    bool result = apps_os_read_pid_stat_freebsd(&p, &proc_info);
    
    assert_false(result);
}

// Test: apps_os_read_pid_stat_freebsd - normal process
static void test_apps_os_read_pid_stat_freebsd_normal_process(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    struct kinfo_proc proc_info = {0};
    proc_info.ki_tdflags = 0;
    proc_info.ki_comm[0] = 't';
    proc_info.ki_comm[1] = 'e';
    proc_info.ki_comm[2] = 's';
    proc_info.ki_comm[3] = 't';
    proc_info.ki_comm[4] = '\0';
    proc_info.ki_ppid = 100;
    proc_info.ki_rusage.ru_minflt = 10;
    proc_info.ki_rusage.ru_majflt = 5;
    proc_info.ki_rusage.ru_utime.tv_sec = 1;
    proc_info.ki_rusage.ru_utime.tv_usec = 500000;
    proc_info.ki_rusage.ru_stime.tv_sec = 0;
    proc_info.ki_rusage.ru_stime.tv_usec = 100000;
    proc_info.ki_numthreads = 4;
    proc_info.ki_start.tv_sec = 999999;
    proc_info.ki_start.tv_usec = 0;
    
    system_current_time_ut = 1000000000000; // microseconds
    
    bool result = apps_os_read_pid_stat_freebsd(&p, &proc_info);
    
    assert_true(result);
}

// Test: apps_os_read_pid_fds_freebsd - sysctl failure getting size
static void test_apps_os_read_pid_fds_freebsd_sysctl_size_failure(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    
    will_return(__wrap_sysctl, -1);
    
    bool result = apps_os_read_pid_fds_freebsd(&p, NULL);
    
    assert_false(result);
}

// Test: apps_os_read_pid_fds_freebsd - sysctl failure getting data
static void test_apps_os_read_pid_fds_freebsd_sysctl_data_failure(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    
    will_return(__wrap_sysctl, 0); // First call succeeds
    will_set_parameter(__wrap_sysctl, size_out, 1024);
    will_return(__wrap_sysctl, -1); // Second call fails
    
    bool result = apps_os_read_pid_fds_freebsd(&p, NULL);
    
    assert_false(result);
}

// Test: apps_os_read_pid_fds_freebsd - with zero size
static void test_apps_os_read_pid_fds_freebsd_zero_size(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234};
    
    will_return(__wrap_sysctl, 0);
    will_set_parameter(__wrap_sysctl, size_out, 0);
    
    bool result = apps_os_read_pid_fds_freebsd(&p, NULL);
    
    assert_true(result);
}

// Test: apps_os_read_pid_fds_freebsd - file descriptor processing
static void test_apps_os_read_pid_fds_freebsd_fd_processing(void **state) {
    (void)state;
    
    struct pid_stat p = {.pid = 1234, .fds = NULL, .fds_size = 0};
    
    will_return(__wrap_sysctl, 0);
    will_set_parameter(__wrap_sysctl, size_out, sizeof(struct kinfo_file));
    will_return(__wrap_sysctl, 0);
    
    bool result = apps_os_read_pid_fds_freebsd(&p, NULL);
    
    assert_true(result);
}

// Test: apps_os_collect_all_pids_freebsd - sysctl failure getting size
static void test_apps_os_collect_all_pids_freebsd_sysctl_size_failure(void **state) {
    (void)state;
    
    will_return(__wrap_sysctl, -1);
    
    bool result = apps_os_collect_all_pids_freebsd();
    
    assert_false(result);
}

// Test: apps_os_collect_all_pids_freebsd - sysctl failure getting data
static void test_apps_os_collect_all_pids_freebsd_sysctl_data_failure(void **state) {
    (void)state;
    
    will_return(__wrap_sysctl, 0);
    will_set_parameter(__wrap_sysctl, size_out, sizeof(struct kinfo_proc));
    will_return(__wrap_sysctl, -1);
    
    bool result = apps_os_collect_all_pids_freebsd();
    
    assert_false(result);
}

// Test: apps_os_collect_all_pids_freebsd - success with processes
static void test_apps_os_collect_all_pids_freebsd_success(void **state) {
    (void)state;
    
    will_return(__wrap_sysctl, 0);
    will_set_parameter(__wrap_sysctl, size_out, sizeof(struct kinfo_proc));
    will_return(__wrap_sysctl, 0);
    will_return(__wrap_gettimeofday, 0);
    
    bool result = apps_os_collect_all_pids_freebsd();
    
    assert_true(result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_apps_os_init_freebsd_statvfs_success),
        cmocka_unit_test(test_apps_os_init_freebsd_statvfs_failure),
        cmocka_unit_test(test_apps_os_get_total_memory_freebsd_success),
        cmocka_unit_test(test_apps_os_get_total_memory_freebsd_sysctl_failure),
        cmocka_unit_test(test_apps_os_read_pid_limits_freebsd),
        cmocka_unit_test(test_apps_os_get_pid_cmdline_freebsd_success),
        cmocka_unit_test(test_apps_os_get_pid_cmdline_freebsd_sysctl_failure),
        cmocka_unit_test(test_apps_os_read_pid_io_freebsd),
        cmocka_unit_test(test_apps_os_read_pid_status_freebsd),
        cmocka_unit_test(test_apps_os_read_pid_stat_freebsd_idle_thread),
        cmocka_unit_test(test_apps_os_read_pid_stat_freebsd_normal_process),
        cmocka_unit_test(test_apps_os_read_pid_fds_freebsd_sysctl_size_failure),
        cmocka_unit_test(test_apps_os_read_pid_fds_freebsd_sysctl_data_failure),
        cmocka_unit_test(test_apps_os_read_pid_fds_freebsd_zero_size),
        cmocka_unit_test(test_apps_os_read_pid_fds_freebsd_fd_processing),
        cmocka_unit_test(test_apps_os_collect_all_pids_freebsd_sysctl_size_failure),
        cmocka_unit_test(test_apps_os_collect_all_pids_freebsd_sysctl_data_failure),
        cmocka_unit_test(test_apps_os_collect_all_pids_freebsd_success),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}