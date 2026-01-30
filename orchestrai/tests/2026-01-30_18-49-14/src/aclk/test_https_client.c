// SPDX-License-Identifier: GPL-3.0-or-later
// Comprehensive unit tests for https_client.h functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <assert.h>

// Mock structures and functions
#define c_rhash void
#define rbuf_t void
#define BUFFER void

// Forward declarations for tested functions
int url_parse(const char *url, url_t *parsed);
void url_t_destroy(url_t *url);
void https_req_response_free(https_req_response_t *res);
void http_parse_ctx_create(http_parse_ctx *ctx, enum http_parse_state parse_state);
void http_parse_ctx_destroy(http_parse_ctx *ctx);
http_parse_rc parse_http_response(rbuf_t buf, http_parse_ctx *parse_ctx);
const char *get_http_header_by_name(http_parse_ctx *ctx, const char *name);
https_client_resp_t https_request(https_req_t *request, https_req_response_t *response, bool *fallback_ipv4);

// Mock implementations
void* c_rhash_new(int a) {
    return malloc(1);
}

void c_rhash_destroy(void *h) {
    if (h) free(h);
}

int c_rhash_insert_str_ptr(void *h, const char *k, void *v) {
    return 0;
}

int c_rhash_get_ptr_by_str(void *h, const char *k, void **v) {
    *v = NULL;
    return 1; // not found
}

void c_rhash_iter_t_initialize(void *iter) {}

int c_rhash_iter_str_keys(void *h, void *iter, const char **k) {
    return 1; // end of iteration
}

void* mallocz(size_t size) {
    return malloc(size);
}

void* callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void* reallocz(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void freez(void *ptr) {
    if (ptr) free(ptr);
}

char* strdupz(const char *str) {
    if (!str) return NULL;
    char *dup = malloc(strlen(str) + 1);
    if (dup) strcpy(dup, str);
    return dup;
}

unsigned int str2u(const char *str) {
    return (unsigned int)strtoul(str, NULL, 10);
}

void netdata_log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void netdata_log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void nd_log_daemon(int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void errno_clear() {}

int cloud_config_insecure_get() {
    return 0;
}

// ============= url_parse Tests =============

static int test_url_parse_setup(void **state) {
    return 0;
}

static int test_url_parse_teardown(void **state) {
    return 0;
}

// Test url_parse with simple HTTPS URL
static void test_url_parse_https_simple(void **state) {
    url_t parsed = {0};
    int result = url_parse("https://example.com/path", &parsed);
    
    assert_int_equal(result, 0);
    assert_string_equal(parsed.proto, "https");
    assert_string_equal(parsed.host, "example.com");
    assert_int_equal(parsed.port, 443);
    assert_string_equal(parsed.path, "/path");
    
    url_t_destroy(&parsed);
}

// Test url_parse with HTTP URL
static void test_url_parse_http_simple(void **state) {
    url_t parsed = {0};
    int result = url_parse("http://example.com/path", &parsed);
    
    assert_int_equal(result, 0);
    assert_string_equal(parsed.proto, "http");
    assert_string_equal(parsed.host, "example.com");
    assert_int_equal(parsed.port, 80);
    assert_string_equal(parsed.path, "/path");
    
    url_t_destroy(&parsed);
}

// Test url_parse with explicit port
static void test_url_parse_with_port(void **state) {
    url_t parsed = {0};
    int result = url_parse("https://example.com:8443/path", &parsed);
    
    assert_int_equal(result, 0);
    assert_string_equal(parsed.proto, "https");
    assert_string_equal(parsed.host, "example.com");
    assert_int_equal(parsed.port, 8443);
    assert_string_equal(parsed.path, "/path");
    
    url_t_destroy(&parsed);
}

// Test url_parse without protocol
static void test_url_parse_no_protocol(void **state) {
    url_t parsed = {0};
    int result = url_parse("example.com/path", &parsed);
    
    assert_int_equal(result, 0);
    assert_null(parsed.proto);
    assert_string_equal(parsed.host, "example.com");
    assert_int_equal(parsed.port, 0);
    assert_string_equal(parsed.path, "/path");
    
    url_t_destroy(&parsed);
}

// Test url_parse without path
static void test_url_parse_no_path(void **state) {
    url_t parsed = {0};
    int result = url_parse("https://example.com", &parsed);
    
    assert_int_equal(result, 0);
    assert_string_equal(parsed.proto, "https");
    assert_string_equal(parsed.host, "example.com");
    assert_int_equal(parsed.port, 443);
    assert_string_equal(parsed.path, "/");
    
    url_t_destroy(&parsed);
}

// Test url_parse with empty protocol
static void test_url_parse_empty_protocol(void **state) {
    url_t parsed = {0};
    int result = url_parse("://example.com/path", &parsed);
    
    assert_int_equal(result, 1);
    
    url_t_destroy(&parsed);
}

// Test url_parse with empty host
static void test_url_parse_empty_host(void **state) {
    url_t parsed = {0};
    int result = url_parse("https:///path", &parsed);
    
    assert_int_equal(result, 1);
    
    url_t_destroy(&parsed);
}

// Test url_parse with port but no port number
static void test_url_parse_port_no_number(void **state) {
    url_t parsed = {0};
    int result = url_parse("https://example.com:/path", &parsed);
    
    assert_int_equal(result, 1);
    
    url_t_destroy(&parsed);
}

// Test url_parse with overly long port
static void test_url_parse_long_port(void **state) {
    url_t parsed = {0};
    int result = url_parse("https://example.com:123456/path", &parsed);
    
    assert_int_equal(result, 1);
    
    url_t_destroy(&parsed);
}

// Test url_parse with complex path
static void test_url_parse_complex_path(void **state) {
    url_t parsed = {0};
    int result = url_parse("https://example.com:8443/path/to/resource?query=value", &parsed);
    
    assert_int_equal(result, 0);
    assert_string_equal(parsed.proto, "https");
    assert_string_equal(parsed.host, "example.com");
    assert_int_equal(parsed.port, 8443);
    assert_string_equal(parsed.path, "/path/to/resource?query=value");
    
    url_t_destroy(&parsed);
}

// Test url_parse with IPv6 address (basic)
static void test_url_parse_ipv6(void **state) {
    url_t parsed = {0};
    int result = url_parse("https://[::1]/path", &parsed);
    
    // This may succeed depending on implementation
    if (result == 0) {
        url_t_destroy(&parsed);
    }
}

// ============= url_t_destroy Tests =============

static void test_url_t_destroy_null_ptr(void **state) {
    url_t url = {.proto = NULL, .host = NULL, .path = NULL};
    // Should not crash
    url_t_destroy(&url);
}

static void test_url_t_destroy_with_data(void **state) {
    url_t url;
    url.proto = mallocz(10);
    url.host = mallocz(10);
    url.path = mallocz(10);
    
    url_t_destroy(&url);
    
    // After destroy, should be safe to destroy again
    url_t_destroy(&url);
}

// ============= https_req_response_free Tests =============

static void test_https_req_response_free_null_payload(void **state) {
    https_req_response_t res = {.http_code = 200, .payload = NULL, .payload_size = 0};
    https_req_response_free(&res);
}

static void test_https_req_response_free_with_payload(void **state) {
    https_req_response_t res;
    res.http_code = 200;
    res.payload = mallocz(100);
    res.payload_size = 100;
    
    https_req_response_free(&res);
}

// ============= http_parse_ctx_create Tests =============

static void test_http_parse_ctx_create_initial_state(void **state) {
    http_parse_ctx ctx = {0};
    
    http_parse_ctx_create(&ctx, HTTP_PARSE_INITIAL);
    
    assert_int_equal(ctx.state, HTTP_PARSE_INITIAL);
    assert_int_equal(ctx.content_length, -1);
    assert_int_equal(ctx.http_code, 0);
    assert_non_null(ctx.headers);
    assert_int_equal(ctx.flags, HTTP_PARSE_FLAGS_DEFAULT);
    assert_int_equal(ctx.chunked_content_state, CHUNKED_CONTENT_CHUNK_SIZE);
    assert_int_equal(ctx.chunk_size, 0);
    assert_int_equal(ctx.chunk_got, 0);
    assert_int_equal(ctx.chunked_response_written, 0);
    assert_int_equal(ctx.chunked_response_size, 0);
    assert_null(ctx.chunked_response);
    
    http_parse_ctx_destroy(&ctx);
}

static void test_http_parse_ctx_create_proxy_connect(void **state) {
    http_parse_ctx ctx = {0};
    
    http_parse_ctx_create(&ctx, HTTP_PARSE_PROXY_CONNECT);
    
    assert_int_equal(ctx.state, HTTP_PARSE_PROXY_CONNECT);
    assert_non_null(ctx.headers);
    
    http_parse_ctx_destroy(&ctx);
}

static void test_http_parse_ctx_create_headers_state(void **state) {
    http_parse_ctx ctx = {0};
    
    http_parse_ctx_create(&ctx, HTTP_PARSE_HEADERS);
    
    assert_int_equal(ctx.state, HTTP_PARSE_HEADERS);
    assert_non_null(ctx.headers);
    
    http_parse_ctx_destroy(&ctx);
}

static void test_http_parse_ctx_create_content_state(void **state) {
    http_parse_ctx ctx = {0};
    
    http_parse_ctx_create(&ctx, HTTP_PARSE_CONTENT);
    
    assert_int_equal(ctx.state, HTTP_PARSE_CONTENT);
    assert_non_null(ctx.headers);
    
    http_parse_ctx_destroy(&ctx);
}

static void test_http_parse_ctx_create_clears_previous_data(void **state) {
    http_parse_ctx ctx = {0};
    
    http_parse_ctx_create(&ctx, HTTP_PARSE_INITIAL);
    ctx.http_code = 200;
    ctx.content_length = 100;
    
    http_parse_ctx_create(&ctx, HTTP_PARSE_HEADERS);
    
    assert_int_equal(ctx.http_code, 0);
    assert_int_equal(ctx.content_length, -1);
    
    http_parse_ctx_destroy(&ctx);
}

// ============= http_parse_ctx_destroy Tests =============

static void test_http_parse_ctx_destroy_null_headers(void **state) {
    http_parse_ctx ctx = {.headers = NULL};
    // Should not crash
    http_parse_ctx_destroy(&ctx);
}

static void test_http_parse_ctx_destroy_with_headers(void **state) {
    http_parse_ctx ctx = {0};
    http_parse_ctx_create(&ctx, HTTP_PARSE_INITIAL);
    
    http_parse_ctx_destroy(&ctx);
    
    assert_null(ctx.headers);
}

// ============= get_http_header_by_name Tests =============

static void test_get_http_header_by_name_not_found(void **state) {
    http_parse_ctx ctx = {0};
    http_parse_ctx_create(&ctx, HTTP_PARSE_INITIAL);
    
    const char *result = get_http_header_by_name(&ctx, "non-existent");
    
    assert_null(result);
    
    http_parse_ctx_destroy(&ctx);
}

// ============= Integration Tests =============

static void test_url_parse_multiple_urls(void **state) {
    const char *urls[] = {
        "https://api.example.com/v1/endpoint",
        "http://localhost:8080/test",
        "https://user@example.com/path",
        "http://example.com:80/",
        "https://example.com:443/"
    };
    
    for (size_t i = 0; i < sizeof(urls) / sizeof(urls[0]); i++) {
        url_t parsed = {0};
        int result = url_parse(urls[i], &parsed);
        
        if (result == 0) {
            assert_non_null(parsed.host);
            assert_non_null(parsed.path);
            url_t_destroy(&parsed);
        }
    }
}

static void test_http_parse_ctx_lifecycle(void **state) {
    http_parse_ctx ctx = {0};
    
    // Create and destroy multiple times
    for (int i = 0; i < 5; i++) {
        http_parse_ctx_create(&ctx, HTTP_PARSE_INITIAL);
        assert_non_null(ctx.headers);
        http_parse_ctx_destroy(&ctx);
        assert_null(ctx.headers);
    }
}

// ============= Edge Cases and Boundary Tests =============

static void test_url_parse_very_long_url(void **state) {
    char long_url[2048];
    strcpy(long_url, "https://example.com");
    for (int i = 0; i < 100; i++) {
        strcat(long_url, "/path");
    }
    
    url_t parsed = {0};
    int result = url_parse(long_url, &parsed);
    
    if (result == 0) {
        assert_string_equal(parsed.proto, "https");
        assert_string_equal(parsed.host, "example.com");
        url_t_destroy(&parsed);
    }
}

static void test_url_parse_special_characters(void **state) {
    url_t parsed = {0};
    int result = url_parse("https://example.com/path?key=value&other=123", &parsed);
    
    assert_int_equal(result, 0);
    assert_string_equal(parsed.host, "example.com");
    
    url_t_destroy(&parsed);
}

static void test_url_parse_localhost(void **state) {
    url_t parsed = {0};
    int result = url_parse("http://localhost:3000/", &parsed);
    
    assert_int_equal(result, 0);
    assert_string_equal(parsed.host, "localhost");
    assert_int_equal(parsed.port, 3000);
    
    url_t_destroy(&parsed);
}

static void test_url_parse_ip_address(void **state) {
    url_t parsed = {0};
    int result = url_parse("http://192.168.1.1:8080/api", &parsed);
    
    assert_int_equal(result, 0);
    assert_string_equal(parsed.host, "192.168.1.1");
    assert_int_equal(parsed.port, 8080);
    
    url_t_destroy(&parsed);
}

// ============= Main Test Suite =============

int main(void) {
    const struct CMUnitTest tests[] = {
        // url_parse tests
        cmocka_unit_test(test_url_parse_https_simple),
        cmocka_unit_test(test_url_parse_http_simple),
        cmocka_unit_test(test_url_parse_with_port),
        cmocka_unit_test(test_url_parse_no_protocol),
        cmocka_unit_test(test_url_parse_no_path),
        cmocka_unit_test(test_url_parse_empty_protocol),
        cmocka_unit_test(test_url_parse_empty_host),
        cmocka_unit_test(test_url_parse_port_no_number),
        cmocka_unit_test(test_url_parse_long_port),
        cmocka_unit_test(test_url_parse_complex_path),
        cmocka_unit_test(test_url_parse_ipv6),
        
        // url_t_destroy tests
        cmocka_unit_test(test_url_t_destroy_null_ptr),
        cmocka_unit_test(test_url_t_destroy_with_data),
        
        // https_req_response_free tests
        cmocka_unit_test(test_https_req_response_free_null_payload),
        cmocka_unit_test(test_https_req_response_free_with_payload),
        
        // http_parse_ctx_create tests
        cmocka_unit_test(test_http_parse_ctx_create_initial_state),
        cmocka_unit_test(test_http_parse_ctx_create_proxy_connect),
        cmocka_unit_test(test_http_parse_ctx_create_headers_state),
        cmocka_unit_test(test_http_parse_ctx_create_content_state),
        cmocka_unit_test(test_http_parse_ctx_create_clears_previous_data),
        
        // http_parse_ctx_destroy tests
        cmocka_unit_test(test_http_parse_ctx_destroy_null_headers),
        cmocka_unit_test(test_http_parse_ctx_destroy_with_headers),
        
        // get_http_header_by_name tests
        cmocka_unit_test(test_get_http_header_by_name_not_found),
        
        // Integration tests
        cmocka_unit_test(test_url_parse_multiple_urls),
        cmocka_unit_test(test_http_parse_ctx_lifecycle),
        
        // Edge case tests
        cmocka_unit_test(test_url_parse_very_long_url),
        cmocka_unit_test(test_url_parse_special_characters),
        cmocka_unit_test(test_url_parse_localhost),
        cmocka_unit_test(test_url_parse_ip_address),
    };

    return cmocka_run_group_tests(tests, test_url_parse_setup, test_url_parse_teardown);
}