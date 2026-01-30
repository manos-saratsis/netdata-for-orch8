#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>

// Mock types
struct alarm_log_entry {
    char *name;
    char *id;
};

struct provide_alarm_configuration {
    char *data;
};

typedef void* alarm_snapshot_proto_ptr_t;

// Forward declarations
char *generate_alarm_log_entry(size_t *payload_size, struct alarm_log_entry *log_entry);
char *generate_provide_alarm_configuration(size_t *payload_size, struct provide_alarm_configuration *cfg);
char *generate_alarm_snapshot_bin(size_t *payload_size, alarm_snapshot_proto_ptr_t snapshot);

// Mock functions
void aclk_send_bin_msg(const char *payload, size_t payload_size, int topic, const char *msg_name);
typedef struct {
    char *payload;
    size_t size;
    int topic;
    const char *msg_name;
} aclk_query_t;

aclk_query_t *aclk_query_new(int type);
void aclk_query_free(aclk_query_t *query);

// Include the actual implementation
#include "../aclk_alarm_api.c"

// ============================================================================
// Tests for aclk_send_alarm_log_entry
// ============================================================================

// Test: aclk_send_alarm_log_entry with valid log entry
static void test_aclk_send_alarm_log_entry_valid(void **state) {
    struct alarm_log_entry log_entry = {
        .name = "test_alarm",
        .id = "alarm_123"
    };
    
    // This should not crash and should call aclk_send_bin_msg
    // We'll verify by checking the function completes without error
    aclk_send_alarm_log_entry(&log_entry);
}

// Test: aclk_send_alarm_log_entry with NULL pointer log entry
static void test_aclk_send_alarm_log_entry_null(void **state) {
    // This tests the edge case of NULL input
    // The function should handle this gracefully
    aclk_send_alarm_log_entry(NULL);
}

// Test: aclk_send_alarm_log_entry with empty log entry
static void test_aclk_send_alarm_log_entry_empty_fields(void **state) {
    struct alarm_log_entry log_entry = {
        .name = "",
        .id = ""
    };
    
    aclk_send_alarm_log_entry(&log_entry);
}

// Test: aclk_send_alarm_log_entry with log entry containing special characters
static void test_aclk_send_alarm_log_entry_special_chars(void **state) {
    struct alarm_log_entry log_entry = {
        .name = "alarm@#$%^&*()",
        .id = "id<>?/\\"
    };
    
    aclk_send_alarm_log_entry(&log_entry);
}

// ============================================================================
// Tests for aclk_send_provide_alarm_cfg
// ============================================================================

// Test: aclk_send_provide_alarm_cfg with valid configuration
static void test_aclk_send_provide_alarm_cfg_valid(void **state) {
    struct provide_alarm_configuration cfg = {
        .data = "config_data"
    };
    
    aclk_send_provide_alarm_cfg(&cfg);
}

// Test: aclk_send_provide_alarm_cfg with NULL configuration
static void test_aclk_send_provide_alarm_cfg_null(void **state) {
    aclk_send_provide_alarm_cfg(NULL);
}

// Test: aclk_send_provide_alarm_cfg with empty configuration data
static void test_aclk_send_provide_alarm_cfg_empty_data(void **state) {
    struct provide_alarm_configuration cfg = {
        .data = ""
    };
    
    aclk_send_provide_alarm_cfg(&cfg);
}

// Test: aclk_send_provide_alarm_cfg with NULL data field
static void test_aclk_send_provide_alarm_cfg_null_data(void **state) {
    struct provide_alarm_configuration cfg = {
        .data = NULL
    };
    
    aclk_send_provide_alarm_cfg(&cfg);
}

// Test: aclk_send_provide_alarm_cfg with large configuration data
static void test_aclk_send_provide_alarm_cfg_large_data(void **state) {
    char *large_data = malloc(10000);
    memset(large_data, 'X', 9999);
    large_data[9999] = '\0';
    
    struct provide_alarm_configuration cfg = {
        .data = large_data
    };
    
    aclk_send_provide_alarm_cfg(&cfg);
    free(large_data);
}

// ============================================================================
// Tests for aclk_send_alarm_snapshot
// ============================================================================

// Test: aclk_send_alarm_snapshot with valid snapshot
static void test_aclk_send_alarm_snapshot_valid(void **state) {
    void *snapshot = malloc(100);
    memset(snapshot, 0, 100);
    
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)snapshot);
    
    free(snapshot);
}

// Test: aclk_send_alarm_snapshot with NULL snapshot
static void test_aclk_send_alarm_snapshot_null(void **state) {
    aclk_send_alarm_snapshot(NULL);
}

// Test: aclk_send_alarm_snapshot with different pointer values
static void test_aclk_send_alarm_snapshot_various_pointers(void **state) {
    // Test with various pointer values
    void *ptr1 = (void *)0x1;
    void *ptr2 = (void *)0xFFFF;
    void *ptr3 = (void *)0x12345678;
    
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)ptr1);
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)ptr2);
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)ptr3);
}

// ============================================================================
// Tests for integration and error scenarios
// ============================================================================

// Test: Multiple calls to aclk_send_alarm_log_entry
static void test_aclk_send_alarm_log_entry_multiple_calls(void **state) {
    struct alarm_log_entry entry1 = {.name = "alarm1", .id = "id1"};
    struct alarm_log_entry entry2 = {.name = "alarm2", .id = "id2"};
    struct alarm_log_entry entry3 = {.name = "alarm3", .id = "id3"};
    
    aclk_send_alarm_log_entry(&entry1);
    aclk_send_alarm_log_entry(&entry2);
    aclk_send_alarm_log_entry(&entry3);
}

// Test: Multiple calls to aclk_send_provide_alarm_cfg
static void test_aclk_send_provide_alarm_cfg_multiple_calls(void **state) {
    struct provide_alarm_configuration cfg1 = {.data = "cfg1"};
    struct provide_alarm_configuration cfg2 = {.data = "cfg2"};
    struct provide_alarm_configuration cfg3 = {.data = "cfg3"};
    
    aclk_send_provide_alarm_cfg(&cfg1);
    aclk_send_provide_alarm_cfg(&cfg2);
    aclk_send_provide_alarm_cfg(&cfg3);
}

// Test: Multiple calls to aclk_send_alarm_snapshot
static void test_aclk_send_alarm_snapshot_multiple_calls(void **state) {
    void *snap1 = malloc(50);
    void *snap2 = malloc(50);
    void *snap3 = malloc(50);
    
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)snap1);
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)snap2);
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)snap3);
    
    free(snap1);
    free(snap2);
    free(snap3);
}

// Test: Interleaved calls to all three functions
static void test_aclk_interleaved_calls(void **state) {
    struct alarm_log_entry log_entry = {.name = "alarm", .id = "id"};
    struct provide_alarm_configuration cfg = {.data = "config"};
    void *snapshot = malloc(100);
    
    aclk_send_alarm_log_entry(&log_entry);
    aclk_send_provide_alarm_cfg(&cfg);
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)snapshot);
    aclk_send_alarm_log_entry(&log_entry);
    aclk_send_provide_alarm_cfg(&cfg);
    aclk_send_alarm_snapshot((alarm_snapshot_proto_ptr_t)snapshot);
    
    free(snapshot);
}

// Test: aclk_send_alarm_log_entry with long strings
static void test_aclk_send_alarm_log_entry_long_strings(void **state) {
    char *long_name = malloc(5000);
    char *long_id = malloc(5000);
    memset(long_name, 'N', 4999);
    long_name[4999] = '\0';
    memset(long_id, 'I', 4999);
    long_id[4999] = '\0';
    
    struct alarm_log_entry log_entry = {
        .name = long_name,
        .id = long_id
    };
    
    aclk_send_alarm_log_entry(&log_entry);
    
    free(long_name);
    free(long_id);
}

// Test: aclk_send_provide_alarm_cfg with binary data
static void test_aclk_send_provide_alarm_cfg_binary_data(void **state) {
    char *binary_data = malloc(256);
    for (int i = 0; i < 256; i++) {
        binary_data[i] = (char)i;
    }
    
    struct provide_alarm_configuration cfg = {
        .data = binary_data
    };
    
    aclk_send_provide_alarm_cfg(&cfg);
    
    free(binary_data);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // Tests for aclk_send_alarm_log_entry
        cmocka_unit_test(test_aclk_send_alarm_log_entry_valid),
        cmocka_unit_test(test_aclk_send_alarm_log_entry_null),
        cmocka_unit_test(test_aclk_send_alarm_log_entry_empty_fields),
        cmocka_unit_test(test_aclk_send_alarm_log_entry_special_chars),
        
        // Tests for aclk_send_provide_alarm_cfg
        cmocka_unit_test(test_aclk_send_provide_alarm_cfg_valid),
        cmocka_unit_test(test_aclk_send_provide_alarm_cfg_null),
        cmocka_unit_test(test_aclk_send_provide_alarm_cfg_empty_data),
        cmocka_unit_test(test_aclk_send_provide_alarm_cfg_null_data),
        cmocka_unit_test(test_aclk_send_provide_alarm_cfg_large_data),
        
        // Tests for aclk_send_alarm_snapshot
        cmocka_unit_test(test_aclk_send_alarm_snapshot_valid),
        cmocka_unit_test(test_aclk_send_alarm_snapshot_null),
        cmocka_unit_test(test_aclk_send_alarm_snapshot_various_pointers),
        
        // Integration and multiple call tests
        cmocka_unit_test(test_aclk_send_alarm_log_entry_multiple_calls),
        cmocka_unit_test(test_aclk_send_provide_alarm_cfg_multiple_calls),
        cmocka_unit_test(test_aclk_send_alarm_snapshot_multiple_calls),
        cmocka_unit_test(test_aclk_interleaved_calls),
        cmocka_unit_test(test_aclk_send_alarm_log_entry_long_strings),
        cmocka_unit_test(test_aclk_send_provide_alarm_cfg_binary_data),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}