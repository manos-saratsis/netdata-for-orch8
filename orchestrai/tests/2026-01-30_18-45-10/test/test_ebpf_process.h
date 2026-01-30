// SPDX-License-Identifier: GPL-3.0-or-later
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Mock header file testing for ebpf_process.h
// Tests the constants and enums defined in the header

static void test_process_module_constants()
{
    // Test module name and description
    assert(strcmp(NETDATA_EBPF_MODULE_NAME_PROCESS, "process") == 0);
    assert(strstr(NETDATA_EBPF_MODULE_PROCESS_DESC, "Monitor information about process life") != NULL);
    
    // Test process group
    assert(strcmp(NETDATA_PROCESS_GROUP, "processes") == 0);
}

static void test_process_chart_names()
{
    // Test global chart names
    assert(strcmp(NETDATA_EXIT_SYSCALL, "exit") == 0);
    assert(strcmp(NETDATA_PROCESS_SYSCALL, "process_thread") == 0);
    assert(strcmp(NETDATA_PROCESS_ERROR_NAME, "task_error") == 0);
    assert(strcmp(NETDATA_PROCESS_STATUS_NAME, "process_status") == 0);
    
    // Test app submenu chart names
    assert(strcmp(NETDATA_SYSCALL_APPS_TASK_PROCESS, "process_create") == 0);
    assert(strcmp(NETDATA_SYSCALL_APPS_TASK_THREAD, "thread_create") == 0);
    assert(strcmp(NETDATA_SYSCALL_APPS_TASK_EXIT, "task_exit") == 0);
    assert(strcmp(NETDATA_SYSCALL_APPS_TASK_CLOSE, "task_close") == 0);
    assert(strcmp(NETDATA_SYSCALL_APPS_TASK_ERROR, "task_error") == 0);
}

static void test_process_config_file()
{
    // Test process config file name
    assert(strcmp(NETDATA_PROCESS_CONFIG_FILE, "process.conf") == 0);
}

static void test_process_contexts()
{
    // Test cgroup contexts
    assert(strcmp(NETDATA_CGROUP_PROCESS_CREATE_CONTEXT, "cgroup.process_create") == 0);
    assert(strcmp(NETDATA_CGROUP_THREAD_CREATE_CONTEXT, "cgroup.thread_create") == 0);
    assert(strcmp(NETDATA_CGROUP_PROCESS_CLOSE_CONTEXT, "cgroup.task_close") == 0);
    assert(strcmp(NETDATA_CGROUP_PROCESS_EXIT_CONTEXT, "cgroup.task_exit") == 0);
    assert(strcmp(NETDATA_CGROUP_PROCESS_ERROR_CONTEXT, "cgroup.task_error") == 0);
    
    // Test systemd contexts
    assert(strcmp(NETDATA_SYSTEMD_PROCESS_CREATE_CONTEXT, "systemd.service.process_create") == 0);
    assert(strcmp(NETDATA_SYSTEMD_THREAD_CREATE_CONTEXT, "systemd.service.thread_create") == 0);
    assert(strcmp(NETDATA_SYSTEMD_PROCESS_CLOSE_CONTEXT, "systemd.service.task_close") == 0);
    assert(strcmp(NETDATA_SYSTEMD_PROCESS_EXIT_CONTEXT, "systemd.service.task_exit") == 0);
    assert(strcmp(NETDATA_SYSTEMD_PROCESS_ERROR_CONTEXT, "systemd.service.task_error") == 0);
}

static void test_process_constants()
{
    // Test cgroup update interval
    assert(NETDATA_EBPF_CGROUP_UPDATE == 30);
}

static void test_process_enum_stats_order()
{
    // Test enum netdata_ebpf_stats_order values
    assert(NETDATA_EBPF_ORDER_STAT_THREADS == 140000);
    assert(NETDATA_EBPF_ORDER_PIDS == 140001);
    assert(NETDATA_EBPF_ORDER_PIDS_IPC == 140002);
    assert(NETDATA_EBPF_ORDER_STAT_LIFE_TIME == 140003);
    assert(NETDATA_EBPF_ORDER_STAT_LOAD_METHOD == 140004);
    assert(NETDATA_EBPF_ORDER_STAT_KERNEL_MEMORY == 140005);
    assert(NETDATA_EBPF_ORDER_STAT_HASH_TABLES == 140006);
    assert(NETDATA_EBPF_ORDER_STAT_HASH_CORE == 140007);
    assert(NETDATA_EBPF_ORDER_STAT_HASH_GLOBAL_TABLE_TOTAL == 140008);
    assert(NETDATA_EBPF_ORDER_STAT_HASH_PID_TABLE_ADDED == 140009);
    assert(NETDATA_EBPF_ORDER_STAT_HASH_PID_TABLE_REMOVED == 140010);
    assert(NETATA_EBPF_ORDER_STAT_ARAL_BEGIN == 140011);
    assert(NETDATA_EBPF_ORDER_FUNCTION_PER_THREAD == 140012);
}

static void test_process_enum_load_mode()
{
    // Test enum netdata_ebpf_load_mode_stats
    assert(NETDATA_EBPF_LOAD_STAT_LEGACY == 0);
    assert(NETDATA_EBPF_LOAD_STAT_CORE == 1);
    assert(NETDATA_EBPF_LOAD_STAT_END == 2);
}

static void test_process_enum_thread_per_core()
{
    // Test enum netdata_ebpf_thread_per_core
    assert(NETDATA_EBPF_THREAD_PER_CORE == 0);
    assert(NETDATA_EBPF_THREAD_UNIQUE == 1);
    assert(NETDATA_EBPF_PER_CORE_END == 2);
}

static void test_process_enum_process_index()
{
    // Test enum ebpf_process_index
    assert(NETDATA_KEY_CALLS_DO_EXIT == 0);
    assert(NETDATA_KEY_CALLS_RELEASE_TASK == 1);
    assert(NETDATA_KEY_CALLS_DO_FORK == 2);
    assert(NETDATA_KEY_ERROR_DO_FORK == 3);
    assert(NETDATA_KEY_CALLS_SYS_CLONE == 4);
    assert(NETDATA_KEY_ERROR_SYS_CLONE == 5);
    assert(NETDATA_KEY_END_VECTOR == 6);
}

static void test_process_enum_publish_process()
{
    // Test enum netdata_publish_process
    assert(NETDATA_KEY_PUBLISH_PROCESS_EXIT == 0);
    assert(NETDATA_KEY_PUBLISH_PROCESS_RELEASE_TASK == 1);
    assert(NETDATA_KEY_PUBLISH_PROCESS_FORK == 2);
    assert(NETDATA_KEY_PUBLISH_PROCESS_CLONE == 3);
    assert(NETDATA_KEY_PUBLISH_PROCESS_END == 4);
}

static void test_process_enum_core_process()
{
    // Test enum netdata_core_process
    assert(PROCESS_RELEASE_TASK_NAME == 0);
    assert(PROCESS_SYS_CLONE == 1);
    assert(PROCESS_SYS_CLONE3 == 2);
    assert(PROCESS_SYS_FORK == 3);
    assert(PROCESS_KERNEL_CLONE == 4);
}

static void test_process_enum_tables()
{
    // Test enum ebpf_process_tables
    assert(NETDATA_PROCESS_PID_TABLE == 0);
    assert(NETDATA_PROCESS_GLOBAL_TABLE == 1);
    assert(NETDATA_PROCESS_CTRL_TABLE == 2);
}

// Test runner
void test_ebpf_process_h()
{
    test_process_module_constants();
    test_process_chart_names();
    test_process_config_file();
    test_process_contexts();
    test_process_constants();
    test_process_enum_stats_order();
    test_process_enum_load_mode();
    test_process_enum_thread_per_core();
    test_process_enum_process_index();
    test_process_enum_publish_process();
    test_process_enum_core_process();
    test_process_enum_tables();
    printf("All ebpf_process.h tests passed!\n");
}