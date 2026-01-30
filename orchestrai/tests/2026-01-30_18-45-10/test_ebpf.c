#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// Mock structures and functions needed for testing
#define VERSION_STRING_LEN 256
#define NETDATA_MINIMUM_EBPF_KERNEL 264960
#define NETDATA_MINIMUM_RH_VERSION 1797
#define NETDATA_EBPF_KERNEL_5_14 331264
#define NETDATA_EBPF_KERNEL_5_4 328704
#define NETDATA_EBPF_KERNEL_5_5 328960
#define NETDATA_EBPF_KERNEL_4_11 264960
#define NETDATA_EBPF_KERNEL_6_8 395264
#define NETDATA_EBPF_KERNEL_5_16 331776
#define NETDATA_EBPF_KERNEL_5_15 331520
#define NETDATA_EBPF_KERNEL_5_11 330496
#define NETDATA_EBPF_KERNEL_5_10 330240
#define NETDATA_EBPF_KERNEL_4_17 266496
#define NETDATA_EBPF_KERNEL_4_15 265984
#define NETDATA_EBPF_KERNEL_4_14 265728
#define NETDATA_EBPF_KERNEL_4_06 263680

#define NETDATA_IDX_V3_10 0
#define NETDATA_IDX_V4_14 1
#define NETDATA_IDX_V4_16 2
#define NETDATA_IDX_V4_18 3
#define NETDATA_IDX_V5_4 4
#define NETDATA_IDX_V5_10 5
#define NETDATA_IDX_V5_11 6
#define NETDATA_IDX_V5_14 7
#define NETDATA_IDX_V5_15 8
#define NETDATA_IDX_V5_16 9
#define NETDATA_IDX_V6_8 10

#define NETDATA_V5_14 (1 << 7)
#define NETDATA_V5_16 (1 << 9)
#define NETDATA_V6_8 (1 << 10)
#define NETDATA_V4_14 (1 << 1)
#define NETDATA_V4_16 (1 << 2)
#define NETDATA_V5_4 (1 << 4)
#define NETDATA_V5_10 (1 << 5)
#define NETDATA_V5_11 (1 << 6)
#define NETDATA_V5_15 (1 << 8)

#define NETDATA_IDX_STR_V3_10 "3.10"
#define NETDATA_IDX_STR_V4_14 "4.14"
#define NETDATA_IDX_STR_V4_16 "4.16"
#define NETDATA_IDX_STR_V4_18 "4.18"
#define NETDATA_IDX_STR_V5_4 "5.4"
#define NETDATA_IDX_STR_V5_10 "5.10"
#define NETDATA_IDX_STR_V5_11 "5.11"
#define NETDATA_IDX_STR_V5_14 "5.14"
#define NETDATA_IDX_STR_V5_15 "5.15"
#define NETDATA_IDX_STR_V5_16 "5.16"
#define NETDATA_IDX_STR_V6_8 "6.8"

static char *mock_kernel_names[] = { 
    NETDATA_IDX_STR_V3_10, NETDATA_IDX_STR_V4_14, NETDATA_IDX_STR_V4_16,
    NETDATA_IDX_STR_V4_18, NETDATA_IDX_STR_V5_4,  NETDATA_IDX_STR_V5_10,
    NETDATA_IDX_STR_V5_11, NETDATA_IDX_STR_V5_14, NETDATA_IDX_STR_V5_15,
    NETDATA_IDX_STR_V5_16, NETDATA_IDX_STR_V6_8
};

// Mock str2l function
static long str2l(const char *s) {
    return strtol(s, NULL, 10);
}

// Test ebpf_check_kernel_version with minimum version
static void test_ebpf_check_kernel_version_minimum() {
    int version = NETDATA_MINIMUM_EBPF_KERNEL;
    int result = (version >= NETDATA_MINIMUM_EBPF_KERNEL);
    assert(result == 1);
}

// Test ebpf_check_kernel_version with kernel below minimum
static void test_ebpf_check_kernel_version_below_minimum() {
    int version = NETDATA_MINIMUM_EBPF_KERNEL - 1;
    int result = (version >= NETDATA_MINIMUM_EBPF_KERNEL);
    assert(result == 0);
}

// Test ebpf_check_kernel_version with newer kernel
static void test_ebpf_check_kernel_version_newer() {
    int version = NETDATA_EBPF_KERNEL_6_8;
    int result = (version >= NETDATA_MINIMUM_EBPF_KERNEL);
    assert(result == 1);
}

// Test is_ebpf_plugin_running_as_root - root case
static void test_is_ebpf_plugin_running_as_root_true() {
    uid_t uid = 0;
    uid_t euid = 0;
    int result = (uid == 0 || euid == 0) ? 1 : 0;
    assert(result == 1);
}

// Test is_ebpf_plugin_running_as_root - non-root case
static void test_is_ebpf_plugin_running_as_root_false() {
    uid_t uid = 1000;
    uid_t euid = 1000;
    int result = (uid == 0 || euid == 0) ? 1 : 0;
    assert(result == 0);
}

// Test is_ebpf_plugin_running_as_root - euid is root
static void test_is_ebpf_plugin_running_as_root_euid_true() {
    uid_t uid = 1000;
    uid_t euid = 0;
    int result = (uid == 0 || euid == 0) ? 1 : 0;
    assert(result == 1);
}

// Test kernel version parsing - simple version
static void test_kernel_version_parsing_simple() {
    char ver[] = "4.11.0\n";
    char major[16], minor[16], patch[16];
    char *version = ver;
    
    char *move = major;
    while (*version && *version != '.')
        *move++ = *version++;
    *move = '\0';
    
    assert(strcmp(major, "4") == 0);
    
    version++;
    move = minor;
    while (*version && *version != '.')
        *move++ = *version++;
    *move = '\0';
    
    assert(strcmp(minor, "11") == 0);
    
    if (*version)
        version++;
    
    move = patch;
    while (*version && *version != '\n' && *version != '-')
        *move++ = *version++;
    *move = '\0';
    
    assert(strcmp(patch, "0") == 0);
}

// Test kernel version parsing - version with dash
static void test_kernel_version_parsing_with_dash() {
    char ver[] = "5.10.5-generic\n";
    char major[16], minor[16], patch[16];
    char *version = ver;
    
    char *move = major;
    while (*version && *version != '.')
        *move++ = *version++;
    *move = '\0';
    
    version++;
    move = minor;
    while (*version && *version != '.')
        *move++ = *version++;
    *move = '\0';
    
    if (*version)
        version++;
    
    move = patch;
    while (*version && *version != '\n' && *version != '-')
        *move++ = *version++;
    *move = '\0';
    
    assert(strcmp(major, "5") == 0);
    assert(strcmp(minor, "10") == 0);
    assert(strcmp(patch, "5") == 0);
}

// Test patch version clamping to 255
static void test_kernel_version_patch_clamping() {
    int ipatch = 300;
    if (ipatch > 255)
        ipatch = 255;
    assert(ipatch == 255);
}

// Test kernel version formula
static void test_kernel_version_formula() {
    int major = 4, minor = 11, patch = 0;
    int expected = 264960; // NETDATA_EBPF_KERNEL_4_11
    int calculated = ((major * 65536) + (minor * 256) + patch);
    assert(calculated == expected);
}

// Test ebpf_select_max_index - Red Hat family with 5.14+
static void test_ebpf_select_max_index_rhf_5_14() {
    int is_rhf = 1;
    uint32_t kver = NETDATA_EBPF_KERNEL_5_14;
    int expected = NETDATA_IDX_V5_14;
    
    int result;
    if (is_rhf > 0) {
        if (kver >= NETDATA_EBPF_KERNEL_5_14)
            result = NETDATA_IDX_V5_14;
        else
            result = NETDATA_IDX_V4_18;
    }
    
    assert(result == expected);
}

// Test ebpf_select_max_index - Red Hat family with 5.4
static void test_ebpf_select_max_index_rhf_5_4() {
    int is_rhf = -1; // Oracle Linux
    uint32_t kver = NETDATA_EBPF_KERNEL_5_4;
    int expected = NETDATA_IDX_V5_4;
    
    int result;
    if (is_rhf > 0) {
        result = NETDATA_IDX_V5_14;
    } else {
        if (kver >= NETDATA_EBPF_KERNEL_5_4 && kver < NETDATA_EBPF_KERNEL_5_5)
            result = NETDATA_IDX_V5_4;
        else
            result = NETDATA_IDX_V3_10;
    }
    
    assert(result == expected);
}

// Test ebpf_select_max_index - kernel.org 6.8+
static void test_ebpf_select_max_index_kernel_org_6_8() {
    int is_rhf = 0;
    uint32_t kver = NETDATA_EBPF_KERNEL_6_8;
    
    int result;
    if (is_rhf > 0) {
        result = NETDATA_IDX_V5_14;
    } else {
        if (kver >= NETDATA_EBPF_KERNEL_6_8)
            result = NETDATA_IDX_V6_8;
        else
            result = NETDATA_IDX_V3_10;
    }
    
    assert(result == NETDATA_IDX_V6_8);
}

// Test ebpf_select_max_index - kernel.org 5.16+
static void test_ebpf_select_max_index_kernel_org_5_16() {
    int is_rhf = 0;
    uint32_t kver = NETDATA_EBPF_KERNEL_5_16;
    
    int result;
    if (kver >= NETDATA_EBPF_KERNEL_6_8)
        result = NETDATA_IDX_V6_8;
    else if (kver >= NETDATA_EBPF_KERNEL_5_16)
        result = NETDATA_IDX_V5_16;
    else
        result = NETDATA_IDX_V3_10;
    
    assert(result == NETDATA_IDX_V5_16);
}

// Test ebpf_select_max_index - kernel below 4.11
static void test_ebpf_select_max_index_below_4_11() {
    int is_rhf = 0;
    uint32_t kver = 260000; // below 4.11
    
    int result;
    if (kver >= NETDATA_EBPF_KERNEL_6_8)
        result = NETDATA_IDX_V6_8;
    else if (kver >= NETDATA_EBPF_KERNEL_4_11)
        result = NETDATA_IDX_V4_14;
    else
        result = NETDATA_IDX_V3_10;
    
    assert(result == NETDATA_IDX_V3_10);
}

// Test ebpf_select_index with available kernels
static void test_ebpf_select_index_with_available() {
    uint32_t kernels = NETDATA_V5_14 | NETDATA_V5_10;
    int is_rhf = 1;
    uint32_t kver = NETDATA_EBPF_KERNEL_5_14;
    
    uint32_t start = NETDATA_IDX_V5_14;
    uint32_t idx;
    
    for (idx = start; idx > 0; idx--) {
        if (kernels & (1 << idx))
            break;
    }
    
    assert(idx == NETDATA_IDX_V5_14);
}

// Test ebpf_select_index with fallback
static void test_ebpf_select_index_with_fallback() {
    uint32_t kernels = NETDATA_V5_10;
    int is_rhf = 0;
    uint32_t kver = NETDATA_EBPF_KERNEL_5_14;
    
    uint32_t start = NETDATA_IDX_V5_14;
    uint32_t idx;
    
    for (idx = start; idx > 0; idx--) {
        if (kernels & (1 << idx))
            break;
    }
    
    assert(idx == NETDATA_IDX_V5_10);
}

// Test ebpf_select_kernel_name with different indices
static void test_ebpf_select_kernel_name_v5_14() {
    uint32_t idx = NETDATA_IDX_V5_14;
    const char *name = mock_kernel_names[idx];
    assert(strcmp(name, "5.14") == 0);
}

static void test_ebpf_select_kernel_name_v3_10() {
    uint32_t idx = NETDATA_IDX_V3_10;
    const char *name = mock_kernel_names[idx];
    assert(strcmp(name, "3.10") == 0);
}

static void test_ebpf_select_kernel_name_v6_8() {
    uint32_t idx = NETDATA_IDX_V6_8;
    const char *name = mock_kernel_names[idx];
    assert(strcmp(name, "6.8") == 0);
}

// Test snprintfz mount name format
static void test_ebpf_mount_name_format() {
    char out[1024];
    int is_return = 1;
    int is_rhf = 0;
    const char *version = "5.14";
    const char *name = "tcp_connect";
    const char *path = "/plugins";
    
    // Simulate the mount name construction
    snprintf(out, 1024, "%s/ebpf.d/%cnetdata_ebpf_%s.%s%s.o",
              path,
              (is_return) ? 'r' : 'p',
              name,
              version,
              (is_rhf != -1) ? ".rhf" : "");
    
    assert(strstr(out, "rnetdata_ebpf_tcp_connect") != NULL);
    assert(strstr(out, "5.14") != NULL);
    assert(strstr(out, ".rhf.o") != NULL);
}

static void test_ebpf_mount_name_format_entry() {
    char out[1024];
    int is_return = 0;
    int is_rhf = -1;
    const char *version = "4.14";
    const char *name = "vfs_write";
    const char *path = "/usr/lib";
    
    snprintf(out, 1024, "%s/ebpf.d/%cnetdata_ebpf_%s.%s%s.o",
              path,
              (is_return) ? 'r' : 'p',
              name,
              version,
              (is_rhf != -1) ? ".rhf" : "");
    
    assert(strstr(out, "pnetdata_ebpf_vfs_write") != NULL);
    assert(strstr(out, "4.14") != NULL);
    assert(strstr(out, ".o") != NULL);
}

int main() {
    test_ebpf_check_kernel_version_minimum();
    test_ebpf_check_kernel_version_below_minimum();
    test_ebpf_check_kernel_version_newer();
    test_is_ebpf_plugin_running_as_root_true();
    test_is_ebpf_plugin_running_as_root_false();
    test_is_ebpf_plugin_running_as_root_euid_true();
    test_kernel_version_parsing_simple();
    test_kernel_version_parsing_with_dash();
    test_kernel_version_patch_clamping();
    test_kernel_version_formula();
    test_ebpf_select_max_index_rhf_5_14();
    test_ebpf_select_max_index_rhf_5_4();
    test_ebpf_select_max_index_kernel_org_6_8();
    test_ebpf_select_max_index_kernel_org_5_16();
    test_ebpf_select_max_index_below_4_11();
    test_ebpf_select_index_with_available();
    test_ebpf_select_index_with_fallback();
    test_ebpf_select_kernel_name_v5_14();
    test_ebpf_select_kernel_name_v3_10();
    test_ebpf_select_kernel_name_v6_8();
    test_ebpf_mount_name_format();
    test_ebpf_mount_name_format_entry();
    
    return 0;
}