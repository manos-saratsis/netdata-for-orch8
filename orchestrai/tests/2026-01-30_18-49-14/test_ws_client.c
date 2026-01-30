// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cmocka.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

// Mock structures and types
typedef struct rbuf_t {
    char *buffer;
    size_t size;
    size_t capacity;
    size_t read_pos;
    size_t write_pos;
} rbuf_t;

// Forward declarations for mocked functions
rbuf_t *rbuf_create(size_t size);
void rbuf_free(rbuf_t *buf);
void rbuf_flush(rbuf_t *buf);
void rbuf_push(rbuf_t *buf, const char *data, size_t size);
size_t rbuf_bytes_available(rbuf_t *buf);
size_t rbuf_bytes_free(rbuf_t *buf);
void rbuf_pop(rbuf_t *buf, char *data, size_t size);
void rbuf_bump_tail(rbuf_t *buf, size_t size);
void rbuf_bump_head(rbuf_t *buf, size_t size);
char *rbuf_find_bytes(rbuf_t *buf, const char *bytes, size_t size, int *idx);
int rbuf_memcmp_n(rbuf_t *buf, const char *cmp, size_t size);
char *rbuf_get_linear_insert_range(rbuf_t *buf, size_t *size);
size_t rbuf_get_capacity(rbuf_t *buf);

void nd_log(int facility, int level, const char *fmt, ...);
void *callocz(size_t count, size_t size);
void freez(void *ptr);
void os_random_bytes(unsigned char *buf, size_t size);
uint32_t os_random32(void);
void netdata_base64_encode(unsigned char *out, unsigned char *in, size_t len);
void worker_is_busy(int worker_id);

// Include the actual header
#include "ws_client.h"

// ==========================
// Test Fixtures
// ==========================

static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    return 0;
}

// ==========================
// ws_client_new Tests
// ==========================

void test_ws_client_new_with_valid_host_and_zero_size(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    
    ws_client *client = ws_client_new(0, host_ptr);
    
    assert_non_null(client);
    assert_ptr_equal(client->host, host_ptr);
    assert_non_null(client->buf_read);
    assert_non_null(client->buf_write);
    assert_non_null(client->buf_to_mqtt);
    
    ws_client_destroy(client);
}

void test_ws_client_new_with_valid_host_and_custom_size(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    size_t custom_size = 2048;
    
    ws_client *client = ws_client_new(custom_size, host_ptr);
    
    assert_non_null(client);
    assert_ptr_equal(client->host, host_ptr);
    assert_non_null(client->buf_read);
    assert_non_null(client->buf_write);
    assert_non_null(client->buf_to_mqtt);
    
    ws_client_destroy(client);
}

void test_ws_client_new_with_null_host_ptr(void **state) {
    ws_client *client = ws_client_new(2048, NULL);
    
    assert_null(client);
}

void test_ws_client_new_initializes_state_to_ws_raw(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    
    ws_client *client = ws_client_new(0, host_ptr);
    
    // Check that initial state is set correctly
    assert_int_equal(client->state, WS_RAW);
    assert_int_equal(client->hs.hdr_state, WS_HDR_HTTP);
    assert_int_equal(client->rx.parse_state, WS_FIRST_2BYTES);
    assert_false(client->rx.remote_closed);
    assert_null(client->hs.nonce_reply);
    assert_null(client->hs.http_reply_msg);
    assert_null(client->hs.headers);
    assert_null(client->hs.headers_tail);
    assert_int_equal(client->hs.hdr_count, 0);
    
    ws_client_destroy(client);
}

// ==========================
// ws_client_free_headers Tests
// ==========================

void test_ws_client_free_headers_with_null_headers(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    ws_client_free_headers(client);
    
    assert_null(client->hs.headers);
    assert_null(client->hs.headers_tail);
    assert_int_equal(client->hs.hdr_count, 0);
    
    ws_client_destroy(client);
}

void test_ws_client_free_headers_with_single_header(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    struct http_header *hdr = calloc(1, sizeof(struct http_header) + 50);
    hdr->key = ((char*)hdr) + sizeof(struct http_header);
    hdr->value = hdr->key + 20;
    hdr->next = NULL;
    strcpy(hdr->key, "Content-Type");
    strcpy(hdr->value, "text/plain");
    
    client->hs.headers = hdr;
    client->hs.headers_tail = hdr;
    client->hs.hdr_count = 1;
    
    ws_client_free_headers(client);
    
    assert_null(client->hs.headers);
    assert_null(client->hs.headers_tail);
    assert_int_equal(client->hs.hdr_count, 0);
    
    ws_client_destroy(client);
}

void test_ws_client_free_headers_with_multiple_headers(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    // Create three headers
    struct http_header *hdr1 = calloc(1, sizeof(struct http_header) + 50);
    hdr1->key = ((char*)hdr1) + sizeof(struct http_header);
    hdr1->value = hdr1->key + 20;
    strcpy(hdr1->key, "header1");
    strcpy(hdr1->value, "value1");
    
    struct http_header *hdr2 = calloc(1, sizeof(struct http_header) + 50);
    hdr2->key = ((char*)hdr2) + sizeof(struct http_header);
    hdr2->value = hdr2->key + 20;
    strcpy(hdr2->key, "header2");
    strcpy(hdr2->value, "value2");
    
    struct http_header *hdr3 = calloc(1, sizeof(struct http_header) + 50);
    hdr3->key = ((char*)hdr3) + sizeof(struct http_header);
    hdr3->value = hdr3->key + 20;
    strcpy(hdr3->key, "header3");
    strcpy(hdr3->value, "value3");
    
    hdr1->next = hdr2;
    hdr2->next = hdr3;
    hdr3->next = NULL;
    
    client->hs.headers = hdr1;
    client->hs.headers_tail = hdr3;
    client->hs.hdr_count = 3;
    
    ws_client_free_headers(client);
    
    assert_null(client->hs.headers);
    assert_null(client->hs.headers_tail);
    assert_int_equal(client->hs.hdr_count, 0);
    
    ws_client_destroy(client);
}

// ==========================
// ws_client_destroy Tests
// ==========================

void test_ws_client_destroy_with_valid_client(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    // Add some data
    client->hs.nonce_reply = malloc(100);
    strcpy(client->hs.nonce_reply, "test_nonce");
    client->hs.http_reply_msg = malloc(100);
    strcpy(client->hs.http_reply_msg, "test_message");
    
    // This should not crash
    ws_client_destroy(client);
}

void test_ws_client_destroy_with_null_fields(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    // All fields should be null initially
    // This should not crash
    ws_client_destroy(client);
}

void test_ws_client_destroy_with_headers(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    struct http_header *hdr = calloc(1, sizeof(struct http_header) + 50);
    hdr->key = ((char*)hdr) + sizeof(struct http_header);
    hdr->value = hdr->key + 20;
    hdr->next = NULL;
    
    client->hs.headers = hdr;
    client->hs.headers_tail = hdr;
    client->hs.hdr_count = 1;
    
    ws_client_destroy(client);
}

// ==========================
// ws_client_reset Tests
// ==========================

void test_ws_client_reset_clears_headers(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    struct http_header *hdr = calloc(1, sizeof(struct http_header) + 50);
    hdr->key = ((char*)hdr) + sizeof(struct http_header);
    hdr->value = hdr->key + 20;
    hdr->next = NULL;
    
    client->hs.headers = hdr;
    client->hs.headers_tail = hdr;
    client->hs.hdr_count = 1;
    
    ws_client_reset(client);
    
    assert_null(client->hs.headers);
    assert_null(client->hs.headers_tail);
    assert_int_equal(client->hs.hdr_count, 0);
    
    ws_client_destroy(client);
}

void test_ws_client_reset_clears_nonce_reply(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    client->hs.nonce_reply = malloc(100);
    strcpy(client->hs.nonce_reply, "test_nonce");
    
    ws_client_reset(client);
    
    assert_null(client->hs.nonce_reply);
    
    ws_client_destroy(client);
}

void test_ws_client_reset_clears_http_reply_msg(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    client->hs.http_reply_msg = malloc(100);
    strcpy(client->hs.http_reply_msg, "test_message");
    
    ws_client_reset(client);
    
    assert_null(client->hs.http_reply_msg);
    
    ws_client_destroy(client);
}

void test_ws_client_reset_resets_state(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    // Change state
    client->state = WS_ESTABLISHED;
    client->hs.hdr_state = WS_HDR_PARSE_HEADERS;
    client->rx.parse_state = WS_PAYLOAD_DATA;
    client->rx.remote_closed = true;
    
    ws_client_reset(client);
    
    assert_int_equal(client->state, WS_RAW);
    assert_int_equal(client->hs.hdr_state, WS_HDR_HTTP);
    assert_int_equal(client->rx.parse_state, WS_FIRST_2BYTES);
    assert_false(client->rx.remote_closed);
    
    ws_client_destroy(client);
}

// ==========================
// ws_client_add_http_header Tests
// ==========================

void test_ws_client_add_http_header_to_empty_list(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    struct http_header *hdr = calloc(1, sizeof(struct http_header) + 50);
    hdr->key = ((char*)hdr) + sizeof(struct http_header);
    hdr->value = hdr->key + 20;
    hdr->next = NULL;
    strcpy(hdr->key, "test-header");
    strcpy(hdr->value, "test-value");
    
    int result = ws_client_add_http_header(client, hdr);
    
    assert_int_equal(result, 0);
    assert_ptr_equal(client->hs.headers, hdr);
    assert_ptr_equal(client->hs.headers_tail, hdr);
    assert_int_equal(client->hs.hdr_count, 1);
    
    ws_client_destroy(client);
}

void test_ws_client_add_http_header_to_non_empty_list(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    struct http_header *hdr1 = calloc(1, sizeof(struct http_header) + 50);
    hdr1->key = ((char*)hdr1) + sizeof(struct http_header);
    hdr1->value = hdr1->key + 20;
    hdr1->next = NULL;
    
    struct http_header *hdr2 = calloc(1, sizeof(struct http_header) + 50);
    hdr2->key = ((char*)hdr2) + sizeof(struct http_header);
    hdr2->value = hdr2->key + 20;
    hdr2->next = NULL;
    
    client->hs.headers = hdr1;
    client->hs.headers_tail = hdr1;
    client->hs.hdr_count = 1;
    
    int result = ws_client_add_http_header(client, hdr2);
    
    assert_int_equal(result, 0);
    assert_ptr_equal(client->hs.headers, hdr1);
    assert_ptr_equal(client->hs.headers_tail, hdr2);
    assert_int_equal(client->hs.hdr_count, 2);
    assert_ptr_equal(hdr1->next, hdr2);
    
    ws_client_destroy(client);
}

void test_ws_client_add_http_header_exceeds_max_count(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    // Set header count to MAX
    client->hs.hdr_count = 128;
    
    struct http_header *hdr = calloc(1, sizeof(struct http_header) + 50);
    hdr->key = ((char*)hdr) + sizeof(struct http_header);
    hdr->value = hdr->key + 20;
    hdr->next = NULL;
    
    int result = ws_client_add_http_header(client, hdr);
    
    assert_int_equal(result, -1);
    
    free(hdr);
    ws_client_destroy(client);
}

// ==========================
// ws_client_want_write Tests
// ==========================

void test_ws_client_want_write_no_data(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    // Buffer should be empty initially
    int result = ws_client_want_write(client);
    
    assert_int_equal(result, 0);
    
    ws_client_destroy(client);
}

void test_ws_client_want_write_with_data(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(0, host_ptr);
    
    // Push some data
    const char *test_data = "test";
    rbuf_push(client->buf_write, test_data, strlen(test_data));
    
    int result = ws_client_want_write(client);
    
    assert_true(result > 0);
    
    ws_client_destroy(client);
}

// ==========================
// ws_client_start_handshake Tests
// ==========================

void test_ws_client_start_handshake_with_valid_host(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    int result = ws_client_start_handshake(client);
    
    assert_int_equal(result, 0);
    assert_int_equal(client->state, WS_HANDSHAKE);
    assert_non_null(client->hs.nonce_reply);
    assert_false(client->rx.remote_closed);
    
    ws_client_destroy(client);
}

void test_ws_client_start_handshake_with_null_host(void **state) {
    char **host_ptr = NULL;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    if (client == NULL) {
        assert_null(client);
        return;
    }
    
    client->host = NULL;
    int result = ws_client_start_handshake(client);
    
    assert_int_equal(result, 1);
    
    ws_client_destroy(client);
}

void test_ws_client_start_handshake_with_empty_host(void **state) {
    char *host = "";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    int result = ws_client_start_handshake(client);
    
    assert_int_equal(result, 1);
    
    ws_client_destroy(client);
}

void test_ws_client_start_handshake_insufficient_buffer_space(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(1, host_ptr);  // Very small buffer
    
    int result = ws_client_start_handshake(client);
    
    assert_int_equal(result, 1);
    
    ws_client_destroy(client);
}

void test_ws_client_start_handshake_generates_nonce(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    int result = ws_client_start_handshake(client);
    
    assert_int_equal(result, 0);
    assert_non_null(client->hs.nonce_reply);
    // Nonce should be non-empty base64 string (28 bytes for WebSocket)
    assert_true(strlen(client->hs.nonce_reply) > 0);
    
    ws_client_destroy(client);
}

void test_ws_client_start_handshake_pushes_upgrade_header(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    int result = ws_client_start_handshake(client);
    
    assert_int_equal(result, 0);
    // Check that data was pushed to write buffer
    int bytes_available = rbuf_bytes_available(client->buf_write);
    assert_true(bytes_available > 0);
    
    ws_client_destroy(client);
}

// ==========================
// ws_client_parse_handshake_resp Tests
// ==========================

void test_ws_client_parse_handshake_resp_http_header_not_enough_bytes(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    // Don't add enough bytes
    rbuf_push(client->buf_read, "HT", 2);
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_NEED_MORE_BYTES);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_http_header_invalid_prefix(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    // Push invalid HTTP header
    rbuf_push(client->buf_read, "INVALID", strlen("INVALID"));
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_PROTOCOL_ERROR);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_http_rc_not_enough_bytes(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    // Push valid HTTP header prefix
    rbuf_push(client->buf_read, "HTTP/1.1 ", strlen("HTTP/1.1 "));
    client->hs.hdr_state = WS_HDR_RC;
    
    // Not enough bytes for status code
    rbuf_pop(client->buf_read, NULL, strlen("HTTP/1.1 "));
    rbuf_push(client->buf_read, "10", 2);
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_NEED_MORE_BYTES);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_http_rc_invalid_format(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_RC;
    
    // Status code without space terminator
    rbuf_push(client->buf_read, "101X", 4);
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_PROTOCOL_ERROR);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_http_rc_out_of_range(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_RC;
    
    // Status code out of valid range
    rbuf_push(client->buf_read, "999 ", 4);
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_PROTOCOL_ERROR);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_http_rc_too_low(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_RC;
    
    // Status code too low
    rbuf_push(client->buf_read, "099 ", 4);
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_PROTOCOL_ERROR);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_http_rc_valid(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_RC;
    
    // Valid status code
    rbuf_push(client->buf_read, "101 ", 4);
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, 0);
    assert_int_equal(client->hs.http_code, 101);
    assert_int_equal(client->hs.hdr_state, WS_HDR_ENDLINE);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_endline_not_enough_bytes(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_ENDLINE;
    
    // Push message without CRLF
    rbuf_push(client->buf_read, "Switching Protocols", strlen("Switching Protocols"));
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_NEED_MORE_BYTES);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_endline_line_too_long(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_ENDLINE;
    
    // Push a very long line
    char long_line[5000];
    memset(long_line, 'A', sizeof(long_line) - 3);
    memcpy(long_line + sizeof(long_line) - 3, "\r\n", 3);
    
    rbuf_push(client->buf_read, long_line, sizeof(long_line));
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_PROTOCOL_ERROR);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_endline_valid(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_ENDLINE;
    
    rbuf_push(client->buf_read, "Switching Protocols\r\n", strlen("Switching Protocols\r\n"));
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, 0);
    assert_non_null(client->hs.http_reply_msg);
    assert_int_equal(client->hs.hdr_state, WS_HDR_PARSE_HEADERS);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_parse_headers_not_enough_bytes(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_PARSE_HEADERS;
    
    rbuf_push(client->buf_read, "test", 4);
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, WS_CLIENT_NEED_MORE_BYTES);
    
    ws_client_destroy(client);
}

void test_ws_client_parse_handshake_resp_parse_headers_empty_line(void **state) {
    char *host = "test.example.com";
    char **host_ptr = &host;
    ws_client *client = ws_client_new(8192, host_ptr);
    
    client->hs.hdr_state = WS_HDR_PARSE_HEADERS;
    client->hs.nonce_matched = 1;  // Assume we already matched nonce
    
    rbuf_push(client->buf_read, "\r\n", 2);
    
    int result = ws_client_parse_handshake_resp(client);
    
    assert_int_equal(result, 0);
    assert_int_equal(client->hs.hdr_state