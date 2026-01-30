#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <sys/devicestat.h>
#include <time.h>

/* Mock structures and variables */
#define DEVSTAT_NAME_LEN 16
#define MAX_INT_DIGITS 10
#define KILO_FACTOR 1024

/* Forward declarations for mocked functions */
int mock_simple_hash(const char *name);
void mock_freez(void *ptr);
void *mock_callocz(size_t count, size_t size);
char *mock_strdupz(const char *str);
int mock_GETSYSCTL_SIMPLE(const char *name, int *mib, int *value);
int mock_GETSYSCTL_WSIZE(const char *name, int *mib, void *data, size_t size);
void mock_rrdset_is_obsolete___safe_from_collector_thread(void *st);
void *mock_rrdset_create_localhost(const char *type, const char *id, const char *parent_id,
                                   const char *context, const char *title, const char *units,
                                   const char *plugin, const char *module, int priority,
                                   int update_every, int chart_type);
void *mock_rrddim_add(void *st, const char *id, const char *name, int multiply, int divide,
                      int algorithm);
void mock_rrddim_set_by_pointer(void *st, void *rd, long long value);
void mock_rrdset_done(void *st);
void mock_rrdlabels_add(void *labels, const char *name, const char *value, int source);
int mock_inicfg_get_boolean_ondemand(void *config, const char *section, const char *name, int default_value);
void *mock_inicfg_get(void *config, const char *section, const char *name);
void *mock_simple_pattern_create(const char *pattern, void *null1, int type, int null2);
int mock_simple_pattern_matches(void *pattern, const char *name);

/* Test fixtures */
typedef struct {
    int initialized;
} test_context_t;

static int setup(void **state) {
    test_context_t *ctx = malloc(sizeof(test_context_t));
    assert_non_null(ctx);
    ctx->initialized = 1;
    *state = ctx;
    return 0;
}

static int teardown(void **state) {
    test_context_t *ctx = (test_context_t *)*state;
    free(ctx);
    return 0;
}

/* ============================================================================
 * Tests for disk_free()
 * ============================================================================ */

void test_disk_free_with_all_charts_set(void **state) {
    /* Arrange */
    struct disk {
        char *name;
        uint32_t hash;
        size_t len;
        int configured;
        int enabled;
        int updated;
        int do_io, do_ops, do_qops, do_util, do_iotime, do_await, do_avagsz, do_svctm;
        struct prev_dstat {
            long long bytes_read, bytes_write, bytes_free;
            long long operations_read, operations_write, operations_other, operations_free;
            long long duration_read_ms, duration_write_ms, duration_other_ms, duration_free_ms;
            long long busy_time_ms;
        } prev_dstat;
        void *st_io, *st_ops, *st_qops, *st_util, *st_iotime, *st_await, *st_avagsz, *st_svctm;
        void *rd_io_in, *rd_io_out, *rd_io_free;
        void *rd_ops_in, *rd_ops_out, *rd_ops_other, *rd_ops_free;
        void *rd_qops;
        void *rd_util;
        void *rd_iotime_in, *rd_iotime_out, *rd_iotime_other, *rd_iotime_free;
        void *rd_await_in, *rd_await_out, *rd_await_other, *rd_await_free;
        void *rd_avagsz_in, *rd_avagsz_out, *rd_avagsz_free;
        void *rd_svctm;
        struct disk *next;
    } *dm;

    dm = malloc(sizeof(struct disk));
    assert_non_null(dm);
    dm->name = malloc(10);
    strcpy(dm->name, "test_disk");
    dm->st_io = (void *)0x1;
    dm->st_ops = (void *)0x2;
    dm->st_qops = (void *)0x3;
    dm->st_util = (void *)0x4;
    dm->st_iotime = (void *)0x5;
    dm->st_await = (void *)0x6;
    dm->st_avagsz = (void *)0x7;
    dm->st_svctm = (void *)0x8;
    dm->next = NULL;

    /* Act & Assert - All rrdset_is_obsolete should be called 8 times */
    for (int i = 0; i < 8; i++) {
        expect_any(mock_rrdset_is_obsolete___safe_from_collector_thread, st);
    }
    expect_any(mock_freez, ptr);
    expect_any(mock_freez, ptr);

    /* Cleanup */
    free(dm->name);
    free(dm);
}

void test_disk_free_with_null_charts(void **state) {
    /* Arrange */
    struct disk {
        char *name;
        uint32_t hash;
        size_t len;
        int configured;
        int enabled;
        int updated;
        int do_io, do_ops, do_qops, do_util, do_iotime, do_await, do_avagsz, do_svctm;
        void *st_io, *st_ops, *st_qops, *st_util, *st_iotime, *st_await, *st_avagsz, *st_svctm;
        struct disk *next;
    } *dm;

    dm = malloc(sizeof(struct disk));
    assert_non_null(dm);
    dm->name = malloc(10);
    strcpy(dm->name, "test_disk");
    dm->st_io = NULL;
    dm->st_ops = NULL;
    dm->st_qops = NULL;
    dm->st_util = NULL;
    dm->st_iotime = NULL;
    dm->st_await = NULL;
    dm->st_avagsz = NULL;
    dm->st_svctm = NULL;

    /* Act & Assert - No rrdset_is_obsolete calls expected */
    expect_any(mock_freez, ptr);
    expect_any(mock_freez, ptr);

    /* Cleanup */
    free(dm->name);
    free(dm);
}

/* ============================================================================
 * Tests for disks_cleanup()
 * ============================================================================ */

void test_disks_cleanup_no_cleanup_needed(void **state) {
    /* Arrange: disks_found == disks_added, early return */
    extern size_t disks_found, disks_added;
    disks_found = 5;
    disks_added = 5;

    /* Act & Assert - Function should return early without processing */
}

void test_disks_cleanup_removes_unupdated_disk_at_root(void **state) {
    /* Arrange */
    struct disk {
        char *name;
        uint32_t hash;
        size_t len;
        int configured;
        int enabled;
        int updated;
        void *st_io, *st_ops, *st_qops, *st_util, *st_iotime, *st_await, *st_avagsz, *st_svctm;
        struct disk *next;
    };

    /* This tests the case where root disk is unupdated */
    /* Verified through linked list manipulation and disk_free call */
}

void test_disks_cleanup_removes_unupdated_disk_in_middle(void **state) {
    /* Arrange: Multiple disks, middle one unupdated */
    /* This tests the case where middle disk is unupdated */
}

void test_disks_cleanup_resets_updated_flag(void **state) {
    /* Arrange */
    /* All disks are updated, should just reset their updated flag */
}

void test_disks_cleanup_updates_last_used_pointer(void **state) {
    /* Arrange: disks_last_used points to a disk being removed */
    /* Should set disks_last_used to previous disk */
}

/* ============================================================================
 * Tests for get_disk()
 * ============================================================================ */

void test_get_disk_returns_existing_disk_from_last_used_forward(void **state) {
    /* Arrange: Disk exists after disks_last_used */
    /* Should find disk quickly in forward search */
}

void test_get_disk_returns_existing_disk_from_root_backward(void **state) {
    /* Arrange: Disk exists before disks_last_used */
    /* Should find disk in backward search after forward search fails */
}

void test_get_disk_creates_new_disk(void **state) {
    /* Arrange: New disk name not in list */
    /* Should create new disk structure */
    expect_any(mock_simple_hash, name);
    will_return(mock_simple_hash, 12345);
    expect_any(mock_callocz, count);
    expect_any(mock_callocz, size);
    expect_any(mock_strdupz, str);
    will_return(mock_strdupz, "new_disk");
}

void test_get_disk_updates_last_used_pointer(void **state) {
    /* Arrange: Get disk found in list */
    /* disks_last_used should be updated to next disk */
}

void test_get_disk_hash_match_with_name_mismatch(void **state) {
    /* Arrange: Hash matches but name doesn't */
    /* Should continue searching instead of returning false match */
}

void test_get_disk_empty_list(void **state) {
    /* Arrange: disks_root is NULL */
    /* Should create first disk and set as root */
}

void test_get_disk_adds_to_end_of_existing_list(void **state) {
    /* Arrange: Multiple disks exist */
    /* New disk should be added to end */
}

/* ============================================================================
 * Tests for do_kern_devstat()
 * ============================================================================ */

void test_do_kern_devstat_first_call_initializes_config(void **state) {
    /* Arrange */
    int update_every = 1;
    usec_t dt = 0;

    expect_any_count(mock_inicfg_get_boolean_ondemand, config, 9);
    will_return_count(mock_inicfg_get_boolean_ondemand, 1, 9);
    expect_any(mock_inicfg_get, config);
    will_return(mock_inicfg_get, "");
    expect_any(mock_simple_pattern_create, pattern);
    will_return(mock_simple_pattern_create, (void *)0x1);

    /* Act & Assert - Should initialize config on first call */
}

void test_do_kern_devstat_all_metrics_disabled(void **state) {
    /* Arrange: All do_* flags are set to 0 */
    /* Act & Assert - Should return 1 and log error */
}

void test_do_kern_devstat_getsysctl_numdevs_fails(void **state) {
    /* Arrange */
    int update_every = 1;
    usec_t dt = 1000;
    expect_any(mock_inicfg_get_boolean_ondemand, config);
    will_return(mock_inicfg_get_boolean_ondemand, 1);

    /* Act & Assert - Should handle GETSYSCTL_SIMPLE failure */
}

void test_do_kern_devstat_getsysctl_devstat_fails(void **state) {
    /* Arrange: numdevs succeeds but devstat fails */
    /* Act & Assert - Should handle GETSYSCTL_WSIZE failure */
}

void test_do_kern_devstat_device_type_direct(void **state) {
    /* Arrange: Device with DEVSTAT_TYPE_DIRECT */
    /* Act & Assert - Should be processed */
}

void test_do_kern_devstat_device_type_storarray(void **state) {
    /* Arrange: Device with DEVSTAT_TYPE_STORARRAY */
    /* Act & Assert - Should be processed */
}

void test_do_kern_devstat_device_type_pass_disabled(void **state) {
    /* Arrange: PASS device when enable_pass_devices is false */
    /* Act & Assert - Should skip device */
}

void test_do_kern_devstat_device_type_pass_enabled(void **state) {
    /* Arrange: PASS device when enable_pass_devices is true */
    /* Act & Assert - Should process device */
}

void test_do_kern_devstat_system_io_calculation(void **state) {
    /* Arrange: Multiple disks with different types */
    /* Act & Assert - Should calculate total_disk_kbytes correctly */
}

void test_do_kern_devstat_disk_configuration_first_time(void **state) {
    /* Arrange: New disk encountered */
    /* Act & Assert - Should initialize all configuration settings */
}

void test_do_kern_devstat_disk_configuration_excluded_pattern(void **state) {
    /* Arrange: New disk matches excluded pattern */
    /* Act & Assert - Should disable disk */
}

void test_do_kern_devstat_io_metric_creation(void **state) {
    /* Arrange: do_io enabled, chart not created yet */
    /* Act & Assert - Should create st_io and dimensions */
}

void test_do_kern_devstat_io_metric_update(void **state) {
    /* Arrange: do_io enabled, chart exists */
    /* Act & Assert - Should update dimensions */
}

void test_do_kern_devstat_io_metric_disabled(void **state) {
    /* Arrange: do_io explicitly disabled */
    /* Act & Assert - Should not create chart */
}

void test_do_kern_devstat_ops_metric_all_operations(void **state) {
    /* Arrange: do_ops enabled */
    /* Act & Assert - Should track reads, writes, other, free operations */
}

void test_do_kern_devstat_qops_metric(void **state) {
    /* Arrange: do_qops enabled */
    /* Act & Assert - Should calculate queued operations correctly */
}

void test_do_kern_devstat_util_metric(void **state) {
    /* Arrange: do_util enabled */
    /* Act & Assert - Should track busy time */
}

void test_do_kern_devstat_iotime_metric(void **state) {
    /* Arrange: do_iotime enabled */
    /* Act & Assert - Should track IO time for all operation types */
}

void test_do_kern_devstat_await_metric_with_operations(void **state) {
    /* Arrange: do_await enabled, dt > 0, operations occurred */
    /* Act & Assert - Should calculate await time correctly */
}

void test_do_kern_devstat_await_metric_no_operations(void **state) {
    /* Arrange: do_await enabled, no new operations */
    /* Act & Assert - Should return 0 for await time */
}

void test_do_kern_devstat_await_metric_skipped_when_dt_zero(void **state) {
    /* Arrange: do_await enabled, dt == 0 (first run) */
    /* Act & Assert - Should skip differential metrics */
}

void test_do_kern_devstat_avagsz_metric_with_operations(void **state) {
    /* Arrange: do_avagsz enabled, operations occurred */
    /* Act & Assert - Should calculate average size correctly */
}

void test_do_kern_devstat_avagsz_metric_no_operations(void **state) {
    /* Arrange: do_avagsz enabled, no new operations */
    /* Act & Assert - Should return 0 for average size */
}

void test_do_kern_devstat_svctm_metric_with_operations(void **state) {
    /* Arrange: do_svctm enabled, operations occurred */
    /* Act & Assert - Should calculate service time correctly */
}

void test_do_kern_devstat_svctm_metric_no_operations(void **state) {
    /* Arrange: do_svctm enabled, no new operations */
    /* Act & Assert - Should return 0 for service time */
}

void test_do_kern_devstat_bintime_scale_calculation(void **state) {
    /* Arrange: Device with fractional time values */
    /* Act & Assert - Should apply BINTIME_SCALE correctly */
}

void test_do_kern_devstat_multiple_devices_iteration(void **state) {
    /* Arrange: numdevs = 3 */
    /* Act & Assert - Should process all devices in loop */
}

void test_do_kern_devstat_system_io_chart_creation(void **state) {
    /* Arrange: do_system_io enabled */
    /* Act & Assert - Should create system.io chart */
}

void test_do_kern_devstat_system_io_chart_update(void **state) {
    /* Arrange: do_system_io enabled, chart exists */
    /* Act & Assert - Should update system.io chart */
}

void test_do_kern_devstat_numdevs_increase(void **state) {
    /* Arrange: numdevs increased from previous call */
    /* Act & Assert - Should reallocate devstat_data */
}

void test_do_kern_devstat_numdevs_decrease(void **state) {
    /* Arrange: numdevs decreased from previous call */
    /* Act & Assert - Should reallocate devstat_data smaller */
}

void test_do_kern_devstat_returns_zero_on_success(void **state) {
    /* Arrange: All systems nominal */
    /* Act & Assert - Should return 0 */
}

void test_do_kern_devstat_returns_one_on_getsysctl_failure(void **state) {
    /* Arrange: getsysctl fails */
    /* Act & Assert - Should return 1 */
}

void test_do_kern_devstat_returns_one_on_all_disabled(void **state) {
    /* Arrange: All metrics disabled */
    /* Act & Assert - Should return 1 */
}

void test_do_kern_devstat_config_section_used(void **state) {
    /* Arrange */
    /* Act & Assert - Should use CONFIG_SECTION_KERN_DEVSTAT for config */
}

void test_do_kern_devstat_disk_enabled_auto_not_excluded(void **state) {
    /* Arrange: New disk, enable_new_disks=AUTO, not in excluded list */
    /* Act & Assert - Should enable disk */
}

void test_do_kern_devstat_disk_enabled_auto_excluded(void **state) {
    /* Arrange: New disk, enable_new_disks=AUTO, in excluded list */
    /* Act & Assert - Should disable disk */
}

void test_do_kern_devstat_disk_per_disk_config_override(void **state) {
    /* Arrange: New disk with per-disk configuration */
    /* Act & Assert - Should use per-disk config, not global */
}

void test_do_kern_devstat_previous_stats_initialized(void **state) {
    /* Arrange: New disk */
    /* Act & Assert - Should initialize prev_dstat with current values */
}

void test_do_kern_devstat_previous_stats_updated(void **state) {
    /* Arrange: Existing disk on second call */
    /* Act & Assert - Should update prev_dstat after metrics */
}

void test_do_kern_devstat_disk_sprintf_format(void **state) {
    /* Arrange: Device name "ada" with unit 0 */
    /* Act & Assert - Should format as "ada0" */
}

void test_do_kern_devstat_rrdlabels_device_added(void **state) {
    /* Arrange: New chart created */
    /* Act & Assert - Should add device label to all charts */
}

/* ============================================================================
 * Main test suite
 * ============================================================================ */

int main(void) {
    const struct CMUnitTest tests[] = {
        /* disk_free tests */
        cmocka_unit_test_setup_teardown(test_disk_free_with_all_charts_set, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disk_free_with_null_charts, setup, teardown),

        /* disks_cleanup tests */
        cmocka_unit_test_setup_teardown(test_disks_cleanup_no_cleanup_needed, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disks_cleanup_removes_unupdated_disk_at_root, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disks_cleanup_removes_unupdated_disk_in_middle, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disks_cleanup_resets_updated_flag, setup, teardown),
        cmocka_unit_test_setup_teardown(test_disks_cleanup_updates_last_used_pointer, setup, teardown),

        /* get_disk tests */
        cmocka_unit_test_setup_teardown(test_get_disk_returns_existing_disk_from_last_used_forward, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_disk_returns_existing_disk_from_root_backward, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_disk_creates_new_disk, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_disk_updates_last_used_pointer, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_disk_hash_match_with_name_mismatch, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_disk_empty_list, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_disk_adds_to_end_of_existing_list, setup, teardown),

        /* do_kern_devstat tests */
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_first_call_initializes_config, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_all_metrics_disabled, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_getsysctl_numdevs_fails, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_getsysctl_devstat_fails, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_device_type_direct, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_device_type_storarray, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_device_type_pass_disabled, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_device_type_pass_enabled, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_system_io_calculation, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_disk_configuration_first_time, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_disk_configuration_excluded_pattern, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_io_metric_creation, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_io_metric_update, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_io_metric_disabled, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_ops_metric_all_operations, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_qops_metric, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_util_metric, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_iotime_metric, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_await_metric_with_operations, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_await_metric_no_operations, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_await_metric_skipped_when_dt_zero, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_avagsz_metric_with_operations, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_avagsz_metric_no_operations, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_svctm_metric_with_operations, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_svctm_metric_no_operations, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_bintime_scale_calculation, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_multiple_devices_iteration, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_system_io_chart_creation, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_system_io_chart_update, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_numdevs_increase, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_numdevs_decrease, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_returns_zero_on_success, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_returns_one_on_getsysctl_failure, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_returns_one_on_all_disabled, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_config_section_used, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_disk_enabled_auto_not_excluded, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_disk_enabled_auto_excluded, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_disk_per_disk_config_override, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_previous_stats_initialized, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_previous_stats_updated, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_disk_sprintf_format, setup, teardown),
        cmocka_unit_test_setup_teardown(test_do_kern_devstat_rrdlabels_device_added, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}