#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

// Mock for system functions
#include "collectors-ipc/ebpf-ipc.h"

// Mock variables for external dependencies
extern netdata_ebpf_pid_stats_t *integration_shm;
extern int shm_fd_ebpf_integration;
extern sem_t *shm_mutex_ebpf_integration;
extern ebpf_user_mem_stat_t ebpf_stat_values;
extern bool using_vector;
static Pvoid_t ebpf_ipc_JudyL_test = NULL;

// Mock functions
int __wrap_shm_open(const char *name, int oflag, mode_t mode) {
    return mock_type(int);
}

int __wrap_sem_open(const char *name, int oflag, ...) {
    return mock_type(int);
}

int __wrap_ftruncate(int fd, off_t length) {
    return mock_type(int);
}

int __wrap_close(int fd) {
    return mock_type(int);
}

int __wrap_sem_close(sem_t *sem) {
    return mock_type(int);
}

void* __wrap_nd_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return mock_type(void*);
}

void __wrap_nd_munmap(void *addr, size_t length) {
    mock_type(void);
}

void __wrap_nd_log(nd_log_source_t source, nd_log_priority_t priority, const char *fmt, ...) {
    mock_type(void);
}

size_t __wrap_os_get_system_pid_max(void) {
    return mock_type(size_t);
}

// Test: netdata_integration_initialize_shm with zero pids
static void test_netdata_integration_initialize_shm_zero_pids(void **state) {
    int result = netdata_integration_initialize_shm(0);
    assert_int_equal(result, -1);
}

// Test: netdata_integration_initialize_shm with valid pids
static void test_netdata_integration_initialize_shm_valid_pids(void **state) {
    size_t pids = 100;
    
    expect_value(__wrap_os_get_system_pid_max, void, 0);
    will_return(__wrap_os_get_system_pid_max, 100);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, 3);
    
    expect_value(__wrap_ftruncate, fd, 3);
    expect_value(__wrap_ftruncate, length, pids * sizeof(netdata_ebpf_pid_stats_t));
    will_return(__wrap_ftruncate, 0);
    
    expect_value(__wrap_nd_mmap, length, pids * sizeof(netdata_ebpf_pid_stats_t));
    netdata_ebpf_pid_stats_t *mock_shm = malloc(pids * sizeof(netdata_ebpf_pid_stats_t));
    will_return(__wrap_nd_mmap, mock_shm);
    
    expect_string(__wrap_sem_open, name, NETDATA_EBPF_SHM_INTEGRATION_NAME);
    sem_t *mock_sem = (sem_t*)malloc(sizeof(sem_t));
    will_return(__wrap_sem_open, mock_sem);
    
    int result = netdata_integration_initialize_shm(pids);
    assert_int_equal(result, 0);
    
    // Cleanup
    free(mock_shm);
    free(mock_sem);
}

// Test: netdata_integration_initialize_shm with shm_open failure
static void test_netdata_integration_initialize_shm_shm_open_failure(void **state) {
    size_t pids = 100;
    
    expect_value(__wrap_os_get_system_pid_max, void, 0);
    will_return(__wrap_os_get_system_pid_max, 100);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, -1);
    
    expect_any(__wrap_nd_log, void);
    will_return(__wrap_nd_log, 0);
    
    int result = netdata_integration_initialize_shm(pids);
    assert_int_equal(result, -1);
}

// Test: netdata_integration_initialize_shm with ftruncate failure
static void test_netdata_integration_initialize_shm_ftruncate_failure(void **state) {
    size_t pids = 100;
    
    expect_value(__wrap_os_get_system_pid_max, void, 0);
    will_return(__wrap_os_get_system_pid_max, 100);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, 3);
    
    expect_value(__wrap_ftruncate, fd, 3);
    will_return(__wrap_ftruncate, -1);
    
    expect_any(__wrap_nd_log, void);
    will_return(__wrap_nd_log, 0);
    
    expect_value(__wrap_close, fd, 3);
    will_return(__wrap_close, 0);
    
    int result = netdata_integration_initialize_shm(pids);
    assert_int_equal(result, -1);
}

// Test: netdata_integration_initialize_shm with nd_mmap failure
static void test_netdata_integration_initialize_shm_mmap_failure(void **state) {
    size_t pids = 100;
    
    expect_value(__wrap_os_get_system_pid_max, void, 0);
    will_return(__wrap_os_get_system_pid_max, 100);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, 3);
    
    expect_value(__wrap_ftruncate, fd, 3);
    will_return(__wrap_ftruncate, 0);
    
    expect_value(__wrap_nd_mmap, length, pids * sizeof(netdata_ebpf_pid_stats_t));
    will_return(__wrap_nd_mmap, NULL);
    
    expect_any(__wrap_nd_log, void);
    will_return(__wrap_nd_log, 0);
    
    expect_value(__wrap_close, fd, 3);
    will_return(__wrap_close, 0);
    
    int result = netdata_integration_initialize_shm(pids);
    assert_int_equal(result, -1);
}

// Test: netdata_integration_initialize_shm with sem_open failure
static void test_netdata_integration_initialize_shm_sem_open_failure(void **state) {
    size_t pids = 100;
    
    expect_value(__wrap_os_get_system_pid_max, void, 0);
    will_return(__wrap_os_get_system_pid_max, 100);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, 3);
    
    expect_value(__wrap_ftruncate, fd, 3);
    will_return(__wrap_ftruncate, 0);
    
    expect_value(__wrap_nd_mmap, length, pids * sizeof(netdata_ebpf_pid_stats_t));
    netdata_ebpf_pid_stats_t *mock_shm = malloc(pids * sizeof(netdata_ebpf_pid_stats_t));
    will_return(__wrap_nd_mmap, mock_shm);
    
    expect_string(__wrap_sem_open, name, NETDATA_EBPF_SHM_INTEGRATION_NAME);
    will_return(__wrap_sem_open, (int)SEM_FAILED);
    
    expect_any(__wrap_nd_log, void);
    will_return(__wrap_nd_log, 0);
    
    expect_value(__wrap_nd_munmap, length, pids * sizeof(netdata_ebpf_pid_stats_t));
    will_return(__wrap_nd_munmap, 0);
    
    expect_value(__wrap_close, fd, 3);
    will_return(__wrap_close, 0);
    
    int result = netdata_integration_initialize_shm(pids);
    assert_int_equal(result, -1);
    
    free(mock_shm);
}

// Test: netdata_integration_cleanup_shm with all resources
static void test_netdata_integration_cleanup_shm_all_resources(void **state) {
    size_t pids = 100;
    netdata_ebpf_pid_stats_t *mock_shm = malloc(pids * sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    shm_fd_ebpf_integration = 3;
    shm_mutex_ebpf_integration = (sem_t*)malloc(sizeof(sem_t));
    ebpf_stat_values.total = pids;
    
    expect_value(__wrap_sem_close, void, 0);
    will_return(__wrap_sem_close, 0);
    
    expect_value(__wrap_nd_munmap, length, pids * sizeof(netdata_ebpf_pid_stats_t));
    will_return(__wrap_nd_munmap, 0);
    
    expect_value(__wrap_close, fd, 3);
    will_return(__wrap_close, 0);
    
    netdata_integration_cleanup_shm();
    
    assert_null(integration_shm);
    assert_int_equal(shm_fd_ebpf_integration, -1);
}

// Test: netdata_integration_cleanup_shm with no resources
static void test_netdata_integration_cleanup_shm_no_resources(void **state) {
    integration_shm = NULL;
    shm_fd_ebpf_integration = -1;
    shm_mutex_ebpf_integration = SEM_FAILED;
    
    netdata_integration_cleanup_shm();
    
    assert_null(integration_shm);
    assert_int_equal(shm_fd_ebpf_integration, -1);
    assert_int_equal(shm_mutex_ebpf_integration, SEM_FAILED);
}

// Test: netdata_integration_cleanup_shm with only shm allocated
static void test_netdata_integration_cleanup_shm_only_shm(void **state) {
    size_t pids = 100;
    netdata_ebpf_pid_stats_t *mock_shm = malloc(pids * sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    shm_fd_ebpf_integration = -1;
    shm_mutex_ebpf_integration = SEM_FAILED;
    ebpf_stat_values.total = pids;
    
    expect_value(__wrap_nd_munmap, length, pids * sizeof(netdata_ebpf_pid_stats_t));
    will_return(__wrap_nd_munmap, 0);
    
    netdata_integration_cleanup_shm();
    
    assert_null(integration_shm);
}

// Test: netdata_ebpf_get_shm_pointer_unsafe with NULL shm
static void test_netdata_ebpf_get_shm_pointer_unsafe_null_shm(void **state) {
    integration_shm = NULL;
    ebpf_stat_values.current = 0;
    ebpf_stat_values.total = 10;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(1, NETDATA_EBPF_PIDS_PROCESS_IDX);
    assert_null(result);
}

// Test: netdata_ebpf_get_shm_pointer_unsafe with full capacity
static void test_netdata_ebpf_get_shm_pointer_unsafe_full_capacity(void **state) {
    size_t pids = 10;
    netdata_ebpf_pid_stats_t *mock_shm = calloc(pids, sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    ebpf_stat_values.current = 9;
    ebpf_stat_values.total = 10;
    using_vector = true;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(1, NETDATA_EBPF_PIDS_PROCESS_IDX);
    assert_null(result);
    
    free(mock_shm);
}

// Test: netdata_ebpf_get_shm_pointer_unsafe with vector mode
static void test_netdata_ebpf_get_shm_pointer_unsafe_vector_mode(void **state) {
    size_t pids = 100;
    netdata_ebpf_pid_stats_t *mock_shm = calloc(pids, sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    ebpf_stat_values.current = 0;
    ebpf_stat_values.total = pids;
    using_vector = true;
    
    uint32_t pid = 10;
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_non_null(result);
    assert_int_equal(result->pid, pid);
    assert_int_equal(ebpf_stat_values.current, 1);
    
    free(mock_shm);
}

// Test: netdata_ebpf_get_shm_pointer_unsafe with vector mode and existing entry
static void test_netdata_ebpf_get_shm_pointer_unsafe_vector_existing(void **state) {
    size_t pids = 100;
    netdata_ebpf_pid_stats_t *mock_shm = calloc(pids, sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    ebpf_stat_values.current = 1;
    ebpf_stat_values.total = pids;
    using_vector = true;
    
    uint32_t pid = 10;
    mock_shm[pid].threads = (NETDATA_EBPF_PIDS_PROCESS_IDX << 1);
    mock_shm[pid].pid = pid;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    
    assert_non_null(result);
    assert_int_equal(ebpf_stat_values.current, 1); // Should not increment
    
    free(mock_shm);
}

// Test: netdata_ebpf_get_shm_pointer_unsafe with pid out of bounds
static void test_netdata_ebpf_get_shm_pointer_unsafe_pid_out_of_bounds(void **state) {
    size_t pids = 100;
    netdata_ebpf_pid_stats_t *mock_shm = calloc(pids, sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    ebpf_stat_values.current = 0;
    ebpf_stat_values.total = pids;
    using_vector = true;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(1000, NETDATA_EBPF_PIDS_PROCESS_IDX);
    assert_null(result);
    
    free(mock_shm);
}

// Test: netdata_ebpf_reset_shm_pointer_unsafe with vector mode
static void test_netdata_ebpf_reset_shm_pointer_unsafe_vector_mode(void **state) {
    size_t pids = 100;
    netdata_ebpf_pid_stats_t *mock_shm = calloc(pids, sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    ebpf_stat_values.current = 1;
    ebpf_stat_values.total = pids;
    using_vector = true;
    
    uint32_t pid = 10;
    mock_shm[pid].threads = (NETDATA_EBPF_PIDS_PROCESS_IDX << 1);
    mock_shm[pid].pid = pid;
    
    expect_value(__wrap_bpf_map_delete_elem, void, 0);
    will_return(__wrap_bpf_map_delete_elem, 0);
    
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(3, pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_false(result);
    assert_int_equal(ebpf_stat_values.current, 0);
    
    free(mock_shm);
}

// Test: netdata_ebpf_reset_shm_pointer_unsafe with vector mode and remaining threads
static void test_netdata_ebpf_reset_shm_pointer_unsafe_vector_remaining_threads(void **state) {
    size_t pids = 100;
    netdata_ebpf_pid_stats_t *mock_shm = calloc(pids, sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    ebpf_stat_values.current = 1;
    ebpf_stat_values.total = pids;
    using_vector = true;
    
    uint32_t pid = 10;
    // Set both PROCESS and SOCKET threads
    mock_shm[pid].threads = (NETDATA_EBPF_PIDS_PROCESS_IDX << 1) | (NETDATA_EBPF_PIDS_SOCKET_IDX << 1);
    mock_shm[pid].pid = pid;
    
    expect_value(__wrap_bpf_map_delete_elem, void, 0);
    will_return(__wrap_bpf_map_delete_elem, 0);
    
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(3, pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_true(result);
    assert_int_equal(ebpf_stat_values.current, 1); // Should not decrement
    
    free(mock_shm);
}

// Test: netdata_ebpf_reset_shm_pointer_unsafe with socket index
static void test_netdata_ebpf_reset_shm_pointer_unsafe_socket_index(void **state) {
    size_t pids = 100;
    netdata_ebpf_pid_stats_t *mock_shm = calloc(pids, sizeof(netdata_ebpf_pid_stats_t));
    
    integration_shm = mock_shm;
    ebpf_stat_values.current = 1;
    ebpf_stat_values.total = pids;
    using_vector = true;
    
    uint32_t pid = 10;
    mock_shm[pid].threads = (NETDATA_EBPF_PIDS_SOCKET_IDX << 1);
    mock_shm[pid].pid = pid;
    
    // Socket index should NOT call bpf_map_delete_elem
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(3, pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    
    assert_false(result);
    assert_int_equal(ebpf_stat_values.current, 0);
    
    free(mock_shm);
}

// Test: netdata_ebpf_reset_shm_pointer_unsafe without integration_shm
static void test_netdata_ebpf_reset_shm_pointer_unsafe_no_shm(void **state) {
    integration_shm = NULL;
    using_vector = true;
    
    expect_value(__wrap_bpf_map_delete_elem, void, 0);
    will_return(__wrap_bpf_map_delete_elem, 0);
    
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(3, 10, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Function should handle NULL shm gracefully
    assert_false(result);
}

// Test: netdata_integration_current_ipc_data
static void test_netdata_integration_current_ipc_data(void **state) {
    ebpf_stat_values.current = 42;
    ebpf_stat_values.total = 100;
    
    ebpf_user_mem_stat_t result;
    netdata_integration_current_ipc_data(&result);
    
    assert_int_equal(result.current, 42);
    assert_int_equal(result.total, 100);
}

// Test: netdata_integration_current_ipc_data with zero values
static void test_netdata_integration_current_ipc_data_zero_values(void **state) {
    ebpf_stat_values.current = 0;
    ebpf_stat_values.total = 0;
    
    ebpf_user_mem_stat_t result;
    netdata_integration_current_ipc_data(&result);
    
    assert_int_equal(result.current, 0);
    assert_int_equal(result.total, 0);
}

// Test: netdata_integration_current_ipc_data with max values
static void test_netdata_integration_current_ipc_data_max_values(void **state) {
    ebpf_stat_values.current = UINT32_MAX;
    ebpf_stat_values.total = UINT32_MAX;
    
    ebpf_user_mem_stat_t result;
    netdata_integration_current_ipc_data(&result);
    
    assert_int_equal(result.current, UINT32_MAX);
    assert_int_equal(result.total, UINT32_MAX);
}

// Test: All ebpf_pids_index enum values
static void test_ebpf_pids_index_enum_values(void **state) {
    assert_int_equal(NETDATA_EBPF_PIDS_PROCESS_IDX, 0);
    assert_int_equal(NETDATA_EBPF_PIDS_SOCKET_IDX, 1);
    assert_int_equal(NETDATA_EBPF_PIDS_CACHESTAT_IDX, 2);
    assert_int_equal(NETDATA_EBPF_PIDS_DCSTAT_IDX, 3);
    assert_int_equal(NETDATA_EBPF_PIDS_SWAP_IDX, 4);
    assert_int_equal(NETDATA_EBPF_PIDS_VFS_IDX, 5);
    assert_int_equal(NETDATA_EBPF_PIDS_FD_IDX, 6);
    assert_int_equal(NETDATA_EBPF_PIDS_SHM_IDX, 7);
    assert_int_equal(NETDATA_EBPF_PIDS_PROC_FILE, 8);
    assert_int_equal(NETDATA_EBPF_PIDS_END_IDX, 9);
}

// Test: Structure sizes
static void test_structure_sizes(void **state) {
    assert_int_equal(sizeof(ebpf_user_mem_stat_t), sizeof(uint32_t) * 2);
    assert_int_equal(sizeof(ebpf_process_stat_t), 
                     sizeof(uint64_t) + 4 * sizeof(uint32_t) + TASK_COMM_LEN + 5 * sizeof(uint32_t));
    assert_int_equal(sizeof(ebpf_publish_process_t),
                     sizeof(uint64_t) + 5 * sizeof(uint32_t));
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_netdata_integration_initialize_shm_zero_pids),
        cmocka_unit_test(test_netdata_integration_initialize_shm_valid_pids),
        cmocka_unit_test(test_netdata_integration_initialize_shm_shm_open_failure),
        cmocka_unit_test(test_netdata_integration_initialize_shm_ftruncate_failure),
        cmocka_unit_test(test_netdata_integration_initialize_shm_mmap_failure),
        cmocka_unit_test(test_netdata_integration_initialize_shm_sem_open_failure),
        cmocka_unit_test(test_netdata_integration_cleanup_shm_all_resources),
        cmocka_unit_test(test_netdata_integration_cleanup_shm_no_resources),
        cmocka_unit_test(test_netdata_integration_cleanup_shm_only_shm),
        cmocka_unit_test(test_netdata_ebpf_get_shm_pointer_unsafe_null_shm),
        cmocka_unit_test(test_netdata_ebpf_get_shm_pointer_unsafe_full_capacity),
        cmocka_unit_test(test_netdata_ebpf_get_shm_pointer_unsafe_vector_mode),
        cmocka_unit_test(test_netdata_ebpf_get_shm_pointer_unsafe_vector_existing),
        cmocka_unit_test(test_netdata_ebpf_get_shm_pointer_unsafe_pid_out_of_bounds),
        cmocka_unit_test(test_netdata_ebpf_reset_shm_pointer_unsafe_vector_mode),
        cmocka_unit_test(test_netdata_ebpf_reset_shm_pointer_unsafe_vector_remaining_threads),
        cmocka_unit_test(test_netdata_ebpf_reset_shm_pointer_unsafe_socket_index),
        cmocka_unit_test(test_netdata_ebpf_reset_shm_pointer_unsafe_no_shm),
        cmocka_unit_test(test_netdata_integration_current_ipc_data),
        cmocka_unit_test(test_netdata_integration_current_ipc_data_zero_values),
        cmocka_unit_test(test_netdata_integration_current_ipc_data_max_values),
        cmocka_unit_test(test_ebpf_pids_index_enum_values),
        cmocka_unit_test(test_structure_sizes),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}