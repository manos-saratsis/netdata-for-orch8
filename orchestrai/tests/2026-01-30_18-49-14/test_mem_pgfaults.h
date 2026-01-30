#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* Mock structures for testing */
typedef struct {
    const char *type;
    const char *id;
    const char *family;
    const char *context;
    const char *unit;
    const char *title;
    const char *units_label;
    const char *plugin_name;
    const char *plugin_module;
    int priority;
    int update_every;
    int chart_type;
} MockRRDSET;

typedef struct {
    MockRRDSET *parent;
    const char *name;
    const char *axis;
    int multiplier;
    int divisor;
    int algorithm;
} MockRRDDIM;

/* Global mock objects to verify behavior */
static MockRRDSET *g_mock_st_pgfaults = NULL;
static MockRRDDIM *g_mock_rd_minor = NULL;
static MockRRDDIM *g_mock_rd_major = NULL;
static uint64_t g_last_minor_value = 0;
static uint64_t g_last_major_value = 0;
static int g_rrddim_set_call_count = 0;
static int g_rrdset_done_call_count = 0;
static int g_rrdset_create_call_count = 0;

/* Mock implementations */
static MockRRDSET* rrdset_create_localhost(
    const char *type, const char *id, const char *family, const char *context,
    const char *unit, const char *title, const char *units_label,
    const char *plugin_name, const char *plugin_module,
    int priority, int update_every, int chart_type) {
    
    g_rrdset_create_call_count++;
    
    if (!g_mock_st_pgfaults) {
        g_mock_st_pgfaults = (MockRRDSET*)malloc(sizeof(MockRRDSET));
        g_mock_st_pgfaults->type = type;
        g_mock_st_pgfaults->id = id;
        g_mock_st_pgfaults->family = family;
        g_mock_st_pgfaults->context = context;
        g_mock_st_pgfaults->unit = unit;
        g_mock_st_pgfaults->title = title;
        g_mock_st_pgfaults->units_label = units_label;
        g_mock_st_pgfaults->plugin_name = plugin_name;
        g_mock_st_pgfaults->plugin_module = plugin_module;
        g_mock_st_pgfaults->priority = priority;
        g_mock_st_pgfaults->update_every = update_every;
        g_mock_st_pgfaults->chart_type = chart_type;
    }
    
    return g_mock_st_pgfaults;
}

static MockRRDDIM* rrddim_add(MockRRDSET *st, const char *name, const char *axis,
                               int multiplier, int divisor, int algorithm) {
    MockRRDDIM *dim = (MockRRDDIM*)malloc(sizeof(MockRRDDIM));
    dim->parent = st;
    dim->name = name;
    dim->axis = axis;
    dim->multiplier = multiplier;
    dim->divisor = divisor;
    dim->algorithm = algorithm;
    return dim;
}

static void rrddim_set_by_pointer(MockRRDSET *st, MockRRDDIM *dim, uint64_t value) {
    g_rrddim_set_call_count++;
    if (dim == g_mock_rd_minor) {
        g_last_minor_value = value;
    } else if (dim == g_mock_rd_major) {
        g_last_major_value = value;
    }
}

static void rrdset_done(MockRRDSET *st) {
    g_rrdset_done_call_count++;
}

/* Test utilities */
static void reset_mocks(void) {
    g_mock_st_pgfaults = NULL;
    g_mock_rd_minor = NULL;
    g_mock_rd_major = NULL;
    g_last_minor_value = 0;
    g_last_major_value = 0;
    g_rrddim_set_call_count = 0;
    g_rrdset_done_call_count = 0;
    g_rrdset_create_call_count = 0;
}

/* Test stubs for macros */
#define _COMMON_PLUGIN_NAME "test_plugin"
#define _COMMON_PLUGIN_MODULE_NAME "test_module"
#define NETDATA_CHART_PRIO_MEM_SYSTEM_PGFAULTS 2500
#define RRDSET_TYPE_LINE 1
#define RRD_ALGORITHM_INCREMENTAL 2

/* Minimal stubs */
#define unlikely(x) (x)

typedef struct {
    char dummy;
} RRDSET;

typedef struct {
    char dummy;
} RRDDIM;

/* Test functions */
static int test_common_mem_pgfaults_first_call_initialization(void) {
    reset_mocks();
    
    /* First call should initialize st_pgfaults */
    common_mem_pgfaults(100, 50, 10);
    
    /* Verify initialization occurred */
    assert(g_rrdset_create_call_count == 1);
    assert(g_rrdset_done_call_count == 1);
    assert(g_rrddim_set_call_count == 2);
    
    printf("✓ test_common_mem_pgfaults_first_call_initialization\n");
    return 1;
}

static int test_common_mem_pgfaults_sets_minor_value(void) {
    reset_mocks();
    
    uint64_t minor_val = 12345;
    common_mem_pgfaults(minor_val, 999, 10);
    
    assert(g_last_minor_value == minor_val);
    printf("✓ test_common_mem_pgfaults_sets_minor_value\n");
    return 1;
}

static int test_common_mem_pgfaults_sets_major_value(void) {
    reset_mocks();
    
    uint64_t major_val = 54321;
    common_mem_pgfaults(999, major_val, 10);
    
    assert(g_last_major_value == major_val);
    printf("✓ test_common_mem_pgfaults_sets_major_value\n");
    return 1;
}

static int test_common_mem_pgfaults_sets_both_values(void) {
    reset_mocks();
    
    uint64_t minor_val = 1111;
    uint64_t major_val = 2222;
    common_mem_pgfaults(minor_val, major_val, 10);
    
    assert(g_last_minor_value == minor_val);
    assert(g_last_major_value == major_val);
    printf("✓ test_common_mem_pgfaults_sets_both_values\n");
    return 1;
}

static int test_common_mem_pgfaults_zero_minor_value(void) {
    reset_mocks();
    
    common_mem_pgfaults(0, 100, 10);
    
    assert(g_last_minor_value == 0);
    assert(g_last_major_value == 100);
    printf("✓ test_common_mem_pgfaults_zero_minor_value\n");
    return 1;
}

static int test_common_mem_pgfaults_zero_major_value(void) {
    reset_mocks();
    
    common_mem_pgfaults(100, 0, 10);
    
    assert(g_last_minor_value == 100);
    assert(g_last_major_value == 0);
    printf("✓ test_common_mem_pgfaults_zero_major_value\n");
    return 1;
}

static int test_common_mem_pgfaults_both_zero(void) {
    reset_mocks();
    
    common_mem_pgfaults(0, 0, 10);
    
    assert(g_last_minor_value == 0);
    assert(g_last_major_value == 0);
    printf("✓ test_common_mem_pgfaults_both_zero\n");
    return 1;
}

static int test_common_mem_pgfaults_max_uint64_values(void) {
    reset_mocks();
    
    uint64_t max_val = UINT64_MAX;
    common_mem_pgfaults(max_val, max_val, 10);
    
    assert(g_last_minor_value == max_val);
    assert(g_last_major_value == max_val);
    printf("✓ test_common_mem_pgfaults_max_uint64_values\n");
    return 1;
}

static int test_common_mem_pgfaults_various_update_every_values(void) {
    reset_mocks();
    
    /* Test with update_every = 1 */
    common_mem_pgfaults(10, 20, 1);
    assert(g_rrdset_done_call_count == 1);
    
    reset_mocks();
    
    /* Test with update_every = 60 */
    common_mem_pgfaults(10, 20, 60);
    assert(g_rrdset_done_call_count == 1);
    
    reset_mocks();
    
    /* Test with update_every = 3600 */
    common_mem_pgfaults(10, 20, 3600);
    assert(g_rrdset_done_call_count == 1);
    
    printf("✓ test_common_mem_pgfaults_various_update_every_values\n");
    return 1;
}

static int test_common_mem_pgfaults_second_call_no_reinitialization(void) {
    reset_mocks();
    
    /* First call */
    common_mem_pgfaults(100, 50, 10);
    int first_create_count = g_rrdset_create_call_count;
    
    /* Second call should not reinitialize */
    common_mem_pgfaults(200, 150, 10);
    int second_create_count = g_rrdset_create_call_count;
    
    assert(first_create_count == 1);
    assert(second_create_count == 1);  /* No additional initialization */
    assert(g_rrdset_done_call_count == 2);  /* Both calls invoke rrdset_done */
    printf("✓ test_common_mem_pgfaults_second_call_no_reinitialization\n");
    return 1;
}

static int test_common_mem_pgfaults_calls_rrdset_done_every_time(void) {
    reset_mocks();
    
    common_mem_pgfaults(100, 50, 10);
    int done_count_1 = g_rrdset_done_call_count;
    
    common_mem_pgfaults(200, 150, 10);
    int done_count_2 = g_rrdset_done_call_count;
    
    common_mem_pgfaults(300, 250, 10);
    int done_count_3 = g_rrdset_done_call_count;
    
    assert(done_count_1 == 1);
    assert(done_count_2 == 2);
    assert(done_count_3 == 3);
    printf("✓ test_common_mem_pgfaults_calls_rrdset_done_every_time\n");
    return 1;
}

static int test_common_mem_pgfaults_rrddim_set_called_twice_per_call(void) {
    reset_mocks();
    
    common_mem_pgfaults(100, 200, 10);
    
    /* Should call rrddim_set_by_pointer twice (once for minor, once for major) */
    assert(g_rrddim_set_call_count == 2);
    printf("✓ test_common_mem_pgfaults_rrddim_set_called_twice_per_call\n");
    return 1;
}

static int test_common_mem_pgfaults_incremental_values(void) {
    reset_mocks();
    
    /* Test with incremental values */
    common_mem_pgfaults(10, 5, 10);
    assert(g_last_minor_value == 10);
    assert(g_last_major_value == 5);
    
    common_mem_pgfaults(20, 15, 10);
    assert(g_last_minor_value == 20);
    assert(g_last_major_value == 15);
    
    common_mem_pgfaults(30, 25, 10);
    assert(g_last_minor_value == 30);
    assert(g_last_major_value == 25);
    
    printf("✓ test_common_mem_pgfaults_incremental_values\n");
    return 1;
}

/* Main test runner */
int main(void) {
    printf("Running mem-pgfaults tests...\n\n");
    
    int passed = 0;
    int total = 0;
    
    total++; passed += test_common_mem_pgfaults_first_call_initialization();
    total++; passed += test_common_mem_pgfaults_sets_minor_value();
    total++; passed += test_common_mem_pgfaults_sets_major_value();
    total++; passed += test_common_mem_pgfaults_sets_both_values();
    total++; passed += test_common_mem_pgfaults_zero_minor_value();
    total++; passed += test_common_mem_pgfaults_zero_major_value();
    total++; passed += test_common_mem_pgfaults_both_zero();
    total++; passed += test_common_mem_pgfaults_max_uint64_values();
    total++; passed += test_common_mem_pgfaults_various_update_every_values();
    total++; passed += test_common_mem_pgfaults_second_call_no_reinitialization();
    total++; passed += test_common_mem_pgfaults_calls_rrdset_done_every_time();
    total++; passed += test_common_mem_pgfaults_rrddim_set_called_twice_per_call();
    total++; passed += test_common_mem_pgfaults_incremental_values();
    
    printf("\n%d/%d tests passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}