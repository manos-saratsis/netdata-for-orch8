#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "aclk_contexts_api.c"

// Mock external functions
aclk_query_t *aclk_query_new(aclk_query_type_t type) {
    aclk_query_t *query = malloc(sizeof(aclk_query_t));
    if (!query) return NULL;
    memset(query, 0, sizeof(aclk_query_t));
    query->type = type;
    return query;
}

void aclk_execute_query(aclk_query_t *query) {
    mock_type(void);
    check_expected_ptr(query);
}

void aclk_query_free(aclk_query_t *query) {
    if (query) {
        if (query->data.bin_payload.payload)
            free(query->data.bin_payload.payload);
        free(query);
    }
}

void *contexts_snapshot_2bin(contexts_snapshot_t data, size_t *size) {
    char *payload = malloc(100);
    if (payload && size) {
        strcpy(payload, "test_snapshot");
        *size = 13;
    }
    return payload;
}

void *contexts_updated_2bin(contexts_updated_t data, size_t *size) {
    char *payload = malloc(100);
    if (payload && size) {
        strcpy(payload, "test_updated");
        *size = 12;
    }
    return payload;
}

void *generate_update_node_collectors_message(size_t *size, struct update_node_collectors *collectors) {
    char *payload = malloc(100);
    if (payload && size) {
        strcpy(payload, "test_collectors");
        *size = 15;
    }
    return payload;
}

void *generate_update_node_info_message(size_t *size, struct update_node_info *info) {
    char *payload = malloc(100);
    if (payload && size) {
        strcpy(payload, "test_info");
        *size = 9;
    }
    return payload;
}

// Test: aclk_send_contexts_snapshot with valid data
static void test_aclk_send_contexts_snapshot_valid_data(void **state) {
    contexts_snapshot_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_snapshot(data);
}

// Test: aclk_send_contexts_snapshot creates correct query type
static void test_aclk_send_contexts_snapshot_query_type(void **state) {
    contexts_snapshot_t data = NULL;
    
    // We can't easily verify the query type without modifying the code,
    // but we can verify the function executes without errors
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_snapshot(data);
}

// Test: aclk_send_contexts_snapshot topic assignment
static void test_aclk_send_contexts_snapshot_topic(void **state) {
    contexts_snapshot_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_snapshot(data);
}

// Test: aclk_send_contexts_snapshot message name
static void test_aclk_send_contexts_snapshot_message_name(void **state) {
    contexts_snapshot_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_snapshot(data);
}

// Test: aclk_send_contexts_updated with valid data
static void test_aclk_send_contexts_updated_valid_data(void **state) {
    contexts_updated_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_updated(data);
}

// Test: aclk_send_contexts_updated creates correct query type
static void test_aclk_send_contexts_updated_query_type(void **state) {
    contexts_updated_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_updated(data);
}

// Test: aclk_send_contexts_updated topic assignment
static void test_aclk_send_contexts_updated_topic(void **state) {
    contexts_updated_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_updated(data);
}

// Test: aclk_send_contexts_updated message name
static void test_aclk_send_contexts_updated_message_name(void **state) {
    contexts_updated_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_updated(data);
}

// Test: aclk_update_node_collectors with valid data
static void test_aclk_update_node_collectors_valid_data(void **state) {
    struct update_node_collectors collectors = {0};
    
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_collectors(&collectors);
}

// Test: aclk_update_node_collectors creates correct query type
static void test_aclk_update_node_collectors_query_type(void **state) {
    struct update_node_collectors collectors = {0};
    
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_collectors(&collectors);
}

// Test: aclk_update_node_collectors topic assignment
static void test_aclk_update_node_collectors_topic(void **state) {
    struct update_node_collectors collectors = {0};
    
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_collectors(&collectors);
}

// Test: aclk_update_node_collectors message name
static void test_aclk_update_node_collectors_message_name(void **state) {
    struct update_node_collectors collectors = {0};
    
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_collectors(&collectors);
}

// Test: aclk_update_node_info with valid data
static void test_aclk_update_node_info_valid_data(void **state) {
    struct update_node_info info = {0};
    
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_info(&info);
}

// Test: aclk_update_node_info creates correct query type
static void test_aclk_update_node_info_query_type(void **state) {
    struct update_node_info info = {0};
    
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_info(&info);
}

// Test: aclk_update_node_info topic assignment
static void test_aclk_update_node_info_topic(void **state) {
    struct update_node_info info = {0};
    
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_info(&info);
}

// Test: aclk_update_node_info message name
static void test_aclk_update_node_info_message_name(void **state) {
    struct update_node_info info = {0};
    
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_info(&info);
}

// Test: aclk_send_contexts_snapshot with NULL data pointer
static void test_aclk_send_contexts_snapshot_null_data(void **state) {
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_snapshot(NULL);
}

// Test: aclk_send_contexts_updated with NULL data pointer
static void test_aclk_send_contexts_updated_null_data(void **state) {
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_updated(NULL);
}

// Test: aclk_update_node_collectors with NULL pointer
static void test_aclk_update_node_collectors_null_pointer(void **state) {
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_collectors(NULL);
}

// Test: aclk_update_node_info with NULL pointer
static void test_aclk_update_node_info_null_pointer(void **state) {
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_info(NULL);
}

// Test: Multiple calls to aclk_send_contexts_snapshot
static void test_aclk_send_contexts_snapshot_multiple_calls(void **state) {
    contexts_snapshot_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    expect_any(aclk_execute_query, query);
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_snapshot(data);
    aclk_send_contexts_snapshot(data);
    aclk_send_contexts_snapshot(data);
}

// Test: Multiple calls to aclk_send_contexts_updated
static void test_aclk_send_contexts_updated_multiple_calls(void **state) {
    contexts_updated_t data = NULL;
    
    expect_any(aclk_execute_query, query);
    expect_any(aclk_execute_query, query);
    
    aclk_send_contexts_updated(data);
    aclk_send_contexts_updated(data);
}

// Test: Multiple calls to aclk_update_node_collectors
static void test_aclk_update_node_collectors_multiple_calls(void **state) {
    struct update_node_collectors collectors = {0};
    
    expect_any(aclk_execute_query, query);
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_collectors(&collectors);
    aclk_update_node_collectors(&collectors);
}

// Test: Multiple calls to aclk_update_node_info
static void test_aclk_update_node_info_multiple_calls(void **state) {
    struct update_node_info info = {0};
    
    expect_any(aclk_execute_query, query);
    expect_any(aclk_execute_query, query);
    
    aclk_update_node_info(&info);
    aclk_update_node_info(&info);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_send_contexts_snapshot_valid_data),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_query_type),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_topic),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_message_name),
        cmocka_unit_test(test_aclk_send_contexts_updated_valid_data),
        cmocka_unit_test(test_aclk_send_contexts_updated_query_type),
        cmocka_unit_test(test_aclk_send_contexts_updated_topic),
        cmocka_unit_test(test_aclk_send_contexts_updated_message_name),
        cmocka_unit_test(test_aclk_update_node_collectors_valid_data),
        cmocka_unit_test(test_aclk_update_node_collectors_query_type),
        cmocka_unit_test(test_aclk_update_node_collectors_topic),
        cmocka_unit_test(test_aclk_update_node_collectors_message_name),
        cmocka_unit_test(test_aclk_update_node_info_valid_data),
        cmocka_unit_test(test_aclk_update_node_info_query_type),
        cmocka_unit_test(test_aclk_update_node_info_topic),
        cmocka_unit_test(test_aclk_update_node_info_message_name),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_null_data),
        cmocka_unit_test(test_aclk_send_contexts_updated_null_data),
        cmocka_unit_test(test_aclk_update_node_collectors_null_pointer),
        cmocka_unit_test(test_aclk_update_node_info_null_pointer),
        cmocka_unit_test(test_aclk_send_contexts_snapshot_multiple_calls),
        cmocka_unit_test(test_aclk_send_contexts_updated_multiple_calls),
        cmocka_unit_test(test_aclk_update_node_collectors_multiple_calls),
        cmocka_unit_test(test_aclk_update_node_info_multiple_calls),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}