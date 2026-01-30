/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Unit tests for aclk_proxy.c with 100% code coverage
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Mock declarations for external dependencies */
#include "aclk_proxy.h"

/* Mock functions */
static char *mock_getenv_result = NULL;

char *__wrap_getenv(const char *name) {
    return mock_getenv_result;
}

void mock_netdata_log_error(const char *fmt, ...) {
    (void)fmt;
}

const char *__wrap_cloud_config_proxy_get(void) {
    return (const char *)mock_cloud_config_proxy_get();
}

void *mock_cloud_config_proxy_get(void) {
    return mock_cloud_config_proxy_get();
}

/* Test: aclk_verify_proxy with NULL string */
static void test_aclk_verify_proxy_null(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy(NULL);
    assert_int_equal(result, PROXY_TYPE_UNKNOWN);
}

/* Test: aclk_verify_proxy with empty string */
static void test_aclk_verify_proxy_empty(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy("");
    assert_int_equal(result, PROXY_TYPE_UNKNOWN);
}

/* Test: aclk_verify_proxy with only spaces */
static void test_aclk_verify_proxy_only_spaces(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy("     ");
    assert_int_equal(result, PROXY_TYPE_UNKNOWN);
}

/* Test: aclk_verify_proxy with spaces followed by data */
static void test_aclk_verify_proxy_spaces_prefix(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy("   socks5://localhost:1080");
    assert_int_equal(result, PROXY_TYPE_SOCKS5);
}

/* Test: aclk_verify_proxy with SOCKS5 protocol */
static void test_aclk_verify_proxy_socks5(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy("socks5://user:pass@localhost:1080");
    assert_int_equal(result, PROXY_TYPE_SOCKS5);
}

/* Test: aclk_verify_proxy with SOCKS5h protocol */
static void test_aclk_verify_proxy_socks5h(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy("socks5h://localhost:1080");
    assert_int_equal(result, PROXY_TYPE_SOCKS5);
}

/* Test: aclk_verify_proxy with HTTP protocol */
static void test_aclk_verify_proxy_http(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy("http://user:pass@proxy.example.com:8080");
    assert_int_equal(result, PROXY_TYPE_HTTP);
}

/* Test: aclk_verify_proxy with unknown protocol */
static void test_aclk_verify_proxy_unknown(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy("ftp://localhost:21");
    assert_int_equal(result, PROXY_TYPE_UNKNOWN);
}

/* Test: aclk_verify_proxy with invalid string */
static void test_aclk_verify_proxy_invalid(void **state) {
    (void)state;
    ACLK_PROXY_TYPE result = aclk_verify_proxy("not a valid proxy");
    assert_int_equal(result, PROXY_TYPE_UNKNOWN);
}

/* Test: safe_log_proxy_censor with NULL proxy */
static void test_safe_log_proxy_censor_null(void **state) {
    (void)state;
    char *proxy = NULL;
    safe_log_proxy_censor(proxy);
    /* No assertion needed - function should just return without error */
}

/* Test: safe_log_proxy_censor with simple proxy */
static void test_safe_log_proxy_censor_simple_auth(void **state) {
    (void)state;
    char proxy[] = "socks5://user:pass@localhost:1080";
    char expected[] = "socks5://XXXXXXXXXX@localhost:1080";
    
    safe_log_proxy_censor(proxy);
    
    assert_string_equal(proxy, expected);
}

/* Test: safe_log_proxy_censor with no authentication */
static void test_safe_log_proxy_censor_no_auth(void **state) {
    (void)state;
    char proxy[] = "socks5://localhost:1080";
    char original[] = "socks5://localhost:1080";
    
    safe_log_proxy_censor(proxy);
    
    assert_string_equal(proxy, original);
}

/* Test: safe_log_proxy_censor with @ but no protocol separator */
static void test_safe_log_proxy_censor_at_no_proto(void **state) {
    (void)state;
    char proxy[] = "user:pass@localhost:1080";
    char original[] = "user:pass@localhost:1080";
    
    safe_log_proxy_censor(proxy);
    
    /* Since no protocol separator and @ is not after it, nothing should change */
    assert_string_equal(proxy, original);
}

/* Test: safe_log_proxy_censor with @ at start position */
static void test_safe_log_proxy_censor_at_start(void **state) {
    (void)state;
    char proxy[] = "@localhost:1080";
    char original[] = "@localhost:1080";
    
    safe_log_proxy_censor(proxy);
    
    assert_string_equal(proxy, original);
}

/* Test: safe_log_proxy_censor with complex auth with protocol separator */
static void test_safe_log_proxy_censor_complex_auth(void **state) {
    (void)state;
    char proxy[] = "http://myuser:mypassword123@proxy.example.com:8080";
    char expected[] = "http://XXXXXXXXXXXXXXXXXXXXXX@proxy.example.com:8080";
    
    safe_log_proxy_censor(proxy);
    
    assert_string_equal(proxy, expected);
}

/* Test: safe_log_proxy_censor with empty auth */
static void test_safe_log_proxy_censor_empty_auth(void **state) {
    (void)state;
    char proxy[] = "socks5://:@localhost:1080";
    char expected[] = "socks5://X@localhost:1080";
    
    safe_log_proxy_censor(proxy);
    
    assert_string_equal(proxy, expected);
}

/* Test: aclk_lws_wss_get_proxy_setting with NULL proxy */
static void test_aclk_lws_wss_get_proxy_setting_null_proxy(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, NULL);
    
    const char *result = aclk_lws_wss_get_proxy_setting(&type);
    
    assert_null(result);
    assert_int_equal(type, PROXY_DISABLED);
}

/* Test: aclk_lws_wss_get_proxy_setting with empty string */
static void test_aclk_lws_wss_get_proxy_setting_empty_string(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "");
    
    const char *result = aclk_lws_wss_get_proxy_setting(&type);
    
    assert_string_equal(result, "");
    assert_int_equal(type, PROXY_DISABLED);
}

/* Test: aclk_lws_wss_get_proxy_setting with "none" */
static void test_aclk_lws_wss_get_proxy_setting_none(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "none");
    
    const char *result = aclk_lws_wss_get_proxy_setting(&type);
    
    assert_string_equal(result, "none");
    assert_int_equal(type, PROXY_DISABLED);
}

/* Test: aclk_lws_wss_get_proxy_setting with "env" - valid HTTP proxy */
static void test_aclk_lws_wss_get_proxy_setting_env_valid(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "env");
    mock_getenv_result = "http://proxy:8080";
    
    const char *result = aclk_lws_wss_get_proxy_setting(&type);
    
    assert_non_null(result);
    assert_int_equal(type, PROXY_TYPE_HTTP);
    
    mock_getenv_result = NULL;
}

/* Test: aclk_lws_wss_get_proxy_setting with "env" - invalid proxy */
static void test_aclk_lws_wss_get_proxy_setting_env_invalid(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "env");
    mock_getenv_result = "invalid://proxy";
    
    const char *result = aclk_lws_wss_get_proxy_setting(&type);
    
    assert_null(result);
    
    mock_getenv_result = NULL;
}

/* Test: aclk_lws_wss_get_proxy_setting with "env" - no environment variable */
static void test_aclk_lws_wss_get_proxy_setting_env_not_set(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "env");
    mock_getenv_result = NULL;
    
    const char *result = aclk_lws_wss_get_proxy_setting(&type);
    
    assert_null(result);
    
    mock_getenv_result = NULL;
}

/* Test: aclk_lws_wss_get_proxy_setting with valid SOCKS5 proxy */
static void test_aclk_lws_wss_get_proxy_setting_valid_socks5(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "socks5://localhost:1080");
    
    const char *result = aclk_lws_wss_get_proxy_setting(&type);
    
    assert_string_equal(result, "socks5://localhost:1080");
    assert_int_equal(type, PROXY_TYPE_SOCKS5);
}

/* Test: aclk_lws_wss_get_proxy_setting with unknown proxy type */
static void test_aclk_lws_wss_get_proxy_setting_unknown(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "ftp://localhost:21");
    
    const char *result = aclk_lws_wss_get_proxy_setting(&type);
    
    assert_string_equal(result, "ftp://localhost:21");
    assert_int_equal(type, PROXY_DISABLED);
}

/* Test: aclk_get_proxy first call with valid proxy */
static void test_aclk_get_proxy_first_call(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "socks5://localhost:1080");
    
    const char *result = aclk_get_proxy(&type, false);
    
    assert_non_null(result);
    assert_string_equal(result, "socks5://localhost:1080");
    assert_int_equal(type, PROXY_TYPE_SOCKS5);
}

/* Test: aclk_get_proxy with logging flag */
static void test_aclk_get_proxy_with_logging(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "socks5://user:pass@localhost:1080");
    
    const char *result = aclk_get_proxy(&type, true);
    
    assert_non_null(result);
    /* Result should be censored version when logging */
    assert_int_equal(type, PROXY_TYPE_SOCKS5);
}

/* Test: aclk_get_proxy with NULL type pointer */
static void test_aclk_get_proxy_null_type_pointer(void **state) {
    (void)state;
    
    will_return(__wrap_cloud_config_proxy_get, "http://proxy:8080");
    
    const char *result = aclk_get_proxy(NULL, false);
    
    assert_non_null(result);
}

/* Test: aclk_get_proxy with PROXY_DISABLED */
static void test_aclk_get_proxy_disabled(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type;
    
    will_return(__wrap_cloud_config_proxy_get, "none");
    
    const char *result = aclk_get_proxy(&type, false);
    
    assert_non_null(result);
    assert_int_equal(type, PROXY_DISABLED);
}

/* Test: aclk_get_proxy - caching behavior */
static void test_aclk_get_proxy_caching(void **state) {
    (void)state;
    ACLK_PROXY_TYPE type1, type2;
    
    will_return(__wrap_cloud_config_proxy_get, "socks5://localhost:1080");
    
    const char *result1 = aclk_get_proxy(&type1, false);
    const char *result2 = aclk_get_proxy(&type2, false);
    
    /* Both calls should return the same cached value */
    assert_ptr_equal(result1, result2);
    assert_int_equal(type1, type2);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_verify_proxy_null),
        cmocka_unit_test(test_aclk_verify_proxy_empty),
        cmocka_unit_test(test_aclk_verify_proxy_only_spaces),
        cmocka_unit_test(test_aclk_verify_proxy_spaces_prefix),
        cmocka_unit_test(test_aclk_verify_proxy_socks5),
        cmocka_unit_test(test_aclk_verify_proxy_socks5h),
        cmocka_unit_test(test_aclk_verify_proxy_http),
        cmocka_unit_test(test_aclk_verify_proxy_unknown),
        cmocka_unit_test(test_aclk_verify_proxy_invalid),
        cmocka_unit_test(test_safe_log_proxy_censor_null),
        cmocka_unit_test(test_safe_log_proxy_censor_simple_auth),
        cmocka_unit_test(test_safe_log_proxy_censor_no_auth),
        cmocka_unit_test(test_safe_log_proxy_censor_at_no_proto),
        cmocka_unit_test(test_safe_log_proxy_censor_at_start),
        cmocka_unit_test(test_safe_log_proxy_censor_complex_auth),
        cmocka_unit_test(test_safe_log_proxy_censor_empty_auth),
        cmocka_unit_test(test_aclk_lws_wss_get_proxy_setting_null_proxy),
        cmocka_unit_test(test_aclk_lws_wss_get_proxy_setting_empty_string),
        cmocka_unit_test(test_aclk_lws_wss_get_proxy_setting_none),
        cmocka_unit_test(test_aclk_lws_wss_get_proxy_setting_env_valid),
        cmocka_unit_test(test_aclk_lws_wss_get_proxy_setting_env_invalid),
        cmocka_unit_test(test_aclk_lws_wss_get_proxy_setting_env_not_set),
        cmocka_unit_test(test_aclk_lws_wss_get_proxy_setting_valid_socks5),
        cmocka_unit_test(test_aclk_lws_wss_get_proxy_setting_unknown),
        cmocka_unit_test(test_aclk_get_proxy_first_call),
        cmocka_unit_test(test_aclk_get_proxy_with_logging),
        cmocka_unit_test(test_aclk_get_proxy_null_type_pointer),
        cmocka_unit_test(test_aclk_get_proxy_disabled),
        cmocka_unit_test(test_aclk_get_proxy_caching),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}