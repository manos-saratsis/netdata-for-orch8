// SPDX-License-Identifier: GPL-3.0-or-later
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Mock header file testing for ebpf_mount.h
// Tests the constants and enums defined in the header

static void test_mount_constants()
{
    // Test module name and description
    assert(strcmp(NETDATA_EBPF_MODULE_NAME_MOUNT, "mount") == 0);
    assert(strcmp(NETDATA_EBPF_MOUNT_MODULE_DESC, "Show calls to syscalls mount(2) and umount(2).") == 0);
    
    // Test syscall count
    assert(NETDATA_EBPF_MOUNT_SYSCALL == 2);
    
    // Test string constants
    assert(strcmp(NETDATA_EBPF_MOUNT_CALLS, "call") == 0);
    assert(strcmp(NETDATA_EBPF_MOUNT_ERRORS, "error") == 0);
    assert(strcmp(NETDATA_EBPF_MOUNT_FAMILY, "mount (eBPF)") == 0);
    
    // Test config file name
    assert(strcmp(NETDATA_MOUNT_CONFIG_FILE, "mount.conf") == 0);
}

static void test_mount_enum_counters()
{
    // Test enum mount_counters values
    assert(NETDATA_KEY_MOUNT_CALL == 0);
    assert(NETDATA_KEY_UMOUNT_CALL == 1);
    assert(NETDATA_KEY_MOUNT_ERROR == 2);
    assert(NETDATA_KEY_UMOUNT_ERROR == 3);
    assert(NETDATA_MOUNT_END == 4);
}

static void test_mount_enum_tables()
{
    // Test enum mount_tables
    assert(NETDATA_KEY_MOUNT_TABLE == 0);
}

static void test_mount_enum_syscalls()
{
    // Test enum netdata_mount_syscalls
    assert(NETDATA_MOUNT_SYSCALL == 0);
    assert(NETDATA_UMOUNT_SYSCALL == 1);
    assert(NETDATA_MOUNT_SYSCALLS_END == 2);
}

// Test runner
void test_ebpf_mount_h()
{
    test_mount_constants();
    test_mount_enum_counters();
    test_mount_enum_tables();
    test_mount_enum_syscalls();
    printf("All ebpf_mount.h tests passed!\n");
}