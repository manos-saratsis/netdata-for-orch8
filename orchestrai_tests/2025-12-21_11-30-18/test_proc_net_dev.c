#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock implementations for dependencies
extern void __wrap_collector_error(const char *fmt, ...);
extern void __wrap_collector_info(const char *fmt, ...);

// Mocked function for file reading
int __wrap_read_txt_file(const char *filename, char *buf, size_t buf_size) {
    // Simulate various file reading scenarios
    check_expected(filename);
    function_called();
    return mock_type(int);
}

int __wrap_read_single_number_file(const char *filename, unsigned long long *value) {
    check_expected(filename);
    function_called();
    if (mock_type(int) == 0) {
        *value = mock_type(unsigned long long);
        return 0;
    }
    return -1;
}

// Test get_duplex_string function
static void test_get_duplex_string(void **state) {
    (void) state;

    assert_string_equal(get_duplex_string(NETDEV_DUPLEX_FULL), "full");
    assert_string_equal(get_duplex_string(NETDEV_DUPLEX_HALF), "half");
    assert_string_equal(get_duplex_string(-1), "unknown");
}

// Test get_operstate function
static void test_get_operstate(void **state) {
    (void) state;

    assert_int_equal(get_operstate("up"), NETDEV_OPERSTATE_UP);
    assert_int_equal(get_operstate("down"), NETDEV_OPERSTATE_DOWN);
    assert_int_equal(get_operstate("notpresent"), NETDEV_OPERSTATE_NOTPRESENT);
    assert_int_equal(get_operstate("lowerlayerdown"), NETDEV_OPERSTATE_LOWERLAYERDOWN);
    assert_int_equal(get_operstate("testing"), NETDEV_OPERSTATE_TESTING);
    assert_int_equal(get_operstate("dormant"), NETDEV_OPERSTATE_DORMANT);
    assert_int_equal(get_operstate("unknown"), NETDEV_OPERSTATE_UNKNOWN);
}

// Test get_operstate_string function
static void test_get_operstate_string(void **state) {
    (void) state;

    assert_string_equal(get_operstate_string(NETDEV_OPERSTATE_UP), "up");
    assert_string_equal(get_operstate_string(NETDEV_OPERSTATE_DOWN), "down");
    assert_string_equal(get_operstate_string(NETDEV_OPERSTATE_NOTPRESENT), "notpresent");
    assert_string_equal(get_operstate_string(NETDEV_OPERSTATE_LOWERLAYERDOWN), "lowerlayerdown");
    assert_string_equal(get_operstate_string(NETDEV_OPERSTATE_TESTING), "testing");
    assert_string_equal(get_operstate_string(NETDEV_OPERSTATE_DORMANT), "dormant");
    assert_string_equal(get_operstate_string(-1), "unknown");
}

// Test interface detection scenarios
static void test_get_netdev_interface_detection(void **state) {
    (void) state;

    // Setup mocks for configuration
    will_return(__wrap_read_txt_file, 0);
    expect_string(__wrap_read_txt_file, filename, "/sys/class/net/eth0/operstate");
    will_return(__wrap_read_txt_file, "up");

    // Simulate network interface configuration
    struct netdev *dev = get_netdev("eth0");
    assert_non_null(dev);
    assert_string_equal(dev->name, "eth0");
    assert_true(dev->configured);
}

// Test device renames
static void test_netdev_rename(void **state) {
    (void) state;

    struct netdev *dev = get_netdev("eth0");
    assert_non_null(dev);

    // Simulate a rename task
    struct rename_task rename = {
        .container_device = "docker0",
        .container_name = "test_container",
        .ctx_prefix = "cgroup.",
        .cgroup_netdev_link = NULL,
        .chart_labels = NULL
    };

    netdev_rename_unsafe(dev, &rename);

    // Verify rename
    assert_true(dev->flipped);
    assert_string_equal(dev->chart_type_net_bytes, "cgroup_test_container");
}

// Test speed file reading
static void test_speed_file_reading(void **state) {
    (void) state;

    // Setup network device
    struct netdev *dev = get_netdev("eth0");
    assert_non_null(dev);

    // Mock speed file reading
    expect_string(__wrap_read_single_number_file, filename, "/sys/class/net/eth0/speed");
    will_return(__wrap_read_single_number_file, 0);
    will_return(__wrap_read_single_number_file, 1000); // 1000 Mbps

    // Simulate reading speed
    unsigned long long speed = 0;
    int result = read_single_number_file("/sys/class/net/eth0/speed", &speed);

    assert_int_equal(result, 0);
    assert_int_equal(speed, 1000);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_get_duplex_string),
        cmocka_unit_test(test_get_operstate),
        cmocka_unit_test(test_get_operstate_string),
        cmocka_unit_test(test_get_netdev_interface_detection),
        cmocka_unit_test(test_netdev_rename),
        cmocka_unit_test(test_speed_file_reading),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}