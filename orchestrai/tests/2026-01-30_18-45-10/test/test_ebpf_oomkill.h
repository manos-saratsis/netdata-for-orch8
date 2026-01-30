// SPDX-License-Identifier: GPL-3.0-or-later
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Mock header file testing for ebpf_oomkill.h
// Tests the constants and data structures defined in the header

static void test_oomkill_constants()
{
    // Test module name and description
    assert(strcmp(NETDATA_EBPF_MODULE_NAME_OOMKILL, "oomkill") == 0);
    assert(strcmp(NETDATA_EBPF_OOMKILL_MODULE_DESC, "Show OOM kills for all applications recognized via the apps.plugin.") == 0);
    
    // Test max entries
    assert(NETDATA_OOMKILL_MAX_ENTRIES == 64);
    
    // Test config file name
    assert(strcmp(NETDATA_OOMKILL_CONFIG_FILE, "oomkill.conf") == 0);
    
    // Test chart name
    assert(strcmp(NETDATA_OOMKILL_CHART, "_ebpf_oomkill") == 0);
    
    // Test unit
    assert(strcmp(EBPF_OOMKILL_UNIT_KILLS, "kills/s") == 0);
}

static void test_oomkill_contexts()
{
    // Test context strings
    assert(strcmp(NETDATA_CGROUP_OOMKILLS_CONTEXT, "cgroup.oomkills") == 0);
    assert(strcmp(NETDATA_SYSTEMD_OOMKILLS_CONTEXT, "systemd.oomkills") == 0);
}

static void test_oomkill_type()
{
    // Test oomkill_ebpf_val_t type size
    assert(sizeof(oomkill_ebpf_val_t) == sizeof(uint8_t));
}

// Test runner
void test_ebpf_oomkill_h()
{
    test_oomkill_constants();
    test_oomkill_contexts();
    test_oomkill_type();
    printf("All ebpf_oomkill.h tests passed!\n");
}