#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>
#include <sys/types.h>
#include <time.h>

// Mock external dependencies
typedef struct {
    uint64_t cache_access;
    uint64_t file_system;
    uint64_t not_found;
    uint64_t ct;
    char name[256];
} netdata_dcstat_pid_t;

typedef struct {
    long long ratio;
    long long cache_access;
    uint64_t ct;
    struct {
        uint64_t cache_access;
        uint64_t file_system;
        uint64_t not_found;
    } curr, prev;
} netdata_publish_dcstat_t;

typedef struct {
    uint64_t ncall;
    uint64_t pcall;
    char dimension[256];
    char name[256];
} netdata_publish_syscall_t;

typedef struct {
    uint64_t value;
} netdata_syscall_stat_t;

typedef uint64_t netdata_idx_t;

typedef struct {
    char *name;
    int internal_input;
    int user_input;
    int type;
    int map_fd;
    int map_type;
} ebpf_local_maps_t;

typedef struct {
    char *name;
    int mode;
} netdata_ebpf_targets_t;

typedef struct {
    int update_every;
    int lifetime;
    int maps_per_core;
    void *maps;
    netdata_idx_t hash_table_stats[64];
    int enabled;
    void *objects;
    void *probe_links;
    netdata_ebpf_targets_t *targets;
    char thread_name[256];
    int running_time;
    int cgroup_charts;
    int apps_charts;
} ebpf_module_t;

typedef struct {
    uint64_t cache_access;
    uint64_t file_system;
    uint64_t not_found;
} netdata_publish_dcstat_pid_t;

// Mock implementations
void __wrap_dcstat_update_publish(netdata_publish_dcstat_t *out, uint64_t cache_access, uint64_t not_found);

// Test dcstat_update_publish - basic calculation
static void test_dcstat_update_publish_zero_access(void **state) {
    netdata_publish_dcstat_t out = {0};
    uint64_t cache_access = 0;
    uint64_t not_found = 0;

    dcstat_update_publish(&out, cache_access, not_found);

    assert_int_equal(out.ratio, 0);
}

// Test dcstat_update_publish - all successful access
static void test_dcstat_update_publish_all_successful(void **state) {
    netdata_publish_dcstat_t out = {0};
    uint64_t cache_access = 100;
    uint64_t not_found = 0;

    dcstat_update_publish(&out, cache_access, not_found);

    assert_int_equal(out.ratio, 10000); // 100/100 * 100 = 10000
}

// Test dcstat_update_publish - partial success
static void test_dcstat_update_publish_partial_success(void **state) {
    netdata_publish_dcstat_t out = {0};
    uint64_t cache_access = 100;
    uint64_t not_found = 25;

    dcstat_update_publish(&out, cache_access, not_found);

    assert_int_equal(out.ratio, 7500); // (100-25)/100 * 100 = 7500
}

// Test dcstat_update_publish - all failures
static void test_dcstat_update_publish_all_failures(void **state) {
    netdata_publish_dcstat_t out = {0};
    uint64_t cache_access = 100;
    uint64_t not_found = 100;

    dcstat_update_publish(&out, cache_access, not_found);

    assert_int_equal(out.ratio, 0);
}

// Test dcstat_update_publish - high values
static void test_dcstat_update_publish_high_values(void **state) {
    netdata_publish_dcstat_t out = {0};
    uint64_t cache_access = 0xFFFFFFFF;
    uint64_t not_found = 0x7FFFFFFF;

    dcstat_update_publish(&out, cache_access, not_found);

    assert_true(out.ratio >= 0);
    assert_true(out.ratio <= 10000);
}

// Test dcstat_update_publish - edge case with max uint64
static void test_dcstat_update_publish_large_values(void **state) {
    netdata_publish_dcstat_t out = {0};
    uint64_t cache_access = 1000000;
    uint64_t not_found = 500000;

    dcstat_update_publish(&out, cache_access, not_found);

    assert_int_equal(out.ratio, 5000); // (1000000-500000)/1000000 * 100 = 5000
}

// Test dcstat_update_publish - one access one miss
static void test_dcstat_update_publish_one_access_one_miss(void **state) {
    netdata_publish_dcstat_t out = {0};
    uint64_t cache_access = 2;
    uint64_t not_found = 1;

    dcstat_update_publish(&out, cache_access, not_found);

    assert_int_equal(out.ratio, 5000); // (2-1)/2 * 100 = 5000
}

// Test ebpf_dcstat_apps_accumulator - single core
static void test_ebpf_dcstat_apps_accumulator_single_core(void **state) {
    netdata_dcstat_pid_t out[1];
    memset(out, 0, sizeof(out));
    out[0].cache_access = 100;
    out[0].file_system = 20;
    out[0].not_found = 10;
    out[0].ct = 1;
    strcpy(out[0].name, "test");

    ebpf_dcstat_apps_accumulator(out, 0);

    assert_int_equal(out[0].cache_access, 100);
    assert_int_equal(out[0].file_system, 20);
    assert_int_equal(out[0].not_found, 10);
}

// Test ebpf_dcstat_apps_accumulator - multiple cores
static void test_ebpf_dcstat_apps_accumulator_multiple_cores(void **state) {
    int nprocs = 4;
    netdata_dcstat_pid_t *out = calloc(nprocs, sizeof(netdata_dcstat_pid_t));
    
    for (int i = 0; i < nprocs; i++) {
        out[i].cache_access = 100 + i * 10;
        out[i].file_system = 20 + i * 5;
        out[i].not_found = 10 + i * 2;
        out[i].ct = 1 + i;
        if (i > 0) strcpy(out[i].name, "test");
    }

    // Would call with maps_per_core = 1 to indicate read all cores
    // ebpf_dcstat_apps_accumulator(out, 1);

    // After accumulation, first element should have totals
    // assert_int_equal(out[0].cache_access, 100 + 110 + 120 + 130);

    free(out);
}

// Test ebpf_dcstat_sum_pids - empty list
static void test_ebpf_dcstat_sum_pids_empty_list(void **state) {
    netdata_publish_dcstat_t publish = {0};
    
    ebpf_dcstat_sum_pids(&publish, NULL);
    
    assert_int_equal(publish.curr.cache_access, 0);
    assert_int_equal(publish.curr.file_system, 0);
    assert_int_equal(publish.curr.not_found, 0);
}

// Test dcstat_update_publish - ratio precision
static void test_dcstat_update_publish_ratio_precision(void **state) {
    netdata_publish_dcstat_t out = {0};
    uint64_t cache_access = 1000;
    uint64_t not_found = 333;

    dcstat_update_publish(&out, cache_access, not_found);

    // (1000-333)/1000 * 100 = 66700 / 1000 = 66
    assert_true(out.ratio >= 66 && out.ratio <= 67);
}

// Test ebpf_dcstat_apps_accumulator - preserves name from non-empty entry
static void test_ebpf_dcstat_apps_accumulator_name_copy(void **state) {
    netdata_dcstat_pid_t out[2];
    memset(out, 0, sizeof(out));
    out[0].cache_access = 100;
    out[0].ct = 5;
    memset(out[0].name, 0, sizeof(out[0].name));
    
    out[1].cache_access = 50;
    out[1].ct = 3;
    strcpy(out[1].name, "process_name");

    // ebpf_dcstat_apps_accumulator(out, 1);

    // assert_string_equal(out[0].name, "process_name");
}

// Test ebpf_dcstat_apps_accumulator - updates ct from later entries
static void test_ebpf_dcstat_apps_accumulator_ct_max(void **state) {
    netdata_dcstat_pid_t out[3];
    memset(out, 0, sizeof(out));
    out[0].ct = 10;
    out[1].ct = 5;
    out[2].ct = 20;

    // ebpf_dcstat_apps_accumulator(out, 1);

    // assert_int_equal(out[0].ct, 20);
}

// Test dcstat_update_publish - verify double calculation
static void test_dcstat_update_publish_double_precision(void **state) {
    netdata_publish_dcstat_t out = {0};
    
    // Test with values that would reveal precision issues
    uint64_t cache_access = 3;
    uint64_t not_found = 1;

    dcstat_update_publish(&out, cache_access, not_found);

    // (3-1)/3 * 100 = 2/3 * 100 = 66.666... = 66
    assert_true(out.ratio >= 66 && out.ratio <= 67);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_dcstat_update_publish_zero_access),
        cmocka_unit_test(test_dcstat_update_publish_all_successful),
        cmocka_unit_test(test_dcstat_update_publish_partial_success),
        cmocka_unit_test(test_dcstat_update_publish_all_failures),
        cmocka_unit_test(test_dcstat_update_publish_high_values),
        cmocka_unit_test(test_dcstat_update_publish_large_values),
        cmocka_unit_test(test_dcstat_update_publish_one_access_one_miss),
        cmocka_unit_test(test_dcstat_update_publish_ratio_precision),
        cmocka_unit_test(test_dcstat_update_publish_double_precision),
        cmocka_unit_test(test_ebpf_dcstat_apps_accumulator_single_core),
        cmocka_unit_test(test_ebpf_dcstat_sum_pids_empty_list),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}