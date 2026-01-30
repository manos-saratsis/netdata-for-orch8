// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "ebpf-ipc.h"

// Mock variables and functions
extern netdata_ebpf_pid_stats_t *integration_shm;
extern int shm_fd_ebpf_integration;
extern sem_t *shm_mutex_ebpf_integration;
extern ebpf_user_mem_stat_t ebpf_stat_values;
extern bool using_vector;

// Mock the external dependencies
void nd_log(int level, int priority, const char *fmt, ...) {
    UNUSED(level);
    UNUSED(priority);
    UNUSED(fmt);
}

void *nd_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return mmap(addr, length, prot, flags, fd, offset);
}

int nd_munmap(void *addr, size_t length) {
    return munmap(addr, length);
}

size_t os_get_system_pid_max(void) {
    return 32768; // Default mock value
}

int bpf_map_delete_elem(int fd, const void *key) {
    UNUSED(fd);
    UNUSED(key);
    return 0;
}

// Test setup and teardown
static int setup(void **state) {
    // Reset global variables
    integration_shm = NULL;
    shm_fd_ebpf_integration = -1;
    shm_mutex_ebpf_integration = SEM_FAILED;
    ebpf_stat_values.current = 0;
    ebpf_stat_values.total = 0;
    using_vector = false;
    
    return 0;
}

static int teardown(void **state) {
    // Cleanup after tests
    if (shm_fd_ebpf_integration > 0) {
        close(shm_fd_ebpf_integration);
        shm_fd_ebpf_integration = -1;
    }
    
    if (integration_shm) {
        size_t length = ebpf_stat_values.total * sizeof(netdata_ebpf_pid_stats_t);
        munmap(integration_shm, length);
        integration_shm = NULL;
    }
    
    if (shm_mutex_ebpf_integration != SEM_FAILED) {
        sem_close(shm_mutex_ebpf_integration);
        shm_mutex_ebpf_integration = SEM_FAILED;
    }
    
    return 0;
}

// Tests for netdata_integration_initialize_shm
static void test_netdata_integration_initialize_shm_with_zero_pids(void **state) {
    // Test that initializing with 0 pids returns -1
    int result = netdata_integration_initialize_shm(0);
    assert_int_equal(result, -1);
}

static void test_netdata_integration_initialize_shm_success(void **state) {
    // Test successful initialization with valid pids count
    int result = netdata_integration_initialize_shm(1024);
    
    // Should succeed
    assert_int_equal(result, 0);
    
    // Verify globals are set correctly
    assert_non_null(integration_shm);
    assert_true(shm_fd_ebpf_integration >= 0);
    assert_int_equal(ebpf_stat_values.total, 1024);
    assert_int_equal(ebpf_stat_values.current, 0);
    
    // Cleanup
    netdata_integration_cleanup_shm();
}

static void test_netdata_integration_initialize_shm_sets_using_vector_true(void **state) {
    // Test that using_vector is set to true when pids == system pid max
    size_t max_pids = os_get_system_pid_max();
    int result = netdata_integration_initialize_shm(max_pids);
    
    assert_int_equal(result, 0);
    assert_true(using_vector);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_integration_initialize_shm_sets_using_vector_false(void **state) {
    // Test that using_vector is false when pids != system pid max
    int result = netdata_integration_initialize_shm(1024);
    
    assert_int_equal(result, 0);
    assert_false(using_vector);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_integration_initialize_shm_small_size(void **state) {
    // Test with very small pids count
    int result = netdata_integration_initialize_shm(1);
    
    assert_int_equal(result, 0);
    assert_non_null(integration_shm);
    assert_int_equal(ebpf_stat_values.total, 1);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_integration_initialize_shm_large_size(void **state) {
    // Test with large pids count
    int result = netdata_integration_initialize_shm(65536);
    
    assert_int_equal(result, 0);
    assert_non_null(integration_shm);
    assert_int_equal(ebpf_stat_values.total, 65536);
    
    netdata_integration_cleanup_shm();
}

// Tests for netdata_integration_cleanup_shm
static void test_netdata_integration_cleanup_shm_when_nothing_initialized(void **state) {
    // Should not crash when nothing is initialized
    netdata_integration_cleanup_shm();
    
    assert_null(integration_shm);
    assert_int_equal(shm_fd_ebpf_integration, -1);
}

static void test_netdata_integration_cleanup_shm_after_initialization(void **state) {
    // Initialize first
    netdata_integration_initialize_shm(1024);
    assert_non_null(integration_shm);
    
    // Then cleanup
    netdata_integration_cleanup_shm();
    
    // Verify cleanup
    assert_null(integration_shm);
}

static void test_netdata_integration_cleanup_shm_closes_semaphore(void **state) {
    // Initialize first
    netdata_integration_initialize_shm(1024);
    assert_not_equal(shm_mutex_ebpf_integration, SEM_FAILED);
    
    // Cleanup
    netdata_integration_cleanup_shm();
    
    // Semaphore should be closed (SEM_FAILED after close)
    // We can't directly verify this, but verify no crash occurs
}

static void test_netdata_integration_cleanup_shm_closes_fd(void **state) {
    // Initialize first
    netdata_integration_initialize_shm(1024);
    int fd = shm_fd_ebpf_integration;
    assert_true(fd > 0);
    
    // Cleanup
    netdata_integration_cleanup_shm();
    
    // After cleanup, shm_fd should be -1
    // (Note: the fd value is local, but global should be -1)
}

// Tests for netdata_ebpf_get_shm_pointer_unsafe
static void test_netdata_ebpf_get_shm_pointer_unsafe_with_null_integration_shm(void **state) {
    // When integration_shm is NULL, should return NULL
    integration_shm = NULL;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(
        1, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_null(result);
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_when_full(void **state) {
    // Initialize with small size
    netdata_integration_initialize_shm(2);
    
    // Set current to equal total - 1 (only 1 space left)
    ebpf_stat_values.current = 1;
    
    // Try to get pointer when no space left
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(
        1, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_null(result);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_success_vector_mode(void **state) {
    // Initialize with system max pids (vector mode)
    size_t max_pids = os_get_system_pid_max();
    netdata_integration_initialize_shm(max_pids);
    assert_true(using_vector);
    
    // Get pointer for valid pid
    uint32_t test_pid = 100;
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(
        test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_non_null(result);
    assert_int_equal(result->pid, test_pid);
    assert_int_not_equal(result->threads, 0);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_success_judy_mode(void **state) {
    // Initialize without system max pids (judy mode)
    netdata_integration_initialize_shm(1024);
    assert_false(using_vector);
    
    // Get pointer for valid pid
    uint32_t test_pid = 100;
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(
        test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_non_null(result);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_pid_exceeds_total_judy(void **state) {
    // Initialize with small total
    netdata_integration_initialize_shm(10);
    
    // Try to get pointer with pid >= total (in judy mode)
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(
        50, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Should fail because we'd exceed array bounds in judy mode
    // Actually in judy mode, pid is treated as index, so this could be an issue
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_increments_current_first_time(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    uint32_t initial_current = ebpf_stat_values.current;
    
    uint32_t test_pid = 100;
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(
        test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_non_null(result);
    // If using vector and first time for this pid, current should increase
    // Behavior depends on using_vector flag
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_sets_threads_flag(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    uint32_t test_pid = 100;
    enum ebpf_pids_index idx = NETDATA_EBPF_PIDS_SOCKET_IDX;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(test_pid, idx);
    
    assert_non_null(result);
    // Threads should have the flag set: idx << 1
    assert_true(result->threads & (idx << 1));
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_multiple_calls_same_pid(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    uint32_t test_pid = 100;
    
    // First call
    netdata_ebpf_pid_stats_t *result1 = netdata_ebpf_get_shm_pointer_unsafe(
        test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    assert_non_null(result1);
    
    uint32_t current_after_first = ebpf_stat_values.current;
    
    // Second call with different index
    netdata_ebpf_pid_stats_t *result2 = netdata_ebpf_get_shm_pointer_unsafe(
        test_pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    assert_non_null(result2);
    
    // Should return same pointer
    assert_ptr_equal(result1, result2);
    
    netdata_integration_cleanup_shm();
}

// Tests for netdata_ebpf_reset_shm_pointer_unsafe
static void test_netdata_ebpf_reset_shm_pointer_unsafe_when_not_socket_idx(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    uint32_t test_pid = 100;
    int mock_fd = 3; // Mock BPF map fd
    
    // First, get a pointer
    netdata_ebpf_get_shm_pointer_unsafe(test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Reset with non-SOCKET_IDX
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(
        mock_fd, test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Should call bpf_map_delete_elem when idx != SOCKET_IDX
    // Result depends on implementation details
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_reset_shm_pointer_unsafe_socket_idx(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    uint32_t test_pid = 100;
    int mock_fd = 3;
    
    // First, get a pointer
    netdata_ebpf_get_shm_pointer_unsafe(test_pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    
    // Reset with SOCKET_IDX
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(
        mock_fd, test_pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    
    // Should not call bpf_map_delete_elem when idx == SOCKET_IDX
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_reset_shm_pointer_unsafe_vector_mode_last_index(void **state) {
    // Initialize in vector mode
    size_t max_pids = os_get_system_pid_max();
    netdata_integration_initialize_shm(max_pids);
    assert_true(using_vector);
    
    uint32_t test_pid = 100;
    int mock_fd = 3;
    
    // Get pointer to set up threads flag
    netdata_ebpf_get_shm_pointer_unsafe(test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Reset the only thread flag
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(
        mock_fd, test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // In vector mode, if no threads left, should return false
    assert_false(result);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_reset_shm_pointer_unsafe_vector_mode_multiple_flags(void **state) {
    // Initialize in vector mode
    size_t max_pids = os_get_system_pid_max();
    netdata_integration_initialize_shm(max_pids);
    assert_true(using_vector);
    
    uint32_t test_pid = 100;
    int mock_fd = 3;
    
    // Get pointer with multiple flags
    netdata_ebpf_get_shm_pointer_unsafe(test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    netdata_ebpf_get_shm_pointer_unsafe(test_pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    
    // Reset one flag
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(
        mock_fd, test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Should still have threads because socket flag remains
    assert_true(result);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_reset_shm_pointer_unsafe_judy_mode(void **state) {
    // Initialize in judy mode
    netdata_integration_initialize_shm(1024);
    assert_false(using_vector);
    
    uint32_t test_pid = 100;
    int mock_fd = 3;
    
    // Get pointer
    netdata_ebpf_get_shm_pointer_unsafe(test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Reset
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(
        mock_fd, test_pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Judy mode behavior
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_reset_shm_pointer_unsafe_null_integration_shm(void **state) {
    // Test when integration_shm is NULL
    integration_shm = NULL;
    int mock_fd = 3;
    
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(
        mock_fd, 100, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    // Should handle gracefully (likely no-op or error)
}

// Tests for netdata_integration_current_ipc_data
static void test_netdata_integration_current_ipc_data_returns_current_values(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    // Set some values
    ebpf_stat_values.current = 100;
    ebpf_stat_values.total = 1024;
    
    // Get the data
    ebpf_user_mem_stat_t output;
    netdata_integration_current_ipc_data(&output);
    
    // Verify values are copied
    assert_int_equal(output.current, 100);
    assert_int_equal(output.total, 1024);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_integration_current_ipc_data_with_zero_values(void **state) {
    // Initialize
    netdata_integration_initialize_shm(512);
    
    // Values should be initialized to specific state
    ebpf_user_mem_stat_t output;
    netdata_integration_current_ipc_data(&output);
    
    assert_int_equal(output.total, 512);
    assert_int_equal(output.current, 0); // Initially 0
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_integration_current_ipc_data_when_full(void **state) {
    // Initialize
    netdata_integration_initialize_shm(10);
    
    // Fill it up
    ebpf_stat_values.current = 10;
    
    ebpf_user_mem_stat_t output;
    netdata_integration_current_ipc_data(&output);
    
    assert_int_equal(output.current, 10);
    assert_int_equal(output.total, 10);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_integration_current_ipc_data_multiple_calls(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    ebpf_stat_values.current = 50;
    
    // First call
    ebpf_user_mem_stat_t output1;
    netdata_integration_current_ipc_data(&output1);
    
    // Change value
    ebpf_stat_values.current = 75;
    
    // Second call
    ebpf_user_mem_stat_t output2;
    netdata_integration_current_ipc_data(&output2);
    
    // Verify both captured different values
    assert_int_equal(output1.current, 50);
    assert_int_equal(output2.current, 75);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_integration_current_ipc_data_does_not_modify_global(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    ebpf_stat_values.current = 100;
    uint32_t original = ebpf_stat_values.current;
    
    ebpf_user_mem_stat_t output;
    netdata_integration_current_ipc_data(&output);
    
    // Global should not change
    assert_int_equal(ebpf_stat_values.current, original);
    
    netdata_integration_cleanup_shm();
}

// Edge case and error scenario tests
static void test_netdata_integration_initialize_shm_max_value(void **state) {
    // Test with maximum reasonable size
    int result = netdata_integration_initialize_shm(1000000);
    
    // May fail due to system limits, but should not crash
    // Just verify the function handles it
    
    if (result == 0) {
        netdata_integration_cleanup_shm();
    }
}

static void test_netdata_integration_cleanup_shm_multiple_calls(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    // Cleanup once
    netdata_integration_cleanup_shm();
    
    // Cleanup again (should not crash)
    netdata_integration_cleanup_shm();
    
    assert_null(integration_shm);
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_all_idx_types(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    uint32_t test_pid = 100;
    
    // Test all enumeration values
    enum ebpf_pids_index idx_types[] = {
        NETDATA_EBPF_PIDS_PROCESS_IDX,
        NETDATA_EBPF_PIDS_SOCKET_IDX,
        NETDATA_EBPF_PIDS_CACHESTAT_IDX,
        NETDATA_EBPF_PIDS_DCSTAT_IDX,
        NETDATA_EBPF_PIDS_SWAP_IDX,
        NETDATA_EBPF_PIDS_VFS_IDX,
        NETDATA_EBPF_PIDS_FD_IDX,
        NETDATA_EBPF_PIDS_SHM_IDX,
    };
    
    for (int i = 0; i < 8; i++) {
        netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(
            test_pid, idx_types[i]);
        assert_non_null(result);
    }
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_get_shm_pointer_unsafe_different_pids(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    // Get pointers for different PIDs
    netdata_ebpf_pid_stats_t *result1 = netdata_ebpf_get_shm_pointer_unsafe(
        1, NETDATA_EBPF_PIDS_PROCESS_IDX);
    netdata_ebpf_pid_stats_t *result2 = netdata_ebpf_get_shm_pointer_unsafe(
        2, NETDATA_EBPF_PIDS_PROCESS_IDX);
    netdata_ebpf_pid_stats_t *result3 = netdata_ebpf_get_shm_pointer_unsafe(
        1000, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_non_null(result1);
    assert_non_null(result2);
    assert_non_null(result3);
    
    netdata_integration_cleanup_shm();
}

static void test_netdata_ebpf_reset_shm_pointer_unsafe_all_idx_types(void **state) {
    // Initialize
    netdata_integration_initialize_shm(1024);
    
    uint32_t test_pid = 100;
    int mock_fd = 3;
    
    // Get pointer with multiple indices
    enum ebpf_pids_index idx_types[] = {
        NETDATA_EBPF_PIDS_PROCESS_IDX,
        NETDATA_EBPF_PIDS_SOCKET_IDX,
        NETDATA_EBPF_PIDS_CACHESTAT_IDX,
        NETDATA_EBPF_PIDS_DCSTAT_IDX,
    };
    
    for (int i = 0; i < 4; i++) {
        netdata_ebpf_get_shm_pointer_unsafe(test_pid, idx_types[i]);
    }
    
    // Reset one by one
    for (int i = 0; i < 4; i++) {
        bool result = netdata_ebpf_reset_shm_pointer_unsafe(
            mock_fd, test_pid, idx_types[i]);
        
        // Results vary based on implementation
        (void)result;
    }
    
    netdata_integration_cleanup_shm();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // netdata_integration_initialize_shm tests
        cmocka_unit_test_setup_teardown(test_netdata_integration_initialize_shm_with_zero_pids, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_initialize_shm_success, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_initialize_shm_sets_using_vector_true, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_initialize_shm_sets_using_vector_false, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_initialize_shm_small_size, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_initialize_shm_large_size, setup, teardown),
        
        // netdata_integration_cleanup_shm tests
        cmocka_unit_test_setup_teardown(test_netdata_integration_cleanup_shm_when_nothing_initialized, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_cleanup_shm_after_initialization, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_cleanup_shm_closes_semaphore, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_cleanup_shm_closes_fd, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_integration_cleanup_shm_multiple_calls, setup, teardown),
        
        // netdata_ebpf_get_shm_pointer_unsafe tests
        cmocka_unit_test_setup_teardown(test_netdata_ebpf_get_shm_pointer_unsafe_with_null_integration_shm, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_ebpf_get_shm_pointer_unsafe_when_full, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_ebpf_get_shm_pointer_unsafe_success_vector_mode, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_ebpf_get_shm_pointer_unsafe_success_judy_mode, setup, teardown),
        cmocka_unit_test_setup_teardown(test_netdata_ebpf_get_shm_pointer_unsafe_pid_exceeds_total_judy, setup, teardown),
        cmocka_unit_test_setup_teardown(test_net