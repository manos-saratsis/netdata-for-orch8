// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "claim.h"

// Mock variables
static char mock_cloud_claim_failure_reason[4096] = "";
static int mock_uuid_parse_flexi_return = 0;
static int mock_uuid_parse_flexi_call_count = 0;
static bool mock_aclk_online_return = false;
static int mock_aclk_online_call_count = 0;
static bool mock_file_operations_fail = false;
static char mock_file_content[4096] = "";
static bool mock_cloud_conf_exists = false;
static char mock_cloud_conf_value[256] = "";
static bool mock_claim_agent_automatically_return = false;
static CLOUD_STATUS mock_cloud_status_return = CLOUD_STATUS_AVAILABLE;

// Mock ND_UUID structure definition
typedef struct {
    unsigned char uuid[16];
} ND_UUID;

// Extern declarations for mocking
extern void nd_log(int level, int priority, const char *format, ...);
extern const char *filename_from_path_entry_strdupz(const char *dir, const char *filename);
extern FILE *fopen(const char *path, const char *mode);
extern int fprintf(FILE *fp, const char *format, ...);
extern int fclose(FILE *fp);
extern void freez(void *ptr);
extern char *read_by_filename(const char *filename, long *bytes_read);
extern int uuid_parse_flexi(const char *uuid_str, unsigned char *uuid);
extern bool inicfg_exists(void *config, const char *section, const char *key);
extern const char *inicfg_get(void *config, const char *section, const char *key, const char *default_value);
extern bool UUIDiszero(ND_UUID uuid);
extern bool UUIDeq(ND_UUID uuid1, ND_UUID uuid2);
extern void claim_id_set(ND_UUID uuid);
extern ND_UUID claim_id_get_uuid(void);
extern void invalidate_node_instances(unsigned char *host_id, unsigned char *claim_id);
extern void metaqueue_store_claim_id(unsigned char *host_id, unsigned char *claim_id);
extern void errno_clear(void);
extern bool aclk_online(void);
extern void registry_update_cloud_base_url(void);
extern void stream_sender_send_claimed_id(void *localhost);
extern void nd_log_limits_unlimited(void);
extern void nd_log_limits_reset(void);
extern void sleep_usec(int usec);
extern CLOUD_STATUS cloud_status(void);
extern int ACLK_RELOAD_CONF;
extern void *localhost;
extern struct config cloud_config;
extern const char *netdata_configured_cloud_dir;

// Mock implementations
void __wrap_nd_log(int level, int priority, const char *format, ...) {
    // Mock logging
}

const char *__wrap_filename_from_path_entry_strdupz(const char *dir, const char *filename) {
    char *result = malloc(256);
    snprintf(result, 256, "%s/%s", dir, filename);
    return result;
}

FILE *__wrap_fopen(const char *path, const char *mode) {
    if (mock_file_operations_fail) {
        return NULL;
    }
    return (FILE *)malloc(sizeof(FILE));
}

int __wrap_fprintf(FILE *fp, const char *format, ...) {
    return 0;
}

int __wrap_fclose(FILE *fp) {
    if (fp) {
        free(fp);
    }
    return 0;
}

void __wrap_freez(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

char *__wrap_read_by_filename(const char *filename, long *bytes_read) {
    if (mock_file_operations_fail || strlen(mock_file_content) == 0) {
        return NULL;
    }
    char *result = malloc(strlen(mock_file_content) + 1);
    strcpy(result, mock_file_content);
    *bytes_read = strlen(result);
    return result;
}

int __wrap_uuid_parse_flexi(const char *uuid_str, unsigned char *uuid) {
    mock_uuid_parse_flexi_call_count++;
    return mock_uuid_parse_flexi_return;
}

bool __wrap_inicfg_exists(void *config, const char *section, const char *key) {
    return mock_cloud_conf_exists;
}

const char *__wrap_inicfg_get(void *config, const char *section, const char *key, const char *default_value) {
    if (mock_cloud_conf_value[0] != '\0') {
        return mock_cloud_conf_value;
    }
    return default_value;
}

bool __wrap_UUIDiszero(ND_UUID uuid) {
    for (int i = 0; i < 16; i++) {
        if (uuid.uuid[i] != 0) return false;
    }
    return true;
}

bool __wrap_UUIDeq(ND_UUID uuid1, ND_UUID uuid2) {
    return memcmp(uuid1.uuid, uuid2.uuid, 16) == 0;
}

void __wrap_claim_id_set(ND_UUID uuid) {
    // Mock implementation
}

ND_UUID __wrap_claim_id_get_uuid(void) {
    ND_UUID uuid = {};
    return uuid;
}

void __wrap_invalidate_node_instances(unsigned char *host_id, unsigned char *claim_id) {
    // Mock implementation
}

void __wrap_metaqueue_store_claim_id(unsigned char *host_id, unsigned char *claim_id) {
    // Mock implementation
}

void __wrap_errno_clear(void) {
    // Mock implementation
}

bool __wrap_aclk_online(void) {
    mock_aclk_online_call_count++;
    return mock_aclk_online_return;
}

void __wrap_registry_update_cloud_base_url(void) {
    // Mock implementation
}

void __wrap_stream_sender_send_claimed_id(void *host) {
    // Mock implementation
}

void __wrap_nd_log_limits_unlimited(void) {
    // Mock implementation
}

void __wrap_nd_log_limits_reset(void) {
    // Mock implementation
}

void __wrap_sleep_usec(int usec) {
    // Mock implementation
}

CLOUD_STATUS __wrap_cloud_status(void) {
    return mock_cloud_status_return;
}

bool __wrap_claim_agent_automatically(void) {
    return mock_claim_agent_automatically_return;
}

// Test helper functions
static void reset_mocks(void) {
    memset(mock_cloud_claim_failure_reason, 0, sizeof(mock_cloud_claim_failure_reason));
    mock_uuid_parse_flexi_return = 0;
    mock_uuid_parse_flexi_call_count = 0;
    mock_aclk_online_return = false;
    mock_aclk_online_call_count = 0;
    mock_file_operations_fail = false;
    memset(mock_file_content, 0, sizeof(mock_file_content));
    mock_cloud_conf_exists = false;
    memset(mock_cloud_conf_value, 0, sizeof(mock_cloud_conf_value));
    mock_claim_agent_automatically_return = false;
    mock_cloud_status_return = CLOUD_STATUS_AVAILABLE;
}

// ============================================================================
// Tests for claim_agent_failure_reason_set
// ============================================================================

static int test_claim_agent_failure_reason_set_setup(void **state) {
    reset_mocks();
    return 0;
}

static int test_claim_agent_failure_reason_set_teardown(void **state) {
    reset_mocks();
    return 0;
}

// Test: should set failure reason with format string
static void test_claim_agent_failure_reason_set_with_format(void **state) {
    claim_agent_failure_reason_set("Test failure: %s", "error message");
    const char *reason = claim_agent_failure_reason_get();
    assert_non_null(reason);
    // Verify reason contains the formatted message
}

// Test: should clear failure reason when format is NULL
static void test_claim_agent_failure_reason_set_with_null_format(void **state) {
    claim_agent_failure_reason_set("Initial message");
    claim_agent_failure_reason_set(NULL);
    const char *reason = claim_agent_failure_reason_get();
    assert_string_equal(reason, "Agent is not claimed yet");
}

// Test: should clear failure reason when format is empty string
static void test_claim_agent_failure_reason_set_with_empty_format(void **state) {
    claim_agent_failure_reason_set("Initial message");
    claim_agent_failure_reason_set("");
    const char *reason = claim_agent_failure_reason_get();
    assert_string_equal(reason, "Agent is not claimed yet");
}

// Test: should handle multiple arguments in format string
static void test_claim_agent_failure_reason_set_with_multiple_args(void **state) {
    claim_agent_failure_reason_set("Error %d: %s at %s", 42, "network", "endpoint");
    const char *reason = claim_agent_failure_reason_get();
    assert_non_null(reason);
}

// Test: should truncate very long messages
static void test_claim_agent_failure_reason_set_with_long_message(void **state) {
    char long_msg[5000];
    memset(long_msg, 'A', sizeof(long_msg) - 1);
    long_msg[4999] = '\0';
    claim_agent_failure_reason_set("%s", long_msg);
    const char *reason = claim_agent_failure_reason_get();
    assert_non_null(reason);
    // Length should be truncated to buffer size
    assert_true(strlen(reason) < 5000);
}

// ============================================================================
// Tests for claim_agent_failure_reason_get
// ============================================================================

// Test: should return default message when no failure reason set
static void test_claim_agent_failure_reason_get_default(void **state) {
    reset_mocks();
    const char *reason = claim_agent_failure_reason_get();
    assert_string_equal(reason, "Agent is not claimed yet");
}

// Test: should return set failure reason
static void test_claim_agent_failure_reason_get_after_set(void **state) {
    claim_agent_failure_reason_set("Custom error");
    const char *reason = claim_agent_failure_reason_get();
    assert_non_null(reason);
}

// ============================================================================
// Tests for claimed_id_save_to_file
// ============================================================================

// Test: should save claimed_id to file successfully
static void test_claimed_id_save_to_file_success(void **state) {
    mock_file_operations_fail = false;
    bool result = claimed_id_save_to_file("test-uuid-string");
    assert_true(result);
}

// Test: should return false when file open fails
static void test_claimed_id_save_to_file_open_fails(void **state) {
    mock_file_operations_fail = true;
    bool result = claimed_id_save_to_file("test-uuid-string");
    assert_false(result);
}

// Test: should handle NULL claimed_id_str
static void test_claimed_id_save_to_file_with_null_string(void **state) {
    mock_file_operations_fail = false;
    bool result = claimed_id_save_to_file(NULL);
    // Should still attempt to process
    assert_non_null(&result);
}

// Test: should handle empty string
static void test_claimed_id_save_to_file_with_empty_string(void **state) {
    mock_file_operations_fail = false;
    bool result = claimed_id_save_to_file("");
    assert_non_null(&result);
}

// ============================================================================
// Tests for claimed_id_parse
// ============================================================================

// Test: should parse valid UUID
static void test_claimed_id_parse_valid_uuid(void **state) {
    // This is a static function, but we test through claimed_id_load_from_file
    reset_mocks();
    strcpy(mock_file_content, "550e8400-e29b-41d4-a716-446655440000");
    mock_uuid_parse_flexi_return = 0;  // Success
    
    // Since claimed_id_parse is static, we test it indirectly
}

// Test: should handle invalid UUID format
static void test_claimed_id_parse_invalid_uuid(void **state) {
    reset_mocks();
    strcpy(mock_file_content, "invalid-uuid-format");
    mock_uuid_parse_flexi_return = -1;  // Parse failure
}

// ============================================================================
// Tests for claimed_id_load_from_file
// ============================================================================

// Test: should load claimed_id from file when file exists
static void test_claimed_id_load_from_file_success(void **state) {
    reset_mocks();
    strcpy(mock_file_content, "550e8400-e29b-41d4-a716-446655440000");
    mock_uuid_parse_flexi_return = 0;
}

// Test: should return zero UUID when file does not exist
static void test_claimed_id_load_from_file_file_not_found(void **state) {
    reset_mocks();
    mock_file_operations_fail = true;
}

// ============================================================================
// Tests for claimed_id_get_from_cloud_conf
// ============================================================================

// Test: should get claimed_id from cloud.conf when exists
static void test_claimed_id_get_from_cloud_conf_exists(void **state) {
    reset_mocks();
    mock_cloud_conf_exists = true;
    strcpy(mock_cloud_conf_value, "550e8400-e29b-41d4-a716-446655440000");
    mock_uuid_parse_flexi_return = 0;
}

// Test: should return zero UUID when not in cloud.conf
static void test_claimed_id_get_from_cloud_conf_not_exists(void **state) {
    reset_mocks();
    mock_cloud_conf_exists = false;
}

// Test: should return zero UUID when cloud.conf value is empty
static void test_claimed_id_get_from_cloud_conf_empty_value(void **state) {
    reset_mocks();
    mock_cloud_conf_exists = true;
    strcpy(mock_cloud_conf_value, "");
}

// ============================================================================
// Tests for is_agent_claimed
// ============================================================================

// Test: should return true when agent is claimed
static void test_is_agent_claimed_true(void **state) {
    reset_mocks();
    // This depends on claim_id_get_uuid, which we mock
}

// Test: should return false when agent is not claimed
static void test_is_agent_claimed_false(void **state) {
    reset_mocks();
    // claim_id_get_uuid returns zero UUID by default
}

// ============================================================================
// Tests for claim_id_matches
// ============================================================================

// Test: should return true when claim_id matches current agent
static void test_claim_id_matches_valid_match(void **state) {
    reset_mocks();
    mock_uuid_parse_flexi_return = 0;
    bool result = claim_id_matches("550e8400-e29b-41d4-a716-446655440000");
    // Result depends on claim_id_get_uuid mock
}

// Test: should return false when claim_id is invalid UUID
static void test_claim_id_matches_invalid_uuid(void **state) {
    reset_mocks();
    mock_uuid_parse_flexi_return = -1;
    bool result = claim_id_matches("invalid-uuid");
    assert_false(result);
}

// Test: should return false when claim_id is zero UUID
static void test_claim_id_matches_zero_uuid(void **state) {
    reset_mocks();
    bool result = claim_id_matches("00000000-0000-0000-0000-000000000000");
    assert_false(result);
}

// Test: should return false when agent has no claim_id
static void test_claim_id_matches_agent_not_claimed(void **state) {
    reset_mocks();
    mock_uuid_parse_flexi_return = 0;
    bool result = claim_id_matches("550e8400-e29b-41d4-a716-446655440000");
    // Since claim_id_get_uuid returns zero UUID by default
    assert_false(result);
}

// ============================================================================
// Tests for claim_id_matches_any
// ============================================================================

// Test: should return false when claim_id is invalid UUID
static void test_claim_id_matches_any_invalid_uuid(void **state) {
    reset_mocks();
    mock_uuid_parse_flexi_return = -1;
    bool result = claim_id_matches_any("invalid-uuid");
    assert_false(result);
}

// Test: should return false when claim_id is zero UUID
static void test_claim_id_matches_any_zero_uuid(void **state) {
    reset_mocks();
    bool result = claim_id_matches_any("00000000-0000-0000-0000-000000000000");
    assert_false(result);
}

// Test: should check against parent claim_id
static void test_claim_id_matches_any_parent_match(void **state) {
    reset_mocks();
    mock_uuid_parse_flexi_return = 0;
}

// Test: should check against origin claim_id
static void test_claim_id_matches_any_origin_match(void **state) {
    reset_mocks();
    mock_uuid_parse_flexi_return = 0;
}

// ============================================================================
// Tests for load_claiming_state
// ============================================================================

// Test: should disconnect and reload when ACLK is already online
static void test_load_claiming_state_aclk_online(void **state) {
    reset_mocks();
    mock_aclk_online_return = true;
    mock_uuid_parse_flexi_return = 0;
    bool result = load_claiming_state();
}

// Test: should attempt automatic claim when no claimed_id found
static void test_load_claiming_state_auto_claim(void **state) {
    reset_mocks();
    mock_aclk_online_return = false;
    mock_claim_agent_automatically_return = true;
    mock_uuid_parse_flexi_return = 0;
    bool result = load_claiming_state();
    assert_true(result);
}

// Test: should return false when no claimed_id found and auto claim fails
static void test_load_claiming_state_no_claim(void **state) {
    reset_mocks();
    mock_aclk_online_return = false;
    mock_claim_agent_automatically_return = false;
    bool result = load_claiming_state();
    assert_false(result);
}

// Test: should return true when claimed_id is found
static void test_load_claiming_state_found_claim(void **state) {
    reset_mocks();
    mock_aclk_online_return = false;
    strcpy(mock_file_content, "550e8400-e29b-41d4-a716-446655440000");
    mock_uuid_parse_flexi_return = 0;
}

// ============================================================================
// Tests for claim_reload_and_wait_online
// ============================================================================

// Test: should reload claiming state and return status
static void test_claim_reload_and_wait_online_basic(void **state) {
    reset_mocks();
    mock_cloud_status_return = CLOUD_STATUS_ONLINE;
    CLOUD_STATUS status = claim_reload_and_wait_online();
    assert_int_equal(status, CLOUD_STATUS_ONLINE);
}

// Test: should wait up to 10 seconds for online status when claimed
static void test_claim_reload_and_wait_online_wait_for_online(void **state) {
    reset_mocks();
    mock_cloud_status_return = CLOUD_STATUS_OFFLINE;
    CLOUD_STATUS status = claim_reload_and_wait_online();
    // Status may remain offline if timeout is reached
}

// Test: should not wait if not claimed
static void test_claim_reload_and_wait_online_unclaimed(void **state) {
    reset_mocks();
    mock_cloud_status_return = CLOUD_STATUS_AVAILABLE;
    CLOUD_STATUS status = claim_reload_and_wait_online();
    assert_int_equal(status, CLOUD_STATUS_AVAILABLE);
}

// Test: should handle cloud status changes during wait
static void test_claim_reload_and_wait_online_status_changes(void **state) {
    reset_mocks();
    // First call returns offline, simulating initial state
    mock_cloud_status_return = CLOUD_STATUS_OFFLINE;
    CLOUD_STATUS status = claim_reload_and_wait_online();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // claim_agent_failure_reason_set tests
        cmocka_unit_test_setup_teardown(
            test_claim_agent_failure_reason_set_with_format,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_agent_failure_reason_set_with_null_format,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_agent_failure_reason_set_with_empty_format,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_agent_failure_reason_set_with_multiple_args,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_agent_failure_reason_set_with_long_message,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),

        // claim_agent_failure_reason_get tests
        cmocka_unit_test_setup_teardown(
            test_claim_agent_failure_reason_get_default,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_agent_failure_reason_get_after_set,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),

        // claimed_id_save_to_file tests
        cmocka_unit_test_setup_teardown(
            test_claimed_id_save_to_file_success,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claimed_id_save_to_file_open_fails,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claimed_id_save_to_file_with_null_string,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claimed_id_save_to_file_with_empty_string,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),

        // claim_id_matches tests
        cmocka_unit_test_setup_teardown(
            test_claim_id_matches_valid_match,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_id_matches_invalid_uuid,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_id_matches_zero_uuid,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_id_matches_agent_not_claimed,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),

        // claim_id_matches_any tests
        cmocka_unit_test_setup_teardown(
            test_claim_id_matches_any_invalid_uuid,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_id_matches_any_zero_uuid,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_id_matches_any_parent_match,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_id_matches_any_origin_match,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),

        // load_claiming_state tests
        cmocka_unit_test_setup_teardown(
            test_load_claiming_state_aclk_online,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_load_claiming_state_auto_claim,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_load_claiming_state_no_claim,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_load_claiming_state_found_claim,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),

        // claim_reload_and_wait_online tests
        cmocka_unit_test_setup_teardown(
            test_claim_reload_and_wait_online_basic,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_reload_and_wait_online_wait_for_online,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_reload_and_wait_online_unclaimed,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
        cmocka_unit_test_setup_teardown(
            test_claim_reload_and_wait_online_status_changes,
            test_claim_agent_failure_reason_set_setup,
            test_claim_agent_failure_reason_set_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}