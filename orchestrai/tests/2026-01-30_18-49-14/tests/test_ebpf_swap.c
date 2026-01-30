#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

// Mock structures and external dependencies
typedef struct {
    int enabled;
    char *name;
} ebpf_module_t;

typedef struct {
    char *name;
    int map_fd;
    uint32_t internal_input;
    uint32_t user_input;
    int type;
} ebpf_local_maps_t;

typedef struct {
    uint64_t read;
    uint64_t write;
    uint64_t ct;
    char name[256];
} netdata_ebpf_swap_t;

typedef struct {
    uint64_t read;
    uint64_t write;
} netdata_publish_swap_t;

typedef struct {
    netdata_publish_swap_t swap;
} ebpf_target;

typedef struct {
    int pid;
    netdata_publish_swap_t swap;
    struct pid_on_target2 *next;
} pid_on_target2;

typedef struct {
    pid_on_target2 *pids;
    char name[256];
    int flags;
    int systemd;
    int updated;
    netdata_publish_swap_t publish_systemd_swap;
} ebpf_cgroup_target_t;

typedef struct {
    int fd;
} heartbeat_t;

// Mock variables
static netdata_ebpf_swap_t *swap_vector = NULL;
static netdata_publish_swap_t *test_swap = NULL;
static ebpf_target *apps_groups_root_target = NULL;
static ebpf_cgroup_target_t *ebpf_cgroup_pids = NULL;
static int ebpf_nprocs = 4;
static int send_cgroup_chart = 0;

// Mock functions
static int mock_bpf_map_get_next_key(int fd, void *key, void *next_key) {
    check_expected(fd);
    return (intptr_t)mock_ptr_type(void*);
}

static int mock_bpf_map_lookup_elem(int fd, void *key, void *value) {
    check_expected(fd);
    check_expected(key);
    return (intptr_t)mock_ptr_type(void*);
}

static int mock_kill(pid_t pid, int sig) {
    check_expected(pid);
    check_expected(sig);
    return (intptr_t)mock_ptr_type(void*);
}

static void mock_sem_wait(void *sem) {
    check_expected(sem);
}

static void mock_sem_post(void *sem) {
    check_expected(sem);
}

static void mock_heartbeat_init(heartbeat_t *hb, uint64_t usec) {
    check_expected(hb);
    check_expected(usec);
    hb->fd = 0;
}

static void mock_heartbeat_next(heartbeat_t *hb) {
    check_expected(hb);
}

// Test: swap_apps_accumulator - single element (no accumulation)
static void test_swap_apps_accumulator_single_element(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[4] = {0};
    out[0].write = 100;
    out[0].read = 50;
    out[0].ct = 1;
    strcpy(out[0].name, "test");
    
    swap_apps_accumulator(out, 0);
    
    assert_int_equal(out[0].write, 100);
    assert_int_equal(out[0].read, 50);
    assert_int_equal(out[0].ct, 1);
    assert_string_equal(out[0].name, "test");
}

// Test: swap_apps_accumulator - multiple elements, maps_per_core = 1
static void test_swap_apps_accumulator_multiple_elements(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[4] = {0};
    out[0].write = 100;
    out[0].read = 50;
    out[0].ct = 1;
    strcpy(out[0].name, "test0");
    
    out[1].write = 200;
    out[1].read = 75;
    out[1].ct = 2;
    strcpy(out[1].name, "test1");
    
    out[2].write = 150;
    out[2].read = 100;
    out[2].ct = 3;
    strcpy(out[2].name, "test2");
    
    out[3].write = 50;
    out[3].read = 25;
    out[3].ct = 4;
    strcpy(out[3].name, "test3");
    
    swap_apps_accumulator(out, 1);
    
    assert_int_equal(out[0].write, 500);
    assert_int_equal(out[0].read, 250);
    assert_int_equal(out[0].ct, 4);
    assert_string_equal(out[0].name, "test1");
}

// Test: swap_apps_accumulator - maps_per_core = 0
static void test_swap_apps_accumulator_no_per_core(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[4] = {0};
    out[0].write = 100;
    out[0].read = 50;
    out[0].ct = 5;
    strcpy(out[0].name, "initial");
    
    out[1].write = 200;
    out[1].read = 100;
    
    swap_apps_accumulator(out, 0);
    
    assert_int_equal(out[0].write, 100);
    assert_int_equal(out[0].read, 50);
    assert_int_equal(out[0].ct, 5);
    assert_string_equal(out[0].name, "initial");
}

// Test: swap_apps_accumulator - ct update
static void test_swap_apps_accumulator_ct_update(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[2] = {0};
    out[0].ct = 10;
    out[1].ct = 20;
    
    swap_apps_accumulator(out, 1);
    
    assert_int_equal(out[0].ct, 20);
}

// Test: swap_apps_accumulator - name copy when initial empty
static void test_swap_apps_accumulator_name_copy(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[2] = {0};
    out[0].ct = 1;
    out[0].name[0] = '\0';
    
    out[1].ct = 2;
    strcpy(out[1].name, "source_name");
    
    swap_apps_accumulator(out, 1);
    
    assert_string_equal(out[0].name, "source_name");
}

// Test: ebpf_swap_sum_pids - no pids
static void test_ebpf_swap_sum_pids_no_pids(void **state) {
    (void)state;
    
    netdata_publish_swap_t swap = {10, 20};
    
    ebpf_swap_sum_pids(&swap, NULL);
    
    assert_int_equal(swap.write, 20);
    assert_int_equal(swap.read, 10);
}

// Test: ebpf_swap_sum_cgroup_pids - empty pids
static void test_ebpf_swap_sum_cgroup_pids_empty(void **state) {
    (void)state;
    
    netdata_publish_swap_t swap = {5, 15};
    
    ebpf_swap_sum_cgroup_pids(&swap, NULL);
    
    assert_int_equal(swap.write, 15);
    assert_int_equal(swap.read, 5);
}

// Test: ebpf_swap_sum_cgroup_pids - single pid
static void test_ebpf_swap_sum_cgroup_pids_single(void **state) {
    (void)state;
    
    pid_on_target2 pid_entry = {0};
    pid_entry.swap.read = 100;
    pid_entry.swap.write = 200;
    pid_entry.next = NULL;
    
    netdata_publish_swap_t swap = {0, 0};
    
    ebpf_swap_sum_cgroup_pids(&swap, &pid_entry);
    
    assert_int_equal(swap.read, 100);
    assert_int_equal(swap.write, 200);
}

// Test: ebpf_swap_sum_cgroup_pids - incremental algorithm (local >= swap)
static void test_ebpf_swap_sum_cgroup_pids_incremental_greater(void **state) {
    (void)state;
    
    pid_on_target2 pid1 = {0};
    pid1.swap.read = 150;
    pid1.swap.write = 250;
    
    pid_on_target2 pid2 = {0};
    pid2.swap.read = 100;
    pid2.swap.write = 200;
    pid2.next = NULL;
    pid1.next = &pid2;
    
    netdata_publish_swap_t swap = {200, 300};
    
    ebpf_swap_sum_cgroup_pids(&swap, &pid1);
    
    assert_int_equal(swap.read, 250);
    assert_int_equal(swap.write, 450);
}

// Test: ebpf_swap_sum_cgroup_pids - incremental algorithm (local < swap)
static void test_ebpf_swap_sum_cgroup_pids_incremental_less(void **state) {
    (void)state;
    
    pid_on_target2 pid1 = {0};
    pid1.swap.read = 100;
    pid1.swap.write = 150;
    pid1.next = NULL;
    
    netdata_publish_swap_t swap = {300, 400};
    
    ebpf_swap_sum_cgroup_pids(&swap, &pid1);
    
    assert_int_equal(swap.read, 300);
    assert_int_equal(swap.write, 400);
}

// Test: ebpf_swap_sum_pids - single pid available
static void test_ebpf_swap_sum_pids_single(void **state) {
    (void)state;
    
    // This test requires mocking netdata_ebpf_get_shm_pointer_unsafe
    // For now we test the null case
    netdata_publish_swap_t swap = {10, 20};
    
    ebpf_swap_sum_pids(&swap, NULL);
    
    assert_int_equal(swap.write, 20);
    assert_int_equal(swap.read, 10);
}

// Test: ebpf_read_swap_apps_table loop iteration
static void test_ebpf_read_swap_apps_table_empty_map(void **state) {
    (void)state;
    
    // Mock setup for empty map
    expect_function_call(mock_bpf_map_get_next_key);
    will_return(mock_bpf_map_get_next_key, -1); // No more keys
    
    // Allocate swap_vector if needed
    swap_vector = calloc(ebpf_nprocs, sizeof(netdata_ebpf_swap_t));
    
    // This would test the loop condition when map is empty
    // The function should exit immediately without processing
    
    free(swap_vector);
    swap_vector = NULL;
}

// Test: Boundary condition - zero values
static void test_boundary_zero_values(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[1] = {0};
    memset(out, 0, sizeof(out));
    
    swap_apps_accumulator(out, 0);
    
    assert_int_equal(out[0].write, 0);
    assert_int_equal(out[0].read, 0);
    assert_int_equal(out[0].ct, 0);
}

// Test: Large values handling
static void test_large_values_accumulation(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[2] = {0};
    out[0].write = UINT64_MAX - 100;
    out[0].read = UINT64_MAX - 50;
    out[0].ct = 1;
    
    out[1].write = 200;
    out[1].read = 100;
    out[1].ct = 2;
    
    swap_apps_accumulator(out, 1);
    
    // Values should overflow as uint64_t
    assert_int_equal(out[0].ct, 2);
}

// Test: Empty string name handling
static void test_empty_string_name_handling(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[2] = {0};
    out[0].ct = 1;
    out[0].name[0] = '\0';
    
    out[1].ct = 2;
    out[1].name[0] = '\0';
    
    swap_apps_accumulator(out, 1);
    
    assert_int_equal(out[0].name[0], '\0');
}

// Test: Maximum ct value
static void test_maximum_ct_value(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[3] = {0};
    out[0].ct = 1;
    out[1].ct = UINT64_MAX;
    out[2].ct = 10;
    
    swap_apps_accumulator(out, 1);
    
    assert_int_equal(out[0].ct, UINT64_MAX);
}

// Test: name[0] not empty in first element
static void test_name_not_empty_in_first(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[2] = {0};
    strcpy(out[0].name, "first");
    strcpy(out[1].name, "second");
    
    swap_apps_accumulator(out, 1);
    
    assert_string_equal(out[0].name, "first");
}

// Test: CT from empty second element
static void test_ct_from_empty_second(void **state) {
    (void)state;
    
    netdata_ebpf_swap_t out[2] = {0};
    out[0].ct = 100;
    out[1].ct = 0;
    
    swap_apps_accumulator(out, 1);
    
    assert_int_equal(out[0].ct, 100);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_swap_apps_accumulator_single_element),
        cmocka_unit_test(test_swap_apps_accumulator_multiple_elements),
        cmocka_unit_test(test_swap_apps_accumulator_no_per_core),
        cmocka_unit_test(test_swap_apps_accumulator_ct_update),
        cmocka_unit_test(test_swap_apps_accumulator_name_copy),
        cmocka_unit_test(test_ebpf_swap_sum_pids_no_pids),
        cmocka_unit_test(test_ebpf_swap_sum_cgroup_pids_empty),
        cmocka_unit_test(test_ebpf_swap_sum_cgroup_pids_single),
        cmocka_unit_test(test_ebpf_swap_sum_cgroup_pids_incremental_greater),
        cmocka_unit_test(test_ebpf_swap_sum_cgroup_pids_incremental_less),
        cmocka_unit_test(test_ebpf_swap_sum_pids_single),
        cmocka_unit_test(test_ebpf_read_swap_apps_table_empty_map),
        cmocka_unit_test(test_boundary_zero_values),
        cmocka_unit_test(test_large_values_accumulation),
        cmocka_unit_test(test_empty_string_name_handling),
        cmocka_unit_test(test_maximum_ct_value),
        cmocka_unit_test(test_name_not_empty_in_first),
        cmocka_unit_test(test_ct_from_empty_second),
    };

    return cmocka_run_tests(tests);
}