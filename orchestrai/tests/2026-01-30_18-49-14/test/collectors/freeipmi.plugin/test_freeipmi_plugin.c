#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <setjmp.h>
#include <cmocka.h>
#include <assert.h>

/* Mock structures for IPMI monitoring library */
typedef void* ipmi_monitoring_ctx_t;

/* Global variables for testing */
static jmp_buf test_jump;
static int mock_return_value = 0;
static char mock_error_msg[256];

/* Test fixtures */
struct test_fixture {
    char key[256];
    struct sensor {
        int sensor_type;
        int sensor_state;
        int sensor_units;
        char *sensor_name;
        int sensor_reading_type;
        union {
            uint8_t bool_value;
            uint32_t uint32_value;
            double double_value;
        } sensor_reading;
        const char *context;
        const char *title;
        const char *units;
        const char *family;
        const char *chart_type;
        const char *dimension;
        int priority;
        const char *type;
        const char *component;
        int multiplier;
        bool do_metric;
        bool do_state;
        bool metric_chart_sent;
        bool state_chart_sent;
        uint64_t last_collected_metric_ut;
        uint64_t last_collected_state_ut;
    } sensor_data;
};

/* ============================================
   Test: netdata_host_is_localhost()
   ============================================ */

void test_netdata_host_is_localhost_localhost_string(void **state) {
    assert_int_equal(netdata_host_is_localhost("localhost"), 1);
}

void test_netdata_host_is_localhost_127_0_0_1(void **state) {
    assert_int_equal(netdata_host_is_localhost("127.0.0.1"), 1);
}

void test_netdata_host_is_localhost_ipv6_localhost(void **state) {
    assert_int_equal(netdata_host_is_localhost("ipv6-localhost"), 1);
}

void test_netdata_host_is_localhost_ipv6_shorthand(void **state) {
    assert_int_equal(netdata_host_is_localhost("::1"), 1);
}

void test_netdata_host_is_localhost_ip6_localhost(void **state) {
    assert_int_equal(netdata_host_is_localhost("ip6-localhost"), 1);
}

void test_netdata_host_is_localhost_ipv6_full(void **state) {
    assert_int_equal(netdata_host_is_localhost("0:0:0:0:0:0:0:1"), 1);
}

void test_netdata_host_is_localhost_remote_host(void **state) {
    assert_int_equal(netdata_host_is_localhost("192.168.1.1"), 0);
}

void test_netdata_host_is_localhost_hostname(void **state) {
    assert_int_equal(netdata_host_is_localhost("example.com"), 0);
}

void test_netdata_host_is_localhost_empty_string(void **state) {
    assert_int_equal(netdata_host_is_localhost(""), 0);
}

void test_netdata_host_is_localhost_case_insensitive_localhost(void **state) {
    assert_int_equal(netdata_host_is_localhost("LOCALHOST"), 1);
}

void test_netdata_host_is_localhost_case_insensitive_ipv6(void **state) {
    assert_int_equal(netdata_host_is_localhost("IPV6-LOCALHOST"), 1);
}

/* ============================================
   Test: netdata_parse_outofband_driver_type()
   ============================================ */

void test_netdata_parse_outofband_driver_type_lan(void **state) {
    int result = netdata_parse_outofband_driver_type("lan");
    assert_int_equal(result, IPMI_MONITORING_PROTOCOL_VERSION_1_5);
}

void test_netdata_parse_outofband_driver_type_lan_2_0(void **state) {
    int result = netdata_parse_outofband_driver_type("lan_2_0");
    assert_int_equal(result, IPMI_MONITORING_PROTOCOL_VERSION_2_0);
}

void test_netdata_parse_outofband_driver_type_lan20(void **state) {
    int result = netdata_parse_outofband_driver_type("lan20");
    assert_int_equal(result, IPMI_MONITORING_PROTOCOL_VERSION_2_0);
}

void test_netdata_parse_outofband_driver_type_lan_20(void **state) {
    int result = netdata_parse_outofband_driver_type("lan_20");
    assert_int_equal(result, IPMI_MONITORING_PROTOCOL_VERSION_2_0);
}

void test_netdata_parse_outofband_driver_type_lan2_0(void **state) {
    int result = netdata_parse_outofband_driver_type("lan2_0");
    assert_int_equal(result, IPMI_MONITORING_PROTOCOL_VERSION_2_0);
}

void test_netdata_parse_outofband_driver_type_lanplus(void **state) {
    int result = netdata_parse_outofband_driver_type("lanplus");
    assert_int_equal(result, IPMI_MONITORING_PROTOCOL_VERSION_2_0);
}

void test_netdata_parse_outofband_driver_type_invalid(void **state) {
    int result = netdata_parse_outofband_driver_type("invalid");
    assert_int_equal(result, -1);
}

void test_netdata_parse_outofband_driver_type_case_insensitive(void **state) {
    int result = netdata_parse_outofband_driver_type("LAN");
    assert_int_equal(result, IPMI_MONITORING_PROTOCOL_VERSION_1_5);
}

/* ============================================
   Test: netdata_parse_inband_driver_type()
   ============================================ */

void test_netdata_parse_inband_driver_type_kcs(void **state) {
    int result = netdata_parse_inband_driver_type("kcs");
    assert_int_equal(result, IPMI_MONITORING_DRIVER_TYPE_KCS);
}

void test_netdata_parse_inband_driver_type_ssif(void **state) {
    int result = netdata_parse_inband_driver_type("ssif");
    assert_int_equal(result, IPMI_MONITORING_DRIVER_TYPE_SSIF);
}

void test_netdata_parse_inband_driver_type_openipmi(void **state) {
    int result = netdata_parse_inband_driver_type("openipmi");
    assert_int_equal(result, IPMI_MONITORING_DRIVER_TYPE_OPENIPMI);
}

void test_netdata_parse_inband_driver_type_open(void **state) {
    int result = netdata_parse_inband_driver_type("open");
    assert_int_equal(result, IPMI_MONITORING_DRIVER_TYPE_OPENIPMI);
}

void test_netdata_parse_inband_driver_type_sunbmc(void **state) {
    int result = netdata_parse_inband_driver_type("sunbmc");
    assert_int_equal(result, IPMI_MONITORING_DRIVER_TYPE_SUNBMC);
}

void test_netdata_parse_inband_driver_type_bmc(void **state) {
    int result = netdata_parse_inband_driver_type("bmc");
    assert_int_equal(result, IPMI_MONITORING_DRIVER_TYPE_SUNBMC);
}

void test_netdata_parse_inband_driver_type_invalid(void **state) {
    int result = netdata_parse_inband_driver_type("invalid");
    assert_int_equal(result, -1);
}

void test_netdata_parse_inband_driver_type_case_insensitive(void **state) {
    int result = netdata_parse_inband_driver_type("KCS");
    assert_int_equal(result, IPMI_MONITORING_DRIVER_TYPE_KCS);
}

/* ============================================
   Test: netdata_collect_type_to_string()
   ============================================ */

void test_netdata_collect_type_to_string_sensors_only(void **state) {
    const char *result = netdata_collect_type_to_string(IPMI_COLLECT_TYPE_SENSORS);
    assert_string_equal(result, "sensors");
}

void test_netdata_collect_type_to_string_sel_only(void **state) {
    const char *result = netdata_collect_type_to_string(IPMI_COLLECT_TYPE_SEL);
    assert_string_equal(result, "sel");
}

void test_netdata_collect_type_to_string_both(void **state) {
    const char *result = netdata_collect_type_to_string(IPMI_COLLECT_TYPE_SENSORS | IPMI_COLLECT_TYPE_SEL);
    assert_string_equal(result, "sensors,sel");
}

void test_netdata_collect_type_to_string_unknown(void **state) {
    const char *result = netdata_collect_type_to_string(0);
    assert_string_equal(result, "unknown");
}

/* ============================================
   Test: netdata_ipmi_get_sensor_type_string()
   ============================================ */

void test_netdata_ipmi_get_sensor_type_string_reserved(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_RESERVED, &component);
    assert_string_equal(result, "Reserved");
}

void test_netdata_ipmi_get_sensor_type_string_temperature(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_TEMPERATURE, &component);
    assert_string_equal(result, "Temperature");
}

void test_netdata_ipmi_get_sensor_type_string_voltage(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_VOLTAGE, &component);
    assert_string_equal(result, "Voltage");
}

void test_netdata_ipmi_get_sensor_type_string_current(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_CURRENT, &component);
    assert_string_equal(result, "Current");
}

void test_netdata_ipmi_get_sensor_type_string_fan(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_FAN, &component);
    assert_string_equal(result, "Fan");
}

void test_netdata_ipmi_get_sensor_type_string_physical_security(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_PHYSICAL_SECURITY, &component);
    assert_string_equal(result, "Physical Security");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_SYSTEM);
}

void test_netdata_ipmi_get_sensor_type_string_processor(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_PROCESSOR, &component);
    assert_string_equal(result, "Processor");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_PROCESSOR);
}

void test_netdata_ipmi_get_sensor_type_string_power_supply(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_POWER_SUPPLY, &component);
    assert_string_equal(result, "Power Supply");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_POWER_SUPPLY);
}

void test_netdata_ipmi_get_sensor_type_string_memory(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_MEMORY, &component);
    assert_string_equal(result, "Memory");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_MEMORY);
}

void test_netdata_ipmi_get_sensor_type_string_network(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_LAN, &component);
    assert_string_equal(result, "LAN");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_NETWORK);
}

void test_netdata_ipmi_get_sensor_type_string_unknown(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_UNKNOWN, &component);
    assert_string_equal(result, "Unknown");
}

void test_netdata_ipmi_get_sensor_type_string_unrecognized(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(999, &component);
    assert_string_equal(result, "Unrecognized");
}

void test_netdata_ipmi_get_sensor_type_string_oem_type(void **state) {
    const char *component = NULL;
    const char *result = netdata_ipmi_get_sensor_type_string(IPMI_MONITORING_SENSOR_TYPE_OEM_MIN + 1, &component);
    assert_string_equal(result, "OEM");
}

/* ============================================
   Test: excluded_record_ids_parse()
   ============================================ */

void test_excluded_record_ids_parse_single_id(void **state) {
    excluded_record_ids_parse("42", false);
    assert_int_equal(excluded_record_ids_length, 1);
    assert_int_equal(excluded_record_ids[0], 42);
}

void test_excluded_record_ids_parse_multiple_ids(void **state) {
    excluded_record_ids_parse("10,20,30", false);
    assert_int_equal(excluded_record_ids_length, 3);
    assert_int_equal(excluded_record_ids[0], 10);
    assert_int_equal(excluded_record_ids[1], 20);
    assert_int_equal(excluded_record_ids[2], 30);
}

void test_excluded_record_ids_parse_null_input(void **state) {
    size_t initial_len = excluded_record_ids_length;
    excluded_record_ids_parse(NULL, false);
    assert_int_equal(excluded_record_ids_length, initial_len);
}

void test_excluded_record_ids_parse_with_spaces(void **state) {
    excluded_record_ids_parse("1 2 3", false);
    assert_int_equal(excluded_record_ids_length, 3);
    assert_int_equal(excluded_record_ids[0], 1);
}

void test_excluded_record_ids_parse_zero_value(void **state) {
    size_t initial_len = excluded_record_ids_length;
    excluded_record_ids_parse("0", false);
    assert_int_equal(excluded_record_ids_length, initial_len);
}

void test_excluded_record_ids_parse_negative_value(void **state) {
    excluded_record_ids_parse("-5", false);
    // Negative values should not be parsed as strtoul handles this
}

void test_excluded_record_ids_parse_empty_string(void **state) {
    size_t initial_len = excluded_record_ids_length;
    excluded_record_ids_parse("", false);
    assert_int_equal(excluded_record_ids_length, initial_len);
}

/* ============================================
   Test: excluded_record_ids_check()
   ============================================ */

void test_excluded_record_ids_check_found(void **state) {
    excluded_record_ids_parse("5,10,15", false);
    assert_int_equal(excluded_record_ids_check(10), 1);
}

void test_excluded_record_ids_check_not_found(void **state) {
    excluded_record_ids_parse("5,10,15", false);
    assert_int_equal(excluded_record_ids_check(99), 0);
}

void test_excluded_record_ids_check_empty_list(void **state) {
    excluded_record_ids = NULL;
    excluded_record_ids_length = 0;
    assert_int_equal(excluded_record_ids_check(1), 0);
}

/* ============================================
   Test: excluded_status_record_ids_parse()
   ============================================ */

void test_excluded_status_record_ids_parse_single(void **state) {
    excluded_status_record_ids_parse("42", false);
    assert_int_equal(excluded_status_record_ids_length, 1);
    assert_int_equal(excluded_status_record_ids[0], 42);
}

void test_excluded_status_record_ids_parse_multiple(void **state) {
    excluded_status_record_ids_parse("1,2,3", false);
    assert_int_equal(excluded_status_record_ids_length, 3);
}

void test_excluded_status_record_ids_parse_null(void **state) {
    size_t initial = excluded_status_record_ids_length;
    excluded_status_record_ids_parse(NULL, false);
    assert_int_equal(excluded_status_record_ids_length, initial);
}

/* ============================================
   Test: excluded_status_record_ids_check()
   ============================================ */

void test_excluded_status_record_ids_check_found(void **state) {
    excluded_status_record_ids_parse("5,10", false);
    assert_int_equal(excluded_status_record_ids_check(5), 1);
}

void test_excluded_status_record_ids_check_not_found(void **state) {
    excluded_status_record_ids_parse("5,10", false);
    assert_int_equal(excluded_status_record_ids_check(99), 0);
}

/* ============================================
   Test: get_sensor_state_string()
   ============================================ */

void test_get_sensor_state_string_nominal(void **state) {
    struct sensor sn = { .sensor_state = IPMI_MONITORING_STATE_NOMINAL };
    const char *result = get_sensor_state_string(&sn);
    assert_string_equal(result, "nominal");
}

void test_get_sensor_state_string_warning(void **state) {
    struct sensor sn = { .sensor_state = IPMI_MONITORING_STATE_WARNING };
    const char *result = get_sensor_state_string(&sn);
    assert_string_equal(result, "warning");
}

void test_get_sensor_state_string_critical(void **state) {
    struct sensor sn = { .sensor_state = IPMI_MONITORING_STATE_CRITICAL };
    const char *result = get_sensor_state_string(&sn);
    assert_string_equal(result, "critical");
}

void test_get_sensor_state_string_unknown(void **state) {
    struct sensor sn = { .sensor_state = IPMI_MONITORING_STATE_UNKNOWN };
    const char *result = get_sensor_state_string(&sn);
    assert_string_equal(result, "unknown");
}

void test_get_sensor_state_string_invalid(void **state) {
    struct sensor sn = { .sensor_state = 999 };
    const char *result = get_sensor_state_string(&sn);
    assert_string_equal(result, "unknown");
}

/* ============================================
   Test: get_sensor_function_priority()
   ============================================ */

void test_get_sensor_function_priority_nominal(void **state) {
    struct sensor sn = { .sensor_state = IPMI_MONITORING_STATE_NOMINAL };
    const char *result = get_sensor_function_priority(&sn);
    assert_string_equal(result, "normal");
}

void test_get_sensor_function_priority_warning(void **state) {
    struct sensor sn = { .sensor_state = IPMI_MONITORING_STATE_WARNING };
    const char *result = get_sensor_function_priority(&sn);
    assert_string_equal(result, "warning");
}

void test_get_sensor_function_priority_critical(void **state) {
    struct sensor sn = { .sensor_state = IPMI_MONITORING_STATE_CRITICAL };
    const char *result = get_sensor_function_priority(&sn);
    assert_string_equal(result, "critical");
}

void test_get_sensor_function_priority_unknown(void **state) {
    struct sensor sn = { .sensor_state = IPMI_MONITORING_STATE_UNKNOWN };
    const char *result = get_sensor_function_priority(&sn);
    assert_string_equal(result, "normal");
}

/* ============================================
   Test: netdata_sensor_set_value()
   ============================================ */

void test_netdata_sensor_set_value_bool_type(void **state) {
    struct sensor sn = {
        .sensor_reading_type = IPMI_MONITORING_SENSOR_READING_TYPE_UNSIGNED_INTEGER8_BOOL
    };
    struct netdata_ipmi_state st = {};
    uint8_t value = 1;
    
    netdata_sensor_set_value(&sn, &value, &st);
    
    assert_int_equal(sn.sensor_reading.bool_value, 1);
}

void test_netdata_sensor_set_value_uint32_type(void **state) {
    struct sensor sn = {
        .sensor_reading_type = IPMI_MONITORING_SENSOR_READING_TYPE_UNSIGNED_INTEGER32
    };
    struct netdata_ipmi_state st = {};
    uint32_t value = 1000;
    
    netdata_sensor_set_value(&sn, &value, &st);
    
    assert_int_equal(sn.sensor_reading.uint32_value, 1000);
}

void test_netdata_sensor_set_value_double_type(void **state) {
    struct sensor sn = {
        .sensor_reading_type = IPMI_MONITORING_SENSOR_READING_TYPE_DOUBLE
    };
    struct netdata_ipmi_state st = {};
    double value = 42.5;
    
    netdata_sensor_set_value(&sn, &value, &st);
    
    assert_true(sn.sensor_reading.double_value > 42.4 && sn.sensor_reading.double_value < 42.6);
}

void test_netdata_sensor_set_value_unknown_type(void **state) {
    struct sensor sn = {
        .sensor_reading_type = IPMI_MONITORING_SENSOR_READING_TYPE_UNKNOWN,
        .do_metric = true
    };
    struct netdata_ipmi_state st = {};
    uint8_t value = 1;
    
    netdata_sensor_set_value(&sn, &value, &st);
    
    assert_false(sn.do_metric);
}

/* ============================================
   Test: netdata_sensor_name_to_component()
   ============================================ */

void test_netdata_sensor_name_to_component_dimm(void **state) {
    const char *component = netdata_sensor_name_to_component("DIMM_1");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_MEMORY_MODULE);
}

void test_netdata_sensor_name_to_component_cpu(void **state) {
    const char *component = netdata_sensor_name_to_component("CPU_TEMP");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_PROCESSOR);
}

void test_netdata_sensor_name_to_component_storage(void **state) {
    const char *component = netdata_sensor_name_to_component("M2_SSD1");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_STORAGE);
}

void test_netdata_sensor_name_to_component_psu(void **state) {
    const char *component = netdata_sensor_name_to_component("PS1");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_POWER_SUPPLY);
}

void test_netdata_sensor_name_to_component_fan(void **state) {
    const char *component = netdata_sensor_name_to_component("SYS_FAN");
    assert_string_equal(component, NETDATA_SENSOR_COMPONENT_SYSTEM);
}

void test_netdata_sensor_name_to_component_unknown(void **state) {
    const char *component = netdata_sensor_name_to_component("CUSTOM_SENSOR");
    assert_string_equal(component, "Other");
}

/* ============================================
   Test: netdata_update_ipmi_sel_events_count()
   ============================================ */

void test_netdata_update_ipmi_sel_events_count(void **state) {
    struct netdata_ipmi_state st = {};
    netdata_update_ipmi_sel_events_count(&st, 42);
    assert_int_equal(st.sel.events, 42);
}

void test_netdata_update_ipmi_sel_events_count_zero(void **state) {
    struct netdata_ipmi_state st = { .sel = { .events = 100 } };
    netdata_update_ipmi_sel_events_count(&st, 0);
    assert_int_equal(st.sel.events, 0);
}

/* ============================================
   Test: initialize_ipmi_config()
   ============================================ */

void test_initialize_ipmi_config(void **state) {
    struct ipmi_monitoring_ipmi_config config;
    memset(&config, 0, sizeof(config));
    
    driver_type = 1;
    driver_address = 0x20;
    hostname = "test.example.com";
    
    initialize_ipmi_config(&config);
    
    assert_int_equal(config.driver_type, 1);
    assert_int_equal(config.driver_address, 0x20);
    assert_string_equal(config.driver_device, driver_device);
}

/* ============================================
   Test Coverage Summary
   ============================================ */

int main(void) {
    const struct CMUnitTest tests[] = {
        /* netdata_host_is_localhost tests */
        cmocka_unit_test(test_netdata_host_is_localhost_localhost_string),
        cmocka_unit_test(test_netdata_host_is_localhost_127_0_0_1),
        cmocka_unit_test(test_netdata_host_is_localhost_ipv6_localhost),
        cmocka_unit_test(test_netdata_host_is_localhost_ipv6_shorthand),
        cmocka_unit_test(test_netdata_host_is_localhost_ip6_localhost),
        cmocka_unit_test(test_netdata_host_is_localhost_ipv6_full),
        cmocka_unit_test(test_netdata_host_is_localhost_remote_host),
        cmocka_unit_test(test_netdata_host_is_localhost_hostname),
        cmocka_unit_test(test_netdata_host_is_localhost_empty_string),
        cmocka_unit_test(test_netdata_host_is_localhost_case_insensitive_localhost),
        cmocka_unit_test(test_netdata_host_is_localhost_case_insensitive_ipv6),
        
        /* netdata_parse_outofband_driver_type tests */
        cmocka_unit_test(test_netdata_parse_outofband_driver_type