#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mqtt_wss_client.h"

/* Global variables for testing */
static int test_exit_flag = 0;
static int port_test = 0;
static int msg_callback_called = 0;
static int log_callback_called = 0;
static char last_callback_topic[256] = {0};
static char last_callback_msg[512] = {0};
static int last_callback_qos = 0;

/* Mock callback functions */
void mock_mqtt_wss_log_cb(mqtt_wss_log_type_t log_type, const char* str)
{
    log_callback_called++;
    (void)log_type;
    (void)str;
}

void mock_msg_callback(const char *topic, const void *msg, size_t msglen, int qos)
{
    msg_callback_called++;
    size_t len = (msglen < 511) ? msglen : 511;
    if (topic) {
        strncpy(last_callback_topic, topic, 255);
        last_callback_topic[255] = '\0';
    }
    if (msg) {
        memcpy(last_callback_msg, msg, len);
        last_callback_msg[len] = '\0';
    }
    last_callback_qos = qos;
}

/* Test setup and teardown */
static int setup(void **state)
{
    test_exit_flag = 0;
    port_test = 0;
    msg_callback_called = 0;
    log_callback_called = 0;
    memset(last_callback_topic, 0, sizeof(last_callback_topic));
    memset(last_callback_msg, 0, sizeof(last_callback_msg));
    last_callback_qos = 0;
    return 0;
}

static int teardown(void **state)
{
    return 0;
}

/* Tests for mqtt_wss_log_cb function */
static void test_mqtt_wss_log_cb_normal_message(void **state)
{
    log_callback_called = 0;
    mock_mqtt_wss_log_cb(MQTT_WSS_LOG_INFO, "Test message");
    assert_int_equal(log_callback_called, 1);
}

static void test_mqtt_wss_log_cb_empty_message(void **state)
{
    log_callback_called = 0;
    mock_mqtt_wss_log_cb(MQTT_WSS_LOG_INFO, "");
    assert_int_equal(log_callback_called, 1);
}

static void test_mqtt_wss_log_cb_long_message(void **state)
{
    log_callback_called = 0;
    char long_msg[1024];
    memset(long_msg, 'a', 1023);
    long_msg[1023] = '\0';
    mock_mqtt_wss_log_cb(MQTT_WSS_LOG_ERROR, long_msg);
    assert_int_equal(log_callback_called, 1);
}

static void test_mqtt_wss_log_cb_various_log_types(void **state)
{
    log_callback_called = 0;
    mock_mqtt_wss_log_cb(MQTT_WSS_LOG_INFO, "Info");
    assert_int_equal(log_callback_called, 1);
    
    mock_mqtt_wss_log_cb(MQTT_WSS_LOG_WARN, "Warning");
    assert_int_equal(log_callback_called, 2);
    
    mock_mqtt_wss_log_cb(MQTT_WSS_LOG_ERROR, "Error");
    assert_int_equal(log_callback_called, 3);
    
    mock_mqtt_wss_log_cb(MQTT_WSS_LOG_DEBUG, "Debug");
    assert_int_equal(log_callback_called, 4);
}

/* Tests for msg_callback function */
static void test_msg_callback_normal_message(void **state)
{
    msg_callback_called = 0;
    const char *test_topic = "test/topic";
    const char *test_msg = "Hello World!";
    
    mock_msg_callback(test_topic, test_msg, strlen(test_msg), 1);
    
    assert_int_equal(msg_callback_called, 1);
    assert_string_equal(last_callback_topic, test_topic);
    assert_string_equal(last_callback_msg, test_msg);
    assert_int_equal(last_callback_qos, 1);
}

static void test_msg_callback_empty_message(void **state)
{
    msg_callback_called = 0;
    const char *test_topic = "test/topic";
    const char *test_msg = "";
    
    mock_msg_callback(test_topic, test_msg, strlen(test_msg), 0);
    
    assert_int_equal(msg_callback_called, 1);
    assert_string_equal(last_callback_topic, test_topic);
    assert_string_equal(last_callback_msg, "");
    assert_int_equal(last_callback_qos, 0);
}

static void test_msg_callback_message_exceeds_buffer(void **state)
{
    msg_callback_called = 0;
    const char *test_topic = "test/topic";
    char long_msg[1024];
    memset(long_msg, 'x', 1023);
    long_msg[1023] = '\0';
    
    mock_msg_callback(test_topic, long_msg, 1023, 2);
    
    assert_int_equal(msg_callback_called, 1);
    assert_string_equal(last_callback_topic, test_topic);
    /* Message should be truncated to TEST_MSGLEN_MAX-1 */
    assert_int_equal(strlen(last_callback_msg), 511);
    assert_int_equal(last_callback_qos, 2);
}

static void test_msg_callback_various_qos_levels(void **state)
{
    msg_callback_called = 0;
    const char *test_topic = "test/topic";
    const char *test_msg = "message";
    
    for (int qos = 0; qos <= 2; qos++) {
        mock_msg_callback(test_topic, test_msg, strlen(test_msg), qos);
        assert_int_equal(last_callback_qos, qos);
    }
}

static void test_msg_callback_null_topic(void **state)
{
    msg_callback_called = 0;
    const char *test_msg = "message";
    
    mock_msg_callback(NULL, test_msg, strlen(test_msg), 1);
    
    assert_int_equal(msg_callback_called, 1);
}

static void test_msg_callback_null_msg(void **state)
{
    msg_callback_called = 0;
    const char *test_topic = "test/topic";
    
    mock_msg_callback(test_topic, NULL, 0, 1);
    
    assert_int_equal(msg_callback_called, 1);
}

static void test_msg_callback_shutdown_command(void **state)
{
    msg_callback_called = 0;
    const char *test_topic = "test/topic";
    const char *shutdown_msg = "shutdown";
    
    mock_msg_callback(test_topic, shutdown_msg, strlen(shutdown_msg), 1);
    
    assert_int_equal(msg_callback_called, 1);
    assert_string_equal(last_callback_msg, "shutdown");
}

static void test_msg_callback_binary_payload(void **state)
{
    msg_callback_called = 0;
    const char *test_topic = "binary/topic";
    unsigned char binary_data[] = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD};
    
    mock_msg_callback(test_topic, binary_data, sizeof(binary_data), 2);
    
    assert_int_equal(msg_callback_called, 1);
    assert_string_equal(last_callback_topic, test_topic);
    assert_int_equal(last_callback_qos, 2);
}

static void test_msg_callback_very_long_topic(void **state)
{
    msg_callback_called = 0;
    char long_topic[512];
    memset(long_topic, 't', 511);
    long_topic[511] = '\0';
    const char *test_msg = "message";
    
    mock_msg_callback(long_topic, test_msg, strlen(test_msg), 1);
    
    assert_int_equal(msg_callback_called, 1);
    assert_int_equal(last_callback_qos, 1);
}

/* Main test runner */
int main(void)
{
    const struct CMUnitTest tests[] = {
        /* mqtt_wss_log_cb tests */
        cmocka_unit_test_setup_teardown(test_mqtt_wss_log_cb_normal_message, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mqtt_wss_log_cb_empty_message, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mqtt_wss_log_cb_long_message, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mqtt_wss_log_cb_various_log_types, setup, teardown),
        
        /* msg_callback tests */
        cmocka_unit_test_setup_teardown(test_msg_callback_normal_message, setup, teardown),
        cmocka_unit_test_setup_teardown(test_msg_callback_empty_message, setup, teardown),
        cmocka_unit_test_setup_teardown(test_msg_callback_message_exceeds_buffer, setup, teardown),
        cmocka_unit_test_setup_teardown(test_msg_callback_various_qos_levels, setup, teardown),
        cmocka_unit_test_setup_teardown(test_msg_callback_null_topic, setup, teardown),
        cmocka_unit_test_setup_teardown(test_msg_callback_null_msg, setup, teardown),
        cmocka_unit_test_setup_teardown(test_msg_callback_shutdown_command, setup, teardown),
        cmocka_unit_test_setup_teardown(test_msg_callback_binary_payload, setup, teardown),
        cmocka_unit_test_setup_teardown(test_msg_callback_very_long_topic, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}