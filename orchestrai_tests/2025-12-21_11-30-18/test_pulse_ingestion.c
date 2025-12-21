#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "pulse-ingestion.h"

// Mock structures and functions for dependencies
typedef struct RRDSET {} RRDSET;
typedef struct RRDDIM {} RRDDIM;

RRDSET* rrdset_create_localhost(const char* a, const char* b, const char* c, 
                                 const char* d, const char* e, const char* f, 
                                 const char* g, const char* h, const char* i, 
                                 int j, int k, int l) { return (RRDSET*)1; }
RRDDIM* rrddim_add(RRDSET* st, const char* a, const char* b, int c, int d, int e) { return (RRDDIM*)1; }
void rrddim_set_by_pointer(RRDSET* st, RRDDIM* rd, long long value) {}
void rrdset_done(RRDSET* st) {}

// Simulating nd_profile struct
struct nd_profile {
    size_t storage_tiers;
} nd_profile = { .storage_tiers = 3 }; // Assumed number of tiers

// Test pulse_queries_rrdset_collection_completed
static void test_pulse_queries_rrdset_collection_completed(void **state) {
    (void) state; // unused
    
    // Simulate points read per tier
    size_t points_read_per_tier_array[3] = {100, 200, 300};
    
    // Call the function
    pulse_queries_rrdset_collection_completed(points_read_per_tier_array);
    
    // Verify points are reset after function call
    for(size_t i = 0; i < 3; i++) {
        assert_int_equal(points_read_per_tier_array[i], 0);
    }
}

// Test pulse_ingestion_do
static void test_pulse_ingestion_do(void **state) {
    (void) state; // unused
    
    // Test without extended stats
    pulse_ingestion_do(false);
    
    // Test with extended stats
    pulse_ingestion_do(true);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pulse_queries_rrdset_collection_completed),
        cmocka_unit_test(test_pulse_ingestion_do),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}