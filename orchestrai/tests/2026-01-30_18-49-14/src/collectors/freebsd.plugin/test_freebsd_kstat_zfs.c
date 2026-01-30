#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock structures and functions
extern struct arcstats {
    uint64_t hits;
    uint64_t misses;
    uint64_t demand_data_hits;
    uint64_t demand_data_misses;
    uint64_t demand_metadata_hits;
    uint64_t demand_metadata_misses;
    uint64_t prefetch_data_hits;
    uint64_t prefetch_data_misses;
    uint64_t prefetch_metadata_hits;
    uint64_t prefetch_metadata_misses;
    uint64_t mru_hits;
    uint64_t mru_ghost_hits;
    uint64_t mfu_hits;
    uint64_t mfu_ghost_hits;
    uint64_t deleted;
    uint64_t mutex_miss;
    uint64_t evict_skip;
    uint64_t hash_elements;
    uint64_t hash_elements_max;
    uint64_t hash_collisions;
    uint64_t hash_chains;
    uint64_t hash_chain_max;
    uint64_t p;
    uint64_t pd;
    uint64_t pm;
    uint64_t c;
    uint64_t c_min;
    uint64_t c_max;
    uint64_t size;
    uint64_t mru_size;
    uint64_t mfu_size;
    uint64_t l2_hits;
    uint64_t l2_misses;
    uint64_t l2_read_bytes;
    uint64_t l2_write_bytes;
    uint64_t l2_size;
    uint64_t l2_asize;
    uint64_t memory_throttle_count;
    int l2exist;
} arcstats;

extern unsigned long long zfs_arcstats_shrinkable_cache_size_bytes;

// Forward declarations
int do_kstat_zfs_misc_arcstats(int update_every, usec_t dt);
int do_kstat_zfs_misc_zio_trim(int update_every, usec_t dt);

// Mock functions
int __wrap_sysctlbyname(const char *name, void *oldp, size_t *oldlenp,
                        void *newp, size_t newlen) {
    check_expected(name);
    return mock_type(int);
}

int __wrap_GETSYSCTL_SIMPLE(const char *name, int *mib, void *var) {
    check_expected(name);
    return mock_type(int);
}

void __wrap_generate_charts_arcstats(const char *plugin, const char *module, int update_every) {
    check_expected(plugin);
    check_expected(module);
    check_expected_value(update_every);
}

void __wrap_generate_charts_arc_summary(const char *plugin, const char *module, int update_every) {
    check_expected(plugin);
    check_expected(module);
    check_expected_value(update_every);
}

void __wrap_collector_error(const char *fmt, ...) {
    (void)fmt;
}

// Test: do_kstat_zfs_misc_arcstats - successful execution
static void test_do_kstat_zfs_misc_arcstats_success(void **state) {
    (void)state;

    // Setup: l2_size sysctlbyname succeeds with non-zero value
    expect_string(__wrap_sysctlbyname, name, "kstat.zfs.misc.arcstats.l2_size");
    will_return(__wrap_sysctlbyname, 0);

    // Setup: all GETSYSCTL_SIMPLE calls succeed
    for (int i = 0; i < 36; i++) {
        expect_any(__wrap_GETSYSCTL_SIMPLE, name);
        will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    }

    // Setup: chart generation calls
    expect_string(__wrap_generate_charts_arcstats, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arcstats, module, "zfs");
    expect_value(__wrap_generate_charts_arcstats, update_every, 10);

    expect_string(__wrap_generate_charts_arc_summary, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arc_summary, module, "zfs");
    expect_value(__wrap_generate_charts_arc_summary, update_every, 10);

    // Set arcstats with values
    arcstats.size = 1000;
    arcstats.c_min = 500;

    // Execute
    int result = do_kstat_zfs_misc_arcstats(10, 1000000);

    // Assert
    assert_int_equal(result, 0);
    assert_int_equal(arcstats.l2exist, 1);
    assert_int_equal(zfs_arcstats_shrinkable_cache_size_bytes, 500);
}

// Test: do_kstat_zfs_misc_arcstats - l2_size sysctlbyname fails
static void test_do_kstat_zfs_misc_arcstats_l2_size_failure(void **state) {
    (void)state;

    expect_string(__wrap_sysctlbyname, name, "kstat.zfs.misc.arcstats.l2_size");
    will_return(__wrap_sysctlbyname, -1);

    int result = do_kstat_zfs_misc_arcstats(10, 1000000);

    assert_int_equal(result, 0);
}

// Test: do_kstat_zfs_misc_arcstats - l2_size is zero
static void test_do_kstat_zfs_misc_arcstats_l2_size_zero(void **state) {
    (void)state;

    expect_string(__wrap_sysctlbyname, name, "kstat.zfs.misc.arcstats.l2_size");
    will_return(__wrap_sysctlbyname, 0);

    for (int i = 0; i < 36; i++) {
        expect_any(__wrap_GETSYSCTL_SIMPLE, name);
        will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    }

    expect_string(__wrap_generate_charts_arcstats, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arcstats, module, "zfs");
    expect_value(__wrap_generate_charts_arcstats, update_every, 10);

    expect_string(__wrap_generate_charts_arc_summary, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arc_summary, module, "zfs");
    expect_value(__wrap_generate_charts_arc_summary, update_every, 10);

    arcstats.size = 100;
    arcstats.c_min = 50;

    int result = do_kstat_zfs_misc_arcstats(10, 1000000);

    assert_int_equal(result, 0);
    assert_int_equal(arcstats.l2exist, 0);
}

// Test: do_kstat_zfs_misc_arcstats - size less than c_min
static void test_do_kstat_zfs_misc_arcstats_size_less_than_c_min(void **state) {
    (void)state;

    expect_string(__wrap_sysctlbyname, name, "kstat.zfs.misc.arcstats.l2_size");
    will_return(__wrap_sysctlbyname, 0);

    for (int i = 0; i < 36; i++) {
        expect_any(__wrap_GETSYSCTL_SIMPLE, name);
        will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    }

    expect_string(__wrap_generate_charts_arcstats, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arcstats, module, "zfs");
    expect_value(__wrap_generate_charts_arcstats, update_every, 10);

    expect_string(__wrap_generate_charts_arc_summary, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arc_summary, module, "zfs");
    expect_value(__wrap_generate_charts_arc_summary, update_every, 10);

    arcstats.size = 100;
    arcstats.c_min = 200;

    int result = do_kstat_zfs_misc_arcstats(10, 1000000);

    assert_int_equal(result, 0);
    assert_int_equal(zfs_arcstats_shrinkable_cache_size_bytes, 0);
}

// Test: do_kstat_zfs_misc_arcstats - GETSYSCTL_SIMPLE fails
static void test_do_kstat_zfs_misc_arcstats_getsysctl_fails(void **state) {
    (void)state;

    expect_string(__wrap_sysctlbyname, name, "kstat.zfs.misc.arcstats.l2_size");
    will_return(__wrap_sysctlbyname, 0);

    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, -1);

    int result = do_kstat_zfs_misc_arcstats(10, 1000000);

    assert_int_equal(result, 0);
}

// Test: do_kstat_zfs_misc_zio_trim - success
static void test_do_kstat_zfs_misc_zio_trim_success(void **state) {
    (void)state;

    // Setup: All GETSYSCTL_SIMPLE calls succeed
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);

    int result = do_kstat_zfs_misc_zio_trim(10, 1000000);

    assert_int_equal(result, 0);
}

// Test: do_kstat_zfs_misc_zio_trim - first GETSYSCTL_SIMPLE fails
static void test_do_kstat_zfs_misc_zio_trim_bytes_fails(void **state) {
    (void)state;

    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, -1);

    int result = do_kstat_zfs_misc_zio_trim(10, 1000000);

    assert_int_equal(result, 1);
}

// Test: do_kstat_zfs_misc_zio_trim - second GETSYSCTL_SIMPLE fails
static void test_do_kstat_zfs_misc_zio_trim_success_fails(void **state) {
    (void)state;

    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, -1);

    int result = do_kstat_zfs_misc_zio_trim(10, 1000000);

    assert_int_equal(result, 1);
}

// Test: do_kstat_zfs_misc_zio_trim - third GETSYSCTL_SIMPLE fails
static void test_do_kstat_zfs_misc_zio_trim_failed_fails(void **state) {
    (void)state;

    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, -1);

    int result = do_kstat_zfs_misc_zio_trim(10, 1000000);

    assert_int_equal(result, 1);
}

// Test: do_kstat_zfs_misc_zio_trim - fourth GETSYSCTL_SIMPLE fails
static void test_do_kstat_zfs_misc_zio_trim_unsupported_fails(void **state) {
    (void)state;

    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, -1);

    int result = do_kstat_zfs_misc_zio_trim(10, 1000000);

    assert_int_equal(result, 1);
}

// Test: Edge case - dt parameter is ignored
static void test_do_kstat_zfs_misc_arcstats_dt_ignored(void **state) {
    (void)state;

    expect_string(__wrap_sysctlbyname, name, "kstat.zfs.misc.arcstats.l2_size");
    will_return(__wrap_sysctlbyname, 0);

    for (int i = 0; i < 36; i++) {
        expect_any(__wrap_GETSYSCTL_SIMPLE, name);
        will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    }

    expect_string(__wrap_generate_charts_arcstats, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arcstats, module, "zfs");
    expect_value(__wrap_generate_charts_arcstats, update_every, 5);

    expect_string(__wrap_generate_charts_arc_summary, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arc_summary, module, "zfs");
    expect_value(__wrap_generate_charts_arc_summary, update_every, 5);

    arcstats.size = 100;
    arcstats.c_min = 50;

    // dt parameter should be ignored - test with different values
    int result1 = do_kstat_zfs_misc_arcstats(5, 0);
    int result2 = do_kstat_zfs_misc_arcstats(5, 9999999);

    assert_int_equal(result1, 0);
    assert_int_equal(result2, 0);
}

// Test: Edge case - large values
static void test_do_kstat_zfs_misc_arcstats_large_values(void **state) {
    (void)state;

    expect_string(__wrap_sysctlbyname, name, "kstat.zfs.misc.arcstats.l2_size");
    will_return(__wrap_sysctlbyname, 0);

    for (int i = 0; i < 36; i++) {
        expect_any(__wrap_GETSYSCTL_SIMPLE, name);
        will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    }

    expect_string(__wrap_generate_charts_arcstats, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arcstats, module, "zfs");
    expect_value(__wrap_generate_charts_arcstats, update_every, 10);

    expect_string(__wrap_generate_charts_arc_summary, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arc_summary, module, "zfs");
    expect_value(__wrap_generate_charts_arc_summary, update_every, 10);

    arcstats.size = 0xFFFFFFFFFFFFFFFFULL;
    arcstats.c_min = 0x7FFFFFFFFFFFFFFFULL;

    int result = do_kstat_zfs_misc_arcstats(10, 1000000);

    assert_int_equal(result, 0);
    assert_int_equal(zfs_arcstats_shrinkable_cache_size_bytes, 0x7FFFFFFFFFFFFFFFULL);
}

// Test: Edge case - zero update_every
static void test_do_kstat_zfs_misc_arcstats_zero_update_every(void **state) {
    (void)state;

    expect_string(__wrap_sysctlbyname, name, "kstat.zfs.misc.arcstats.l2_size");
    will_return(__wrap_sysctlbyname, 0);

    for (int i = 0; i < 36; i++) {
        expect_any(__wrap_GETSYSCTL_SIMPLE, name);
        will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    }

    expect_string(__wrap_generate_charts_arcstats, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arcstats, module, "zfs");
    expect_value(__wrap_generate_charts_arcstats, update_every, 0);

    expect_string(__wrap_generate_charts_arc_summary, plugin, "freebsd.plugin");
    expect_string(__wrap_generate_charts_arc_summary, module, "zfs");
    expect_value(__wrap_generate_charts_arc_summary, update_every, 0);

    arcstats.size = 100;
    arcstats.c_min = 50;

    int result = do_kstat_zfs_misc_arcstats(0, 1000000);

    assert_int_equal(result, 0);
}

// Test: Edge case - zio_trim with zero values
static void test_do_kstat_zfs_misc_zio_trim_zero_values(void **state) {
    (void)state;

    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);
    expect_any(__wrap_GETSYSCTL_SIMPLE, name);
    will_return(__wrap_GETSYSCTL_SIMPLE, 0);

    int result = do_kstat_zfs_misc_zio_trim(10, 1000000);

    assert_int_equal(result, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_kstat_zfs_misc_arcstats_success),
        cmocka_unit_test(test_do_kstat_zfs_misc_arcstats_l2_size_failure),
        cmocka_unit_test(test_do_kstat_zfs_misc_arcstats_l2_size_zero),
        cmocka_unit_test(test_do_kstat_zfs_misc_arcstats_size_less_than_c_min),
        cmocka_unit_test(test_do_kstat_zfs_misc_arcstats_getsysctl_fails),
        cmocka_unit_test(test_do_kstat_zfs_misc_zio_trim_success),
        cmocka_unit_test(test_do_kstat_zfs_misc_zio_trim_bytes_fails),
        cmocka_unit_test(test_do_kstat_zfs_misc_zio_trim_success_fails),
        cmocka_unit_test(test_do_kstat_zfs_misc_zio_trim_failed_fails),
        cmocka_unit_test(test_do_kstat_zfs_misc_zio_trim_unsupported_fails),
        cmocka_unit_test(test_do_kstat_zfs_misc_arcstats_dt_ignored),
        cmocka_unit_test(test_do_kstat_zfs_misc_arcstats_large_values),
        cmocka_unit_test(test_do_kstat_zfs_misc_arcstats_zero_update_every),
        cmocka_unit_test(test_do_kstat_zfs_misc_zio_trim_zero_values),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}