```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <json-c/json.h>
#include "aclk.h"

// Test for aclk_state function
static void test_aclk_state(void **state) {
    (void) state; // unused

    // Temporarily modify global state for testing
    time_t original_last_conn_time_mqtt = last_conn_time_mqtt;
    time_t original_last_conn_time_appl = last_conn_time_appl;
    time_t original_last_disconnect_time = last_disconnect_time;

    last_conn_time_mqtt = time(NULL);
    last_conn_time_appl = time(NULL);
    last_disconnect_time = time(NULL);

    // Call the function and validate the output
    char *aclk_state_str = aclk_state();
    assert_non_null(aclk_state_str);

    // Perform some basic checks on the returned string
    assert_true(strstr(aclk_state_str, "ACLK Available: Yes") != NULL);
    assert_true(strstr(aclk_state_str, "ACLK Version: 2") != NULL);

    // Free the dynamically allocated string
    free(aclk_state_str);

    // Restore original values
    last_conn_time_mqtt = original_last_conn_time_mqtt;
    last_conn_time_appl = original_last_conn_time_appl;
    last_disconnect_time = original_last_disconnect_time;
}

// Test for aclk_state_json function
static void test_aclk_state_json(void **state) {
    (void) state; // unused

    // Temporarily modify global state for testing
    time_t original_last_conn_time_mqtt = last_conn_time_mqtt;
    time_t original_last_conn_time_appl = last_conn_time_appl;
    time_t original_last_disconnect_time = last_disconnect_time;

    last_conn_time_mqtt = time(NULL);
    last_conn_time_appl = time(NULL);
    last_disconnect_time = time(NULL);

    // Call the function and validate the output
    char *aclk_state_json_str = aclk_state_json();
    assert_non_null(aclk_state_json_str);

    // Parse JSON to validate structure
    json_object *parsed_json = json_tokener_parse(aclk_state_json_str);
    assert_non_null(parsed_json);

    // Validate specific JSON keys
    json_object *tmp;
    assert_true(json_object_object_get_ex(parsed_json, "aclk-version", &tmp));
    assert_int_equal(json_object_get_int(tmp), 2);

    assert_true(json_object_object_get_ex(parsed_json, "mqtt-version", &tmp));
    assert_int_equal(json_object_get_int(tmp), 5);

    // Free the JSON object and dynamically allocated string
    json_object_put(parsed_json);
    free(aclk_state_json_str);

    // Restore original values
    last_conn_time_mqtt = original_last_conn_time_mqtt;
    last_conn_time_appl = original_last_conn_time_appl;
    last_disconnect_time = original_last_disconnect_time;
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_state),
        cmocka_unit_test(test_aclk_state_json),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
```