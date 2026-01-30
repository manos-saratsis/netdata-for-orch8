#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>
#include <sys/types.h>
#include <time.h>

// Mock types
typedef struct {
    char family[33];
    uint32_t dev;
    uint32_t major;
    uint32_t minor;
    uint32_t bootsector_key;
    uint64_t start;
    uint64_t end;
    uint32_t flags;
    time_t last_update;
    char *boot_chart;
    struct netdata_ebpf_disks *main;
    struct netdata_ebpf_disks *boot_partition;
} netdata_ebpf_disks_t;

typedef struct {
    uint64_t value;
} netdata_syscall_stat_t;

typedef struct {
    uint64_t ncall;
    char dimension[256];
} netdata_publish_syscall_t;

typedef uint64_t netdata_idx_t;

typedef struct {
    uint32_t bin;
    uint32_t dev;
} block_key_t;

typedef struct {
    char *name;
    int internal_input;
    int user_input;
    int type;
    int map_fd;
} ebpf_local_maps_t;

typedef struct {
    int update_every;
    int lifetime;
    int maps_per_core;
    void *maps;
    int enabled;
    void *objects;
    void *probe_links;
    char thread_name[256];
    int running_time;
} ebpf_module_t;

// Test netdata_new_encode_dev
static void test_netdata_new_encode_dev_zero(void **state) {
    // Test with 0, 0
    uint32_t result = netdata_new_encode_dev(0, 0);
    assert_int_equal(result, 0);
}

static void test_netdata_new_encode_dev_minor_only(void **state) {
    // Test with major=0, minor=0xFF
    uint32_t result = netdata_new_encode_dev(0, 0xFF);
    assert_int_equal(result, 0xFF);
}

static void test_netdata_new_encode_dev_major_only(void **state) {
    // Test with major=1, minor=0
    uint32_t result = netdata_new_encode_dev(1, 0);
    assert_int_equal(result, 1 << 8);
    assert_int_equal(result, 256);
}

static void test_netdata_new_encode_dev_typical(void **state) {
    // Test with typical values: major=8, minor=0 (sda)
    uint32_t result = netdata_new_encode_dev(8, 0);
    assert_int_equal(result, 8 << 8);
    assert_int_equal(result, 2048);
}

static void test_netdata_new_encode_dev_minor_overflow(void **state) {
    // Test with minor > 0xFF
    uint32_t result = netdata_new_encode_dev(8, 0x1FF);
    uint32_t expected = (0x1FF & 0xff) | (8 << 8) | ((0x1FF & ~0xff) << 12);
    assert_int_equal(result, expected);
}

static void test_netdata_new_encode_dev_max_major(void **state) {
    // Test with max major
    uint32_t result = netdata_new_encode_dev(0xFFFFFFFF, 0);
    // Major is shifted by 8 bits
    assert_int_equal(result & 0xFFFFFF00, (0xFFFFFFFF & 0xFFFFFF) << 8);
}

static void test_netdata_new_encode_dev_partition(void **state) {
    // Test partition case: major=8, minor=1 (sda1)
    uint32_t result = netdata_new_encode_dev(8, 1);
    uint32_t expected = (1 & 0xff) | (8 << 8) | ((1 & ~0xff) << 12);
    assert_int_equal(result, expected);
}

static void test_netdata_new_encode_dev_nvme_like(void **state) {
    // Test NVMe-like major
    uint32_t result = netdata_new_encode_dev(259, 0);
    assert_int_equal(result, 259 << 8);
}

static void test_netdata_new_encode_dev_consistency(void **state) {
    // Same inputs should give same output
    uint32_t result1 = netdata_new_encode_dev(8, 16);
    uint32_t result2 = netdata_new_encode_dev(8, 16);
    assert_int_equal(result1, result2);
}

static void test_netdata_new_encode_dev_different_values(void **state) {
    // Different inputs should give different outputs
    uint32_t result1 = netdata_new_encode_dev(8, 0);
    uint32_t result2 = netdata_new_encode_dev(8, 1);
    assert_int_not_equal(result1, result2);
}

static void test_netdata_new_encode_dev_major_affects_result(void **state) {
    // Different major with same minor
    uint32_t result1 = netdata_new_encode_dev(8, 0);
    uint32_t result2 = netdata_new_encode_dev(9, 0);
    assert_int_not_equal(result1, result2);
}

static void test_netdata_new_encode_dev_large_minor(void **state) {
    // Test with large minor value
    uint32_t result = netdata_new_encode_dev(8, 0x12345);
    // Minor should be encoded as: (minor & 0xff) | ((minor & ~0xff) << 12)
    uint32_t expected = (0x12345 & 0xff) | (8 << 8) | ((0x12345 & ~0xff) << 12);
    assert_int_equal(result, expected);
}

// Test ebpf_compare_disks
static void test_ebpf_compare_disks_equal(void **state) {
    netdata_ebpf_disks_t a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.dev = 100;
    b.dev = 100;

    int result = ebpf_compare_disks(&a, &b);
    assert_int_equal(result, 0);
}

static void test_ebpf_compare_disks_a_greater(void **state) {
    netdata_ebpf_disks_t a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.dev = 200;
    b.dev = 100;

    int result = ebpf_compare_disks(&a, &b);
    assert_int_equal(result, 1);
}

static void test_ebpf_compare_disks_a_less(void **state) {
    netdata_ebpf_disks_t a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.dev = 50;
    b.dev = 100;

    int result = ebpf_compare_disks(&a, &b);
    assert_int_equal(result, -1);
}

static void test_ebpf_compare_disks_zero_dev(void **state) {
    netdata_ebpf_disks_t a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.dev = 0;
    b.dev = 0;

    int result = ebpf_compare_disks(&a, &b);
    assert_int_equal(result, 0);
}

static void test_ebpf_compare_disks_max_uint32(void **state) {
    netdata_ebpf_disks_t a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.dev = 0xFFFFFFFF;
    b.dev = 0xFFFFFFFF;

    int result = ebpf_compare_disks(&a, &b);
    assert_int_equal(result, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_netdata_new_encode_dev_zero),
        cmocka_unit_test(test_netdata_new_encode_dev_minor_only),
        cmocka_unit_test(test_netdata_new_encode_dev_major_only),
        cmocka_unit_test(test_netdata_new_encode_dev_typical),
        cmocka_unit_test(test_netdata_new_encode_dev_minor_overflow),
        cmocka_unit_test(test_netdata_new_encode_dev_max_major),
        cmocka_unit_test(test_netdata_new_encode_dev_partition),
        cmocka_unit_test(test_netdata_new_encode_dev_nvme_like),
        cmocka_unit_test(test_netdata_new_encode_dev_consistency),
        cmocka_unit_test(test_netdata_new_encode_dev_different_values),
        cmocka_unit_test(test_netdata_new_encode_dev_major_affects_result),
        cmocka_unit_test(test_netdata_new_encode_dev_large_minor),
        cmocka_unit_test(test_ebpf_compare_disks_equal),
        cmocka_unit_test(test_ebpf_compare_disks_a_greater),
        cmocka_unit_test(test_ebpf_compare_disks_a_less),
        cmocka_unit_test(test_ebpf_compare_disks_zero_dev),
        cmocka_unit_test(test_ebpf_compare_disks_max_uint32),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}