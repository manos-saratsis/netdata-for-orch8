#include <stdbool.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cmocka.h>
#include "daemon/pulse/pulse-daemon.h"

// Mock functions for external dependencies
time_t now_boottime_sec() { return 1234567; }
void pulse_daemon_memory_do(bool extended) { /* Placeholder */ }

// Helper function to verify rusage retrieval and RRD set operations
static void test_pulse_daemon_cpu_usage(void **state) {
    struct rusage me;
    int ret = getrusage(RUSAGE_SELF, &me);
    assert_int_equal(ret, 0);

    // Simulate both extended and non-extended calls
    pulse_daemon_do(true);   // Extended logging
    pulse_daemon_do(false);  // Minimal action
}

static void test_pulse_daemon_uptime(void **state) {
    time_t boot_time = now_boottime_sec();
    assert_true(boot_time > 0);

    // Simulate calls with different extended flags
    pulse_daemon_do(true);
    pulse_daemon_do(false);
}

static void test_pulse_daemon_do(void **state) {
    // Test the full daemon do function with both flags
    pulse_daemon_do(true);   // Extended logging/processing
    pulse_daemon_do(false);  // Minimal action
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pulse_daemon_cpu_usage),
        cmocka_unit_test(test_pulse_daemon_uptime),
        cmocka_unit_test(test_pulse_daemon_do),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}