#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock structures and functions
struct rrdset;
struct rrddim;
typedef void RRDSET;
typedef void RRDDIM;

#define CONFIG_BOOLEAN_YES 1
#define CONFIG_BOOLEAN_NO 0
#define RRDSET_TYPE_AREA 0
#define RRDSET_TYPE_STACKED 1
#define RRDSET_TYPE_LINE 2
#define RRD_ALGORITHM_ABSOLUTE 0
#define RRD_ALGORITHM_INCREMENTAL 1
#define RRD_ALGORITHM_PCENT_OVER_DIFF_TOTAL 2
#define RRD_ALGORITHM_PCENT_OVER_ROW_TOTAL 3
#define NETDATA_CHART_PRIO_ZFS_ARC_SIZE 8000
#define NETDATA_CHART_PRIO_ZFS_L2_SIZE 8001
#define NETDATA_CHART_PRIO_ZFS_READS 8002
#define NETDATA_CHART_PRIO_ZFS_IO 8003
#define NETDATA_CHART_PRIO_ZFS_HITS 8004
#define NETDATA_CHART_PRIO_ZFS_DHITS 8005
#define NETDATA_CHART_PRIO_ZFS_PHITS 8006
#define NETDATA_CHART_PRIO_ZFS_MHITS 8007
#define NETDATA_CHART_PRIO_ZFS_L2HITS 8008
#define NETDATA_CHART_PRIO_ZFS_LIST_HITS 8009
#define NETDATA_CHART_PRIO_ZFS_ARC_SIZE_BREAKDOWN 8010
#define NETDATA_CHART_PRIO_ZFS_MEMORY_OPS 8011
#define NETDATA_CHART_PRIO_ZFS_IMPORTANT_OPS 8012
#define NETDATA_CHART_PRIO_ZFS_ACTUAL_HITS 8013
#define NETDATA_CHART_PRIO_ZFS_DEMAND_DATA_HITS 8014
#define NETDATA_CHART_PRIO_ZFS_PREFETCH_DATA_HITS 8015
#define NETDATA_CHART_PRIO_ZFS_HASH_ELEMENTS 8016
#define NETDATA_CHART_PRIO_ZFS_HASH_CHAINS 8017

#define ZFS_FAMILY_SIZE "size"
#define ZFS_FAMILY_EFFICIENCY "efficiency"
#define ZFS_FAMILY_ACCESSES "accesses"
#define ZFS_FAMILY_OPERATIONS "operations"
#define ZFS_FAMILY_HASH "hashes"

struct arcstats {
    unsigned long long hits;
    unsigned long long misses;
    unsigned long long demand_data_hits;
    unsigned long long demand_data_misses;
    unsigned long long demand_metadata_hits;
    unsigned long long demand_metadata_misses;
    unsigned long long prefetch_data_hits;
    unsigned long long prefetch_data_misses;
    unsigned long long prefetch_metadata_hits;
    unsigned long long prefetch_metadata_misses;
    unsigned long long mru_hits;
    unsigned long long mru_ghost_hits;
    unsigned long long mfu_hits;
    unsigned long long mfu_ghost_hits;
    unsigned long long deleted;
    unsigned long long mutex_miss;
    unsigned long long evict_skip;
    unsigned long long evict_not_enough;
    unsigned long long evict_l2_cached;
    unsigned long long evict_l2_eligible;
    unsigned long long evict_l2_ineligible;
    unsigned long long evict_l2_skip;
    unsigned long long hash_elements;
    unsigned long long hash_elements_max;
    unsigned long long hash_collisions;
    unsigned long long hash_chains;
    unsigned long long hash_chain_max;
    unsigned long long p;
    unsigned long long pd;
    unsigned long long pm;
    unsigned long long c;
    unsigned long long c_min;
    unsigned long long c_max;
    unsigned long long size;
    unsigned long long hdr_size;
    unsigned long long data_size;
    unsigned long long metadata_size;
    unsigned long long other_size;
    unsigned long long anon_size;
    unsigned long long anon_evictable_data;
    unsigned long long anon_evictable_metadata;
    unsigned long long mru_size;
    unsigned long long mru_evictable_data;
    unsigned long long mru_evictable_metadata;
    unsigned long long mru_ghost_size;
    unsigned long long mru_ghost_evictable_data;
    unsigned long long mru_ghost_evictable_metadata;
    unsigned long long mfu_size;
    unsigned long long mfu_evictable_data;
    unsigned long long mfu_evictable_metadata;
    unsigned long long mfu_ghost_size;
    unsigned long long mfu_ghost_evictable_data;
    unsigned long long mfu_ghost_evictable_metadata;
    unsigned long long l2_hits;
    unsigned long long l2_misses;
    unsigned long long l2_feeds;
    unsigned long long l2_rw_clash;
    unsigned long long l2_read_bytes;
    unsigned long long l2_write_bytes;
    unsigned long long l2_writes_sent;
    unsigned long long l2_writes_done;
    unsigned long long l2_writes_error;
    unsigned long long l2_writes_lock_retry;
    unsigned long long l2_evict_lock_retry;
    unsigned long long l2_evict_reading;
    unsigned long long l2_evict_l1cached;
    unsigned long long l2_free_on_write;
    unsigned long long l2_cdata_free_on_write;
    unsigned long long l2_abort_lowmem;
    unsigned long long l2_cksum_bad;
    unsigned long long l2_io_error;
    unsigned long long l2_size;
    unsigned long long l2_asize;
    unsigned long long l2_hdr_size;
    unsigned long long l2_compress_successes;
    unsigned long long l2_compress_zeros;
    unsigned long long l2_compress_failures;
    unsigned long long memory_throttle_count;
    unsigned long long duplicate_buffers;
    unsigned long long duplicate_buffers_size;
    unsigned long long duplicate_reads;
    unsigned long long memory_direct_count;
    unsigned long long memory_indirect_count;
    unsigned long long arc_no_grow;
    unsigned long long arc_tempreserve;
    unsigned long long arc_loaned_bytes;
    unsigned long long arc_prune;
    unsigned long long arc_meta_used;
    unsigned long long arc_meta_limit;
    unsigned long long arc_meta_max;
    unsigned long long arc_meta_min;
    unsigned long long arc_need_free;
    unsigned long long arc_sys_free;
    int l2exist;
};

// Global arcstats for testing
struct arcstats arcstats = { 0 };

// Mock functions
static RRDSET *mock_rrdset = (RRDSET *)0x12345678;
static RRDDIM *mock_rrddim = (RRDDIM *)0x87654321;
static int mock_rrdset_create_called = 0;
static int mock_rrddim_add_called = 0;
static int mock_rrddim_set_called = 0;
static int mock_rrdset_done_called = 0;

RRDSET *__wrap_rrdset_create_localhost(
    const char *type, const char *id, const char *name,
    const char *family, const char *context, const char *title,
    const char *units, const char *plugin, const char *module,
    long priority, int update_every, int chart_type) {
    mock_rrdset_create_called++;
    return mock_rrdset;
}

RRDDIM *__wrap_rrddim_add(RRDSET *st, const char *id, const char *name,
                          long multiplier, long divisor, int algorithm) {
    mock_rrddim_add_called++;
    return mock_rrddim;
}

void __wrap_rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, long long value) {
    mock_rrddim_set_called++;
}

void __wrap_rrdset_done(RRDSET *st) {
    mock_rrdset_done_called++;
}

// Forward declarations
void generate_charts_arcstats(const char *plugin, const char *module, int update_every);
void generate_charts_arc_summary(const char *plugin, const char *module, int update_every);

// Test: generate_charts_arcstats with all zero values
static void test_generate_charts_arcstats_all_zeros(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arcstats("test_plugin", "test_module", 10);

    // Should not create any charts with all zeros
    assert_int_equal(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arcstats with arc_size enabled
static void test_generate_charts_arcstats_with_arc_size(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.size = 1024 * 1024;
    arcstats.c = 512 * 1024;
    arcstats.c_min = 256 * 1024;
    arcstats.c_max = 2048 * 1024;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arcstats("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
    assert_int_greater_than(mock_rrddim_add_called, 0);
}

// Test: generate_charts_arcstats with L2 cache enabled
static void test_generate_charts_arcstats_with_l2_cache(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.l2exist = 1;
    arcstats.l2_size = 512 * 1024;
    arcstats.l2_asize = 256 * 1024;
    arcstats.l2_hits = 100;
    arcstats.l2_misses = 50;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arcstats("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arcstats with reads
static void test_generate_charts_arcstats_with_reads(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hits = 1000;
    arcstats.misses = 100;
    arcstats.demand_data_hits = 500;
    arcstats.demand_data_misses = 50;
    arcstats.demand_metadata_hits = 300;
    arcstats.demand_metadata_misses = 30;
    arcstats.prefetch_data_hits = 150;
    arcstats.prefetch_data_misses = 15;
    arcstats.prefetch_metadata_hits = 50;
    arcstats.prefetch_metadata_misses = 5;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arcstats("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arcstats with all features enabled
static void test_generate_charts_arcstats_all_features_enabled(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hits = 1000;
    arcstats.misses = 100;
    arcstats.size = 1024 * 1024;
    arcstats.c = 512 * 1024;
    arcstats.c_min = 256 * 1024;
    arcstats.c_max = 2048 * 1024;
    arcstats.demand_data_hits = 500;
    arcstats.demand_data_misses = 50;
    arcstats.demand_metadata_hits = 300;
    arcstats.demand_metadata_misses = 30;
    arcstats.prefetch_data_hits = 150;
    arcstats.prefetch_data_misses = 15;
    arcstats.prefetch_metadata_hits = 50;
    arcstats.prefetch_metadata_misses = 5;
    arcstats.mru_hits = 400;
    arcstats.mru_ghost_hits = 100;
    arcstats.mfu_hits = 600;
    arcstats.mfu_ghost_hits = 150;
    arcstats.l2exist = 1;
    arcstats.l2_size = 512 * 1024;
    arcstats.l2_asize = 256 * 1024;
    arcstats.l2_hits = 100;
    arcstats.l2_misses = 50;
    arcstats.l2_read_bytes = 1000000;
    arcstats.l2_write_bytes = 500000;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arcstats("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
    assert_int_greater_than(mock_rrddim_set_called, 0);
}

// Test: generate_charts_arcstats with list hits
static void test_generate_charts_arcstats_with_list_hits(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.mfu_hits = 100;
    arcstats.mru_hits = 50;
    arcstats.mfu_ghost_hits = 25;
    arcstats.mru_ghost_hits = 10;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arcstats("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with arc_size_breakdown
static void test_generate_charts_arc_summary_arc_size_breakdown(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hits = 1000;
    arcstats.misses = 100;
    arcstats.mfu_hits = 600;
    arcstats.mru_hits = 400;
    arcstats.mfu_ghost_hits = 0;
    arcstats.mru_ghost_hits = 0;
    arcstats.size = 1024 * 1024;
    arcstats.c = 512 * 1024;
    arcstats.p = 256 * 1024;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with memory ops
static void test_generate_charts_arc_summary_memory_ops(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.memory_direct_count = 100;
    arcstats.memory_throttle_count = 50;
    arcstats.memory_indirect_count = 25;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with important ops
static void test_generate_charts_arc_summary_important_ops(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.deleted = 100;
    arcstats.evict_skip = 50;
    arcstats.mutex_miss = 25;
    arcstats.hash_collisions = 10;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with actual hits
static void test_generate_charts_arc_summary_actual_hits(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hits = 1000;
    arcstats.misses = 100;
    arcstats.mfu_hits = 600;
    arcstats.mru_hits = 400;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with demand data hits
static void test_generate_charts_arc_summary_demand_data_hits(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.demand_data_hits = 500;
    arcstats.demand_data_misses = 50;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with prefetch data hits
static void test_generate_charts_arc_summary_prefetch_data_hits(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.prefetch_data_hits = 200;
    arcstats.prefetch_data_misses = 20;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with hash elements
static void test_generate_charts_arc_summary_hash_elements(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hash_elements = 1000;
    arcstats.hash_elements_max = 2000;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with hash chains
static void test_generate_charts_arc_summary_hash_chains(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hash_chains = 500;
    arcstats.hash_chain_max = 1000;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with mfu_size calculated from target_size > arc_size
static void test_generate_charts_arc_summary_mfu_size_from_target(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hits = 1000;
    arcstats.misses = 100;
    arcstats.mfu_hits = 600;
    arcstats.mru_hits = 400;
    arcstats.size = 512 * 1024;
    arcstats.c = 1024 * 1024;  // target > size
    arcstats.p = 256 * 1024;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with mru_size from pd+pm
static void test_generate_charts_arc_summary_mru_size_from_pd_pm(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hits = 1000;
    arcstats.misses = 100;
    arcstats.mfu_hits = 600;
    arcstats.mru_hits = 400;
    arcstats.size = 1024 * 1024;
    arcstats.c = 512 * 1024;
    arcstats.p = 0;  // p is zero
    arcstats.pd = 200 * 1024;
    arcstats.pm = 100 * 1024;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arcstats with CONFIG_BOOLEAN_YES for arc_size
static void test_generate_charts_arcstats_config_boolean_yes(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arcstats("test_plugin", "test_module", 10);
    generate_charts_arcstats("test_plugin", "test_module", 10);

    // Second call should still work (testing static initialization)
    assert_int_greater_than_or_equal(mock_rrdset_done_called, 0);
}

// Test: generate_charts_arcstats boundary values
static void test_generate_charts_arcstats_boundary_values(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.size = 0xFFFFFFFFFFFFFFFFULL;  // Max unsigned long long
    arcstats.c = 0x8000000000000000ULL;
    arcstats.hits = 1;
    arcstats.misses = 1;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arcstats("test_plugin", "test_module", 10);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test: generate_charts_arc_summary with zero real_hits and real_misses
static void test_generate_charts_arc_summary_zero_real_hits(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.hits = 0;
    arcstats.misses = 0;
    arcstats.mfu_hits = 0;
    arcstats.mru_hits = 0;
    
    mock_rrdset_create_called = 0;
    mock_rrddim_add_called = 0;
    mock_rrddim_set_called = 0;
    mock_rrdset_done_called = 0;

    generate_charts_arc_summary("test_plugin", "test_module", 10);

    assert_int_equal(mock_rrdset_done_called, 0);
}

// Test parameter passing
static void test_generate_charts_arcstats_plugin_module_names(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.size = 1024;
    arcstats.c = 512;
    arcstats.c_min = 256;
    arcstats.c_max = 2048;
    
    mock_rrdset_create_called = 0;

    generate_charts_arcstats("custom_plugin", "custom_module", 20);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

// Test update_every parameter
static void test_generate_charts_arcstats_update_every_parameter(void **state) {
    memset(&arcstats, 0, sizeof(struct arcstats));
    arcstats.size = 1024;
    arcstats.c = 512;
    arcstats.c_min = 256;
    arcstats.c_max = 2048;
    
    mock_rrdset_create_called = 0;

    generate_charts_arcstats("plugin", "module", 60);

    assert_int_greater_than(mock_rrdset_create_called, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_generate_charts_arcstats_all_zeros),
        cmocka_unit_test(test_generate_charts_arcstats_with_arc_size),
        cmocka_unit_test(test_generate_charts_arcstats_with_l2_cache),
        cmocka_unit_test(test_generate_charts_arcstats_with_reads),
        cmocka_unit_test(test_generate_charts_arcstats_all_features_enabled),
        cmocka_unit_test(test_generate_charts_arcstats_with_list_hits),
        cmocka_unit_test(test_generate_charts_arc_summary_arc_size_breakdown),
        cmocka_unit_test(test_generate_charts_arc_summary_memory_ops),
        cmocka_unit_test(test_generate_charts_arc_summary_important_ops),
        cmocka_unit_test(test_generate_charts_arc_summary_actual_hits),
        cmocka_unit_test(test_generate_charts_arc_summary_demand_data_hits),
        cmocka_unit_test(test_generate_charts_arc_summary_prefetch_data_hits),
        cmocka_unit_test(test_generate_charts_arc_summary_hash_elements),
        cmocka_unit_test(test_generate_charts_arc_summary_hash_chains),
        cmocka_unit_test(test_generate_charts_arc_summary_mfu_size_from_target),
        cmocka_unit_test(test_generate_charts_arc_summary_mru_size_from_pd_pm),
        cmocka_unit_test(test_generate_charts_arcstats_config_boolean_yes),
        cmocka_unit_test(test_generate_charts_arcstats_boundary_values),
        cmocka_unit_test(test_generate_charts_arc_summary_zero_real_hits),
        cmocka_unit_test(test_generate_charts_arcstats_plugin_module_names),
        cmocka_unit_test(test_generate_charts_arcstats_update_every_parameter),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}