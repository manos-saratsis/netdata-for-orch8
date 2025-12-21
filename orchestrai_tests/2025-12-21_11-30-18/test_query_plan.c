```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "query-plan.c"

// Mock structures and functions to simulate dependencies
typedef struct {
    size_t storage_tiers;
    time_t update_every;
} MockNetdataProfile;

static MockNetdataProfile mock_nd_profile = {0};

// Function mocks and test setup
static int setup(void **state) {
    // Initialize mock data
    mock_nd_profile.storage_tiers = 3;
    mock_nd_profile.update_every = 10;
    nd_profile = mock_nd_profile;
    return 0;
}

static int teardown(void **state) {
    // Clean up any allocated resources
    return 0;
}

// Test query_metric_is_valid_tier function
static void test_query_metric_is_valid_tier(void **state) {
    QUERY_METRIC qm = {0};
    
    // Populate tiers with invalid data
    qm.tiers[0].smh = NULL;
    qm.tiers[0].db_first_time_s = 0;
    qm.tiers[0].db_last_time_s = 0;
    qm.tiers[0].db_update_every_s = 0;
    
    assert_false(query_metric_is_valid_tier(&qm, 0));
    
    // Populate with valid data
    qm.tiers[0].smh = (void*)0x1;
    qm.tiers[0].db_first_time_s = 100;
    qm.tiers[0].db_last_time_s = 200;
    qm.tiers[0].db_update_every_s = 10;
    
    assert_true(query_metric_is_valid_tier(&qm, 0));
}

// Test query_metric_first_working_tier function
static void test_query_metric_first_working_tier(void **state) {
    QUERY_METRIC qm = {0};
    
    // All tiers invalid
    assert_int_equal(query_metric_first_working_tier(&qm), 0);
    
    // Some tiers valid
    qm.tiers[1].smh = (void*)0x1;
    qm.tiers[1].db_first_time_s = 100;
    qm.tiers[1].db_last_time_s = 200;
    qm.tiers[1].db_update_every_s = 10;
    
    assert_int_equal(query_metric_first_working_tier(&qm), 1);
}

// Test query_plan_points_coverage_weight function
static void test_query_plan_points_coverage_weight(void **state) {
    // Invalid input scenarios
    assert_int_equal(query_plan_points_coverage_weight(0, 0, 0, 100, 200, 10, 0), -LONG_MAX);
    assert_int_equal(query_plan_points_coverage_weight(50, 300, 10, 100, 200, 10, 0), 1000000);
    
    // Edge case: points wanted matches available points
    long weight = query_plan_points_coverage_weight(100, 200, 10, 100, 200, 10, 1);
    assert_true(weight > 0);
}

// Test query_metric_best_tier_for_timeframe function
// Note: This would require more complex mocking due to internal complexity
static void test_query_metric_best_tier_for_timeframe(void **state) {
    QUERY_METRIC qm = {0};
    
    // Minimum tiers
    mock_nd_profile.storage_tiers = 1;
    nd_profile = mock_nd_profile;
    assert_int_equal(query_metric_best_tier_for_timeframe(&qm, 100, 200, 10), 0);
    
    // Multiple tiers
    mock_nd_profile.storage_tiers = 3;
    nd_profile = mock_nd_profile;
    
    // Populate tiers with sample data
    qm.tiers[0].smh = (void*)0x1;
    qm.tiers[0].db_first_time_s = 50;
    qm.tiers[0].db_last_time_s = 150;
    qm.tiers[0].db_update_every_s = 10;
    
    qm.tiers[1].smh = (void*)0x1;
    qm.tiers[1].db_first_time_s = 75;
    qm.tiers[1].db_last_time_s = 200;
    qm.tiers[1].db_update_every_s = 5;
    
    qm.tiers[2].smh = (void*)0x1;
    qm.tiers[2].db_first_time_s = 100;
    qm.tiers[2].db_last_time_s = 250;
    qm.tiers[2].db_update_every_s = 2;
    
    size_t best_tier = query_metric_best_tier_for_timeframe(&qm, 100, 200, 10);
    assert_true(best_tier >= 0 && best_tier < 3);
}

// Main test suite setup
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_query_metric_is_valid_tier, setup, teardown),
        cmocka_unit_test_setup_teardown(test_query_metric_first_working_tier, setup, teardown),
        cmocka_unit_test_setup_teardown(test_query_plan_points_coverage_weight, setup, teardown),
        cmocka_unit_test_setup_teardown(test_query_metric_best_tier_for_timeframe, setup, teardown),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
```

This test file covers several key functions in the `query-plan.c` file:

1. `query_metric_is_valid_tier()` - Tests both valid and invalid tier scenarios
2. `query_metric_first_working_tier()` - Checks finding the first valid tier
3. `query_plan_points_coverage_weight()` - Tests various input conditions
4. `query_metric_best_tier_for_timeframe()` - Validates tier selection logic

Key testing strategies:
- Edge cases (0 values, boundary conditions)
- Invalid input scenarios
- Multiple tier configurations
- Positive and negative test paths

Note: Complete coverage would require extensive mocking of complex internal structures like `QUERY_METRIC`, `QUERY_TARGET`, and others. The tests provide a solid starting point for verification.

Would you like me to continue generating tests for the remaining files in the list?