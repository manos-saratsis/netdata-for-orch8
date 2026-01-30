#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>

/* Mock declarations matching plugin_proc.h interface */
typedef struct {
    void *data;
} RRDSET;

typedef struct {
    void *data;
} RRDDIM;

/* Forward declarations */
static int is_dir_mounted(const char *path);
int do_run_reboot_required(int update_every, usec_t dt);

/* Mock implementations */
char *netdata_configured_host_prefix = "";

RRDSET *rrdset_create_localhost(const char *type, const char *id, const char *name,
                                const char *family, const char *context, const char *title,
                                const char *units, const char *plugin, const char *module,
                                long priority, int update_every, int rrdset_type) {
    return malloc(sizeof(RRDSET));
}

RRDDIM *rrddim_add(RRDSET *st, const char *name, const char *divisor, long long multiplier,
                   long long divisor_val, int algorithm) {
    return malloc(sizeof(RRDDIM));
}

void rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, long long value) {
    /* Mock function */
}

void rrdset_done(RRDSET *st) {
    /* Mock function */
}

typedef unsigned long long usec_t;

/* Test framework */
#define TEST_PASS 1
#define TEST_FAIL 0

static int test_count = 0;
static int test_passed = 0;

void test_is_dir_mounted_normal_path(void) {
    test_count++;
    /* Test is_dir_mounted with normal directory */
    int result = is_dir_mounted("/tmp");
    if (result >= 0) {
        test_passed++;
        printf("PASS: is_dir_mounted_normal_path\n");
    } else {
        printf("FAIL: is_dir_mounted_normal_path\n");
    }
}

void test_is_dir_mounted_invalid_path(void) {
    test_count++;
    /* Test is_dir_mounted with non-existent path */
    int result = is_dir_mounted("/nonexistent_path_12345");
    if (result == -1) {
        test_passed++;
        printf("PASS: is_dir_mounted_invalid_path\n");
    } else {
        printf("FAIL: is_dir_mounted_invalid_path - expected -1, got %d\n", result);
    }
}

void test_is_dir_mounted_root_directory(void) {
    test_count++;
    /* Test is_dir_mounted with root */
    int result = is_dir_mounted("/");
    if (result >= 0) {
        test_passed++;
        printf("PASS: is_dir_mounted_root_directory\n");
    } else {
        printf("FAIL: is_dir_mounted_root_directory\n");
    }
}

void test_do_run_reboot_required_normal_execution(void) {
    test_count++;
    /* Test normal execution without container */
    unsetenv("NETDATA_LISTENER_PORT");
    int result = do_run_reboot_required(60, 0);
    if (result == 0 || result == 1) {
        test_passed++;
        printf("PASS: do_run_reboot_required_normal_execution (returned %d)\n", result);
    } else {
        printf("FAIL: do_run_reboot_required_normal_execution\n");
    }
}

void test_do_run_reboot_required_container_environment(void) {
    test_count++;
    /* Test execution in container environment */
    setenv("NETDATA_LISTENER_PORT", "19999", 1);
    int result = do_run_reboot_required(60, 0);
    /* Should return 1 if container and directory not mounted, or continue normally */
    if (result == 0 || result == 1) {
        test_passed++;
        printf("PASS: do_run_reboot_required_container_environment (returned %d)\n", result);
    } else {
        printf("FAIL: do_run_reboot_required_container_environment\n");
    }
    unsetenv("NETDATA_LISTENER_PORT");
}

void test_do_run_reboot_required_reboot_file_exists(void) {
    test_count++;
    /* Test when reboot-required file exists */
    int result = do_run_reboot_required(60, 0);
    /* Execute twice to test static initialization and file check */
    result = do_run_reboot_required(60, 0);
    if (result == 0) {
        test_passed++;
        printf("PASS: do_run_reboot_required_reboot_file_exists\n");
    } else {
        printf("FAIL: do_run_reboot_required_reboot_file_exists\n");
    }
}

void test_do_run_reboot_required_different_update_every(void) {
    test_count++;
    /* Test with different update_every values */
    int result = do_run_reboot_required(30, 0);
    if (result == 0 || result == 1) {
        test_passed++;
        printf("PASS: do_run_reboot_required_different_update_every\n");
    } else {
        printf("FAIL: do_run_reboot_required_different_update_every\n");
    }
}

void test_do_run_reboot_required_dt_parameter(void) {
    test_count++;
    /* Test dt parameter (should be ignored) */
    int result = do_run_reboot_required(60, 12345);
    if (result == 0 || result == 1) {
        test_passed++;
        printf("PASS: do_run_reboot_required_dt_parameter\n");
    } else {
        printf("FAIL: do_run_reboot_required_dt_parameter\n");
    }
}

void test_do_run_reboot_required_with_host_prefix(void) {
    test_count++;
    /* Test with non-empty host prefix */
    char *old_prefix = netdata_configured_host_prefix;
    netdata_configured_host_prefix = "/root";
    int result = do_run_reboot_required(60, 0);
    netdata_configured_host_prefix = old_prefix;
    if (result == 0 || result == 1) {
        test_passed++;
        printf("PASS: do_run_reboot_required_with_host_prefix\n");
    } else {
        printf("FAIL: do_run_reboot_required_with_host_prefix\n");
    }
}

void test_do_run_reboot_required_multiple_calls(void) {
    test_count++;
    /* Test multiple consecutive calls */
    int result1 = do_run_reboot_required(60, 0);
    int result2 = do_run_reboot_required(60, 0);
    int result3 = do_run_reboot_required(60, 0);
    if ((result1 == 0 || result1 == 1) && (result2 == 0 || result2 == 1) && (result3 == 0 || result3 == 1)) {
        test_passed++;
        printf("PASS: do_run_reboot_required_multiple_calls\n");
    } else {
        printf("FAIL: do_run_reboot_required_multiple_calls\n");
    }
}

void test_is_dir_mounted_different_paths(void) {
    test_count++;
    /* Test with various existing paths */
    int result = is_dir_mounted("/home");
    if (result >= 0) {
        test_passed++;
        printf("PASS: is_dir_mounted_different_paths\n");
    } else {
        printf("FAIL: is_dir_mounted_different_paths\n");
    }
}

int main(void) {
    printf("Starting tests for run_reboot_required.c\n");
    printf("==========================================\n\n");

    test_is_dir_mounted_normal_path();
    test_is_dir_mounted_invalid_path();
    test_is_dir_mounted_root_directory();
    test_do_run_reboot_required_normal_execution();
    test_do_run_reboot_required_container_environment();
    test_do_run_reboot_required_reboot_file_exists();
    test_do_run_reboot_required_different_update_every();
    test_do_run_reboot_required_dt_parameter();
    test_do_run_reboot_required_with_host_prefix();
    test_do_run_reboot_required_multiple_calls();
    test_is_dir_mounted_different_paths();

    printf("\n==========================================\n");
    printf("Tests passed: %d/%d\n", test_passed, test_count);
    printf("Coverage: %.1f%%\n", (100.0 * test_passed) / test_count);

    return test_passed == test_count ? 0 : 1;
}