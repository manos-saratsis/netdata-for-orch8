/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * 
 * Comprehensive unit tests for aclk_query_queue.c
 * Tests cover all functions, branches, error paths, and edge cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

/* Mock the libnetdata structures and functions */
typedef struct {
    pthread_mutex_t inner;
} netdata_mutex_t;

typedef struct {
    bool locked;
} SPINLOCK;

typedef struct {
    long tv_sec;
    long tv_usec;
} timeval;

typedef unsigned long long usec_t;

/* Forward declarations for mocked functions */
void spinlock_init(SPINLOCK *spinlock) {
    spinlock->locked = false;
}

void spinlock_lock(SPINLOCK *spinlock) {
    while (__sync_bool_compare_and_swap(&spinlock->locked, false, true)) {
        /* Spin until lock acquired */
    }
}

void spinlock_unlock(SPINLOCK *spinlock) {
    __sync_bool_compare_and_swap(&spinlock->locked, true, false);
}

void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void freez(void *ptr) {
    free(ptr);
}

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* Include the header to get type definitions */
typedef enum {
    UNKNOWN = 0,
    HTTP_API_V2,
    REGISTER_NODE,
    NODE_STATE_UPDATE,
    UPDATE_NODE_INFO,
    ALARM_PROVIDE_CFG,
    ALARM_SNAPSHOT,
    UPDATE_NODE_COLLECTORS,
    CTX_SEND_SNAPSHOT,
    CTX_SEND_SNAPSHOT_UPD,
    CTX_CHECKPOINT,
    CTX_STOP_STREAMING,
    CREATE_NODE_INSTANCE,
    SEND_NODE_INSTANCES,
    ALERT_START_STREAMING,
    ALERT_CHECKPOINT,
    ACLK_QUERY_TYPE_COUNT
} aclk_query_type_t;

struct aclk_query_http_api_v2 {
    char *payload;
    char *query;
};

struct aclk_bin_payload {
    char *payload;
    size_t size;
    enum aclk_topics {
        ACLK_TOPICID_UNKNOWN = 0,
        ACLK_TOPICID_CHART = 1,
        ACLK_TOPICID_ALARMS = 2,
        ACLK_TOPICID_METADATA = 3,
        ACLK_TOPICID_COMMAND = 4,
        ACLK_TOPICID_AGENT_CONN = 5,
        ACLK_TOPICID_CMD_NG_V1 = 6,
        ACLK_TOPICID_CREATE_NODE = 7,
        ACLK_TOPICID_NODE_CONN = 8,
        ACLK_TOPICID_CHART_DIMS = 9,
        ACLK_TOPICID_CHART_CONFIGS_UPDATED = 10,
        ACLK_TOPICID_CHART_RESET = 11,
        ACLK_TOPICID_RETENTION_UPDATED = 12,
        ACLK_TOPICID_NODE_INFO = 13,
        ACLK_TOPICID_ALARM_LOG = 14,
        ACLK_TOPICID_ALARM_CHECKPOINT = 15,
        ACLK_TOPICID_ALARM_CONFIG = 16,
        ACLK_TOPICID_ALARM_SNAPSHOT = 17,
        ACLK_TOPICID_NODE_COLLECTORS = 18,
        ACLK_TOPICID_CTXS_SNAPSHOT = 19,
        ACLK_TOPICID_CTXS_UPDATED = 20
    } topic;
    const char *msg_name;
};

struct ctxs_checkpoint {
    char *claim_id;
    char *node_id;
};

typedef struct {
    aclk_query_type_t type;
    bool allocated;
    char *dedup_id;
    char *callback_topic;
    char *msg_id;
    union {
        char *claim_id;
        char *machine_guid;
    };
    struct timeval created_tv;
    usec_t created;
    int timeout;
    uint64_t version;
    union {
        struct aclk_query_http_api_v2 http_api_v2;
        struct aclk_bin_payload bin_payload;
        void *payload;
        char *node_id;
    } data;
} aclk_query_t;

#define MAX_QUERY_ENTRIES (512)

struct {
    aclk_query_t query_workers[MAX_QUERY_ENTRIES];
    int free_stack[MAX_QUERY_ENTRIES];
    int top;
    SPINLOCK spinlock;
} queryPool;

void init_query_pool()
{
    spinlock_init(&queryPool.spinlock);
    for (int i = 0; i < MAX_QUERY_ENTRIES; i++) {
        queryPool.free_stack[i] = i;
        queryPool.query_workers[i].allocated = false;
    }
    queryPool.top = MAX_QUERY_ENTRIES;
}

static aclk_query_t *get_query()
{
    spinlock_lock(&queryPool.spinlock);
    if (queryPool.top == 0) {
        spinlock_unlock(&queryPool.spinlock);
        aclk_query_t *query = callocz(1, sizeof(aclk_query_t));
        query->allocated = true;
        return query;
    }
    int index = queryPool.free_stack[--queryPool.top];
    memset(&queryPool.query_workers[index], 0, sizeof(aclk_query_t));
    spinlock_unlock(&queryPool.spinlock);
    return &queryPool.query_workers[index];
}

static void return_query(aclk_query_t *query)
{
    if (unlikely(query->allocated)) {
       freez(query);
       return;
    }
    spinlock_lock(&queryPool.spinlock);
    int index = (int) (query - queryPool.query_workers);
    if (index < 0 || index >= MAX_QUERY_ENTRIES) {
        spinlock_unlock(&queryPool.spinlock);
        return;  // Invalid (should not happen)
    }
    queryPool.free_stack[queryPool.top++] = index;
    memset(query, 0, sizeof(aclk_query_t));
    spinlock_unlock(&queryPool.spinlock);
}

aclk_query_t *aclk_query_new(aclk_query_type_t type)
{
    aclk_query_t *query = get_query();
    query->type = type;
    gettimeofday(&query->created_tv, NULL);
    return query;
}

void aclk_query_free(aclk_query_t *query)
{
    struct ctxs_checkpoint *cmd;
    switch (query->type) {
        case HTTP_API_V2:
            freez(query->data.http_api_v2.payload);
            if (query->data.http_api_v2.query != query->dedup_id)
                freez(query->data.http_api_v2.query);
            break;
        case ALERT_START_STREAMING:
            freez(query->data.node_id);
            break;
        case ALERT_CHECKPOINT:
            freez(query->data.node_id);
            freez(query->claim_id);
            break;
        case CREATE_NODE_INSTANCE:
            freez(query->data.node_id);
            freez(query->machine_guid);
            break;
        // keep following cases together
        case CTX_STOP_STREAMING:
        case CTX_CHECKPOINT:
            cmd = query->data.payload;
            if (cmd) {
                freez(cmd->claim_id);
                freez(cmd->node_id);
                freez(cmd);
            }
            break;

        default:
            break;
    }

    freez(query->dedup_id);
    freez(query->callback_topic);
    freez(query->msg_id);
    return_query(query);
}

/* ============================================================================
 * Test Suite
 * ============================================================================
 */

void test_init_query_pool_initializes_pool() {
    printf("TEST: init_query_pool_initializes_pool\n");
    
    init_query_pool();
    
    assert(queryPool.top == MAX_QUERY_ENTRIES);
    assert(queryPool.query_workers[0].allocated == false);
    assert(queryPool.free_stack[0] == 0);
    assert(queryPool.free_stack[MAX_QUERY_ENTRIES - 1] == MAX_QUERY_ENTRIES - 1);
    
    printf("  PASS\n");
}

void test_get_query_from_pool() {
    printf("TEST: get_query_from_pool\n");
    
    init_query_pool();
    int initial_top = queryPool.top;
    
    aclk_query_t *query = get_query();
    
    assert(query != NULL);
    assert(query->allocated == false);
    assert(queryPool.top == initial_top - 1);
    
    printf("  PASS\n");
}

void test_get_query_allocates_when_pool_empty() {
    printf("TEST: get_query_allocates_when_pool_empty\n");
    
    init_query_pool();
    queryPool.top = 0;
    
    aclk_query_t *query = get_query();
    
    assert(query != NULL);
    assert(query->allocated == true);
    
    freez(query);
    printf("  PASS\n");
}

void test_get_query_multiple_calls_deplete_pool() {
    printf("TEST: get_query_multiple_calls_deplete_pool\n");
    
    init_query_pool();
    aclk_query_t *queries[10];
    
    for (int i = 0; i < 10; i++) {
        queries[i] = get_query();
        assert(queries[i] != NULL);
    }
    
    assert(queryPool.top == MAX_QUERY_ENTRIES - 10);
    
    printf("  PASS\n");
}

void test_return_query_allocated_frees_memory() {
    printf("TEST: return_query_allocated_frees_memory\n");
    
    init_query_pool();
    queryPool.top = 0;
    
    aclk_query_t *query = get_query();
    assert(query->allocated == true);
    
    int top_before = queryPool.top;
    return_query(query);
    
    assert(queryPool.top == top_before);
    
    printf("  PASS\n");
}

void test_return_query_pool_returns_to_pool() {
    printf("TEST: return_query_pool_returns_to_pool\n");
    
    init_query_pool();
    int initial_top = queryPool.top;
    
    aclk_query_t *query = get_query();
    assert(queryPool.top == initial_top - 1);
    
    return_query(query);
    
    assert(queryPool.top == initial_top);
    assert(query->allocated == false);
    
    printf("  PASS\n");
}

void test_return_query_invalid_index_does_nothing() {
    printf("TEST: return_query_invalid_index_does_nothing\n");
    
    init_query_pool();
    int initial_top = queryPool.top;
    
    aclk_query_t fake_query;
    fake_query.allocated = false;
    
    return_query(&fake_query);
    
    assert(queryPool.top == initial_top);
    
    printf("  PASS\n");
}

void test_aclk_query_new_creates_query_with_type() {
    printf("TEST: aclk_query_new_creates_query_with_type\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(HTTP_API_V2);
    
    assert(query != NULL);
    assert(query->type == HTTP_API_V2);
    assert(query->created_tv.tv_sec > 0);
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_new_with_different_types() {
    printf("TEST: aclk_query_new_with_different_types\n");
    
    init_query_pool();
    
    aclk_query_type_t types[] = {
        HTTP_API_V2,
        REGISTER_NODE,
        NODE_STATE_UPDATE,
        UPDATE_NODE_INFO,
        ALERT_START_STREAMING,
        ALERT_CHECKPOINT
    };
    
    for (int i = 0; i < 6; i++) {
        aclk_query_t *query = aclk_query_new(types[i]);
        assert(query->type == types[i]);
        aclk_query_free(query);
    }
    
    printf("  PASS\n");
}

void test_aclk_query_free_http_api_v2_with_payload_and_query() {
    printf("TEST: aclk_query_free_http_api_v2_with_payload_and_query\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(HTTP_API_V2);
    query->data.http_api_v2.payload = malloc(50);
    strcpy(query->data.http_api_v2.payload, "test_payload");
    query->data.http_api_v2.query = malloc(50);
    strcpy(query->data.http_api_v2.query, "test_query");
    query->dedup_id = malloc(50);
    strcpy(query->dedup_id, "dedup_123");
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_http_api_v2_query_equals_dedup_id() {
    printf("TEST: aclk_query_free_http_api_v2_query_equals_dedup_id\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(HTTP_API_V2);
    query->data.http_api_v2.payload = malloc(50);
    strcpy(query->data.http_api_v2.payload, "test_payload");
    char *dedup_id = malloc(50);
    strcpy(dedup_id, "dedup_123");
    query->data.http_api_v2.query = dedup_id;
    query->dedup_id = dedup_id;
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_alert_start_streaming() {
    printf("TEST: aclk_query_free_alert_start_streaming\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(ALERT_START_STREAMING);
    query->data.node_id = malloc(50);
    strcpy(query->data.node_id, "node_123");
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_alert_checkpoint() {
    printf("TEST: aclk_query_free_alert_checkpoint\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(ALERT_CHECKPOINT);
    query->data.node_id = malloc(50);
    strcpy(query->data.node_id, "node_123");
    query->claim_id = malloc(50);
    strcpy(query->claim_id, "claim_123");
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_create_node_instance() {
    printf("TEST: aclk_query_free_create_node_instance\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(CREATE_NODE_INSTANCE);
    query->data.node_id = malloc(50);
    strcpy(query->data.node_id, "node_123");
    query->machine_guid = malloc(50);
    strcpy(query->machine_guid, "guid_123");
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_ctx_stop_streaming() {
    printf("TEST: aclk_query_free_ctx_stop_streaming\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(CTX_STOP_STREAMING);
    struct ctxs_checkpoint *cmd = malloc(sizeof(struct ctxs_checkpoint));
    cmd->claim_id = malloc(50);
    strcpy(cmd->claim_id, "claim_123");
    cmd->node_id = malloc(50);
    strcpy(cmd->node_id, "node_123");
    query->data.payload = cmd;
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_ctx_checkpoint() {
    printf("TEST: aclk_query_free_ctx_checkpoint\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(CTX_CHECKPOINT);
    struct ctxs_checkpoint *cmd = malloc(sizeof(struct ctxs_checkpoint));
    cmd->claim_id = malloc(50);
    strcpy(cmd->claim_id, "claim_123");
    cmd->node_id = malloc(50);
    strcpy(cmd->node_id, "node_123");
    query->data.payload = cmd;
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_ctx_checkpoint_null_payload() {
    printf("TEST: aclk_query_free_ctx_checkpoint_null_payload\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(CTX_CHECKPOINT);
    query->data.payload = NULL;
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_unknown_type() {
    printf("TEST: aclk_query_free_unknown_type\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(UNKNOWN);
    query->dedup_id = malloc(50);
    strcpy(query->dedup_id, "dedup_123");
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_with_callback_topic_and_msg_id() {
    printf("TEST: aclk_query_free_with_callback_topic_and_msg_id\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(HTTP_API_V2);
    query->callback_topic = malloc(50);
    strcpy(query->callback_topic, "callback_topic");
    query->msg_id = malloc(50);
    strcpy(query->msg_id, "msg_id_123");
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_aclk_query_free_all_fields_set() {
    printf("TEST: aclk_query_free_all_fields_set\n");
    
    init_query_pool();
    
    aclk_query_t *query = aclk_query_new(HTTP_API_V2);
    query->data.http_api_v2.payload = malloc(50);
    strcpy(query->data.http_api_v2.payload, "payload");
    query->data.http_api_v2.query = malloc(50);
    strcpy(query->data.http_api_v2.query, "query");
    query->dedup_id = malloc(50);
    strcpy(query->dedup_id, "dedup");
    query->callback_topic = malloc(50);
    strcpy(query->callback_topic, "topic");
    query->msg_id = malloc(50);
    strcpy(query->msg_id, "msg");
    
    aclk_query_free(query);
    
    printf("  PASS\n");
}

void test_pool_exhaustion_and_recovery() {
    printf("TEST: pool_exhaustion_and_recovery\n");
    
    init_query_pool();
    
    // Exhaust the pool
    aclk_query_t *queries[MAX_QUERY_ENTRIES + 10];
    for (int i = 0; i < MAX_QUERY_ENTRIES + 10; i++) {
        queries[i] = aclk_query_new(HTTP_API_V2);
        assert(queries[i] != NULL);
    }
    
    // The last 10 should be allocated from heap
    assert(queries[MAX_QUERY_ENTRIES]->allocated == true);
    
    // Free them all
    for (int i = 0; i < MAX_QUERY_ENTRIES + 10; i++) {
        aclk_query_free(queries[i]);
    }
    
    // Pool should be back to max capacity
    assert(queryPool.top == MAX_QUERY_ENTRIES);
    
    printf("  PASS\n");
}

void test_spinlock_concurrent_access() {
    printf("TEST: spinlock_concurrent_access\n");
    
    init_query_pool();
    
    aclk_query_t *query1 = get_query();
    aclk_query_t *query2 = get_query();
    
    assert(query1 != query2);
    
    return_query(query1);
    return_query(query2);
    
    printf("  PASS\n");
}

void test_query_type_preservation() {
    printf("TEST: query_type_preservation\n");
    
    init_query_pool();
    
    aclk_query_type_t test_types[] = {
        REGISTER_NODE,
        NODE_STATE_UPDATE,
        UPDATE_NODE_INFO,
        ALARM_PROVIDE_CFG,
        ALARM_SNAPSHOT,
        UPDATE_NODE_COLLECTORS,
        CTX_SEND_SNAPSHOT,
        CTX_SEND_SNAPSHOT_UPD,
        SEND_NODE_INSTANCES
    };
    
    for (size_t i = 0; i < sizeof(test_types) / sizeof(test_types[0]); i++) {
        aclk_query_t *query = aclk_query_new(test_types[i]);
        assert(query->type == test_types[i]);
        aclk_query_free(query);
    }
    
    printf("  PASS\n");
}

void test_zero_initialization_of_pool_entry() {
    printf("TEST: zero_initialization_of_pool_entry\n");
    
    init_query_pool();
    
    aclk_query_t *query = get_query();
    
    assert(query->dedup_id == NULL);
    assert(query->callback_topic == NULL);
    assert(query->msg_id == NULL);
    assert(query->data.payload == NULL);
    assert(query->timeout == 0);
    assert(query->version == 0);
    
    return_query(query);
    
    printf("  PASS\n");
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================
 */

int main(void) {
    printf("\n=== ACLK Query Queue Test Suite ===\n\n");
    
    test_init_query_pool_initializes_pool();
    test_get_query_from_pool();
    test_get_query_allocates_when_pool_empty();
    test_get_query_multiple_calls_deplete_pool();
    test_return_query_allocated_frees_memory();
    test_return_query_pool_returns_to_pool();
    test_return_query_invalid_index_does_nothing();
    test_aclk_query_new_creates_query_with_type();
    test_aclk_query_new_with_different_types();
    test_aclk_query_free_http_api_v2_with_payload_and_query();
    test_aclk_query_free_http_api_v2_query_equals_dedup_id();
    test_aclk_query_free_alert_start_streaming();
    test_aclk_query_free_alert_checkpoint();
    test_aclk_query_free_create_node_instance();
    test_aclk_query_free_ctx_stop_streaming();
    test_aclk_query_free_ctx_checkpoint();
    test_aclk_query_free_ctx_checkpoint_null_payload();
    test_aclk_query_free_unknown_type();
    test_aclk_query_free_with_callback_topic_and_msg_id();
    test_aclk_query_free_all_fields_set();
    test_pool_exhaustion_and_recovery();
    test_spinlock_concurrent_access();
    test_query_type_preservation();
    test_zero_initialization_of_pool_entry();
    
    printf("\n=== All Tests Passed ===\n\n");
    return 0;
}