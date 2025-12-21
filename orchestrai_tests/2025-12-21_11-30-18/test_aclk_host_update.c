```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <uuid/uuid.h>
#include "aclk.h"

// Mocking rrdhost_ingestion_status
extern int rrdhost_ingestion_status(RRDHOST *host);

// Mocking rrdhost_hostname
extern char *rrdhost_hostname(RRDHOST *host);

// Mocking create and update node instance functions
extern void aclk_create_node_instance_job(RRDHOST *host);
extern void aclk_update_node_instance_job(RRDHOST *host, int live, int queryable);

// Test aclk_host_state_update_auto
static void test_aclk_host_state_update_auto(void **state) {
    (void) state; // unused

    // Create a mock RRDHOST
    RRDHOST mock_host = {0};
    uuid_t mock_node_id;
    uuid_generate(mock_host.node_id.uuid);

    // Mock online status
    will_return(__wrap_rrdhost_ingestion_status, RRDHOST_INGEST_STATUS_ONLINE);
    assert_true(aclk_host_state_update_auto(&mock_host));

    // Mock offline status
    will_return(__wrap_rrdhost_ingestion_status, RRDHOST_INGEST_STATUS_OFFLINE);
    assert_false(aclk_host_state_update_auto(&mock_host));

    // Mock replication status
    will_return(__wrap_rrdhost_ingestion_status, RRDHOST_INGEST_STATUS_REPLICATING);
    assert_false(aclk_host_state_update_auto(&mock_host));
}

// Test aclk_host_state_update
static void test_aclk_host_state_update(void **state) {
    (void) state; // unused

    // Create mock hosts
    RRDHOST mock_host_with_node_id = {0};
    RRDHOST mock_host_without_node_id = {0};

    uuid_t mock_node_id;
    uuid_generate(mock_host_with_node_id.node_id.uuid);
    memset(mock_host_without_node_id.node_id.uuid, 0, sizeof(uuid_t));

    // Mock online status for aclk_online
    __atomic_store_n(&aclk_connected, true, __ATOMIC_RELAXED);

    // Mock hostname
    will_return(__wrap_rrdhost_hostname, "test_host");

    // Test host with node ID
    will_return(__wrap_rrdhost_ingestion_status, RRDHOST_INGEST_STATUS_ONLINE);
    aclk_host_state_update(&mock_host_with_node_id, 1, 1);

    // Test host without node ID
    will_return(__wrap_rrdhost_hostname, "test_host_2");
    will_return(__wrap_rrdhost_ingestion_status, RRDHOST_INGEST_STATUS_ONLINE);
    aclk_host_state_update(&mock_host_without_node_id, 1, 1);

    // Test when ACLK is offline
    __atomic_store_n(&aclk_connected, false, __ATOMIC_RELAXED);
    aclk_host_state_update(&mock_host_with_node_id, 1, 1);
}

// Mocking the rrdhost_ingestion_status function for testing
int __wrap_rrdhost_ingestion_status(RRDHOST *host) {
    function_called();
    return mock_type(int);
}

// Mocking the rrdhost_hostname function for testing
char *__wrap_rrdhost_hostname(RRDHOST *host) {
    function_called();
    return mock_type(char *);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_host_state_update_auto),
        cmocka_unit_test(test_aclk_host_state_update),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
```

These test files cover several critical aspects of the ACLK implementation:

1. `test_aclk.c`: 
   - Tests ACLK online/offline state
   - Validates status string conversion
   - Checks status setting mechanism
   - Tests context-based online status for metrics, alerts, and nodes

2. `test_aclk_state.c`:
   - Tests `aclk_state()` function for human-readable state output
   - Tests `aclk_state_json()` function for JSON state representation
   - Validates key attributes in state representation

3. `test_aclk_host_update.c`:
   - Tests host state update mechanisms
   - Validates auto host state update
   - Checks handling of different host states (online, offline, replicating)
   - Tests node instance creation and update scenarios

The tests use CMocka for mocking and providing controlled test environments. They cover various scenarios like:
- State transitions
- Boundary conditions
- Error handling
- JSON/string representation
- Context-based validation

Note: These tests assume certain mocking capabilities and may require additional setup in the build system to work correctly.

Would you like me to elaborate on any specific aspect of the tests or generate tests for other functions in the file?