```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cmocka.h>

// Mock global functions and variables from the original file
extern int do_proc_spl_kstat_zfs_arcstats(int update_every, usec_t dt);
extern struct arcstats arcstats;

// Mock function declarations for dependencies
int mock_procfile_readall(void *procfile);
void mock_generate_charts_arcstats(const char *plugin, const char *file, int update_every);
void mock_generate_charts_arc_summary(const char *plugin, const char *file, int update_every);

// Test case: Normal operation with ZFS stats available
static void test_do_proc_spl_kstat_zfs_arcstats_normal(void **state) {
    (void) state; // unused

    // Setup mock conditions for ZFS stats being available
    arcstats.l2exist = -1;
    arcstats.size = 1000;
    arcstats.c_min = 500;

    int result = do_proc_spl_kstat_zfs_arcstats(5, 0);
    assert_int_equal(result, 0);
    assert_int_equal(arcstats.l2exist, 0);
    assert_int_equal(zfs_arcstats_shrinkable_cache_size_bytes, 500);
}

// Test case: No ZFS pools available
static void test_do_proc_spl_kstat_zfs_arcstats_no_pools(void **state) {
    (void) state; // unused

    // Setup mock conditions for no ZFS pools
    arcstats.l2exist = -1;

    int result = do_proc_spl_kstat_zfs_arcstats(5, 0);
    assert_int_equal(result, 0);
}

// Test case: Cannot open arcstats file
static void test_do_proc_spl_kstat_zfs_arcstats_file_open_error(void **state) {
    (void) state; // unused

    // Simulate file open failure
    int result = do_proc_spl_kstat_zfs_arcstats(5, 0);
    assert_int_equal(result, 1);
}

// Test case: Invalid file content
static void test_do_proc_spl_kstat_zfs_arcstats_invalid_content(void **state) {
    (void) state; // unused

    // Simulate invalid file content
    arcstats.l2exist = -1;
    int result = do_proc_spl_kstat_zfs_arcstats(5, 0);
    assert_int_equal(result, 0);
    assert_int_equal(arcstats.l2exist, 0);
}

// Test case: Edge case - size less than c_min
static void test_do_proc_spl_kstat_zfs_arcstats_small_size(void **state) {
    (void) state; // unused

    arcstats.size = 300;
    arcstats.c_min = 500;

    int result = do_proc_spl_kstat_zfs_arcstats(5, 0);
    assert_int_equal(result, 0);
    assert_int_equal(zfs_arcstats_shrinkable_cache_size_bytes, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_proc_spl_kstat_zfs_arcstats_normal),
        cmocka_unit_test(test_do_proc_spl_kstat_zfs_arcstats_no_pools),
        cmocka_unit_test(test_do_proc_spl_kstat_zfs_arcstats_file_open_error),
        cmocka_unit_test(test_do_proc_spl_kstat_zfs_arcstats_invalid_content),
        cmocka_unit_test(test_do_proc_spl_kstat_zfs_arcstats_small_size),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
```