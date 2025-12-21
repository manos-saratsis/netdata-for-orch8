```c
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "munit/munit.h"
#include "src/daemon/analytics.h"

// Mocks and test helpers
static struct rrdhost_system_info mock_system_info;

static void test_set_late_analytics_variables(void) {
    // Test setting late analytics variables
    set_late_analytics_variables(&mock_system_info);
    // Add appropriate assertions based on implementation
}

static void test_analytics_free_data(void) {
    // Initialize some dummy data
    analytics_data.netdata_config_stream_enabled = strdup("test");
    
    // Call free function
    analytics_free_data();
    
    // Assert data is properly freed
    assert(analytics_data.netdata_config_stream_enabled == NULL);
}

static void test_analytics_log_functions(void) {
    // Reset hit counters
    analytics_data.prometheus_hits = 0;
    analytics_data.shell_hits = 0;
    analytics_data.json_hits = 0;
    analytics_data.dashboard_hits = 0;
    
    // Test logging functions
    analytics_log_shell();
    assert(analytics_data.shell_hits == 1);
    
    analytics_log_json();
    assert(analytics_data.json_hits == 1);
    
    analytics_log_prometheus();
    assert(analytics_data.prometheus_hits == 1);
    
    analytics_log_dashboard();
    assert(analytics_data.dashboard_hits == 1);
}

static void test_analytics_gather_mutable_meta_data(void) {
    // Call function and check for no crashes
    analytics_gather_mutable_meta_data();
}

static void test_analytics_report_oom_score(void) {
    // Test OOM score reporting with various inputs
    analytics_report_oom_score(0);
    analytics_report_oom_score(-1000);
    analytics_report_oom_score(1000);
}

static void test_get_system_timezone(void) {
    // Call function and check for no crashes
    get_system_timezone();
}

static void test_analytics_reset(void) {
    // Initialize some dummy data
    analytics_data.prometheus_hits = 10;
    analytics_data.shell_hits = 20;
    
    // Reset analytics
    analytics_reset();
    
    // Assert hits are reset to 0
    assert(analytics_data.prometheus_hits == 0);
    assert(analytics_data.shell_hits == 0);
}

static void test_analytics_init(void) {
    // Call initialization
    analytics_init();
    
    // Add assertions based on expected initialization behavior
}

static void test_analytics_check_enabled(void) {
    // Test both true and false scenarios
    analytics_data.exporting_enabled = true;
    assert(analytics_check_enabled() == true);
    
    analytics_data.exporting_enabled = false;
    assert(analytics_check_enabled() == false);
}

// Test suite setup
static void *test_analytics_setup(const MunitParameter params[], void *user_data) {
    memset(&analytics_data, 0, sizeof(analytics_data));
    return NULL;
}

static void test_analytics_tear_down(void *fixture) {
    // Free any allocated resources
    analytics_free_data();
}

static MunitTest analytics_tests[] = {
    {
        "/set_late_analytics_variables",
        test_set_late_analytics_variables,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/analytics_free_data",
        test_analytics_free_data,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/analytics_log_functions",
        test_analytics_log_functions,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/analytics_gather_mutable_meta_data",
        test_analytics_gather_mutable_meta_data,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/analytics_report_oom_score",
        test_analytics_report_oom_score,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/get_system_timezone",
        test_get_system_timezone,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/analytics_reset",
        test_analytics_reset,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/analytics_init",
        test_analytics_init,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/analytics_check_enabled",
        test_analytics_check_enabled,
        test_analytics_setup,
        test_analytics_tear_down,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite analytics_test_suite = {
    "/analytics_tests",
    analytics_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[]) {
    return munit_suite_main(&analytics_test_suite, NULL, argc, argv);
}
```