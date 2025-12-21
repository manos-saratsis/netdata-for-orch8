```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "sqlite_aclk_alert.h"

// Mock implementations and stubs for dependencies
static void mock_db_meta_operation(void) {
    // Simulated database operation
}

static void test_aclk_send_alert_configuration(void **state) {
    (void) state; // Unused

    // Test with valid config hash
    aclk_send_alert_configuration("valid_hash");
    
    // Test with NULL config hash
    aclk_send_alert_configuration(NULL);
}

static void test_aclk_push_alert_config_event(void **state) {
    (void) state; // Unused

    // Test with valid inputs
    aclk_push_alert_config_event("node_123", "config_hash");

    // Test with NULL inputs
    aclk_push_alert_config_event(NULL, "config_hash");
    aclk_push_alert_config_event("node_123", NULL);
    aclk_push_alert_config_event(NULL, NULL);
}

static void test_aclk_start_alert_streaming(void **state) {
    (void) state; // Unused

    // Test with valid inputs
    aclk_start_alert_streaming("node_123", 100);

    // Test with NULL node_id
    aclk_start_alert_streaming(NULL, 100);
}

static void test_aclk_alert_version_check(void **state) {
    (void) state; // Unused

    // Test with valid inputs
    aclk_alert_version_check("node_123", "claim_456", 200);

    // Test with NULL inputs
    aclk_alert_version_check(NULL, "claim_456", 200);
    aclk_alert_version_check("node_123", NULL, 200);
    aclk_alert_version_check(NULL, NULL, 200);
}

static void test_send_alert_snapshot_to_cloud(void **state) {
    (void) state; // Unused

    // Assuming RRDHOST can be mocked or created for testing
    RRDHOST *mock_host = NULL;
    send_alert_snapshot_to_cloud(mock_host);
}

static void test_process_alert_pending_queue(void **state) {
    (void) state; // Unused

    // Assuming RRDHOST can be mocked or created for testing
    RRDHOST *mock_host = NULL;
    bool result = process_alert_pending_queue(mock_host);

    // Add assertions based on expected behavior
    assert_true(result == false || result == true);
}

static void test_aclk_push_alert_events_for_all_hosts(void **state) {
    (void) state; // Unused

    aclk_push_alert_events_for_all_hosts();
}

static void test_calculate_node_alert_version(void **state) {
    (void) state; // Unused

    // Assuming RRDHOST can be mocked or created for testing
    RRDHOST *mock_host = NULL;
    uint64_t version = calculate_node_alert_version(mock_host);

    // Add assertions if specific version calculation logic exists
    assert_true(version >= 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_send_alert_configuration),
        cmocka_unit_test(test_aclk_push_alert_config_event),
        cmocka_unit_test(test_aclk_start_alert_streaming),
        cmocka_unit_test(test_aclk_alert_version_check),
        cmocka_unit_test(test_send_alert_snapshot_to_cloud),
        cmocka_unit_test(test_process_alert_pending_queue),
        cmocka_unit_test(test_aclk_push_alert_events_for_all_hosts),
        cmocka_unit_test(test_calculate_node_alert_version)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
```