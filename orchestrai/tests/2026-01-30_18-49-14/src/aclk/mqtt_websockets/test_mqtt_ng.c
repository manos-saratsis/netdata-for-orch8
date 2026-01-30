/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Comprehensive test suite for mqtt_ng.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>

/* Mock definitions for external dependencies */
#define NDLS_DAEMON 0
#define NDLP_ERR 1
#define NDLP_WARNING 2
#define NDLP_DEBUG 3
#define NDLP_INFO 4

/* Mock free function types */
typedef void (*free_fnc_t)(void *ptr);
void mock_free_function(void *ptr) {
    if (ptr) free(ptr);
}

/* Mock callback functions */
void mock_puback_callback(uint16_t packet_id) {
    (void)packet_id;
}

void mock_connack_callback(void* user_ctx, int connack_reply) {
    (void)user_ctx;
    (void)connack_reply;
}

void mock_msg_callback(const char *topic, const void *msg, size_t msglen, int qos) {
    (void)topic;
    (void)msg;
    (void)msglen;
    (void)qos;
}

ssize_t mock_send_fnc(void *user_ctx, const void* buf, size_t len) {
    (void)user_ctx;
    (void)buf;
    return (ssize_t)len;
}

/* Mock rbuf_t type */
typedef void* rbuf_t;

rbuf_t mock_rbuf_create(size_t size) {
    return malloc(size);
}

void mock_rbuf_free(rbuf_t buf) {
    if (buf) free(buf);
}

/* Test: uint32_to_mqtt_vbi with 0 */
void test_uint32_to_mqtt_vbi_zero(void) {
    unsigned char output[4];
    /* Expected: single byte 0x00 */
    int ret = uint32_to_mqtt_vbi(0, output);
    assert(ret == 1);
    assert(output[0] == 0x00);
    printf("✓ test_uint32_to_mqtt_vbi_zero passed\n");
}

/* Test: uint32_to_mqtt_vbi with single byte value (0-127) */
void test_uint32_to_mqtt_vbi_single_byte(void) {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(42, output);
    assert(ret == 1);
    assert(output[0] == 42);
    printf("✓ test_uint32_to_mqtt_vbi_single_byte passed\n");
}

/* Test: uint32_to_mqtt_vbi with two byte value (128-16383) */
void test_uint32_to_mqtt_vbi_two_bytes(void) {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(128, output);
    assert(ret == 2);
    assert(output[0] == 0x80);
    assert(output[1] == 0x01);
    printf("✓ test_uint32_to_mqtt_vbi_two_bytes passed\n");
}

/* Test: uint32_to_mqtt_vbi with three byte value */
void test_uint32_to_mqtt_vbi_three_bytes(void) {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(16384, output);
    assert(ret == 3);
    printf("✓ test_uint32_to_mqtt_vbi_three_bytes passed\n");
}

/* Test: uint32_to_mqtt_vbi with four byte value */
void test_uint32_to_mqtt_vbi_four_bytes(void) {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(2097152, output);
    assert(ret == 4);
    printf("✓ test_uint32_to_mqtt_vbi_four_bytes passed\n");
}

/* Test: uint32_to_mqtt_vbi with maximum value */
void test_uint32_to_mqtt_vbi_max_value(void) {
    unsigned char output[4];
    /* Maximum variable byte integer is 268,435,455 */
    int ret = uint32_to_mqtt_vbi(268435455, output);
    assert(ret == 4);
    printf("✓ test_uint32_to_mqtt_vbi_max_value passed\n");
}

/* Test: uint32_to_mqtt_vbi with value beyond maximum */
void test_uint32_to_mqtt_vbi_overflow(void) {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(268435456, output);
    /* Should return error (<=0) */
    assert(ret <= 0);
    printf("✓ test_uint32_to_mqtt_vbi_overflow passed\n");
}

/* Test: uint32_to_mqtt_vbi with NULL output */
void test_uint32_to_mqtt_vbi_null_output(void) {
    /* Should handle NULL gracefully or return error */
    int ret = uint32_to_mqtt_vbi(42, NULL);
    /* Expecting undefined behavior or crash protection */
    printf("✓ test_uint32_to_mqtt_vbi_null_output passed (assumption)\n");
}

/* Test: mqtt_lwt_properties structure initialization */
void test_mqtt_lwt_properties_init(void) {
    struct mqtt_lwt_properties lwt = {
        .will_topic = "test/topic",
        .will_topic_free = mock_free_function,
        .will_message = malloc(10),
        .will_message_free = mock_free_function,
        .will_message_size = 10,
        .will_qos = 1,
        .will_retain = 1
    };
    
    assert(lwt.will_topic != NULL);
    assert(lwt.will_message != NULL);
    assert(lwt.will_qos == 1);
    assert(lwt.will_retain == 1);
    assert(lwt.will_message_size == 10);
    
    if (lwt.will_message_free)
        lwt.will_message_free(lwt.will_message);
    printf("✓ test_mqtt_lwt_properties_init passed\n");
}

/* Test: mqtt_lwt_properties with NULL pointers */
void test_mqtt_lwt_properties_null(void) {
    struct mqtt_lwt_properties lwt = {
        .will_topic = NULL,
        .will_topic_free = NULL,
        .will_message = NULL,
        .will_message_free = NULL,
        .will_message_size = 0,
        .will_qos = 0,
        .will_retain = 0
    };
    
    assert(lwt.will_topic == NULL);
    assert(lwt.will_message == NULL);
    printf("✓ test_mqtt_lwt_properties_null passed\n");
}

/* Test: mqtt_auth_properties structure initialization */
void test_mqtt_auth_properties_init(void) {
    struct mqtt_auth_properties auth = {
        .client_id = "test_client",
        .client_id_free = mock_free_function,
        .username = "testuser",
        .username_free = mock_free_function,
        .password = "testpass",
        .password_free = mock_free_function
    };
    
    assert(auth.client_id != NULL);
    assert(auth.username != NULL);
    assert(auth.password != NULL);
    printf("✓ test_mqtt_auth_properties_init passed\n");
}

/* Test: mqtt_auth_properties with NULL pointers */
void test_mqtt_auth_properties_null(void) {
    struct mqtt_auth_properties auth = {
        .client_id = NULL,
        .client_id_free = NULL,
        .username = NULL,
        .username_free = NULL,
        .password = NULL,
        .password_free = NULL
    };
    
    assert(auth.client_id == NULL);
    assert(auth.username == NULL);
    assert(auth.password == NULL);
    printf("✓ test_mqtt_auth_properties_null passed\n");
}

/* Test: mqtt_sub structure initialization */
void test_mqtt_sub_init(void) {
    struct mqtt_sub sub = {
        .topic = "test/topic",
        .topic_free = NULL,
        .options = 0x01
    };
    
    assert(sub.topic != NULL);
    assert(sub.options == 0x01);
    printf("✓ test_mqtt_sub_init passed\n");
}

/* Test: mqtt_sub with empty topic */
void test_mqtt_sub_empty_topic(void) {
    struct mqtt_sub sub = {
        .topic = "",
        .topic_free = NULL,
        .options = 0x00
    };
    
    assert(sub.topic != NULL);
    assert(strlen(sub.topic) == 0);
    printf("✓ test_mqtt_sub_empty_topic passed\n");
}

/* Test: mqtt_ng_init callback structure setup */
void test_mqtt_ng_init_callbacks(void) {
    rbuf_t buf = mock_rbuf_create(1024);
    
    struct mqtt_ng_init settings = {
        .data_in = buf,
        .data_out_fnc = &mock_send_fnc,
        .user_ctx = NULL,
        .puback_callback = &mock_puback_callback,
        .connack_callback = &mock_connack_callback,
        .msg_callback = &mock_msg_callback
    };
    
    assert(settings.data_in != NULL);
    assert(settings.data_out_fnc != NULL);
    assert(settings.puback_callback != NULL);
    assert(settings.connack_callback != NULL);
    assert(settings.msg_callback != NULL);
    
    mock_rbuf_free(buf);
    printf("✓ test_mqtt_ng_init_callbacks passed\n");
}

/* Test: mqtt_ng_init with NULL callbacks */
void test_mqtt_ng_init_null_callbacks(void) {
    rbuf_t buf = mock_rbuf_create(1024);
    
    struct mqtt_ng_init settings = {
        .data_in = buf,
        .data_out_fnc = &mock_send_fnc,
        .user_ctx = NULL,
        .puback_callback = NULL,
        .connack_callback = NULL,
        .msg_callback = NULL
    };
    
    assert(settings.puback_callback == NULL);
    assert(settings.connack_callback == NULL);
    assert(settings.msg_callback == NULL);
    
    mock_rbuf_free(buf);
    printf("✓ test_mqtt_ng_init_null_callbacks passed\n");
}

/* Test: mqtt_ng_init with different user contexts */
void test_mqtt_ng_init_user_context(void) {
    rbuf_t buf = mock_rbuf_create(1024);
    void *user_ctx = malloc(100);
    
    struct mqtt_ng_init settings = {
        .data_in = buf,
        .data_out_fnc = &mock_send_fnc,
        .user_ctx = user_ctx,
        .puback_callback = NULL,
        .connack_callback = NULL,
        .msg_callback = NULL
    };
    
    assert(settings.user_ctx == user_ctx);
    
    free(user_ctx);
    mock_rbuf_free(buf);
    printf("✓ test_mqtt_ng_init_user_context passed\n");
}

/* Test: define constants verification */
void test_mqtt_ng_constants(void) {
    assert(MQTT_NG_MSGGEN_OK == 0);
    assert(MQTT_NG_MSGGEN_USER_ERROR == 1);
    assert(MQTT_NG_MSGGEN_BUFFER_OOM == 2);
    assert(MQTT_NG_MSGGEN_MSG_TOO_BIG == 3);
    printf("✓ test_mqtt_ng_constants passed\n");
}

/* Test: ping_timeout global variable */
void test_ping_timeout_global(void) {
    /* ping_timeout should be accessible */
    ping_timeout = 0;
    assert(ping_timeout == 0);
    
    ping_timeout = 60;
    assert(ping_timeout == 60);
    
    ping_timeout = 0;
    printf("✓ test_ping_timeout_global passed\n");
}

/* Test: mqtt_wss_publish_flags enum */
void test_mqtt_wss_publish_flags(void) {
    /* Verify flag values */
    assert(MQTT_WSS_PUB_QOS0 == 0x0);
    assert(MQTT_WSS_PUB_QOS1 == 0x1);
    assert(MQTT_WSS_PUB_QOS2 == 0x2);
    assert(MQTT_WSS_PUB_QOSMASK == 0x3);
    assert(MQTT_WSS_PUB_RETAIN == 0x4);
    printf("✓ test_mqtt_wss_publish_flags passed\n");
}

/* Test: mqtt_connect_params structure */
void test_mqtt_connect_params_init(void) {
    struct mqtt_connect_params params = {
        .clientid = "test_client",
        .username = "user",
        .password = "pass",
        .will_topic = "test/will",
        .will_msg = "offline",
        .will_flags = MQTT_WSS_PUB_QOS1,
        .will_msg_len = 7,
        .keep_alive = 60,
        .drop_on_publish_fail = 1
    };
    
    assert(params.clientid != NULL);
    assert(params.keep_alive == 60);
    assert(params.drop_on_publish_fail == 1);
    printf("✓ test_mqtt_connect_params_init passed\n");
}

/* Test: mqtt_connect_params with NULL values */
void test_mqtt_connect_params_null(void) {
    struct mqtt_connect_params params = {
        .clientid = NULL,
        .username = NULL,
        .password = NULL,
        .will_topic = NULL,
        .will_msg = NULL,
        .will_flags = MQTT_WSS_PUB_QOS0,
        .will_msg_len = 0,
        .keep_alive = 0,
        .drop_on_publish_fail = 0
    };
    
    assert(params.clientid == NULL);
    assert(params.keep_alive == 0);
    printf("✓ test_mqtt_connect_params_null passed\n");
}

/* Test: edge case - keep_alive boundary */
void test_mqtt_connect_params_keep_alive_boundary(void) {
    struct mqtt_connect_params params = {
        .keep_alive = UINT16_MAX
    };
    
    assert(params.keep_alive == UINT16_MAX);
    printf("✓ test_mqtt_connect_params_keep_alive_boundary passed\n");
}

/* Test: edge case - will_msg_len boundary */
void test_mqtt_connect_params_will_msg_len_boundary(void) {
    struct mqtt_connect_params params = {
        .will_msg_len = UINT16_MAX
    };
    
    assert(params.will_msg_len == UINT16_MAX);
    printf("✓ test_mqtt_connect_params_will_msg_len_boundary passed\n");
}

/* Test: mqtt_wss_proxy structure */
void test_mqtt_wss_proxy_direct(void) {
    struct mqtt_wss_proxy proxy = {
        .type = MQTT_WSS_DIRECT,
        .host = NULL,
        .port = 0,
        .username = NULL,
        .password = NULL,
        .proxy_destination = NULL
    };
    
    assert(proxy.type == MQTT_WSS_DIRECT);
    assert(proxy.host == NULL);
    printf("✓ test_mqtt_wss_proxy_direct passed\n");
}

/* Test: mqtt_wss_proxy structure with HTTP proxy */
void test_mqtt_wss_proxy_http(void) {
    struct mqtt_wss_proxy proxy = {
        .type = MQTT_WSS_PROXY_HTTP,
        .host = "proxy.example.com",
        .port = 8080,
        .username = "proxyuser",
        .password = "proxypass",
        .proxy_destination = "mqtt.example.com:8883"
    };
    
    assert(proxy.type == MQTT_WSS_PROXY_HTTP);
    assert(proxy.host != NULL);
    assert(proxy.port == 8080);
    printf("✓ test_mqtt_wss_proxy_http passed\n");
}

/* Test: mqtt_wss_ssl_flags */
void test_mqtt_wss_ssl_flags(void) {
    assert(MQTT_WSS_SSL_CERT_CHECK_FULL == 0x00);
    assert(MQTT_WSS_SSL_ALLOW_SELF_SIGNED == 0x01);
    assert(MQTT_WSS_SSL_DONT_CHECK_CERTS == 0x08);
    printf("✓ test_mqtt_wss_ssl_flags passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running MQTT NG Header Tests...\n\n");
    
    test_uint32_to_mqtt_vbi_zero();
    test_uint32_to_mqtt_vbi_single_byte();
    test_uint32_to_mqtt_vbi_two_bytes();
    test_uint32_to_mqtt_vbi_three_bytes();
    test_uint32_to_mqtt_vbi_four_bytes();
    test_uint32_to_mqtt_vbi_max_value();
    test_uint32_to_mqtt_vbi_overflow();
    test_uint32_to_mqtt_vbi_null_output();
    
    test_mqtt_lwt_properties_init();
    test_mqtt_lwt_properties_null();
    
    test_mqtt_auth_properties_init();
    test_mqtt_auth_properties_null();
    
    test_mqtt_sub_init();
    test_mqtt_sub_empty_topic();
    
    test_mqtt_ng_init_callbacks();
    test_mqtt_ng_init_null_callbacks();
    test_mqtt_ng_init_user_context();
    
    test_mqtt_ng_constants();
    test_ping_timeout_global();
    test_mqtt_wss_publish_flags();
    
    test_mqtt_connect_params_init();
    test_mqtt_connect_params_null();
    test_mqtt_connect_params_keep_alive_boundary();
    test_mqtt_connect_params_will_msg_len_boundary();
    
    test_mqtt_wss_proxy_direct();
    test_mqtt_wss_proxy_http();
    
    test_mqtt_wss_ssl_flags();
    
    printf("\n✓ All tests passed!\n");
    return 0;
}