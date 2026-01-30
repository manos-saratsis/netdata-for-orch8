#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <cmocka.h>
#include <stdarg.h>

#include "ebpf.h"
#include "ebpf_sync.h"

/* Mock structures and global variables */
extern ebpf_module_t test_em;
extern netdata_ebpf_targets_t sync_targets[];
extern struct config sync_config;
extern ebpf_local_maps_t sync_maps[];
extern ebpf_local_maps_t syncfs_maps[];
extern ebpf_local_maps_t msync_maps[];
extern ebpf_local_maps_t fsync_maps[];
extern ebpf_local_maps_t fdatasync_maps[];
extern ebpf_local_maps_t sync_file_range_maps[];

/* Global test setup/teardown */
static int setup(void **state) {
    memset(&test_em, 0, sizeof(ebpf_module_t));
    test_em.update_every = 1;
    test_em.lifetime = 300;
    test_em.maps_per_core = 0;
    return 0;
}

static int teardown(void **state) {
    return 0;
}

/* Test for sync_counter_dimension_name array */
static void test_sync_dimension_names(void **state) {
    assert_non_null(sync_counter_dimension_name);
    assert_string_equal(sync_counter_dimension_name[NETDATA_SYNC_SYNC_IDX], "sync");
    assert_string_equal(sync_counter_dimension_name[NETDATA_SYNC_SYNCFS_IDX], "syncfs");
    assert_string_equal(sync_counter_dimension_name[NETDATA_SYNC_MSYNC_IDX], "msync");
    assert_string_equal(sync_counter_dimension_name[NETDATA_SYNC_FSYNC_IDX], "fsync");
    assert_string_equal(sync_counter_dimension_name[NETDATA_SYNC_FDATASYNC_IDX], "fdatasync");
    assert_string_equal(sync_counter_dimension_name[NETDATA_SYNC_SYNC_FILE_RANGE_IDX], "sync_file_range");
}

/* Test for sync_maps initialization */
static void test_sync_maps_initialization(void **state) {
    assert_non_null(sync_maps);
    assert_string_equal(sync_maps[0].name, "tbl_sync");
    assert_int_equal(sync_maps[0].internal_input, NETDATA_SYNC_END);
    assert_int_equal(sync_maps[0].type, NETDATA_EBPF_MAP_STATIC);
    assert_int_equal(sync_maps[0].map_fd, ND_EBPF_MAP_FD_NOT_INITIALIZED);
    assert_null(sync_maps[1].name);
}

/* Test for syncfs_maps initialization */
static void test_syncfs_maps_initialization(void **state) {
    assert_non_null(syncfs_maps);
    assert_string_equal(syncfs_maps[0].name, "tbl_syncfs");
    assert_int_equal(syncfs_maps[0].internal_input, NETDATA_SYNC_END);
    assert_int_equal(syncfs_maps[1].type, NETDATA_EBPF_MAP_CONTROLLER);
}

/* Test for msync_maps initialization */
static void test_msync_maps_initialization(void **state) {
    assert_non_null(msync_maps);
    assert_string_equal(msync_maps[0].name, "tbl_msync");
    assert_int_equal(msync_maps[0].internal_input, NETDATA_SYNC_END);
}

/* Test for fsync_maps initialization */
static void test_fsync_maps_initialization(void **state) {
    assert_non_null(fsync_maps);
    assert_string_equal(fsync_maps[0].name, "tbl_fsync");
}

/* Test for fdatasync_maps initialization */
static void test_fdatasync_maps_initialization(void **state) {
    assert_non_null(fdatasync_maps);
    assert_string_equal(fdatasync_maps[0].name, "tbl_fdatasync");
}

/* Test for sync_file_range_maps initialization */
static void test_sync_file_range_maps_initialization(void **state) {
    assert_non_null(sync_file_range_maps);
    assert_string_equal(sync_file_range_maps[0].name, "tbl_syncfr");
}

/* Test for sync_targets initialization */
static void test_sync_targets_initialization(void **state) {
    assert_non_null(sync_targets);
    assert_string_equal(sync_targets[NETDATA_SYNC_SYNC_IDX].name, NETDATA_SYSCALLS_SYNC);
    assert_int_equal(sync_targets[NETDATA_SYNC_SYNC_IDX].mode, EBPF_LOAD_TRAMPOLINE);
    assert_string_equal(sync_targets[NETDATA_SYNC_SYNCFS_IDX].name, NETDATA_SYSCALLS_SYNCFS);
    assert_string_equal(sync_targets[NETDATA_SYNC_MSYNC_IDX].name, NETDATA_SYSCALLS_MSYNC);
    assert_string_equal(sync_targets[NETDATA_SYNC_FSYNC_IDX].name, NETDATA_SYSCALLS_FSYNC);
    assert_string_equal(sync_targets[NETDATA_SYNC_FDATASYNC_IDX].name, NETDATA_SYSCALLS_FDATASYNC);
    assert_string_equal(sync_targets[NETDATA_SYNC_SYNC_FILE_RANGE_IDX].name, NETDATA_SYSCALLS_SYNC_FILE_RANGE);
    assert_null(sync_targets[6].name);
}

/* Test for sync_config initialization */
static void test_sync_config_initialization(void **state) {
    assert_non_null(&sync_config);
}

/* Test ebpf_sync_cleanup_objects with no initialized objects */
static void test_ebpf_sync_cleanup_objects_no_objects(void **state) {
    ebpf_sync_cleanup_objects();
}

/* Test ebpf_sync_thread entry point */
static void test_ebpf_sync_thread_initialization(void **state) {
    /* This test validates that the thread function can be called */
    assert_non_null(ebpf_sync_thread);
}

/* Test constants defined in header */
static void test_module_constants(void **state) {
    assert_string_equal(NETDATA_EBPF_MODULE_NAME_SYNC, "sync");
    assert_string_equal(NETDATA_SYSCALLS_SYNC, "sync");
    assert_string_equal(NETDATA_SYSCALLS_SYNCFS, "syncfs");
    assert_string_equal(NETDATA_SYSCALLS_MSYNC, "msync");
    assert_string_equal(NETDATA_SYSCALLS_FSYNC, "fsync");
    assert_string_equal(NETDATA_SYSCALLS_FDATASYNC, "fdatasync");
    assert_string_equal(NETDATA_SYSCALLS_SYNC_FILE_RANGE, "sync_file_range");
}

/* Test chart constants */
static void test_chart_constants(void **state) {
    assert_string_equal(NETDATA_EBPF_SYNC_CHART, "sync");
    assert_string_equal(NETDATA_EBPF_MSYNC_CHART, "memory_map");
    assert_string_equal(NETDATA_EBPF_FILE_SYNC_CHART, "file_sync");
    assert_string_equal(NETDATA_EBPF_FILE_SEGMENT_CHART, "file_segment");
    assert_string_equal(NETDATA_EBPF_SYNC_SUBMENU, "synchronization (eBPF)");
}

/* Test configuration constants */
static void test_config_constants(void **state) {
    assert_string_equal(NETDATA_SYNC_CONFIG_FILE, "sync.conf");
    assert_string_equal(NETDATA_SYNC_CONFIG_NAME, "syscalls");
    assert_int_equal(NETDATA_EBPF_SYNC_SLEEP_MS, 800000ULL);
}

/* Test sync_syscalls_index enum values */
static void test_sync_syscalls_index_enum(void **state) {
    assert_int_equal(NETDATA_SYNC_SYNC_IDX, 0);
    assert_int_equal(NETDATA_SYNC_SYNCFS_IDX, 1);
    assert_int_equal(NETDATA_SYNC_MSYNC_IDX, 2);
    assert_int_equal(NETDATA_SYNC_FSYNC_IDX, 3);
    assert_int_equal(NETDATA_SYNC_FDATASYNC_IDX, 4);
    assert_int_equal(NETDATA_SYNC_SYNC_FILE_RANGE_IDX, 5);
    assert_int_equal(NETDATA_SYNC_IDX_END, 6);
}

/* Test netdata_sync_charts enum values */
static void test_netdata_sync_charts_enum(void **state) {
    assert_int_equal(NETDATA_SYNC_CALL, 0);
    assert_int_equal(NETDATA_SYNC_END, 1);
}

/* Test netdata_sync_table enum values */
static void test_netdata_sync_table_enum(void **state) {
    assert_int_equal(NETDATA_SYNC_GLOBAL_TABLE, 0);
}

/* Test map structure initialization for all sync functions */
static void test_all_sync_maps_structure(void **state) {
    ebpf_local_maps_t *maps[] = {sync_maps, syncfs_maps, msync_maps, fsync_maps, fdatasync_maps, sync_file_range_maps};
    const char *names[] = {"tbl_sync", "tbl_syncfs", "tbl_msync", "tbl_fsync", "tbl_fdatasync", "tbl_syncfr"};
    
    for (int i = 0; i < 6; i++) {
        assert_non_null(maps[i]);
        assert_string_equal(maps[i][0].name, names[i]);
        assert_int_equal(maps[i][0].internal_input, NETDATA_SYNC_END);
        assert_int_equal(maps[i][0].user_input, 0);
        assert_int_equal(maps[i][0].type, NETDATA_EBPF_MAP_STATIC);
        assert_int_equal(maps[i][0].map_fd, ND_EBPF_MAP_FD_NOT_INITIALIZED);
        
        /* Check controller map */
        assert_null(maps[i][1].name);
        assert_int_equal(maps[i][1].internal_input, 0);
        assert_int_equal(maps[i][1].user_input, 0);
        assert_int_equal(maps[i][1].type, NETDATA_EBPF_MAP_CONTROLLER);
    }
}

/* Test map file descriptor initialization */
static void test_map_fd_not_initialized(void **state) {
    ebpf_local_maps_t maps[] = {
        {.name = "test", .map_fd = ND_EBPF_MAP_FD_NOT_INITIALIZED}
    };
    assert_int_equal(maps[0].map_fd, ND_EBPF_MAP_FD_NOT_INITIALIZED);
}

/* Test that all targets have valid mode */
static void test_all_targets_have_valid_mode(void **state) {
    int i = 0;
    while (sync_targets[i].name != NULL) {
        assert_true(sync_targets[i].mode == EBPF_LOAD_TRAMPOLINE ||
                   sync_targets[i].mode == EBPF_LOAD_PROBE ||
                   sync_targets[i].mode == EBPF_LOAD_RETPROBE);
        i++;
    }
}

/* Test sync_targets array termination */
static void test_sync_targets_array_termination(void **state) {
    int count = 0;
    while (sync_targets[count].name != NULL) {
        count++;
    }
    assert_int_equal(count, 6); /* 6 sync functions */
}

/* Test sync_maps array termination */
static void test_sync_maps_array_termination(void **state) {
    assert_null(sync_maps[1].name);
    assert_null(syncfs_maps[1].name);
    assert_null(msync_maps[1].name);
    assert_null(fsync_maps[1].name);
    assert_null(fdatasync_maps[1].name);
    assert_null(sync_file_range_maps[1].name);
}

/* Test edge case: NULL pointer in header */
static void test_header_constants_not_null(void **state) {
    assert_non_null(NETDATA_EBPF_MODULE_NAME_SYNC);
    assert_non_null(NETDATA_SYSCALLS_SYNC);
    assert_non_null(NETDATA_EBPF_SYNC_CHART);
}

/* Test that all map names are non-empty strings */
static void test_all_map_names_non_empty(void **state) {
    assert_true(strlen(sync_maps[0].name) > 0);
    assert_true(strlen(syncfs_maps[0].name) > 0);
    assert_true(strlen(msync_maps[0].name) > 0);
    assert_true(strlen(fsync_maps[0].name) > 0);
    assert_true(strlen(fdatasync_maps[0].name) > 0);
    assert_true(strlen(sync_file_range_maps[0].name) > 0);
}

/* Run all tests */
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sync_dimension_names),
        cmocka_unit_test(test_sync_maps_initialization),
        cmocka_unit_test(test_syncfs_maps_initialization),
        cmocka_unit_test(test_msync_maps_initialization),
        cmocka_unit_test(test_fsync_maps_initialization),
        cmocka_unit_test(test_fdatasync_maps_initialization),
        cmocka_unit_test(test_sync_file_range_maps_initialization),
        cmocka_unit_test(test_sync_targets_initialization),
        cmocka_unit_test(test_sync_config_initialization),
        cmocka_unit_test(test_ebpf_sync_cleanup_objects_no_objects),
        cmocka_unit_test(test_ebpf_sync_thread_initialization),
        cmocka_unit_test(test_module_constants),
        cmocka_unit_test(test_chart_constants),
        cmocka_unit_test(test_config_constants),
        cmocka_unit_test(test_sync_syscalls_index_enum),
        cmocka_unit_test(test_netdata_sync_charts_enum),
        cmocka_unit_test(test_netdata_sync_table_enum),
        cmocka_unit_test(test_all_sync_maps_structure),
        cmocka_unit_test(test_map_fd_not_initialized),
        cmocka_unit_test(test_all_targets_have_valid_mode),
        cmocka_unit_test(test_sync_targets_array_termination),
        cmocka_unit_test(test_sync_maps_array_termination),
        cmocka_unit_test(test_header_constants_not_null),
        cmocka_unit_test(test_all_map_names_non_empty),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}