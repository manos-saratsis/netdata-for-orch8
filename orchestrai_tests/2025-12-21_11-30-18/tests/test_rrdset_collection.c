```c
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../src/database/rrdset-collection.h"
#include "../src/database/rrdset.h"

// Mock function to simulate service status
bool mock_service_running(SERVICE_STATUS status) {
    return true;
}

// Test rrdset_set_update_every_s with different scenarios
void test_rrdset_set_update_every_s() {
    // Create a mock RRDSET
    RRDSET mock_st = {0};
    mock_st.update_every = 10;
    mock_st.rrdhost = malloc(sizeof(RRDHOST));
    mock_st.rrdhost->db = malloc(sizeof(STORAGE_TIER) * 3);  // 3 tiers
    
    // Test no change scenario
    time_t result = rrdset_set_update_every_s(&mock_st, 10);
    assert(result == 10);
    
    // Test change scenario
    result = rrdset_set_update_every_s(&mock_st, 20);
    assert(result == 10);
    assert(mock_st.update_every == 20);
    
    free(mock_st.rrdhost->db);
    free(mock_st.rrdhost);
}

// Test rrdset_finalize_collection with mock data
void test_rrdset_finalize_collection() {
    // Create mock RRDSET
    RRDSET mock_st = {0};
    mock_st.rrdhost = malloc(sizeof(RRDHOST));
    mock_st.rrdhost->db = malloc(sizeof(STORAGE_TIER) * 3);  // 3 tiers
    
    // Simulate flags and dimensions
    rrdset_flag_set(&mock_st, RRDSET_FLAG_COLLECTION_FINISHED);
    
    // Call function
    rrdset_finalize_collection(&mock_st, true);
    
    // Assert expected state (ideally with more setup and precise checks)
    assert(rrdset_flag_check(&mock_st, RRDSET_FLAG_COLLECTION_FINISHED));
    
    free(mock_st.rrdhost->db);
    free(mock_st.rrdhost);
}

// Test rrdset_collection_reset internal function
void test_rrdset_collection_reset() {
    RRDSET mock_st = {0};
    mock_st.last_collected_time.tv_sec = 100;
    mock_st.last_collected_time.tv_usec = 500000;
    mock_st.last_updated.tv_sec = 100;
    mock_st.last_updated.tv_usec = 500000;
    mock_st.counter = 10;
    mock_st.counter_done = 5;
    
    // Use internal function to reset
    rrdset_collection_reset(&mock_st);
    
    // Check reset values
    assert(mock_st.last_collected_time.tv_sec == 0);
    assert(mock_st.last_collected_time.tv_usec == 0);
    assert(mock_st.last_updated.tv_sec == 0);
    assert(mock_st.last_updated.tv_usec == 0);
    assert(mock_st.counter == 0);
    assert(mock_st.counter_done == 0);
}

int main() {
    test_rrdset_set_update_every_s();
    test_rrdset_finalize_collection();
    test_rrdset_collection_reset();
    
    printf("All rrdset collection tests passed!\n");
    return 0;
}
```