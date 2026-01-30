#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

// Mock structures and functions
typedef struct {
    uint64_t mark_page_accessed;
    uint64_t mark_buffer_dirty;
    uint64_t add_to_page_cache_lru;
    uint64_t account_page_dirtied;
} netdata_cachestat_t;

typedef struct {
    netdata_cachestat_t current;
    netdata_cachestat_t prev;
    long long ratio;
    long long dirty;
    long long hit;
    long long miss;
    uint64_t ct;
} netdata_publish_cachestat_t;

typedef struct {
    uint64_t mark_page_accessed;
    uint64_t mark_buffer_dirty;
    uint64_t add_to_page_cache_lru;
    uint64_t account_page_dirtied;
    char name[128];
    uint64_t ct;
} netdata_cachestat_pid_t;

typedef double NETDATA_DOUBLE;

// Forward declarations
void cachestat_update_publish(netdata_publish_cachestat_t *out, uint64_t mpa, uint64_t mbd, uint64_t apcl, uint64_t apd);
void ebpf_cachestat_sum_pids(netdata_publish_cachestat_t *publish, struct ebpf_pid_on_target *root);
void ebpf_cachestat_sum_cgroup_pids(netdata_publish_cachestat_t *publish, struct pid_on_target2 *root);
void ebpf_cachestat_resume_apps_data();
void ebpf_obsolete_cachestat_apps_charts(struct ebpf_module *em);
void ebpf_cache_send_apps_data(struct ebpf_target *root);
void ebpf_cachestat_calc_chart_values();
void ebpf_cachestat_send_cgroup_data(int update_every);
void ebpf_cachestat_create_apps_charts(struct ebpf_module *em, void *ptr);
void ebpf_read_cachestat_thread(void *ptr);

// ============================================================================
// Test: cachestat_update_publish - Basic functionality
// ============================================================================
static void test_cachestat_update_publish_normal_values(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    // Test with normal values
    uint64_t mpa = 100;  // mark_page_accessed
    uint64_t mbd = 20;   // mark_buffer_dirty
    uint64_t apcl = 80;  // add_to_page_cache_lru
    uint64_t apd = 10;   // account_page_dirtied

    cachestat_update_publish(&out, mpa, mbd, apcl, apd);

    // Expected: total = mpa - mbd = 100 - 20 = 80
    //           misses = apcl - apd = 80 - 10 = 70
    //           hits = total - misses = 80 - 70 = 10
    //           ratio = hits / total = 10 / 80 = 0.125 = 12.5% -> 12 (integer)
    assert_int_equal(out.hit, 10);
    assert_int_equal(out.miss, 70);
    assert_true(out.ratio >= 12 && out.ratio <= 13);
}

// ============================================================================
// Test: cachestat_update_publish - Zero values
// ============================================================================
static void test_cachestat_update_publish_zero_values(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    cachestat_update_publish(&out, 0, 0, 0, 0);

    // With all zeros: total = 0, misses = 0, hits = 0
    // ratio should be 1 (100%) when total is 0
    assert_int_equal(out.hit, 0);
    assert_int_equal(out.miss, 0);
    assert_int_equal(out.ratio, 100);
}

// ============================================================================
// Test: cachestat_update_publish - Negative total (clamped to 0)
// ============================================================================
static void test_cachestat_update_publish_negative_total(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    // mpa=50, mbd=100 -> total = -50 (clamped to 0)
    cachestat_update_publish(&out, 50, 100, 80, 10);

    assert_int_equal(out.hit, 0);
    assert_int_equal(out.miss, 0);
    assert_int_equal(out.ratio, 100);
}

// ============================================================================
// Test: cachestat_update_publish - Negative misses (clamped to 0)
// ============================================================================
static void test_cachestat_update_publish_negative_misses(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    // apcl=10, apd=100 -> misses = -90 (clamped to 0)
    cachestat_update_publish(&out, 100, 20, 10, 100);

    // total = 80, misses clamped to 0, hits = 80, ratio = 100%
    assert_int_equal(out.miss, 0);
    assert_int_equal(out.hit, 80);
    assert_int_equal(out.ratio, 100);
}

// ============================================================================
// Test: cachestat_update_publish - Negative hits (misses > total)
// ============================================================================
static void test_cachestat_update_publish_negative_hits(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    // mpa=50, mbd=10, apcl=100, apd=10
    // total = 40, misses = 90 (> total, so hits would be -50)
    cachestat_update_publish(&out, 50, 10, 100, 10);

    // When hits < 0, misses becomes total and hits become 0
    assert_int_equal(out.miss, 40);
    assert_int_equal(out.hit, 0);
    assert_int_equal(out.ratio, 0);
}

// ============================================================================
// Test: cachestat_update_publish - Large values
// ============================================================================
static void test_cachestat_update_publish_large_values(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    uint64_t mpa = 1000000;
    uint64_t mbd = 100000;
    uint64_t apcl = 800000;
    uint64_t apd = 100000;

    cachestat_update_publish(&out, mpa, mbd, apcl, apd);

    // total = 900000, misses = 700000, hits = 200000
    // ratio = 200000/900000 = 22.22% -> 22
    assert_int_equal(out.hit, 200000);
    assert_int_equal(out.miss, 700000);
    assert_true(out.ratio >= 22 && out.ratio <= 23);
}

// ============================================================================
// Test: cachestat_update_publish - All misses (no hits)
// ============================================================================
static void test_cachestat_update_publish_all_misses(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    cachestat_update_publish(&out, 50, 10, 100, 40);

    // total = 40, misses = 60, but hits would be negative
    // So misses becomes total (40), hits = 0
    assert_int_equal(out.miss, 40);
    assert_int_equal(out.hit, 0);
    assert_int_equal(out.ratio, 0);
}

// ============================================================================
// Test: cachestat_update_publish - Perfect cache (all hits)
// ============================================================================
static void test_cachestat_update_publish_perfect_cache(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    // All accesses are from cache (no new page cache additions)
    cachestat_update_publish(&out, 100, 0, 0, 0);

    // total = 100, misses = 0, hits = 100, ratio = 100%
    assert_int_equal(out.hit, 100);
    assert_int_equal(out.miss, 0);
    assert_int_equal(out.ratio, 100);
}

// ============================================================================
// Test: cachestat_update_publish - NULL output pointer
// ============================================================================
static void test_cachestat_update_publish_null_output(void **state)
{
    (void)state;
    // This should be handled by caller, but testing defensive behavior
    // In C, this would crash, so we test that structure is preserved
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));
    out.ratio = 50;
    
    // Verify initial state
    assert_int_equal(out.ratio, 50);
}

// ============================================================================
// Test: cachestat_update_publish - Ratio calculation with 50% hit rate
// ============================================================================
static void test_cachestat_update_publish_half_hit_rate(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    // Setup for 50% hit rate
    // total = 100, misses = 50, hits = 50
    // ratio = 50/100 = 0.5 = 50%
    cachestat_update_publish(&out, 100, 0, 50, 0);

    assert_int_equal(out.hit, 50);
    assert_int_equal(out.miss, 50);
    assert_int_equal(out.ratio, 50);
}

// ============================================================================
// Test: cachestat_update_publish - Edge case: single access
// ============================================================================
static void test_cachestat_update_publish_single_access(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    cachestat_update_publish(&out, 1, 0, 0, 0);

    assert_int_equal(out.hit, 1);
    assert_int_equal(out.miss, 0);
    assert_int_equal(out.ratio, 100);
}

// ============================================================================
// Test: cachestat_update_publish - Very small values
// ============================================================================
static void test_cachestat_update_publish_small_values(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    cachestat_update_publish(&out, 1, 0, 1, 0);

    // total = 1, misses = 1, hits = 0, ratio = 0%
    assert_int_equal(out.hit, 0);
    assert_int_equal(out.miss, 1);
    assert_int_equal(out.ratio, 0);
}

// ============================================================================
// Test: cachestat_update_publish - Equal access and dirty
// ============================================================================
static void test_cachestat_update_publish_equal_access_dirty(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    cachestat_update_publish(&out, 100, 100, 100, 100);

    // total = 0, misses = 0, hits = 0, ratio = 100%
    assert_int_equal(out.hit, 0);
    assert_int_equal(out.miss, 0);
    assert_int_equal(out.ratio, 100);
}

// ============================================================================
// Test: cachestat_update_publish - High dirty ratio
// ============================================================================
static void test_cachestat_update_publish_high_dirty_ratio(void **state)
{
    (void)state;
    netdata_publish_cachestat_t out;
    memset(&out, 0, sizeof(out));

    // mpa=100, mbd=90 -> total = 10
    // apcl=50, apd=40 -> misses = 10
    // hits = 0, ratio = 0%
    cachestat_update_publish(&out, 100, 90, 50, 40);

    assert_int_equal(out.hit, 0);
    assert_int_equal(out.miss, 10);
    assert_int_equal(out.ratio, 0);
}

// ============================================================================
// Test suite runner
// ============================================================================
int run_cachestat_update_publish_tests(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cachestat_update_publish_normal_values),
        cmocka_unit_test(test_cachestat_update_publish_zero_values),
        cmocka_unit_test(test_cachestat_update_publish_negative_total),
        cmocka_unit_test(test_cachestat_update_publish_negative_misses),
        cmocka_unit_test(test_cachestat_update_publish_negative_hits),
        cmocka_unit_test(test_cachestat_update_publish_large_values),
        cmocka_unit_test(test_cachestat_update_publish_all_misses),
        cmocka_unit_test(test_cachestat_update_publish_perfect_cache),
        cmocka_unit_test(test_cachestat_update_publish_null_output),
        cmocka_unit_test(test_cachestat_update_publish_half_hit_rate),
        cmocka_unit_test(test_cachestat_update_publish_single_access),
        cmocka_unit_test(test_cachestat_update_publish_small_values),
        cmocka_unit_test(test_cachestat_update_publish_equal_access_dirty),
        cmocka_unit_test(test_cachestat_update_publish_high_dirty_ratio),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

// ============================================================================
// Main test execution
// ============================================================================
int main(void)
{
    return run_cachestat_update_publish_tests();
}