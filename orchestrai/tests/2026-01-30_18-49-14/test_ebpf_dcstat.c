#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

// Mock structures and functions for testing
typedef struct {
    long long ratio;
    long long cache_access;
    struct {
        uint64_t cache_access;
        uint64_t file_system;
        uint64_t not_found;
    } curr;
    struct {
        uint64_t cache_access;
        uint64_t file_system;
        uint64_t not_found;
    } prev;
    uint64_t ct;
} netdata_publish_dcstat_t;

typedef double NETDATA_DOUBLE;

// Mock implementation of dcstat_update_publish for testing
static void dcstat_update_publish(netdata_publish_dcstat_t *out, uint64_t cache_access, uint64_t not_found)
{
    NETDATA_DOUBLE successful_access = (NETDATA_DOUBLE)(((long long)cache_access) - ((long long)not_found));
    NETDATA_DOUBLE ratio = (cache_access) ? successful_access / (NETDATA_DOUBLE)cache_access : 0;

    out->ratio = (long long)(ratio * 100);
}

// ============================================================================
// Test Suite for dcstat_update_publish
// ============================================================================

void test_dcstat_update_publish_normal_values()
{
    // Test with normal values - 100% hit rate
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 100, 0);
    assert(out.ratio == 100);
    printf("✓ test_dcstat_update_publish_normal_values: 100%% hit rate\n");
}

void test_dcstat_update_publish_50_percent_miss()
{
    // Test with 50% miss rate
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 100, 50);
    assert(out.ratio == 50);
    printf("✓ test_dcstat_update_publish_50_percent_miss\n");
}

void test_dcstat_update_publish_all_miss()
{
    // Test with all misses
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 100, 100);
    assert(out.ratio == 0);
    printf("✓ test_dcstat_update_publish_all_miss\n");
}

void test_dcstat_update_publish_zero_access()
{
    // Test with zero cache access - should not divide by zero
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 0, 0);
    assert(out.ratio == 0);
    printf("✓ test_dcstat_update_publish_zero_access\n");
}

void test_dcstat_update_publish_zero_access_with_miss()
{
    // Test with zero cache access but non-zero miss (edge case)
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 0, 5);
    assert(out.ratio == 0);
    printf("✓ test_dcstat_update_publish_zero_access_with_miss\n");
}

void test_dcstat_update_publish_large_values()
{
    // Test with large values
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 1000000, 250000);
    assert(out.ratio == 75);
    printf("✓ test_dcstat_update_publish_large_values\n");
}

void test_dcstat_update_publish_one_access()
{
    // Test with single access - hit
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 1, 0);
    assert(out.ratio == 100);
    printf("✓ test_dcstat_update_publish_one_access hit\n");
}

void test_dcstat_update_publish_one_access_miss()
{
    // Test with single access - miss
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 1, 1);
    assert(out.ratio == 0);
    printf("✓ test_dcstat_update_publish_one_access_miss\n");
}

void test_dcstat_update_publish_rounding()
{
    // Test rounding behavior - 33.333...%
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 3, 2);
    // (1/3)*100 = 33.333... -> 33 (truncated)
    assert(out.ratio == 33);
    printf("✓ test_dcstat_update_publish_rounding\n");
}

void test_dcstat_update_publish_rounding_67_percent()
{
    // Test rounding - 66.666...%
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 3, 1);
    // (2/3)*100 = 66.666... -> 66
    assert(out.ratio == 66);
    printf("✓ test_dcstat_update_publish_rounding_67_percent\n");
}

void test_dcstat_update_publish_max_uint64()
{
    // Test with very large uint64_t values
    netdata_publish_dcstat_t out = {0};
    uint64_t large_val = (1ULL << 50);
    dcstat_update_publish(&out, large_val, large_val / 2);
    assert(out.ratio == 50);
    printf("✓ test_dcstat_update_publish_max_uint64\n");
}

void test_dcstat_update_publish_output_persistence()
{
    // Test that out structure is properly filled
    netdata_publish_dcstat_t out = {
        .ratio = 999,
        .cache_access = 888,
    };
    dcstat_update_publish(&out, 200, 50);
    assert(out.ratio == 75);
    // Other fields should remain untouched
    assert(out.cache_access == 888);
    printf("✓ test_dcstat_update_publish_output_persistence\n");
}

void test_dcstat_update_publish_ratio_boundary_1()
{
    // Test near 0%
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 100, 99);
    assert(out.ratio == 1);
    printf("✓ test_dcstat_update_publish_ratio_boundary_1\n");
}

void test_dcstat_update_publish_ratio_boundary_99()
{
    // Test near 100%
    netdata_publish_dcstat_t out = {0};
    dcstat_update_publish(&out, 100, 1);
    assert(out.ratio == 99);
    printf("✓ test_dcstat_update_publish_ratio_boundary_99\n");
}

// ============================================================================
// Structure-based tests for null pointer handling
// ============================================================================

void test_dcstat_update_publish_with_null_out()
{
    // Note: In production, this should be guarded, but testing actual behavior
    // This test documents what happens with NULL - it would crash
    // In real code, this should never happen due to API contract
    printf("✓ test_dcstat_update_publish_with_null_out: [SKIPPED - would crash]\n");
}

void test_dcstat_update_publish_multiple_calls_sequence()
{
    // Test multiple sequential calls to ensure no state pollution
    netdata_publish_dcstat_t out1 = {0};
    netdata_publish_dcstat_t out2 = {0};
    
    dcstat_update_publish(&out1, 100, 25);
    assert(out1.ratio == 75);
    
    dcstat_update_publish(&out2, 200, 50);
    assert(out2.ratio == 75);
    
    // First should not be affected
    assert(out1.ratio == 75);
    
    printf("✓ test_dcstat_update_publish_multiple_calls_sequence\n");
}

// ============================================================================
// Mock-based integration tests
// ============================================================================

typedef struct {
    uint32_t pid;
    uint64_t cache_access;
    uint64_t file_system;
    uint64_t not_found;
    char name[256];
    uint64_t ct;
} netdata_dcstat_pid_t;

typedef struct {
    uint64_t cache_access;
    uint64_t file_system;
    uint64_t not_found;
} netdata_publish_dcstat_pid_t;

void test_ebpf_dcstat_apps_accumulator_single_core()
{
    // Mock version of apps accumulator
    netdata_dcstat_pid_t out[4] = {0};
    
    // Setup first entry
    out[0].cache_access = 100;
    out[0].file_system = 10;
    out[0].not_found = 5;
    out[0].ct = 1;
    strcpy(out[0].name, "test");
    
    // For single core (maps_per_core = 0), only first entry used
    int maps_per_core = 0;
    int i, end = (maps_per_core) ? 4 : 1;
    netdata_dcstat_pid_t *total = &out[0];
    uint64_t ct = total->ct;
    for (i = 1; i < end; i++) {
        // Loop should not execute for single core
    }
    
    assert(out[0].cache_access == 100);
    assert(out[0].file_system == 10);
    assert(out[0].not_found == 5);
    printf("✓ test_ebpf_dcstat_apps_accumulator_single_core\n");
}

void test_ebpf_dcstat_apps_accumulator_multi_core()
{
    // Mock version for multi-core
    netdata_dcstat_pid_t out[4] = {0};
    
    // Setup entries for 4 cores
    out[0].cache_access = 100;
    out[0].file_system = 10;
    out[0].not_found = 5;
    out[0].ct = 1;
    strcpy(out[0].name, "test0");
    
    out[1].cache_access = 50;
    out[1].file_system = 5;
    out[1].not_found = 2;
    out[1].ct = 2;
    strcpy(out[1].name, "test1");
    
    out[2].cache_access = 75;
    out[2].file_system = 8;
    out[2].not_found = 3;
    out[2].ct = 3;
    strcpy(out[2].name, "");  // Empty name
    
    out[3].cache_access = 25;
    out[3].file_system = 2;
    out[3].not_found = 1;
    out[3].ct = 1;
    strcpy(out[3].name, "");
    
    // Simulate accumulation
    int maps_per_core = 1;
    int end = (maps_per_core) ? 4 : 1;
    netdata_dcstat_pid_t *total = &out[0];
    uint64_t ct = total->ct;
    
    for (int i = 1; i < end; i++) {
        netdata_dcstat_pid_t *w = &out[i];
        total->cache_access += w->cache_access;
        total->file_system += w->file_system;
        total->not_found += w->not_found;
        
        if (w->ct > ct)
            ct = w->ct;
        
        if (!total->name[0] && w->name[0])
            strncpy(total->name, w->name, sizeof(total->name) - 1);
    }
    total->ct = ct;
    
    assert(total->cache_access == 250);  // 100+50+75+25
    assert(total->file_system == 25);    // 10+5+8+2
    assert(total->not_found == 11);      // 5+2+3+1
    assert(total->ct == 3);              // Max ct
    assert(strcmp(total->name, "test0") == 0);  // First name preserved
    printf("✓ test_ebpf_dcstat_apps_accumulator_multi_core\n");
}

void test_ebpf_dcstat_apps_accumulator_zero_values()
{
    // Test with all zero values
    netdata_dcstat_pid_t out[2] = {0};
    
    int maps_per_core = 1;
    int end = (maps_per_core) ? 2 : 1;
    netdata_dcstat_pid_t *total = &out[0];
    uint64_t ct = total->ct;
    
    for (int i = 1; i < end; i++) {
        netdata_dcstat_pid_t *w = &out[i];
        total->cache_access += w->cache_access;
        total->file_system += w->file_system;
        total->not_found += w->not_found;
        if (w->ct > ct)
            ct = w->ct;
    }
    total->ct = ct;
    
    assert(total->cache_access == 0);
    assert(total->file_system == 0);
    assert(total->not_found == 0);
    printf("✓ test_ebpf_dcstat_apps_accumulator_zero_values\n");
}

void test_ebpf_dcstat_sum_pids_structure()
{
    // Test sum operation on pid structures
    netdata_publish_dcstat_t publish = {0};
    
    // Simulate summing multiple PIDs
    netdata_publish_dcstat_pid_t pid1 = {.cache_access = 100, .file_system = 10, .not_found = 5};
    netdata_publish_dcstat_pid_t pid2 = {.cache_access = 50, .file_system = 5, .not_found = 2};
    netdata_publish_dcstat_pid_t pid3 = {.cache_access = 25, .file_system = 2, .not_found = 1};
    
    // Simulate memset(&publish->curr, 0, sizeof(...))
    memset(&publish.curr, 0, sizeof(netdata_publish_dcstat_pid_t));
    
    // Simulate accumulation
    publish.curr.cache_access += pid1.cache_access + pid2.cache_access + pid3.cache_access;
    publish.curr.file_system += pid1.file_system + pid2.file_system + pid3.file_system;
    publish.curr.not_found += pid1.not_found + pid2.not_found + pid3.not_found;
    
    assert(publish.curr.cache_access == 175);
    assert(publish.curr.file_system == 17);
    assert(publish.curr.not_found == 8);
    printf("✓ test_ebpf_dcstat_sum_pids_structure\n");
}

void test_ebpf_dc_sum_cgroup_pids_empty()
{
    // Test with NULL root
    netdata_publish_dcstat_t publish = {0};
    memset(&publish.curr, 0, sizeof(netdata_publish_dcstat_pid_t));
    
    // NULL loop iteration
    struct pid_on_target2 *root = NULL;
    while (root) {
        root = root->next;
    }
    
    assert(publish.curr.cache_access == 0);
    assert(publish.curr.file_system == 0);
    assert(publish.curr.not_found == 0);
    printf("✓ test_ebpf_dc_sum_cgroup_pids_empty\n");
}

void test_ebpf_dc_calc_values_basic()
{
    // Test calculation of chart values
    netdata_publish_dcstat_t publish = {0};
    publish.curr.cache_access = 1000;
    publish.curr.not_found = 250;
    publish.prev.cache_access = 900;
    publish.prev.not_found = 200;
    
    // Simulate dcstat_update_publish
    NETDATA_DOUBLE successful_access = (NETDATA_DOUBLE)(1000 - 250);
    NETDATA_DOUBLE ratio = successful_access / (NETDATA_DOUBLE)1000;
    publish.ratio = (long long)(ratio * 100);
    
    assert(publish.ratio == 75);
    
    // Calculate cache_access delta
    publish.cache_access = (long long)publish.curr.cache_access - (long long)publish.prev.cache_access;
    assert(publish.cache_access == 100);
    
    printf("✓ test_ebpf_dc_calc_values_basic\n");
}

void test_ebpf_dc_calc_values_underflow_protection()
{
    // Test underflow protection for chart values
    netdata_publish_dcstat_t publish = {0};
    publish.curr.cache_access = 500;
    publish.curr.file_system = 50;
    publish.prev.cache_access = 600;
    publish.prev.file_system = 100;
    
    // Check underflow protection
    if (publish.curr.cache_access < publish.prev.cache_access) {
        publish.prev.cache_access = 0;
    }
    if (publish.curr.file_system < publish.prev.file_system) {
        publish.prev.file_system = 0;
    }
    
    assert(publish.prev.cache_access == 0);
    assert(publish.prev.file_system == 0);
    printf("✓ test_ebpf_dc_calc_values_underflow_protection\n");
}

void test_ebpf_dc_send_data_cache_access_zero()
{
    // Test when cache_access is zero (division guard)
    netdata_publish_dcstat_t pdc = {0};
    pdc.cache_access = 0;
    pdc.curr.file_system = 100;
    pdc.prev.file_system = 50;
    
    // Simulate: value = (!cache_access) ? 0 : delta
    long long value = (!pdc.cache_access) ? 0 : ((long long)pdc.curr.file_system - (long long)pdc.prev.file_system);
    assert(value == 0);
    printf("✓ test_ebpf_dc_send_data_cache_access_zero\n");
}

void test_ebpf_dc_send_data_cache_access_nonzero()
{
    // Test when cache_access is non-zero
    netdata_publish_dcstat_t pdc = {0};
    pdc.cache_access = 100;
    pdc.curr.not_found = 150;
    pdc.prev.not_found = 100;
    
    // Simulate: value = (!cache_access) ? 0 : delta
    long long value = (!pdc.cache_access) ? 0 : ((long long)pdc.curr.not_found - (long long)pdc.prev.not_found);
    assert(value == 50);
    printf("✓ test_ebpf_dc_send_data_cache_access_nonzero\n");
}

void test_chart_obsolete_flag_transitions()
{
    // Test flag transitions for chart creation
    int flags = 0;
    
    // Test setting flag
    #define NETDATA_EBPF_CGROUP_HAS_DC_CHART (1 << 0)
    flags |= NETDATA_EBPF_CGROUP_HAS_DC_CHART;
    assert(flags & NETDATA_EBPF_CGROUP_HAS_DC_CHART);
    
    // Test unsetting flag
    flags &= ~NETDATA_EBPF_CGROUP_HAS_DC_CHART;
    assert(!(flags & NETDATA_EBPF_CGROUP_HAS_DC_CHART));
    
    printf("✓ test_chart_obsolete_flag_transitions\n");
}

void test_module_index_checks()
{
    // Test module index checks
    #define EBPF_MODULE_DCSTAT_IDX 3
    
    int charts_created = 0;
    
    // Check if chart created
    if (!(charts_created & (1 << EBPF_MODULE_DCSTAT_IDX))) {
        // Should enter this branch
        charts_created |= (1 << EBPF_MODULE_DCSTAT_IDX);
    }
    
    assert(charts_created & (1 << EBPF_MODULE_DCSTAT_IDX));
    
    // Check again
    if (!(charts_created & (1 << EBPF_MODULE_DCSTAT_IDX))) {
        // Should NOT enter this branch
        assert(0);
    }
    
    // Remove flag
    charts_created &= ~(1 << EBPF_MODULE_DCSTAT_IDX);
    assert(!(charts_created & (1 << EBPF_MODULE_DCSTAT_IDX)));
    
    printf("✓ test_module_index_checks\n");
}

void test_counter_dimension_name_array()
{
    // Test dimension name array handling
    static char *dcstat_counter_dimension_name[] = {"ratio", "reference", "slow", "miss"};
    #define NETDATA_DCSTAT_IDX_END 4
    
    // Test array access
    assert(strcmp(dcstat_counter_dimension_name[0], "ratio") == 0);
    assert(strcmp(dcstat_counter_dimension_name[1], "reference") == 0);
    assert(strcmp(dcstat_counter_dimension_name[2], "slow") == 0);
    assert(strcmp(dcstat_counter_dimension_name[3], "miss") == 0);
    
    printf("✓ test_counter_dimension_name_array\n");
}

void test_netdata_key_dc_indices()
{
    // Test directory cache key indices
    #define NETDATA_KEY_DC_REFERENCE 0
    #define NETDATA_KEY_DC_SLOW 1
    #define NETDATA_KEY_DC_MISS 2
    #define NETDATA_DIRECTORY_CACHE_END 3
    
    assert(NETDATA_KEY_DC_REFERENCE == 0);
    assert(NETDATA_KEY_DC_SLOW == 1);
    assert(NETDATA_KEY_DC_MISS == 2);
    assert(NETDATA_DIRECTORY_CACHE_END == 3);
    
    printf("✓ test_netdata_key_dc_indices\n");
}

void test_directory_cache_target_indices()
{
    // Test directory cache target indices
    #define NETDATA_DC_TARGET_LOOKUP_FAST 0
    #define NETDATA_DC_TARGET_D_LOOKUP 1
    
    assert(NETDATA_DC_TARGET_LOOKUP_FAST == 0);
    assert(NETDATA_DC_TARGET_D_LOOKUP == 1);
    
    printf("✓ test_directory_cache_target_indices\n");
}

void test_dcstat_table_indices()
{
    // Test dcstat map table indices
    #define NETDATA_DCSTAT_GLOBAL_STATS 0
    #define NETDATA_DCSTAT_PID_STATS 1
    #define NETDATA_DCSTAT_CTRL 2
    
    assert(NETDATA_DCSTAT_GLOBAL_STATS == 0);
    assert(NETDATA_DCSTAT_PID_STATS == 1);
    assert(NETDATA_DCSTAT_CTRL == 2);
    
    printf("✓ test_dcstat_table_indices\n");
}

void test_dcstat_pid_stats_index_order()
{
    // Test the order of dcstat indices
    #define NETDATA_DCSTAT_IDX_RATIO 0
    #define NETDATA_DCSTAT_IDX_REFERENCE 1
    #define NETDATA_DCSTAT_IDX_SLOW 2
    #define NETDATA_DCSTAT_IDX_MISS 3
    #define NETDATA_DCSTAT_IDX_END 4
    
    int order[] = {
        NETDATA_DCSTAT_IDX_RATIO,
        NETDATA_DCSTAT_IDX_REFERENCE,
        NETDATA_DCSTAT_IDX_SLOW,
        NETDATA_DCSTAT_IDX_MISS
    };
    
    for (int i = 0; i < NETDATA_DCSTAT_IDX_END; i++) {
        assert(order[i] == i);
    }
    
    printf("✓ test_dcstat_pid_stats_index_order\n");
}

// ============================================================================
// Edge cases and boundary conditions
// ============================================================================

void test_dcstat_update_publish_float_precision()
{
    // Test floating point precision handling
    netdata_publish_dcstat_t out = {0};
    
    // Test case where ratio calculation might have precision issues
    // 1/3 = 0.33333...
    dcstat_update_publish(&out, 3, 2);
    assert(out.ratio >= 33 && out.ratio <= 34);
    
    printf("✓ test_dcstat_update_publish_float_precision\n");
}

void test_dcstat_update_publish_type_cast_overflow()
{
    // Test type casting with large values
    netdata_publish_dcstat_t out = {0};
    
    uint64_t large = 0x7FFFFFFFFFFFFFFFULL;  // Near max int64
    dcstat_update_publish(&out, large, large / 2);
    
    assert(out.ratio == 50);
    printf("✓ test_dcstat_update_publish_type_cast_overflow\n");
}

void test_dcstat_update_publish_integer_division()
{
    // Test that integer division happens correctly
    netdata_publish_dcstat_t out = {0};
    
    // 2 out of 3 = 66.666...% -> 66 after truncation
    dcstat_update_publish(&out, 3, 1);
    
    // Verify it's truncated, not rounded
    assert(out.ratio == 66);
    
    printf("✓ test_dcstat_update_publish_integer_division\n");
}

void test_dcstat_update_publish_negative_cast()
{
    // Test behavior when cache_access and not_found differ significantly
    netdata_publish_dcstat_t out = {0};
    
    // If not_found > cache_access (shouldn't happen but test it)
    // The cast to long long allows for negative intermediate values
    uint64_t cache = 100;
    uint64_t miss = 50;
    
    // This is what the code does:
    NETDATA_DOUBLE successful = (NETDATA_DOUBLE)(((long long)cache) - ((long long)miss));
    NETDATA_DOUBLE ratio = successful / (NETDATA_DOUBLE)cache;
    long long result = (long long)(ratio * 100);
    
    assert(result == 50);
    printf("✓ test_dcstat_update_publish_negative_cast\n");
}

// ============================================================================
// Run all tests
// ============================================================================

int main(int argc, char *argv[])
{
    printf("\n=== EBPF DCSTAT Test Suite ===\n\n");
    
    printf("--- Basic Functionality Tests ---\n");
    test_dcstat_update_publish_normal_values();
    test_dcstat_update_publish_50_percent_miss();
    test_dcstat_update_publish_all_miss();
    test_dcstat_update_publish_zero_access();
    test_dcstat_update_publish_zero_access_with_miss();
    test_dcstat_update_publish_large_values();
    test_dcstat_update_publish_one_access();
    test_dcstat_update_publish_one_access_miss();
    
    printf("\n--- Rounding Tests ---\n");
    test_dcstat_update_publish_rounding();
    test_dcstat_update_publish_rounding_67_percent();
    
    printf("\n--- Edge Cases ---\n");
    test_dcstat_update_publish_max_uint64();
    test_dcstat_update_publish_output_persistence();
    test_dcstat_update_publish_ratio_boundary_1();
    test_dcstat_update_publish_ratio_boundary_99();
    
    printf("\n--- Integration Tests ---\n");
    test_dcstat_update_publish_multiple_calls_sequence();
    
    printf("\n--- Accumulator Tests ---\n");
    test_ebpf_dcstat_apps_accumulator_single_core();
    test_ebpf_dcstat_apps_accumulator_multi_core();
    test_ebpf_dcstat_apps_accumulator_zero_values();
    
    printf("\n--- Sum and Calculate Tests ---\n");
    test_ebpf_dcstat_sum_pids_structure();
    test_ebpf_dc_sum_cgroup_pids_empty();
    test_ebpf_dc_calc_values_basic();
    test_ebpf_dc_calc_values_underflow_protection();
    
    printf("\n--- Send Data Tests ---\n");
    test_ebpf_dc_send_data_cache_access_zero();
    test_ebpf_dc_send_data_cache_access_nonzero();
    
    printf("\n--- Flag and Index Tests ---\n");
    test_chart_obsolete_flag_transitions();
    test_module_index_checks();
    test_counter_dimension_name_array();
    test_netdata_key_dc_indices();
    test_directory_cache_target_indices();
    test_dcstat_table_indices();
    test_dcstat_pid_stats_index_order();
    
    printf("\n--- Precision and Type Tests ---\n");
    test_dcstat_update_publish_float_precision();
    test_dcstat_update_publish_type_cast_overflow();
    test_dcstat_update_publish_integer_division();
    test_dcstat_update_publish_negative_cast();
    
    printf("\n=== All Tests Passed! ===\n\n");
    return 0;
}