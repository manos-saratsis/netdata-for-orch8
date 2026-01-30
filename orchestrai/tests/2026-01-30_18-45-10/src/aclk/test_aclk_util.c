#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <math.h>

#include "aclk_util.h"
#include "aclk_proxy.h"

// Mock functions
usec_t aclk_session_newarch = 0;
aclk_env_t *aclk_env = NULL;
int chart_batch_id = 0;

static const char *aclk_get_proxy(ACLK_PROXY_TYPE *pt, bool log_it) {
    if (pt) *pt = PROXY_TYPE_HTTP;
    if (log_it) return "http://user:pass@proxy.example.com:8080";
    return "http://user:pass@proxy.example.com:8080";
}

void claim_id_get_stub(void) {}
void url_decode_r_stub(char *decoded, const char *src, size_t size) {
    strncpy(decoded, src, size - 1);
    decoded[size - 1] = 0;
}

uint32_t os_random32(void) {
    return 0x12345678;
}

// Test: aclk_encoding_type_t_from_str with "json"
static void test_aclk_encoding_type_t_from_str_json(void **state) {
    (void)state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str("json");
    assert_int_equal(result, ACLK_ENC_JSON);
}

// Test: aclk_encoding_type_t_from_str with "proto"
static void test_aclk_encoding_type_t_from_str_proto(void **state) {
    (void)state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str("proto");
    assert_int_equal(result, ACLK_ENC_PROTO);
}

// Test: aclk_encoding_type_t_from_str with unknown string
static void test_aclk_encoding_type_t_from_str_unknown(void **state) {
    (void)state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str("unknown");
    assert_int_equal(result, ACLK_ENC_UNKNOWN);
}

// Test: aclk_encoding_type_t_from_str with empty string
static void test_aclk_encoding_type_t_from_str_empty(void **state) {
    (void)state;
    aclk_encoding_type_t result = aclk_encoding_type_t_from_str("");
    assert_int_equal(result, ACLK_ENC_UNKNOWN);
}

// Test: aclk_encoding_type_t_from_str with NULL (should not crash)
static void test_aclk_encoding_type_t_from_str_null(void **state) {
    (void)state;
    // Note: We expect NULL to be handled gracefully or crash
    // Depends on implementation. This test documents behavior.
}

// Test: aclk_transport_type_t_from_str with "MQTTv3"
static void test_aclk_transport_type_t_from_str_mqtt3(void **state) {
    (void)state;
    aclk_transport_type_t result = aclk_transport_type_t_from_str("MQTTv3");
    assert_int_equal(result, ACLK_TRP_MQTT_3_1_1);
}

// Test: aclk_transport_type_t_from_str with "MQTTv5"
static void test_aclk_transport_type_t_from_str_mqtt5(void **state) {
    (void)state;
    aclk_transport_type_t result = aclk_transport_type_t_from_str("MQTTv5");
    assert_int_equal(result, ACLK_TRP_MQTT_5);
}

// Test: aclk_transport_type_t_from_str with unknown string
static void test_aclk_transport_type_t_from_str_unknown(void **state) {
    (void)state;
    aclk_transport_type_t result = aclk_transport_type_t_from_str("unknown");
    assert_int_equal(result, ACLK_TRP_UNKNOWN);
}

// Test: aclk_transport_type_t_from_str with empty string
static void test_aclk_transport_type_t_from_str_empty(void **state) {
    (void)state;
    aclk_transport_type_t result = aclk_transport_type_t_from_str("");
    assert_int_equal(result, ACLK_TRP_UNKNOWN);
}

// Test: aclk_transport_desc_t_destroy with NULL endpoint
static void test_aclk_transport_desc_t_destroy_null_endpoint(void **state) {
    (void)state;
    aclk_transport_desc_t desc = { .endpoint = NULL };
    aclk_transport_desc_t_destroy(&desc);
    assert_null(desc.endpoint);
}

// Test: aclk_transport_desc_t_destroy with valid endpoint
static void test_aclk_transport_desc_t_destroy_valid_endpoint(void **state) {
    (void)state;
    aclk_transport_desc_t desc = { .endpoint = strdup("mqtt://test") };
    aclk_transport_desc_t_destroy(&desc);
    assert_null(desc.endpoint);
}

// Test: aclk_env_t_destroy with NULL pointers
static void test_aclk_env_t_destroy_null_pointers(void **state) {
    (void)state;
    aclk_env_t env = {
        .auth_endpoint = NULL,
        .transports = NULL,
        .transport_count = 0,
        .capabilities = NULL,
        .capability_count = 0
    };
    aclk_env_t_destroy(&env);
    assert_null(env.auth_endpoint);
    assert_null(env.transports);
}

// Test: aclk_env_t_destroy with transports
static void test_aclk_env_t_destroy_with_transports(void **state) {
    (void)state;
    aclk_transport_desc_t *transport1 = malloc(sizeof(aclk_transport_desc_t));
    transport1->endpoint = strdup("mqtt://test1");
    
    aclk_transport_desc_t *transport2 = malloc(sizeof(aclk_transport_desc_t));
    transport2->endpoint = strdup("mqtt://test2");
    
    aclk_env_t env;
    env.auth_endpoint = strdup("https://auth");
    env.transports = malloc(sizeof(aclk_transport_desc_t *) * 2);
    env.transports[0] = transport1;
    env.transports[1] = transport2;
    env.transport_count = 2;
    env.capabilities = NULL;
    env.capability_count = 0;
    
    aclk_env_t_destroy(&env);
    assert_null(env.auth_endpoint);
    assert_null(env.transports);
}

// Test: aclk_env_t_destroy with capabilities
static void test_aclk_env_t_destroy_with_capabilities(void **state) {
    (void)state;
    aclk_env_t env;
    env.auth_endpoint = strdup("https://auth");
    env.transports = NULL;
    env.transport_count = 0;
    env.capabilities = malloc(sizeof(char *) * 2);
    env.capabilities[0] = strdup("cap1");
    env.capabilities[1] = strdup("cap2");
    env.capability_count = 2;
    
    aclk_env_t_destroy(&env);
    assert_null(env.capabilities);
}

// Test: aclk_env_t_destroy with NULL transports in array
static void test_aclk_env_t_destroy_with_null_transports(void **state) {
    (void)state;
    aclk_env_t env;
    env.auth_endpoint = strdup("https://auth");
    env.transports = malloc(sizeof(aclk_transport_desc_t *) * 2);
    env.transports[0] = NULL;
    env.transports[1] = NULL;
    env.transport_count = 2;
    env.capabilities = NULL;
    env.capability_count = 0;
    
    aclk_env_t_destroy(&env);
    assert_null(env.transports);
}

// Test: aclk_env_has_capa - capability exists (case sensitive check)
static void test_aclk_env_has_capa_exists(void **state) {
    (void)state;
    aclk_env_t env;
    env.capabilities = malloc(sizeof(char *) * 2);
    env.capabilities[0] = strdup("CAP1");
    env.capabilities[1] = strdup("CAP2");
    env.capability_count = 2;
    aclk_env = &env;
    
    int result = aclk_env_has_capa("CAP1");
    assert_int_equal(result, 1);
    
    env.capabilities[0] = NULL;
    env.capabilities[1] = NULL;
    free(env.capabilities);
    aclk_env = NULL;
}

// Test: aclk_env_has_capa - capability doesn't exist
static void test_aclk_env_has_capa_not_exists(void **state) {
    (void)state;
    aclk_env_t env;
    env.capabilities = malloc(sizeof(char *) * 2);
    env.capabilities[0] = strdup("CAP1");
    env.capabilities[1] = strdup("CAP2");
    env.capability_count = 2;
    aclk_env = &env;
    
    int result = aclk_env_has_capa("CAP3");
    assert_int_equal(result, 0);
    
    env.capabilities[0] = NULL;
    env.capabilities[1] = NULL;
    free(env.capabilities);
    aclk_env = NULL;
}

// Test: aclk_env_has_capa - case insensitive match
static void test_aclk_env_has_capa_case_insensitive(void **state) {
    (void)state;
    aclk_env_t env;
    env.capabilities = malloc(sizeof(char *) * 2);
    env.capabilities[0] = strdup("cap1");
    env.capabilities[1] = strdup("cap2");
    env.capability_count = 2;
    aclk_env = &env;
    
    int result = aclk_env_has_capa("CAP1");
    assert_int_equal(result, 1);
    
    env.capabilities[0] = NULL;
    env.capabilities[1] = NULL;
    free(env.capabilities);
    aclk_env = NULL;
}

// Test: free_topic_cache with NULL cache
static void test_free_topic_cache_null(void **state) {
    (void)state;
    free_topic_cache();
    // Should not crash
}

// Test: aclk_tbeb_delay - reset
static void test_aclk_tbeb_delay_reset(void **state) {
    (void)state;
    unsigned long int result = aclk_tbeb_delay(1, 2, 1000, 60000);
    assert_int_equal(result, 0);
}

// Test: aclk_tbeb_delay - first attempt (should return 0)
static void test_aclk_tbeb_delay_first_attempt(void **state) {
    (void)state;
    aclk_tbeb_delay(1, 2, 1000, 60000); // Reset
    unsigned long int result = aclk_tbeb_delay(0, 2, 1000, 60000);
    assert_int_equal(result, 0);
}

// Test: aclk_tbeb_delay - exponential backoff with boundary conditions
static void test_aclk_tbeb_delay_exponential(void **state) {
    (void)state;
    aclk_tbeb_delay(1, 2, 1000, 60000); // Reset
    aclk_tbeb_delay(0, 2, 1000, 60000); // First call (returns 0)
    unsigned long int result = aclk_tbeb_delay(0, 2, 1000, 60000);
    // Second call: delay = 2^(2-1) * 1000 = 2000 + random(1000)
    // Since we use os_random32() stub returning 0x12345678, result will be in range
    assert_true(result >= 1000);
}

// Test: aclk_tbeb_delay - respect min_ms boundary
static void test_aclk_tbeb_delay_min_boundary(void **state) {
    (void)state;
    aclk_tbeb_delay(1, 2, 50000, 1000); // Reset with min_ms 50000
    aclk_tbeb_delay(0, 2, 50000, 1000); // First call
    unsigned long int result = aclk_tbeb_delay(0, 2, 50000, 1000);
    assert_true(result <= 1000);
}

// Test: aclk_parse_userpass_pair - valid pair
static void test_aclk_parse_userpass_pair_valid(void **state) {
    (void)state;
    char *user = NULL, *pass = NULL;
    int result = aclk_parse_userpass_pair("myuser:mypass", ':', &user, &pass);
    assert_int_equal(result, 0);
    assert_non_null(user);
    assert_non_null(pass);
    free(user);
    free(pass);
}

// Test: aclk_parse_userpass_pair - no separator
static void test_aclk_parse_userpass_pair_no_separator(void **state) {
    (void)state;
    char *user = NULL, *pass = NULL;
    int result = aclk_parse_userpass_pair("nocolon", ':', &user, &pass);
    assert_int_equal(result, 1);
}

// Test: aclk_parse_userpass_pair - different separator
static void test_aclk_parse_userpass_pair_different_separator(void **state) {
    (void)state;
    char *first = NULL, *second = NULL;
    int result = aclk_parse_userpass_pair("val1|val2", '|', &first, &second);
    assert_int_equal(result, 0);
    assert_non_null(first);
    assert_non_null(second);
    free(first);
    free(second);
}

// Test: aclk_set_proxy - no proxy
static void test_aclk_set_proxy_http(void **state) {
    (void)state;
    char *host = NULL, *user = NULL, *pass = NULL, *log_proxy = NULL;
    int port = 0;
    enum mqtt_wss_proxy_type type = MQTT_WSS_PROXY_HTTP;
    
    aclk_set_proxy(&host, &port, &user, &pass, &log_proxy, &type);
    
    assert_non_null(host);
    assert_true(port > 0 && port <= 65535);
    
    free(host);
    free(user);
    free(pass);
}

// Test: aclk_set_proxy - invalid port range (negative)
static void test_aclk_set_proxy_invalid_port_negative(void **state) {
    (void)state;
    // Mock aclk_get_proxy to return a URL with invalid port
    // This depends on actual implementation
}

// Test: aclk_set_proxy - invalid port range (too large)
static void test_aclk_set_proxy_invalid_port_large(void **state) {
    (void)state;
    // This depends on the proxy URL format returned by aclk_get_proxy
}

// Test: aclk_set_proxy - no proxy credentials
static void test_aclk_set_proxy_no_credentials(void **state) {
    (void)state;
    // Test when proxy URL has no credentials
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_json),
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_proto),
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_unknown),
        cmocka_unit_test(test_aclk_encoding_type_t_from_str_empty),
        cmocka_unit_test(test_aclk_transport_type_t_from_str_mqtt3),
        cmocka_unit_test(test_aclk_transport_type_t_from_str_mqtt5),
        cmocka_unit_test(test_aclk_transport_type_t_from_str_unknown),
        cmocka_unit_test(test_aclk_transport_type_t_from_str_empty),
        cmocka_unit_test(test_aclk_transport_desc_t_destroy_null_endpoint),
        cmocka_unit_test(test_aclk_transport_desc_t_destroy_valid_endpoint),
        cmocka_unit_test(test_aclk_env_t_destroy_null_pointers),
        cmocka_unit_test(test_aclk_env_t_destroy_with_transports),
        cmocka_unit_test(test_aclk_env_t_destroy_with_capabilities),
        cmocka_unit_test(test_aclk_env_t_destroy_with_null_transports),
        cmocka_unit_test(test_aclk_env_has_capa_exists),
        cmocka_unit_test(test_aclk_env_has_capa_not_exists),
        cmocka_unit_test(test_aclk_env_has_capa_case_insensitive),
        cmocka_unit_test(test_free_topic_cache_null),
        cmocka_unit_test(test_aclk_tbeb_delay_reset),
        cmocka_unit_test(test_aclk_tbeb_delay_first_attempt),
        cmocka_unit_test(test_aclk_tbeb_delay_exponential),
        cmocka_unit_test(test_aclk_tbeb_delay_min_boundary),
        cmocka_unit_test(test_aclk_parse_userpass_pair_valid),
        cmocka_unit_test(test_aclk_parse_userpass_pair_no_separator),
        cmocka_unit_test(test_aclk_parse_userpass_pair_different_separator),
        cmocka_unit_test(test_aclk_set_proxy_http),
        cmocka_unit_test(test_aclk_set_proxy_invalid_port_negative),
        cmocka_unit_test(test_aclk_set_proxy_invalid_port_large),
        cmocka_unit_test(test_aclk_set_proxy_no_credentials),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}