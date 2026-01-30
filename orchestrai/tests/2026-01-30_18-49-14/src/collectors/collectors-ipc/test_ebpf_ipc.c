// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "ebpf-ipc.h"

// Mock function declarations
int __wrap_bpf_map_delete_elem(int fd, const void *key);
int __wrap_JudyLGet(Pvoid_t PArray, Word_t Index, PJE0);
Pvoid_t *__wrap_JudyLIns(Pvoid_t *PArray, Word_t Index, PJE0);
int __wrap_JudyLDel(Pvoid_t *PArray, Word_t Index, PJE0);
void *__wrap_callocz(size_t count, size_t size);
int __wrap_shm_open(const char *name, int oflag, mode_t mode);
int __wrap_ftruncate(int fd, off_t length);
void *__wrap_nd_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int __wrap_nd_munmap(void *addr, size_t length);
int __wrap_close(int fd);
sem_t *__wrap_sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
int __wrap_sem_close(sem_t *sem);
void __wrap_nd_log(int level, int priority, const char *fmt, ...);
size_t __wrap_os_get_system_pid_max(void);
int __wrap_memcpy(void *dest, const void *src, size_t n);
int __wrap_memset(void *s, int c, size_t n);

// Global state for mocking
extern netdata_ebpf_pid_stats_t *integration_shm;
extern int shm_fd_ebpf_integration;
extern sem_t *shm_mutex_ebpf_integration;
extern Pvoid_t ebpf_ipc_JudyL;
extern ebpf_user_mem_stat_t ebpf_stat_values;
extern bool using_vector;

// ============================================================================
// Test: ebpf_shm_find_index_unsafe - NULL case
// ============================================================================
static int setup_find_index_null(void **state)
{
    ebpf_ipc_JudyL = NULL;
    return 0;
}

static void test_ebpf_shm_find_index_unsafe_null(void **state)
{
    // When JudyL is NULL, function should return NULL
    uint32_t *result = ebpf_shm_find_index_unsafe(12345);
    assert_null(result);
}

// ============================================================================
// Test: ebpf_shm_find_index_unsafe - With valid entry
// ============================================================================
static void test_ebpf_shm_find_index_unsafe_valid(void **state)
{
    // Setup
    uint32_t test_idx = 42;
    uint32_t pid = 12345;
    
    // Mock Judy array with a value
    will_return(__wrap_JudyLGet, &test_idx);
    
    uint32_t *result = ebpf_shm_find_index_unsafe(pid);
    
    assert_non_null(result);
    assert_int_equal(*result, test_idx);
}

// ============================================================================
// Test: ebpf_find_pid_shm_del_unsafe - NULL lpid
// ============================================================================
static void test_ebpf_find_pid_shm_del_unsafe_null_lpid(void **state)
{
    ebpf_stat_values.current = 10;
    uint32_t pid = 100;
    
    will_return(__wrap_JudyLGet, NULL);
    
    bool result = ebpf_find_pid_shm_del_unsafe(pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_false(result);
}

// ============================================================================
// Test: ebpf_find_pid_shm_del_unsafe - Zero current stat
// ============================================================================
static void test_ebpf_find_pid_shm_del_unsafe_zero_current(void **state)
{
    ebpf_stat_values.current = 0;
    uint32_t pid = 100;
    
    will_return(__wrap_JudyLGet, NULL);
    
    bool result = ebpf_find_pid_shm_del_unsafe(pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_false(result);
}

// ============================================================================
// Test: ebpf_find_pid_shm_del_unsafe - Thread bits remaining
// ============================================================================
static void test_ebpf_find_pid_shm_del_unsafe_threads_remain(void **state)
{
    // Setup
    uint32_t idx = 5;
    uint32_t pid = 100;
    ebpf_stat_values.current = 10;
    
    // Create mock shared memory with threads set
    netdata_ebpf_pid_stats_t shm_data[20];
    memset(shm_data, 0, sizeof(shm_data));
    shm_data[5].threads = 0xFF; // Multiple threads active
    
    integration_shm = shm_data;
    
    will_return(__wrap_JudyLGet, &idx);
    
    bool result = ebpf_find_pid_shm_del_unsafe(pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_true(result);
    assert_int_equal(ebpf_stat_values.current, 10); // Should remain unchanged
}

// ============================================================================
// Test: ebpf_find_pid_shm_del_unsafe - All threads removed, zero current
// ============================================================================
static void test_ebpf_find_pid_shm_del_unsafe_all_threads_removed_zero(void **state)
{
    // Setup
    uint32_t idx = 5;
    uint32_t pid = 100;
    ebpf_stat_values.current = 1;
    
    netdata_ebpf_pid_stats_t shm_data[20];
    memset(shm_data, 0, sizeof(shm_data));
    shm_data[5].threads = (NETDATA_EBPF_PIDS_PROCESS_IDX << 1);
    shm_data[5].pid = pid;
    
    integration_shm = shm_data;
    
    will_return(__wrap_JudyLGet, &idx);
    will_return(__wrap_JudyLDel, 0);
    
    bool result = ebpf_find_pid_shm_del_unsafe(pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_false(result);
    assert_int_equal(ebpf_stat_values.current, 0);
}

// ============================================================================
// Test: ebpf_find_pid_shm_del_unsafe - All threads removed, move last entry
// ============================================================================
static void test_ebpf_find_pid_shm_del_unsafe_with_move(void **state)
{
    // Setup
    uint32_t idx = 0;
    uint32_t pid = 100;
    uint32_t move_idx = 1;
    uint32_t moved_pid = 200;
    
    ebpf_stat_values.current = 2;
    ebpf_stat_values.total = 100;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    memset(shm_data, 0, sizeof(shm_data));
    
    shm_data[0].threads = (NETDATA_EBPF_PIDS_PROCESS_IDX << 1);
    shm_data[0].pid = pid;
    
    shm_data[1].threads = (NETDATA_EBPF_PIDS_SOCKET_IDX << 1);
    shm_data[1].pid = moved_pid;
    
    integration_shm = shm_data;
    
    will_return(__wrap_JudyLGet, &idx);
    will_return(__wrap_JudyLDel, 0);
    will_return(__wrap_JudyLGet, &move_idx);
    
    bool result = ebpf_find_pid_shm_del_unsafe(pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_false(result);
    assert_int_equal(ebpf_stat_values.current, 1);
}

// ============================================================================
// Test: ebpf_find_or_create_index_pid - Existing index
// ============================================================================
static void test_ebpf_find_or_create_index_pid_existing(void **state)
{
    uint32_t existing_idx = 7;
    uint32_t pid = 500;
    
    will_return(__wrap_JudyLGet, &existing_idx);
    
    uint32_t result = ebpf_find_or_create_index_pid(pid);
    
    assert_int_equal(result, existing_idx);
}

// ============================================================================
// Test: ebpf_find_or_create_index_pid - New index creation
// ============================================================================
static void test_ebpf_find_or_create_index_pid_new(void **state)
{
    uint32_t pid = 600;
    uint32_t initial_current = 5;
    
    ebpf_stat_values.current = initial_current;
    
    will_return(__wrap_JudyLGet, NULL);
    will_return(__wrap_JudyLIns, NULL); // Will set up new entry
    expect_value(__wrap_callocz, count, 1);
    expect_value(__wrap_callocz, size, sizeof(uint32_t));
    will_return(__wrap_callocz, malloc(sizeof(uint32_t)));
    
    uint32_t result = ebpf_find_or_create_index_pid(pid);
    
    assert_int_equal(result, initial_current);
    assert_int_equal(ebpf_stat_values.current, initial_current + 1);
}

// ============================================================================
// Test: netdata_ebpf_reset_shm_pointer_unsafe - Non-socket index, vector mode
// ============================================================================
static void test_netdata_ebpf_reset_shm_pointer_unsafe_vector_mode(void **state)
{
    using_vector = true;
    ebpf_stat_values.current = 10;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    memset(shm_data, 0, sizeof(shm_data));
    integration_shm = shm_data;
    
    int fd = 5;
    uint32_t pid = 3;
    
    shm_data[pid].threads = 0xFF;
    
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(fd, pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_true(result);
}

// ============================================================================
// Test: netdata_ebpf_reset_shm_pointer_unsafe - Socket index, vector mode
// ============================================================================
static void test_netdata_ebpf_reset_shm_pointer_unsafe_socket_vector(void **state)
{
    using_vector = true;
    ebpf_stat_values.current = 10;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    memset(shm_data, 0, sizeof(shm_data));
    integration_shm = shm_data;
    
    int fd = 5;
    uint32_t pid = 3;
    
    shm_data[pid].threads = 0xFF;
    
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(fd, pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    
    assert_true(result);
}

// ============================================================================
// Test: netdata_ebpf_reset_shm_pointer_unsafe - Vector mode, last thread removed
// ============================================================================
static void test_netdata_ebpf_reset_shm_pointer_unsafe_last_thread_removed(void **state)
{
    using_vector = true;
    ebpf_stat_values.current = 1;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    memset(shm_data, 0, sizeof(shm_data));
    integration_shm = shm_data;
    
    int fd = 5;
    uint32_t pid = 0;
    
    shm_data[pid].threads = (NETDATA_EBPF_PIDS_PROCESS_IDX << 1);
    
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(fd, pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_false(result);
    assert_int_equal(ebpf_stat_values.current, 0);
}

// ============================================================================
// Test: netdata_ebpf_reset_shm_pointer_unsafe - Non-vector mode
// ============================================================================
static void test_netdata_ebpf_reset_shm_pointer_unsafe_non_vector(void **state)
{
    using_vector = false;
    ebpf_stat_values.current = 5;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    memset(shm_data, 0, sizeof(shm_data));
    integration_shm = shm_data;
    
    int fd = 5;
    uint32_t pid = 100;
    uint32_t idx = 2;
    
    shm_data[2].threads = 0xFF;
    
    expect_value(__wrap_bpf_map_delete_elem, fd, fd);
    will_return(__wrap_bpf_map_delete_elem, 0);
    will_return(__wrap_JudyLGet, &idx);
    
    bool result = netdata_ebpf_reset_shm_pointer_unsafe(fd, pid, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_true(result);
}

// ============================================================================
// Test: netdata_ebpf_get_shm_pointer_unsafe - NULL integration_shm
// ============================================================================
static void test_netdata_ebpf_get_shm_pointer_unsafe_null_shm(void **state)
{
    integration_shm = NULL;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(100, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_null(result);
}

// ============================================================================
// Test: netdata_ebpf_get_shm_pointer_unsafe - SHM full
// ============================================================================
static void test_netdata_ebpf_get_shm_pointer_unsafe_shm_full(void **state)
{
    netdata_ebpf_pid_stats_t shm_data[10];
    integration_shm = shm_data;
    
    ebpf_stat_values.current = 9;
    ebpf_stat_values.total = 10;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(100, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_null(result);
}

// ============================================================================
// Test: netdata_ebpf_get_shm_pointer_unsafe - Vector mode, PID >= total
// ============================================================================
static void test_netdata_ebpf_get_shm_pointer_unsafe_vector_pid_exceeds_total(void **state)
{
    using_vector = true;
    ebpf_stat_values.current = 5;
    ebpf_stat_values.total = 10;
    
    netdata_ebpf_pid_stats_t shm_data[10];
    integration_shm = shm_data;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(15, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_null(result);
}

// ============================================================================
// Test: netdata_ebpf_get_shm_pointer_unsafe - Non-vector mode, valid
// ============================================================================
static void test_netdata_ebpf_get_shm_pointer_unsafe_non_vector_valid(void **state)
{
    using_vector = false;
    ebpf_stat_values.current = 5;
    ebpf_stat_values.total = 100;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    memset(shm_data, 0, sizeof(shm_data));
    integration_shm = shm_data;
    
    uint32_t idx = 5;
    will_return(__wrap_JudyLGet, NULL);
    will_return(__wrap_JudyLIns, NULL);
    expect_value(__wrap_callocz, count, 1);
    expect_value(__wrap_callocz, size, sizeof(uint32_t));
    will_return(__wrap_callocz, malloc(sizeof(uint32_t)));
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(200, NETDATA_EBPF_PIDS_PROCESS_IDX);
    
    assert_non_null(result);
    assert_int_equal(result->threads, (NETDATA_EBPF_PIDS_PROCESS_IDX << 1));
}

// ============================================================================
// Test: netdata_ebpf_get_shm_pointer_unsafe - Vector mode, new thread added
// ============================================================================
static void test_netdata_ebpf_get_shm_pointer_unsafe_vector_new_thread(void **state)
{
    using_vector = true;
    ebpf_stat_values.current = 5;
    ebpf_stat_values.total = 100;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    memset(shm_data, 0, sizeof(shm_data));
    integration_shm = shm_data;
    
    uint32_t pid = 7;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    
    assert_non_null(result);
    assert_int_equal(result->threads, (NETDATA_EBPF_PIDS_SOCKET_IDX << 1));
    assert_int_equal(ebpf_stat_values.current, 6);
}

// ============================================================================
// Test: netdata_ebpf_get_shm_pointer_unsafe - Vector mode, thread already exists
// ============================================================================
static void test_netdata_ebpf_get_shm_pointer_unsafe_vector_existing_thread(void **state)
{
    using_vector = true;
    ebpf_stat_values.current = 5;
    ebpf_stat_values.total = 100;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    memset(shm_data, 0, sizeof(shm_data));
    shm_data[7].threads = (NETDATA_EBPF_PIDS_PROCESS_IDX << 1);
    
    integration_shm = shm_data;
    
    uint32_t pid = 7;
    
    netdata_ebpf_pid_stats_t *result = netdata_ebpf_get_shm_pointer_unsafe(pid, NETDATA_EBPF_PIDS_SOCKET_IDX);
    
    assert_non_null(result);
    assert_int_equal(result->threads, ((NETDATA_EBPF_PIDS_PROCESS_IDX << 1) | (NETDATA_EBPF_PIDS_SOCKET_IDX << 1)));
    assert_int_equal(ebpf_stat_values.current, 5); // Unchanged
}

// ============================================================================
// Test: netdata_integration_cleanup_shm - All resources initialized
// ============================================================================
static void test_netdata_integration_cleanup_shm_all_initialized(void **state)
{
    sem_t mock_sem;
    shm_mutex_ebpf_integration = &mock_sem;
    shm_fd_ebpf_integration = 10;
    
    netdata_ebpf_pid_stats_t shm_data[100];
    integration_shm = shm_data;
    ebpf_stat_values.total = 100;
    
    expect_any(__wrap_sem_close, sem);
    will_return(__wrap_sem_close, 0);
    expect_value(__wrap_nd_munmap, addr, shm_data);
    expect_value(__wrap_nd_munmap, length, 100 * sizeof(netdata_ebpf_pid_stats_t));
    will_return(__wrap_nd_munmap, 0);
    expect_value(__wrap_close, fd, 10);
    will_return(__wrap_close, 0);
    
    netdata_integration_cleanup_shm();
}

// ============================================================================
// Test: netdata_integration_cleanup_shm - Minimal initialization
// ============================================================================
static void test_netdata_integration_cleanup_shm_minimal(void **state)
{
    shm_mutex_ebpf_integration = SEM_FAILED;
    shm_fd_ebpf_integration = -1;
    integration_shm = NULL;
    
    // Should complete without errors
    netdata_integration_cleanup_shm();
}

// ============================================================================
// Test: netdata_ebpf_select_access_mode - Vector selection
// ============================================================================
static void test_netdata_ebpf_select_access_mode_vector(void **state)
{
    size_t pids = 4194304; // Typical system PID max
    using_vector = false;
    
    expect_value(__wrap_os_get_system_pid_max, );
    will_return(__wrap_os_get_system_pid_max, 4194304);
    
    netdata_ebpf_select_access_mode(pids);
    
    assert_true(using_vector);
}

// ============================================================================
// Test: netdata_ebpf_select_access_mode - Non-vector selection
// ============================================================================
static void test_netdata_ebpf_select_access_mode_non_vector(void **state)
{
    size_t pids = 1024; // Less than system PID max
    using_vector = true;
    
    expect_value(__wrap_os_get_system_pid_max, );
    will_return(__wrap_os_get_system_pid_max, 4194304);
    
    netdata_ebpf_select_access_mode(pids);
    
    assert_false(using_vector);
}

// ============================================================================
// Test: netdata_integration_initialize_shm - Zero pids
// ============================================================================
static void test_netdata_integration_initialize_shm_zero_pids(void **state)
{
    int result = netdata_integration_initialize_shm(0);
    
    assert_int_equal(result, -1);
}

// ============================================================================
// Test: netdata_integration_initialize_shm - shm_open failure
// ============================================================================
static void test_netdata_integration_initialize_shm_shm_open_fail(void **state)
{
    expect_value(__wrap_os_get_system_pid_max, );
    will_return(__wrap_os_get_system_pid_max, 4194304);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, -1);
    
    expect_any(__wrap_nd_log, level);
    expect_any(__wrap_nd_log, priority);
    expect_any(__wrap_nd_log, fmt);
    
    int result = netdata_integration_initialize_shm(1024);
    
    assert_int_equal(result, -1);
}

// ============================================================================
// Test: netdata_integration_initialize_shm - ftruncate failure
// ============================================================================
static void test_netdata_integration_initialize_shm_ftruncate_fail(void **state)
{
    expect_value(__wrap_os_get_system_pid_max, );
    will_return(__wrap_os_get_system_pid_max, 4194304);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, 10);
    
    expect_value(__wrap_ftruncate, fd, 10);
    will_return(__wrap_ftruncate, -1);
    
    expect_any(__wrap_nd_log, level);
    expect_any(__wrap_nd_log, priority);
    expect_any(__wrap_nd_log, fmt);
    expect_value(__wrap_close, fd, 10);
    will_return(__wrap_close, 0);
    
    int result = netdata_integration_initialize_shm(1024);
    
    assert_int_equal(result, -1);
}

// ============================================================================
// Test: netdata_integration_initialize_shm - mmap failure
// ============================================================================
static void test_netdata_integration_initialize_shm_mmap_fail(void **state)
{
    size_t length = 1024 * sizeof(netdata_ebpf_pid_stats_t);
    
    expect_value(__wrap_os_get_system_pid_max, );
    will_return(__wrap_os_get_system_pid_max, 4194304);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, 10);
    
    expect_value(__wrap_ftruncate, fd, 10);
    will_return(__wrap_ftruncate, 0);
    
    expect_any(__wrap_nd_mmap, addr);
    expect_value(__wrap_nd_mmap, length, length);
    will_return(__wrap_nd_mmap, NULL);
    
    expect_any(__wrap_nd_log, level);
    expect_any(__wrap_nd_log, priority);
    expect_any(__wrap_nd_log, fmt);
    expect_value(__wrap_close, fd, 10);
    will_return(__wrap_close, 0);
    
    int result = netdata_integration_initialize_shm(1024);
    
    assert_int_equal(result, -1);
}

// ============================================================================
// Test: netdata_integration_initialize_shm - sem_open failure
// ============================================================================
static void test_netdata_integration_initialize_shm_sem_open_fail(void **state)
{
    size_t length = 1024 * sizeof(netdata_ebpf_pid_stats_t);
    netdata_ebpf_pid_stats_t *mock_shm = malloc(length);
    
    expect_value(__wrap_os_get_system_pid_max, );
    will_return(__wrap_os_get_system_pid_max, 4194304);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, 10);
    
    expect_value(__wrap_ftruncate, fd, 10);
    will_return(__wrap_ftruncate, 0);
    
    expect_any(__wrap_nd_mmap, addr);
    expect_value(__wrap_nd_mmap, length, length);
    will_return(__wrap_nd_mmap, mock_shm);
    
    expect_string(__wrap_sem_open, name, NETDATA_EBPF_SHM_INTEGRATION_NAME);
    will_return(__wrap_sem_open, SEM_FAILED);
    
    expect_any(__wrap_nd_log, level);
    expect_any(__wrap_nd_log, priority);
    expect_any(__wrap_nd_log, fmt);
    
    expect_value(__wrap_nd_munmap, addr, mock_shm);
    expect_value(__wrap_nd_munmap, length, length);
    will_return(__wrap_nd_munmap, 0);
    
    expect_value(__wrap_close, fd, 10);
    will_return(__wrap_close, 0);
    
    int result = netdata_integration_initialize_shm(1024);
    
    assert_int_equal(result, -1);
    assert_null(integration_shm);
}

// ============================================================================
// Test: netdata_integration_initialize_shm - Success path
// ============================================================================
static void test_netdata_integration_initialize_shm_success(void **state)
{
    size_t length = 1024 * sizeof(netdata_ebpf_pid_stats_t);
    netdata_ebpf_pid_stats_t *mock_shm = malloc(length);
    sem_t mock_sem;
    
    expect_value(__wrap_os_get_system_pid_max, );
    will_return(__wrap_os_get_system_pid_max, 4194304);
    
    expect_string(__wrap_shm_open, name, NETDATA_EBPF_INTEGRATION_NAME);
    will_return(__wrap_shm_open, 10);
    
    expect_value(__wrap_ftruncate, fd, 10);
    will_return(__wrap_ftruncate, 0);
    
    expect_any(__wrap_nd_mmap, addr);
    expect_value(__wrap_nd_mmap, length, length);
    will_return(__wrap_nd_mmap, mock_shm);
    
    expect_string(__wrap_sem_open, name, NETDATA_EBPF_SHM_INTEGRATION_NAME);
    will_return(__wrap_sem_open, &mock_sem);
    
    int result = netdata_integration_initialize_shm(1024);
    
    assert_int_equal(result, 0);