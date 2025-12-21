```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "aclk.h"

// Mock functions for external dependencies
extern void mqtt_wss_destroy(mqtt_wss_client client) {}
extern void free_topic_cache() {}
extern void aclk_env_t_destroy(aclk_env_t *env) {}

// Test function to validate aclk_online functionality
static void test_aclk_online(void **state) {
    (void) state; // unused

    // Test initial offline state
    assert_false(aclk_online());

    // Test setting connection state
    __atomic_store_n(&aclk_connected, true, __ATOMIC_RELAXED);
    assert_true(aclk_online());

    // Test resetting connection state
    __atomic_store_n(&aclk_connected, false, __ATOMIC_RELAXED);
    assert_false(aclk_online());
}

// Test function to validate aclk status conversion
static void test_aclk_status_to_string(void **state) {
    (void) state; // unused

    // Test various status codes
    assert_string_equal(aclk_status_to_string(ACLK_STATUS_CONNECTED), "connected");
    assert_string_equal(aclk_status_to_string(ACLK_STATUS_OFFLINE), "offline");
    assert_string_equal(aclk_status_to_string(ACLK_STATUS_DISABLED), "disabled");
}

// Test function to validate aclk status setting
static void test_aclk_status_set(void **state) {
    (void) state; // unused

    // Test setting different statuses
    aclk_status_set(ACLK_STATUS_CONNECTED);
    assert_int_equal(aclk_status, ACLK_STATUS_CONNECTED);

    aclk_status_set(ACLK_STATUS_OFFLINE);
    assert_int_equal(aclk_status, ACLK_STATUS_OFFLINE);
}

// Test function to validate add_aclk_host_labels
static void test_add_aclk_host_labels(void **state) {
    (void) state; // unused

    // This requires more complex setup with mocking rrdlabels
    // For now, just ensure it doesn't crash
    add_aclk_host_labels();
}

// Test function to validate context-based online status
static void test_aclk_online_contexts(void **state) {
    (void) state; // unused

    // Mocking the aclk_online and aclk_query_scope_has functions
    __atomic_store_n(&aclk_connected, true, __ATOMIC_RELAXED);

    // Test metrics context
    extern bool aclk_query_scope_has(int query_type);
    will_return(__wrap_aclk_query_scope_has, true);
    assert_true(aclk_online_for_contexts());

    will_return(__wrap_aclk_query_scope_has, false);
    assert_false(aclk_online_for_contexts());

    // Test alerts context
    will_return(__wrap_aclk_query_scope_has, true);
    assert_true(aclk_online_for_alerts());

    will_return(__wrap_aclk_query_scope_has, false);
    assert_false(aclk_online_for_alerts());

    // Test nodes context
    will_return(__wrap_aclk_query_scope_has, true);
    assert_true(aclk_online_for_nodes());

    will_return(__wrap_aclk_query_scope_has, false);
    assert_false(aclk_online_for_nodes());
}

// Mocking the aclk_query_scope_has function for testing
bool __wrap_aclk_query_scope_has(int query_type) {
    function_called();
    return mock_type(bool);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_online),
        cmocka_unit_test(test_aclk_status_to_string),
        cmocka_unit_test(test_aclk_status_set),
        cmocka_unit_test(test_add_aclk_host_labels),
        cmocka_unit_test(test_aclk_online_contexts),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
```