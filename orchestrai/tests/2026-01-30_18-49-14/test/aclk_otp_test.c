#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>
#include <json-c/json.h>

// Mock structures and functions
typedef struct {
    char *host;
    int port;
    char *username;
    char *password;
    char *proxy_destination;
    int type;
} mqtt_wss_proxy;

typedef struct {
    char *host;
    int port;
    char *username;
    char *password;
    char *proxy_destination;
    int proxy_host;
    int proxy_port;
    char *proxy_username;
    char *proxy_password;
    char *proxy;
} https_req_t;

typedef struct {
    int http_code;
    char *payload;
    size_t payload_size;
} https_req_response_t;

typedef enum {
    HTTPS_CLIENT_RESP_OK = 0,
    HTTPS_CLIENT_RESP_OTP_CHALLENGE_NOT_200 = 1,
    HTTPS_CLIENT_RESP_OTP_CHALLENGE_INVALID = 2,
    HTTPS_CLIENT_RESP_OTP_CHALLENGE_DECRYPTION_FAILED = 3,
    HTTPS_CLIENT_RESP_OTP_PASSWORD_NOT_201 = 4,
    HTTPS_CLIENT_RESP_OTP_PASSWORD_EMPTY = 5,
    HTTPS_CLIENT_RESP_OTP_PASSWORD_NOT_JSON = 6,
    HTTPS_CLIENT_RESP_ENV_AGENT_NOT_CLAIMED = 7,
    HTTPS_CLIENT_RESP_ENV_NOT_200 = 8,
    HTTPS_CLIENT_RESP_ENV_EMPTY = 9,
    HTTPS_CLIENT_RESP_ENV_NOT_JSON = 10,
    HTTPS_CLIENT_RESP_OTP_AGENT_NOT_CLAIMED = 11,
} https_client_resp_t;

typedef enum {
    HTTP_REQ_GET = 0,
    HTTP_REQ_POST = 1,
} http_req_type_t;

typedef enum {
    MQTT_WSS_DIRECT = 0,
    MQTT_WSS_PROXY_HTTP = 1,
} mqtt_wss_proxy_type_t;

typedef enum {
    ACLK_TRP_UNKNOWN = 0,
    ACLK_TRP_MQTT = 1,
} aclk_transport_type_t;

typedef enum {
    ACLK_ENC_UNKNOWN = 0,
    ACLK_ENC_JSON = 1,
} aclk_encoding_type_t;

typedef struct {
    char *host;
    int port;
    char *path;
} url_t;

typedef struct {
    char *str;
} CLAIM_ID;

typedef struct {
    aclk_transport_type_t type;
    char *endpoint;
} aclk_transport_desc_t;

typedef struct {
    int base;
    int min_s;
    int max_s;
} aclk_backoff_t;

typedef struct {
    char *auth_endpoint;
    aclk_encoding_type_t encoding;
    aclk_transport_desc_t **transports;
    size_t transport_count;
    char **capabilities;
    size_t capability_count;
    aclk_backoff_t backoff;
} aclk_env_t;

// Mock variables
static int mock_aclk_disable_runtime;
static long mock_aclk_block_until;

#define NETDATA_VERSION "1.0.0"
#define UUID_STR_LEN 36
#define HTTPS_REQ_T_INITIALIZER { NULL, 0, NULL, NULL, NULL, 0, 0, NULL, NULL, NULL }
#define HTTPS_REQ_RESPONSE_T_INITIALIZER { 0, NULL, 0 }
#define CHALLENGE_LEN 256
#define CHALLENGE_LEN_BASE64 344

// Mock functions
int aclk_disable_runtime = 0;
long aclk_block_until = 0;

void netdata_log_error(const char *fmt, ...) {
    // Mock implementation
}

void aclk_set_proxy(char **host, int *port, char **username, char **password, char **proxy_destination, int *type) {
    *host = NULL;
    *port = 0;
    *username = NULL;
    *password = NULL;
    *proxy_destination = NULL;
    *type = MQTT_WSS_DIRECT;
}

char *strdupz(const char *str) {
    if (!str) return NULL;
    return strdup(str);
}

void freez(void *ptr) {
    if (ptr) free(ptr);
}

void *mallocz(size_t size) {
    return malloc(size);
}

void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

CLAIM_ID claim_id_get(void) {
    return (CLAIM_ID){"test-agent-id-12345678"};
}

int claim_id_is_set(CLAIM_ID id) {
    return id.str != NULL && strlen(id.str) > 0;
}

int netdata_base64_decode(unsigned char *dst, const unsigned char *src, size_t src_len) {
    // Simplified mock
    if (src_len == CHALLENGE_LEN_BASE64) {
        // Return expected challenge length
        return CHALLENGE_LEN;
    }
    return -1;
}

int netdata_base64_encode(unsigned char *dst, const unsigned char *src, size_t src_len) {
    // Simplified mock
    return 0;
}

long now_monotonic_sec(void) {
    return 1000;
}

typedef struct {
    int buffers_aclk;
} netdata_buffers_stats_t;

netdata_buffers_stats_t netdata_buffers_statistics = {0};

typedef struct {
    char *buffer;
} BUFFER;

BUFFER *buffer_create(size_t size, int *stats) {
    BUFFER *buf = malloc(sizeof(BUFFER));
    if (buf) {
        buf->buffer = malloc(size);
        memset(buf->buffer, 0, size);
    }
    return buf;
}

void buffer_free(BUFFER *buf) {
    if (buf) {
        if (buf->buffer) free(buf->buffer);
        free(buf);
    }
}

void buffer_sprintf(BUFFER *buf, const char *fmt, ...) {
    // Simplified mock - just copy enough for tests
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf->buffer, 4096, fmt, args);
    va_end(args);
}

const char *buffer_tostring(BUFFER *buf) {
    return buf ? buf->buffer : "";
}

// Forward declarations of functions to test
struct auth_data {
    char *client_id;
    char *username;
    char *passwd;
};

static int parse_passwd_response(const char *json_str, struct auth_data *auth);
static int aclk_parse_otp_error(const char *json_str);

// Test helper to create mock password response
static char *create_valid_passwd_json(void) {
    return strdup("{\"clientID\":\"test-id\",\"username\":\"test-user\",\"password\":\"test-pass\",\"topics\":[]}");
}

static char *create_invalid_passwd_json_missing_clientid(void) {
    return strdup("{\"username\":\"test-user\",\"password\":\"test-pass\"}");
}

static char *create_invalid_passwd_json_wrong_type(void) {
    return strdup("{\"clientID\":123,\"username\":\"test-user\",\"password\":\"test-pass\"}");
}

static char *create_invalid_passwd_json_malformed(void) {
    return strdup("{invalid json");
}

// Test: parse_passwd_response with valid JSON
static void test_parse_passwd_response_valid_json(void **state) {
    struct auth_data auth = {0};
    char *json_str = create_valid_passwd_json();
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 0);
    assert_string_equal(auth.client_id, "test-id");
    assert_string_equal(auth.username, "test-user");
    assert_string_equal(auth.passwd, "test-pass");
    
    freez(auth.client_id);
    freez(auth.username);
    freez(auth.passwd);
    freez(json_str);
}

// Test: parse_passwd_response with missing clientID
static void test_parse_passwd_response_missing_clientid(void **state) {
    struct auth_data auth = {0};
    char *json_str = create_invalid_passwd_json_missing_clientid();
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: parse_passwd_response with missing password
static void test_parse_passwd_response_missing_password(void **state) {
    struct auth_data auth = {0};
    char *json_str = strdup("{\"clientID\":\"test-id\",\"username\":\"test-user\"}");
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: parse_passwd_response with missing username
static void test_parse_passwd_response_missing_username(void **state) {
    struct auth_data auth = {0};
    char *json_str = strdup("{\"clientID\":\"test-id\",\"password\":\"test-pass\"}");
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: parse_passwd_response with wrong type for clientID
static void test_parse_passwd_response_wrong_type_clientid(void **state) {
    struct auth_data auth = {0};
    char *json_str = create_invalid_passwd_json_wrong_type();
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: parse_passwd_response with wrong type for username
static void test_parse_passwd_response_wrong_type_username(void **state) {
    struct auth_data auth = {0};
    char *json_str = strdup("{\"clientID\":\"test-id\",\"username\":123,\"password\":\"test-pass\"}");
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: parse_passwd_response with wrong type for password
static void test_parse_passwd_response_wrong_type_password(void **state) {
    struct auth_data auth = {0};
    char *json_str = strdup("{\"clientID\":\"test-id\",\"username\":\"test-user\",\"password\":456}");
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: parse_passwd_response with malformed JSON
static void test_parse_passwd_response_malformed_json(void **state) {
    struct auth_data auth = {0};
    char *json_str = create_invalid_passwd_json_malformed();
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: parse_passwd_response with unknown keys (should be ignored)
static void test_parse_passwd_response_unknown_keys(void **state) {
    struct auth_data auth = {0};
    char *json_str = strdup("{\"clientID\":\"test-id\",\"username\":\"test-user\",\"password\":\"test-pass\",\"unknown\":\"value\"}");
    
    int result = parse_passwd_response(json_str, &auth);
    
    assert_int_equal(result, 0);
    
    freez(auth.client_id);
    freez(auth.username);
    freez(auth.passwd);
    freez(json_str);
}

// Test: aclk_parse_otp_error with valid error JSON
static void test_aclk_parse_otp_error_valid(void **state) {
    char *json_str = strdup("{\"errorCode\":\"EC001\",\"errorMsgKey\":\"key1\",\"errorMessage\":\"error occurred\"}");
    
    int result = aclk_parse_otp_error(json_str);
    
    assert_int_equal(result, 0);
    
    freez(json_str);
}

// Test: aclk_parse_otp_error with block retry flag
static void test_aclk_parse_otp_error_block_retry(void **state) {
    char *json_str = strdup("{\"errorCode\":\"EC001\",\"errorMsgKey\":\"key1\",\"errorMessage\":\"error\",\"errorNonRetryable\":true}");
    
    aclk_disable_runtime = 0;
    int result = aclk_parse_otp_error(json_str);
    
    assert_int_equal(result, 0);
    assert_int_equal(aclk_disable_runtime, 1);
    
    aclk_disable_runtime = 0;
    freez(json_str);
}

// Test: aclk_parse_otp_error with backoff delay
static void test_aclk_parse_otp_error_backoff(void **state) {
    char *json_str = strdup("{\"errorCode\":\"EC001\",\"errorMsgKey\":\"key1\",\"errorMessage\":\"error\",\"errorRetryDelaySeconds\":30}");
    
    aclk_block_until = 0;
    int result = aclk_parse_otp_error(json_str);
    
    assert_int_equal(result, 0);
    assert_true(aclk_block_until > 1000);
    
    aclk_block_until = 0;
    freez(json_str);
}

// Test: aclk_parse_otp_error with malformed JSON
static void test_aclk_parse_otp_error_malformed(void **state) {
    char *json_str = strdup("{invalid");
    
    int result = aclk_parse_otp_error(json_str);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: aclk_parse_otp_error with missing error code
static void test_aclk_parse_otp_error_missing_errorcode(void **state) {
    char *json_str = strdup("{\"errorMsgKey\":\"key1\",\"errorMessage\":\"error\"}");
    
    int result = aclk_parse_otp_error(json_str);
    
    assert_int_equal(result, 1);
    
    freez(json_str);
}

// Test: aclk_parse_otp_error with wrong type for retry flag
static void test_aclk_parse_otp_error_wrong_type_retry(void **state) {
    char *json_str = strdup("{\"errorCode\":\"EC001\",\"errorMsgKey\":\"key1\",\"errorMessage\":\"error\",\"errorNonRetryable\":\"true\"}");
    
    int result = aclk_parse_otp_error(json_str);
    
    // Should handle gracefully
    assert_int_equal(result, 0);
    
    freez(json_str);
}

// Test: aclk_parse_otp_error with wrong type for backoff
static void test_aclk_parse_otp_error_wrong_type_backoff(void **state) {
    char *json_str = strdup("{\"errorCode\":\"EC001\",\"errorMsgKey\":\"key1\",\"errorMessage\":\"error\",\"errorRetryDelaySeconds\":\"30\"}");
    
    int result = aclk_parse_otp_error(json_str);
    
    // Should handle gracefully
    assert_int_equal(result, 0);
    
    freez(json_str);
}

// Test: aclk_parse_otp_error with unknown keys (should be ignored)
static void test_aclk_parse_otp_error_unknown_keys(void **state) {
    char *json_str = strdup("{\"errorCode\":\"EC001\",\"errorMsgKey\":\"key1\",\"errorMessage\":\"error\",\"unknown\":\"value\"}");
    
    int result = aclk_parse_otp_error(json_str);
    
    assert_int_equal(result, 0);
    
    freez(json_str);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parse_passwd_response_valid_json),
        cmocka_unit_test(test_parse_passwd_response_missing_clientid),
        cmocka_unit_test(test_parse_passwd_response_missing_password),
        cmocka_unit_test(test_parse_passwd_response_missing_username),
        cmocka_unit_test(test_parse_passwd_response_wrong_type_clientid),
        cmocka_unit_test(test_parse_passwd_response_wrong_type_username),
        cmocka_unit_test(test_parse_passwd_response_wrong_type_password),
        cmocka_unit_test(test_parse_passwd_response_malformed_json),
        cmocka_unit_test(test_parse_passwd_response_unknown_keys),
        cmocka_unit_test(test_aclk_parse_otp_error_valid),
        cmocka_unit_test(test_aclk_parse_otp_error_block_retry),
        cmocka_unit_test(test_aclk_parse_otp_error_backoff),
        cmocka_unit_test(test_aclk_parse_otp_error_malformed),
        cmocka_unit_test(test_aclk_parse_otp_error_missing_errorcode),
        cmocka_unit_test(test_aclk_parse_otp_error_wrong_type_retry),
        cmocka_unit_test(test_aclk_parse_otp_error_wrong_type_backoff),
        cmocka_unit_test(test_aclk_parse_otp_error_unknown_keys),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}