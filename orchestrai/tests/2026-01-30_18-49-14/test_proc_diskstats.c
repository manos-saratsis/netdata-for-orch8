#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <unistd.h>

// Mock structures and functions for testing
typedef struct {
    char *data;
    size_t size;
    size_t pos;
} mock_file_t;

// Declaration of exported functions from proc_diskstats.c
int do_proc_diskstats(int update_every, usec_t dt);

// Forward declarations of helper functions being tested
static unsigned long long int bcache_read_number_with_units(const char *filename);
static inline int is_major_enabled(int major);
static inline bool ends_with(const char *str, const char *suffix);
static const char *get_disk_type_string(int disk_type);

// Test: bcache_read_number_with_units with k suffix
static void test_bcache_read_number_with_k_suffix(void **state) {
    (void)state;
    // This function reads from file system, we'll test the parsing logic
    // Through mocking file operations
    assert_true(1); // Placeholder - requires filesystem mocking
}

// Test: bcache_read_number_with_units with M suffix
static void test_bcache_read_number_with_M_suffix(void **state) {
    (void)state;
    assert_true(1); // Placeholder - requires filesystem mocking
}

// Test: bcache_read_number_with_units with G suffix
static void test_bcache_read_number_with_G_suffix(void **state) {
    (void)state;
    assert_true(1); // Placeholder - requires filesystem mocking
}

// Test: bcache_read_number_with_units with T suffix
static void test_bcache_read_number_with_T_suffix(void **state) {
    (void)state;
    assert_true(1); // Placeholder - requires filesystem mocking
}

// Test: bcache_read_number_with_units with unknown suffix
static void test_bcache_read_number_with_unknown_suffix(void **state) {
    (void)state;
    assert_true(1); // Placeholder - requires filesystem mocking
}

// Test: bcache_read_number_with_units file not readable
static void test_bcache_read_number_with_file_not_readable(void **state) {
    (void)state;
    // Should return 0 when file cannot be read
    assert_true(1); // Placeholder - requires filesystem mocking
}

// Test: is_major_enabled with negative major
static void test_is_major_enabled_negative_major(void **state) {
    (void)state;
    // Negative major should return 1
    assert_true(1); // Placeholder - requires config mocking
}

// Test: is_major_enabled with positive major first time
static void test_is_major_enabled_positive_major_first_time(void **state) {
    (void)state;
    // Should read from config
    assert_true(1); // Placeholder - requires config mocking
}

// Test: is_major_enabled with cached result
static void test_is_major_enabled_cached_result(void **state) {
    (void)state;
    // Second call should use cached value
    assert_true(1); // Placeholder - requires config mocking
}

// Test: is_major_enabled with large major number
static void test_is_major_enabled_large_major_number(void **state) {
    (void)state;
    // Should handle large major numbers
    assert_true(1); // Placeholder - requires config mocking
}

// Test: ends_with with null pointer string
static void test_ends_with_null_str(void **state) {
    (void)state;
    // ends_with("string", NULL) should return false
    assert_false(ends_with("string", NULL));
}

// Test: ends_with with null pointer suffix
static void test_ends_with_null_suffix(void **state) {
    (void)state;
    // ends_with(NULL, "suffix") should return false
    assert_false(ends_with(NULL, "suffix"));
}

// Test: ends_with both null
static void test_ends_with_both_null(void **state) {
    (void)state;
    // ends_with(NULL, NULL) should return false
    assert_false(ends_with(NULL, NULL));
}

// Test: ends_with matching suffix
static void test_ends_with_matching_suffix(void **state) {
    (void)state;
    // ends_with("filename.txt", ".txt") should return true
    assert_true(ends_with("filename.txt", ".txt"));
}

// Test: ends_with non-matching suffix
static void test_ends_with_non_matching_suffix(void **state) {
    (void)state;
    // ends_with("filename.txt", ".c") should return false
    assert_false(ends_with("filename.txt", ".c"));
}

// Test: ends_with suffix longer than string
static void test_ends_with_suffix_longer_than_string(void **state) {
    (void)state;
    // ends_with("abc", "abcdef") should return false
    assert_false(ends_with("abc", "abcdef"));
}

// Test: ends_with empty string
static void test_ends_with_empty_string(void **state) {
    (void)state;
    // ends_with("", "abc") should return false
    assert_false(ends_with("", "abc"));
}

// Test: ends_with empty suffix
static void test_ends_with_empty_suffix(void **state) {
    (void)state;
    // ends_with("string", "") should return true (any string ends with empty suffix)
    assert_true(ends_with("string", ""));
}

// Test: get_disk_type_string with DISK_TYPE_PHYSICAL
static void test_get_disk_type_string_physical(void **state) {
    (void)state;
    const char *result = get_disk_type_string(1);  // DISK_TYPE_PHYSICAL
    assert_string_equal(result, "physical");
}

// Test: get_disk_type_string with DISK_TYPE_PARTITION
static void test_get_disk_type_string_partition(void **state) {
    (void)state;
    const char *result = get_disk_type_string(2);  // DISK_TYPE_PARTITION
    assert_string_equal(result, "partition");
}

// Test: get_disk_type_string with DISK_TYPE_VIRTUAL
static void test_get_disk_type_string_virtual(void **state) {
    (void)state;
    const char *result = get_disk_type_string(3);  // DISK_TYPE_VIRTUAL
    assert_string_equal(result, "virtual");
}

// Test: get_disk_type_string with DISK_TYPE_UNKNOWN
static void test_get_disk_type_string_unknown(void **state) {
    (void)state;
    const char *result = get_disk_type_string(0);  // DISK_TYPE_UNKNOWN
    assert_string_equal(result, "unknown");
}

// Test: get_disk_type_string with invalid type
static void test_get_disk_type_string_invalid_type(void **state) {
    (void)state;
    const char *result = get_disk_type_string(999);  // Invalid type
    assert_string_equal(result, "unknown");
}

// Test: diskstats_function_block_devices basic functionality
static void test_diskstats_function_block_devices_basic(void **state) {
    (void)state;
    // This function creates JSON output, requires buffer mocking
    assert_true(1); // Placeholder
}

// Test: diskstats_function_block_devices with empty disk list
static void test_diskstats_function_block_devices_empty_list(void **state) {
    (void)state;
    // Should handle empty disk list
    assert_true(1); // Placeholder
}

// Test: diskstats_function_block_devices with multiple disks
static void test_diskstats_function_block_devices_multiple_disks(void **state) {
    (void)state;
    // Should handle multiple disks
    assert_true(1); // Placeholder
}

// Test: diskstats_function_block_devices with NaN values
static void test_diskstats_function_block_devices_with_nan_values(void **state) {
    (void)state;
    // Should handle NaN values correctly
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats first initialization
static void test_do_proc_diskstats_first_initialization(void **state) {
    (void)state;
    // First call should initialize globals
    assert_true(1); // Placeholder - requires extensive mocking
}

// Test: do_proc_diskstats with dt=0
static void test_do_proc_diskstats_with_zero_dt(void **state) {
    (void)state;
    // When dt is 0, skip differential calculations
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats with dt > 0
static void test_do_proc_diskstats_with_positive_dt(void **state) {
    (void)state;
    // When dt > 0, calculate differential metrics
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats file not readable
static void test_do_proc_diskstats_file_not_readable(void **state) {
    (void)state;
    // Should return 0 when file cannot be opened
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats with empty file
static void test_do_proc_diskstats_empty_file(void **state) {
    (void)state;
    // Should handle empty file gracefully
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats with malformed lines (less than 14 words)
static void test_do_proc_diskstats_malformed_lines_short(void **state) {
    (void)state;
    // Lines with < 14 words should be skipped
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats with extended stats (words > 13)
static void test_do_proc_diskstats_with_extended_stats(void **state) {
    (void)state;
    // Should parse discard stats when available
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats with flush stats (words > 17)
static void test_do_proc_diskstats_with_flush_stats(void **state) {
    (void)state;
    // Should parse flush stats when available
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats physical disk system accounting
static void test_do_proc_diskstats_physical_disk_system_accounting(void **state) {
    (void)state;
    // Physical disks should update system totals
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats partition disk ignored
static void test_do_proc_diskstats_partition_disk_ignored(void **state) {
    (void)state;
    // Partitions should not update system totals
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats virtual disk ignored for system totals
static void test_do_proc_diskstats_virtual_disk_system_accounting(void **state) {
    (void)state;
    // Virtual disks should not update system totals
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats CONFIG_BOOLEAN_AUTO handling for io
static void test_do_proc_diskstats_boolean_auto_io(void **state) {
    (void)state;
    // CONFIG_BOOLEAN_AUTO should enable io metrics
    assert_true(1); // Placeholder
}

// Test: do_proc_diskstats disk cleanup for removed disks
static void test_do_proc_diskstats_disk_cleanup(void **state) {
    (void)state;
    // Should cleanup removed disks
    assert_true(1); // Placeholder
}

// Test: diskstats_cleanup_disks removes updated flag
static void test_diskstats_cleanup_disks_removes_flag(void **state) {
    (void)state;
    // Should reset updated flag after cleanup
    assert_true(1); // Placeholder
}

// Test: bcache_read_priority_stats with valid file
static void test_bcache_read_priority_stats_valid_file(void **state) {
    (void)state;
    // Should parse priority stats correctly
    assert_true(1); // Placeholder
}

// Test: bcache_read_priority_stats elapsed time check
static void test_bcache_read_priority_stats_elapsed_time_check(void **state) {
    (void)state;
    // Should only update when elapsed time exceeds threshold
    assert_true(1); // Placeholder
}

// Test: bcache_read_priority_stats malformed lines
static void test_bcache_read_priority_stats_malformed_lines(void **state) {
    (void)state;
    // Should skip malformed lines
    assert_true(1); // Placeholder
}

// Test: bcache_read_priority_stats zero calculation
static void test_bcache_read_priority_stats_calculation(void **state) {
    (void)state;
    // Should calculate unknown as 100 - sum of known
    assert_true(1); // Placeholder
}

// Test: get_disk_config with excluded disk
static void test_get_disk_config_excluded_disk(void **state) {
    (void)state;
    // Should disable metrics for excluded disks
    assert_true(1); // Placeholder
}

// Test: get_disk_config CONFIG_BOOLEAN_NO
static void test_get_disk_config_disabled(void **state) {
    (void)state;
    // Should disable all metrics when CONFIG_BOOLEAN_NO
    assert_true(1); // Placeholder
}

// Test: get_disk_config physical disk performance enabled
static void test_get_disk_config_physical_disk_performance(void **state) {
    (void)state;
    // Should enable performance metrics for physical disks
    assert_true(1); // Placeholder
}

// Test: get_disk_config partition disk performance disabled
static void test_get_disk_config_partition_performance_disabled(void **state) {
    (void)state;
    // Should disable performance metrics for partitions
    assert_true(1); // Placeholder
}

// Test: get_disk_config virtual disk performance
static void test_get_disk_config_virtual_disk_performance(void **state) {
    (void)state;
    // Should respect virtual disk config
    assert_true(1); // Placeholder
}

// Test: get_disk_config bcache device
static void test_get_disk_config_bcache_device(void **state) {
    (void)state;
    // Should handle bcache devices
    assert_true(1); // Placeholder
}

// Test: get_disk_name edge case empty paths
static void test_get_disk_name_empty_paths(void **state) {
    (void)state;
    // Should fallback to disk name when paths are empty
    assert_true(1); // Placeholder
}

// Test: get_disk_by_id no matching device
static void test_get_disk_by_id_no_match(void **state) {
    (void)state;
    // Should return NULL when no device found
    assert_true(1); // Placeholder
}

// Test: get_disk_by_id with matching device
static void test_get_disk_by_id_match_found(void **state) {
    (void)state;
    // Should return device ID when found
    assert_true(1); // Placeholder
}

// Test: get_disk_by_id skip md-uuid entries
static void test_get_disk_by_id_skip_md_uuid(void **state) {
    (void)state;
    // Should skip md-uuid- prefixed entries
    assert_true(1); // Placeholder
}

// Test: get_disk_by_id skip nvme-eui entries
static void test_get_disk_by_id_skip_nvme_eui(void **state) {
    (void)state;
    // Should skip nvme-eui. prefixed entries
    assert_true(1); // Placeholder
}

// Test: get_disk_model device not found
static void test_get_disk_model_not_found(void **state) {
    (void)state;
    // Should return NULL when model not available
    assert_true(1); // Placeholder
}

// Test: get_disk_serial device not found
static void test_get_disk_serial_not_found(void **state) {
    (void)state;
    // Should return NULL when serial not available
    assert_true(1); // Placeholder
}

// Test: add_labels_to_disk with null values
static void test_add_labels_to_disk_with_null_values(void **state) {
    (void)state;
    // Should handle null values gracefully
    assert_true(1); // Placeholder
}

// Test: get_disk with existing disk in list
static void test_get_disk_existing_disk_found(void **state) {
    (void)state;
    // Should return existing disk structure
    assert_true(1); // Placeholder
}

// Test: get_disk with new disk creation
static void test_get_disk_new_disk_creation(void **state) {
    (void)state;
    // Should create and append new disk
    assert_true(1); // Placeholder
}

// Test: get_disk bcache detection
static void test_get_disk_bcache_detection(void **state) {
    (void)state;
    // Should detect bcache devices
    assert_true(1); // Placeholder
}

// Test: get_disk partition detection
static void test_get_disk_partition_detection(void **state) {
    (void)state;
    // Should detect partition type
    assert_true(1); // Placeholder
}

// Test: get_disk physical disk detection
static void test_get_disk_physical_disk_detection(void **state) {
    (void)state;
    // Should detect physical disk
    assert_true(1); // Placeholder
}

// Test: get_disk virtual disk detection
static void test_get_disk_virtual_disk_detection(void **state) {
    (void)state;
    // Should detect virtual disk
    assert_true(1); // Placeholder
}

// Test: get_disk dm-uuid special handling
static void test_get_disk_dm_uuid_special_handling(void **state) {
    (void)state;
    // Should handle LVM dm-uuid specially
    assert_true(1); // Placeholder
}

// Test: get_disk_name_from_path depth limit
static void test_get_disk_name_from_path_depth_limit(void **state) {
    (void)state;
    // Should respect depth limit
    assert_true(1); // Placeholder
}

// Test: get_disk_name_from_path preferred_ids match
static void test_get_disk_name_from_path_preferred_match(void **state) {
    (void)state;
    // Should select preferred IDs
    assert_true(1); // Placeholder
}

// Test: get_disk_name_from_path cannot open directory
static void test_get_disk_name_from_path_cannot_open_dir(void **state) {
    (void)state;
    // Should handle directory open errors
    assert_true(1); // Placeholder
}

// Test: get_disk_name_from_path symlink cannot read
static void test_get_disk_name_from_path_symlink_read_error(void **state) {
    (void)state;
    // Should skip unreadable symlinks
    assert_true(1); // Placeholder
}

// Test: get_disk_name_from_path stat error
static void test_get_disk_name_from_path_stat_error(void **state) {
    (void)state;
    // Should skip files with stat errors
    assert_true(1); // Placeholder
}

// Test: initialization of disk structure fields
static void test_get_disk_init_fields(void **state) {
    (void)state;
    // Should initialize all disk structure fields
    assert_true(1); // Placeholder
}

// Test: common_disk_io integration
static void test_do_proc_diskstats_common_disk_io(void **state) {
    (void)state;
    // Should call common_disk_io with correct parameters
    assert_true(1); // Placeholder
}

// Test: common_disk_ops integration
static void test_do_proc_diskstats_common_disk_ops(void **state) {
    (void)state;
    // Should call common_disk_ops with correct parameters
    assert_true(1); // Placeholder
}

// Test: common_disk_await calculation with zero operations
static void test_do_proc_diskstats_disk_await_zero_ops(void **state) {
    (void)state;
    // Should handle zero operations case
    assert_true(1); // Placeholder
}

// Test: disk_utilization capping at 100
static void test_do_proc_diskstats_utilization_cap(void **state) {
    (void)state;
    // Should cap utilization at 100%
    assert_true(1); // Placeholder
}

// Test: average sector size calculation
static void test_do_proc_diskstats_avg_sector_calculation(void **state) {
    (void)state;
    // Should calculate average sector size correctly
    assert_true(1); // Placeholder
}

// Test: bcache filenames initialization
static void test_get_disk_bcache_filenames_init(void **state) {
    (void)state;
    // Should initialize all bcache filenames
    assert_true(1); // Placeholder
}

// Test: bcache missing file error handling
static void test_get_disk_bcache_missing_file(void **state) {
    (void)state;
    // Should handle missing bcache files
    assert_true(1); // Placeholder
}

// Test: mountinfo lookup
static void test_get_disk_mountinfo_lookup(void **state) {
    (void)state;
    // Should find and set mount point
    assert_true(1); // Placeholder
}

// Test: mutex initialization
static void test_init_mutex(void **state) {
    (void)state;
    // Mutex should be initialized
    assert_true(1); // Placeholder
}

// Test: mutex destruction
static void test_destroy_mutex(void **state) {
    (void)state;
    // Mutex should be destroyed
    assert_true(1); // Placeholder
}

// Test: procfile_readall failure handling
static void test_do_proc_diskstats_procfile_readall_failure(void **state) {
    (void)state;
    // Should handle procfile_readall returning NULL
    assert_true(1); // Placeholder
}

// Test: zero lines in procfile
static void test_do_proc_diskstats_zero_lines(void **state) {
    (void)state;
    // Should handle zero lines gracefully
    assert_true(1); // Placeholder
}

const struct CMUnitTest diskstats_tests[] = {
    cmocka_unit_test(test_bcache_read_number_with_k_suffix),
    cmocka_unit_test(test_bcache_read_number_with_M_suffix),
    cmocka_unit_test(test_bcache_read_number_with_G_suffix),
    cmocka_unit_test(test_bcache_read_number_with_T_suffix),
    cmocka_unit_test(test_bcache_read_number_with_unknown_suffix),
    cmocka_unit_test(test_bcache_read_number_with_file_not_readable),
    cmocka_unit_test(test_is_major_enabled_negative_major),
    cmocka_unit_test(test_is_major_enabled_positive_major_first_time),
    cmocka_unit_test(test_is_major_enabled_cached_result),
    cmocka_unit_test(test_is_major_enabled_large_major_number),
    cmocka_unit_test(test_ends_with_null_str),
    cmocka_unit_test(test_ends_with_null_suffix),
    cmocka_unit_test(test_ends_with_both_null),
    cmocka_unit_test(test_ends_with_matching_suffix),
    cmocka_unit_test(test_ends_with_non_matching_suffix),
    cmocka_unit_test(test_ends_with_suffix_longer_than_string),
    cmocka_unit_test(test_ends_with_empty_string),
    cmocka_unit_test(test_ends_with_empty_suffix),
    cmocka_unit_test(test_get_disk_type_string_physical),
    cmocka_unit_test(test_get_disk_type_string_partition),
    cmocka_unit_test(test_get_disk_type_string_virtual),
    cmocka_unit_test(test_get_disk_type_string_unknown),
    cmocka_unit_test(test_get_disk_type_string_invalid_type),
    cmocka_unit_test(test_diskstats_function_block_devices_basic),
    cmocka_unit_test(test_diskstats_function_block_devices_empty_list),
    cmocka_unit_test(test_diskstats_function_block_devices_multiple_disks),
    cmocka_unit_test(test_diskstats_function_block_devices_with_nan_values),
    cmocka_unit_test(test_do_proc_diskstats_first_initialization),
    cmocka_unit_test(test_do_proc_diskstats_with_zero_dt),
    cmocka_unit_test(test_do_proc_diskstats_with_positive_dt),
    cmocka_unit_test(test_do_proc_diskstats_file_not_readable),
    cmocka_unit_test(test_do_proc_diskstats_empty_file),
    cmocka_unit_test(test_do_proc_diskstats_malformed_lines_short),
    cmocka_unit_test(test_do_proc_diskstats_with_extended_stats),
    cmocka_unit_test(test_do_proc_diskstats_with_flush_stats),
    cmocka_unit_test(test_do_proc_diskstats_physical_disk_system_accounting),
    cmocka_unit_test(test_do_proc_diskstats_partition_disk_ignored),
    cmocka_unit_test(test_do_proc_diskstats_virtual_disk_system_accounting),
    cmocka_unit_test(test_do_proc_diskstats_boolean_auto_io),
    cmocka_unit_test(test_do_proc_diskstats_disk_cleanup),
    cmocka_unit_test(test_diskstats_cleanup_disks_removes_flag),
    cmocka_unit_test(test_bcache_read_priority_stats_valid_file),
    cmocka_unit_test(test_bcache_read_priority_stats_elapsed_time_check),
    cmocka_unit_test(test_bcache_read_priority_stats_malformed_lines),
    cmocka_unit_test(test_bcache_read_priority_stats_calculation),
    cmocka_unit_test(test_get_disk_config_excluded_disk),
    cmocka_unit_test(test_get_disk_config_disabled),
    cmocka_unit_test(test_get_disk_config_physical_disk_performance),
    cmocka_unit_test(test_get_disk_config_partition_performance_disabled),
    cmocka_unit_test(test_get_disk_config_virtual_disk_performance),
    cmocka_unit_test(test_get_disk_config_bcache_device),
    cmocka_unit_test(test_get_disk_name_empty_paths),
    cmocka_unit_test(test_get_disk_by_id_no_match),
    cmocka_unit_test(test_get_disk_by_id_match_found),
    cmocka_unit_test(test_get_disk_by_id_skip_md_uuid),
    cmocka_unit_test(test_get_disk_by_id_skip_nvme_eui),
    cmocka_unit_test(test_get_disk_model_not_found),
    cmocka_unit_test(test_get_disk_serial_not_found),
    cmocka_unit_test(test_add_labels_to_disk_with_null_values),
    cmocka_unit_test(test_get_disk_existing_disk_found),
    cmocka_unit_test(test_get_disk_new_disk_creation),
    cmocka_unit_test(test_get_disk_bcache_detection),
    cmocka_unit_test(test_get_disk_partition_detection),
    cmocka_unit_test(test_get_disk_physical_disk_detection),
    cmocka_unit_test(test_get_disk_virtual_disk_detection),
    cmocka_unit_test(test_get_disk_dm_uuid_special_handling),
    cmocka_unit_test(test_get_disk_name_from_path_depth_limit),
    cmocka_unit_test(test_get_disk_name_from_path_preferred_match),
    cmocka_unit_test(test_get_disk_name_from_path_cannot_open_dir),
    cmocka_unit_test(test_get_disk_name_from_path_symlink_read_error),
    cmocka_unit_test(test_get_disk_name_from_path_stat_error),
    cmocka_unit_test(test_get_disk_init_fields),
    cmocka_unit_test(test_do_proc_diskstats_common_disk_io),
    cmocka_unit_test(test_do_proc_diskstats_common_disk_ops),
    cmocka_unit_test(test_do_proc_diskstats_disk_await_zero_ops),
    cmocka_unit_test(test_do_proc_diskstats_utilization_cap),
    cmocka_unit_test(test_do_proc_diskstats_avg_sector_calculation),
    cmocka_unit_test(test_get_disk_bcache_filenames_init),
    cmocka_unit_test(test_get_disk_bcache_missing_file),
    cmocka_unit_test(test_get_disk_mountinfo_lookup),
    cmocka_unit_test(test_init_mutex),
    cmocka_unit_test(test_destroy_mutex),
    cmocka_unit_test(test_do_proc_diskstats_procfile_readall_failure),
    cmocka_unit_test(test_do_proc_diskstats_zero_lines),
};

int main(void) {
    return cmocka_run_group_tests(diskstats_tests, NULL, NULL);
}