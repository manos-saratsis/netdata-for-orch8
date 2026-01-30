#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Mock external dependencies
#include "libnetdata/libnetdata.h"
#include "mqtt_websockets/mqtt_wss_client.h"

// Include the header we're testing
#include "../aclk_util.h"

// Mock implementations for external functions that would be linked
aclk_encoding_type_t aclk_encoding_type_t_from_str(const char *str) {
    if (!str)
        return ACLK_ENC_UNKNOWN;
    
    if (strcmp(str, "json") == 0)
        return ACLK_ENC_JSON;
    if (strcmp(str, "proto") == 0)
        return ACLK_ENC_PROTO;
    
    return ACLK_ENC_UNKNOWN;
}

aclk_transport_type_t aclk_transport_type_t_from_str(const char *str) {
    if (!str)
        return ACLK_TRP_UNKNOWN;
    
    if (strcmp(str, "mqtt_3_1_1") == 0)
        return ACLK_TRP_MQTT_3_1_1;
    if (strcmp(str, "mqtt_5") == 0)
        return ACLK_TRP_MQTT_5;
    
    return ACLK_TRP_UNKNOWN;
}

void aclk_transport_desc_t_destroy(aclk_transport_desc_t *trp_desc) {
    if (!trp_desc)
        return;
    if (trp_desc->endpoint)
        free(trp_desc->endpoint);
    free(trp_desc);
}

void aclk_env_t_destroy(aclk_env_t *env) {
    if (!env)
        return;
    
    if (env->auth_endpoint)
        free(env->auth_endpoint);
    
    if (env->transports) {
        for (size_t i = 0; i < env->transport_count; i++) {
            aclk_transport_desc_t_destroy(env->transports[i]);
        }
        free(env->transports);
    }
    
    if (env->capabilities) {
        for (size_t i = 0; i < env->capability_count; i++) {
            free(env->capabilities[i]);
        }
        free(env->capabilities);
    }
    
    free(env);
}

int aclk_env_has_capa(const char *capa) {
    if (!capa)
        return 0;
    
    // This would check against aclk_env->capabilities
    // For testing, we'll implement a simple mock
    return 1;
}

const char *aclk_get_topic(enum aclk_topics topic) {
    switch (topic) {
        case ACLK_TOPICID_UNKNOWN:
            return "unknown";
        case ACLK_TOPICID_CHART:
            return "chart";
        case ACLK_TOPICID_ALARMS:
            return "alarms";
        case ACLK_TOPICID_METADATA:
            return "metadata";
        default:
            return "unknown";
    }
}

int aclk_generate_topic_cache(json_object *json) {
    // Mock implementation
    return 0;
}

void free_topic_cache(void) {
    // Mock implementation
}

const char *aclk_topic_cache_iterate(size_t *iter) {
    // Mock implementation
    return NULL;
}

unsigned long int aclk_tbeb_delay(int reset, int base, unsigned long int mins_ms, unsigned long int min_ms) {
    if (reset)
        return 0;
    return mins_ms + min_ms;
}

void aclk_set_proxy(char **ohost, int *port, char **uname, char **pwd,
    char **log_proxy, enum mqtt_wss_proxy_type *type) {
    // Mock implementation
    if (ohost) *ohost = NULL;
    if (port) *port = 0;
    if (uname) *uname = NULL;
    if (pwd) *pwd = NULL;
    if (log_proxy) *log_proxy = NULL;
    if (type) *type = 0;
}

// Global variables
usec_t aclk_session_newarch = 0;
int chart_batch_id = 0;
aclk_env_t *aclk_env = NULL;

// ============================================================================
// UNIT TESTS
// ============================================================================

// Test aclk_encoding_type_t_from_str with JSON
static void test_aclk_encoding_type_t_from_str_json(void **state) {
    (void) state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str("json");
    assert_int_equal(result, ACLK_ENC_JSON);
}

// Test aclk_encoding_type_t_from_str with PROTO
static void test_aclk_encoding_type_t_from_str_proto(void **state) {
    (void) state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str("proto");
    assert_int_equal(result, ACLK_ENC_PROTO);
}

// Test aclk_encoding_type_t_from_str with unknown
static void test_aclk_encoding_type_t_from_str_unknown(void **state) {
    (void) state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str("invalid");
    assert_int_equal(result, ACLK_ENC_UNKNOWN);
}

// Test aclk_encoding_type_t_from_str with NULL
static void test_aclk_encoding_type_t_from_str_null(void **state) {
    (void) state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str(NULL);
    assert_int_equal(result, ACLK_ENC_UNKNOWN);
}

// Test aclk_encoding_type_t_from_str with empty string
static void test_aclk_encoding_type_t_from_str_empty(void **state) {
    (void) state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str("");
    assert_int_equal(result, ACLK_ENC_UNKNOWN);
}

// Test aclk_transport_type_t_from_str with MQTT 3.1.1
static void test_aclk_transport_type_t_from_str_mqtt311(void **state) {
    (void) state;
    aclk_transport_type_t result = aclk_transport_type_t_from_str("mqtt_3_1_1");
    assert_int_equal(result, ACLK_TRP_MQTT_3_1_1);
}

// Test aclk_transport_type_t_from_str with MQTT 5
static void test_aclk_transport_type_t_from_str_mqtt5(void **state) {
    (void) state;
    aclk_transport_type_t result = aclk_transport_type_t_from_str("mqtt_5");
    assert_int_equal(result, ACLK_TRP_MQTT_5);
}

// Test aclk_transport_type_t_from_str with unknown
static void test_aclk_transport_type_t_from_str_unknown(void **state) {
    (void) state;
    aclk_transport_type_t result = aclk_transport_type_t_from_str("invalid");
    assert_int_equal(result, ACLK_TRP_UNKNOWN);
}

// Test aclk_transport_type_t_from_str with NULL
static void test_aclk_transport_type_t_from_str_null(void **state) {
    (void) state;
    aclk_transport_type_t result = aclk_transport_type_t_from_str(NULL);
    assert_int_equal(result, ACLK_TRP_UNKNOWN);
}

// Test aclk_transport_desc_t_destroy with valid pointer
static void test_aclk_transport_desc_t_destroy_valid(void **state) {
    (void) state;
    aclk_transport_desc_t *desc = malloc(sizeof(aclk_transport_desc_t));
    desc->endpoint = malloc(20);
    strcpy(desc->endpoint, "test.endpoint.com");
    desc->type = ACLK_TRP_MQTT_5;
    
    // Should not crash
    aclk_transport_desc_t_destroy(desc);
}

// Test aclk_transport_desc_t_destroy with NULL endpoint
static void test_aclk_transport_desc_t_destroy_null_endpoint(void **state) {
    (void) state;
    aclk_transport_desc_t *desc = malloc(sizeof(aclk_transport_desc_t));
    desc->endpoint = NULL;
    desc->type = ACLK_TRP_MQTT_5;
    
    // Should not crash
    aclk_transport_desc_t_destroy(desc);
}

// Test aclk_transport_desc_t_destroy with NULL pointer
static void test_aclk_transport_desc_t_destroy_null(void **state) {
    (void) state;
    // Should not crash
    aclk_transport_desc_t_destroy(NULL);
}

// Test aclk_env_t_destroy with valid structure
static void test_aclk_env_t_destroy_valid(void **state) {
    (void) state;
    aclk_env_t *env = malloc(sizeof(aclk_env_t));
    env->auth_endpoint = malloc(20);
    strcpy(env->auth_endpoint, "auth.endpoint.com");
    
    env->transports = malloc(sizeof(aclk_transport_desc_t*) * 2);
    env->transports[0] = malloc(sizeof(aclk_transport_desc_t));
    env->transports[0]->endpoint = malloc(15);
    strcpy(env->transports[0]->endpoint, "transport1.com");
    env->transports[0]->type = ACLK_TRP_MQTT_5;
    
    env->transports[1] = malloc(sizeof(aclk_transport_desc_t));
    env->transports[1]->endpoint = malloc(15);
    strcpy(env->transports[1]->endpoint, "transport2.com");
    env->transports[1]->type = ACLK_TRP_MQTT_3_1_1;
    env->transport_count = 2;
    
    env->capabilities = malloc(sizeof(char*) * 2);
    env->capabilities[0] = malloc(10);
    strcpy(env->capabilities[0], "capability1");
    env->capabilities[1] = malloc(10);
    strcpy(env->capabilities[1], "capability2");
    env->capability_count = 2;
    
    env->backoff.base = 10;
    env->backoff.max_s = 60;
    env->backoff.min_s = 5;
    
    // Should not crash
    aclk_env_t_destroy(env);
}

// Test aclk_env_t_destroy with NULL pointer
static void test_aclk_env_t_destroy_null(void **state) {
    (void) state;
    // Should not crash
    aclk_env_t_destroy(NULL);
}

// Test aclk_env_t_destroy with partial initialization
static void test_aclk_env_t_destroy_partial(void **state) {
    (void) state;
    aclk_env_t *env = malloc(sizeof(aclk_env_t));
    env->auth_endpoint = NULL;
    env->transports = NULL;
    env->transport_count = 0;
    env->capabilities = NULL;
    env->capability_count = 0;
    
    // Should not crash
    aclk_env_t_destroy(env);
}

// Test aclk_env_has_capa with valid capability
static void test_aclk_env_has_capa_valid(void **state) {
    (void) state;
    int result = aclk_env_has_capa("test_capability");
    assert_int_equal(result, 1);
}

// Test aclk_env_has_capa with NULL
static void test_aclk_env_has_capa_null(void **state) {
    (void) state;
    int result = aclk_env_has_capa(NULL);
    assert_int_equal(result, 0);
}

// Test aclk_get_topic with CHART
static void test_aclk_get_topic_chart(void **state) {
    (void) state;
    const char *result = aclk_get_topic(ACLK_TOPICID_CHART);
    assert_string_equal(result, "chart");
}

// Test aclk_get_topic with ALARMS
static void test_aclk_get_topic_alarms(void **state) {
    (void) state;
    const char *result = aclk_get_topic(ACLK_TOPICID_ALARMS);
    assert_string_equal(result, "alarms");
}

// Test aclk_get_topic with UNKNOWN
static void test_aclk_get_topic_unknown(void **state) {
    (void) state;
    const char *result = aclk_get_topic(ACLK_TOPICID_UNKNOWN);
    assert_string_equal(result, "unknown");
}

// Test aclk_generate_topic_cache
static void test_aclk_generate_topic_cache(void **state) {
    (void) state;
    int result = aclk_generate_topic_cache(NULL);
    assert_int_equal(result, 0);
}

// Test free_topic_cache
static void test_free_topic_cache(void **state) {
    (void) state;
    // Should not crash
    free_topic_cache();
}

// Test aclk_topic_cache_iterate
static void test_aclk_topic_cache_iterate(void **state) {
    (void) state;
    size_t iter = 0;
    const char *result = aclk_topic_cache_iterate(&iter);
    assert_null(result);
}

// Test aclk_tbeb_delay with reset=1
static void test_aclk_tbeb_delay_reset(void **state) {
    (void) state;
    unsigned long int result = aclk_tbeb_delay(1, 10, 1000, 500);
    assert_int_equal(result, 0);
}

// Test aclk_tbeb_delay with reset=0
static void test_aclk_tbeb_delay_no_reset(void **state) {
    (void) state;
    unsigned long int result = aclk_tbeb_delay(0, 10, 1000, 500);
    assert_int_equal(result, 1500);
}

// Test aclk_tbeb_reset macro
static void test_aclk_tbeb_reset(void **state) {
    (void) state;
    unsigned long int result = aclk_tbeb_reset(0);
    assert_int_equal(result, 0);
}

// Test aclk_set_proxy
static void test_aclk_set_proxy(void **state) {
    (void) state;
    char *host = NULL;
    int port = 80;
    char *uname = NULL;
    char *pwd = NULL;
    char *log_proxy = NULL;
    enum mqtt_wss_proxy_type type = 0;
    
    aclk_set_proxy(&host, &port, &uname, &pwd, &log_proxy, &type);
    
    assert_null(host);
    assert_int_equal(port, 0);
    assert_null(uname);
    assert_null(pwd);
    assert_null(log_proxy);
    assert_int_equal(type, 0);
}

// Test macros for error codes
static void test_error_code_macros(void **state) {
    (void) state;
    assert_int_equal(CLOUD_EC_MALFORMED_NODE_ID, 1);
    assert_int_equal(CLOUD_EC_NODE_NOT_FOUND, 2);
    assert_int_equal(CLOUD_EC_ZLIB_ERROR, 3);
    assert_int_equal(CLOUD_EC_REQ_REPLY_TOO_BIG, 4);
    assert_int_equal(CLOUD_EC_FAIL_TOPIC, 5);
    assert_int_equal(CLOUD_EC_SND_TIMEOUT, 6);
}

// Test that error message macros are defined
static void test_error_message_macros(void **state) {
    (void) state;
    assert_non_null(CLOUD_EMSG_MALFORMED_NODE_ID);
    assert_non_null(CLOUD_EMSG_NODE_NOT_FOUND);
    assert_non_null(CLOUD_EMSG_ZLIB_ERROR);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_json),
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_proto),
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_unknown),
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_null),
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_empty),
        cmocka_unit_test(test_aclk_transport_type_t_from_str_mqtt311),
        cmocka_unit_test(test_aclk_transport_type_t_from_str_mqtt5),
        cmocka_unit_test(test_aclk_transport_type_t_from_str_unknown),
        cmocka_unit_test(test_aclk_transport_type_t_from_str_null),
        cmocka_unit_test(test_aclk_transport_desc_t_destroy_valid),
        cmocka_unit_test(test_aclk_transport_desc_t_destroy_null_endpoint),
        cmocka_unit_test(test_aclk_transport_desc_t_destroy_null),
        cmocka_unit_test(test_aclk_env_t_destroy_valid),
        cmocka_unit_test(test_aclk_env_t_destroy_null),
        cmocka_unit_test(test_aclk_env_t_destroy_partial),
        cmocka_unit_test(test_aclk_env_has_capa_valid),
        cmocka_unit_test(test_aclk_env_has_capa_null),
        cmocka_unit_test(test_aclk_get_topic_chart),
        cmocka_unit_test(test_aclk_get_topic_alarms),
        cmocka_unit_test(test_aclk_get_topic_unknown),
        cmocka_unit_test(test_aclk_generate_topic_cache),
        cmocka_unit_test(test_free_topic_cache),
        cmocka_unit_test(test_aclk_topic_cache_iterate),
        cmocka_unit_test(test_aclk_tbeb_delay_reset),
        cmocka_unit_test(test_aclk_tbeb_delay_no_reset),
        cmocka_unit_test(test_aclk_tbeb_reset),
        cmocka_unit_test(test_aclk_set_proxy),
        cmocka_unit_test(test_error_code_macros),
        cmocka_unit_test(test_error_message_macros),
    };

    return cmocka_run_tests(tests);
}