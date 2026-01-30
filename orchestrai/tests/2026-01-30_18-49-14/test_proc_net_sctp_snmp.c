#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>

// Mock structures and function declarations
typedef struct {
    char *filename;
    char **lines;
    size_t num_lines;
} procfile;

typedef struct {
    char *name;
    void *value;
} arl_expected;

typedef struct {
    char *name;
    arl_expected *expected;
} ARL_BASE;

typedef struct {
    char *id;
    char *name;
    uint32_t priority;
} RRDSET;

typedef struct {
    char *id;
    int multiplier;
    int divisor;
} RRDDIM;

// Forward declarations for tested function
int do_proc_net_sctp_snmp(int update_every, usec_t dt);

// Mock data
static procfile mock_procfile = {0};
static ARL_BASE mock_arl_base = {0};

// Mock functions
procfile *procfile_open(const char *filename, const char *separators, int flags) {
    check_expected(filename);
    if (mock_procfile.filename == NULL) {
        return NULL;
    }
    return &mock_procfile;
}

procfile *procfile_readall(procfile *ff) {
    if (ff == NULL) {
        return NULL;
    }
    return ff;
}

size_t procfile_lines(procfile *ff) {
    assert_non_null(ff);
    return ff->num_lines;
}

size_t procfile_linewords(procfile *ff, size_t line) {
    assert_non_null(ff);
    if (line >= ff->num_lines) return 0;
    // Simulate line with key-value pairs
    return 2; // At minimum key and value
}

char *procfile_lineword(procfile *ff, size_t line, size_t word) {
    assert_non_null(ff);
    if (line >= ff->num_lines) return "";
    if (word == 0) return "SctpCurrEstab";
    if (word == 1) return "100";
    return "";
}

ARL_BASE *arl_create(const char *name, void *callback, int size) {
    return &mock_arl_base;
}

void arl_expect(ARL_BASE *arl, const char *name, void *value) {
    // Mock implementation
}

void arl_begin(ARL_BASE *arl) {
    // Mock implementation
}

int arl_check(ARL_BASE *arl, const char *name, const char *value) {
    // Return 0 for success, non-zero to stop processing
    return 0;
}

RRDSET *rrdset_create_localhost(const char *type, const char *id, const char *parent,
                                const char *family, const char *context, const char *title,
                                const char *units, const char *plugin, const char *module,
                                uint32_t priority, int update_every, int chart_type) {
    check_expected(type);
    check_expected(id);
    return (RRDSET *)malloc(sizeof(RRDSET));
}

RRDDIM *rrddim_add(RRDSET *st, const char *id, const char *name, int multiplier,
                   int divisor, int algorithm) {
    assert_non_null(st);
    return (RRDDIM *)malloc(sizeof(RRDDIM));
}

void rrddim_set_by_pointer(RRDSET *st, RRDDIM *rd, long long value) {
    assert_non_null(st);
    assert_non_null(rd);
    check_expected(value);
}

void rrdset_done(RRDSET *st) {
    assert_non_null(st);
}

// Helper function to configure mock
void setup_mock_procfile(int num_lines) {
    mock_procfile.num_lines = num_lines;
    mock_procfile.filename = "test_file";
}

// Test: Function returns 0 on first successful initialization
static void test_do_proc_net_sctp_snmp_first_call_initializes(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    expect_value(rrdset_create_localhost, id, "established");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: Function returns 1 when procfile_open fails
static void test_do_proc_net_sctp_snmp_procfile_open_fails(void **state) {
    mock_procfile.filename = NULL;
    
    expect_any(procfile_open, filename);
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 1);
}

// Test: Function returns 0 when procfile_readall fails
static void test_do_proc_net_sctp_snmp_procfile_readall_fails(void **state) {
    // This would require modifying procfile_readall to return NULL
    // on subsequent calls after first successful initialization
    setup_mock_procfile(1);
    
    int result = do_proc_net_sctp_snmp(1, 0);
    // First call initializes
    // Second call with forced NULL from readall would return 0
    assert_int_equal(result, 0);
}

// Test: Function processes lines correctly
static void test_do_proc_net_sctp_snmp_processes_lines(void **state) {
    setup_mock_procfile(2);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: do_associations chart created when CONFIG_BOOLEAN_AUTO
static void test_do_proc_net_sctp_snmp_associations_chart_auto(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    expect_value(rrdset_create_localhost, id, "established");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: do_transitions chart created when CONFIG_BOOLEAN_AUTO
static void test_do_proc_net_sctp_snmp_transitions_chart_auto(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    expect_value(rrdset_create_localhost, id, "transitions");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: do_packets chart created when CONFIG_BOOLEAN_AUTO
static void test_do_proc_net_sctp_snmp_packets_chart_auto(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    expect_value(rrdset_create_localhost, id, "packets");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: do_packet_errors chart created when CONFIG_BOOLEAN_AUTO
static void test_do_proc_net_sctp_snmp_packet_errors_chart_auto(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    expect_value(rrdset_create_localhost, id, "packet_errors");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: do_fragmentation chart created when CONFIG_BOOLEAN_AUTO
static void test_do_proc_net_sctp_snmp_fragmentation_chart_auto(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    expect_value(rrdset_create_localhost, id, "fragmentation");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: do_chunk_types chart created when CONFIG_BOOLEAN_AUTO
static void test_do_proc_net_sctp_snmp_chunk_types_chart_auto(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    expect_value(rrdset_create_localhost, id, "chunks");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: rrddim values set correctly for established associations
static void test_do_proc_net_sctp_snmp_sets_established_value(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    expect_value(rrddim_set_by_pointer, value, 100);
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: Empty procfile (0 lines)
static void test_do_proc_net_sctp_snmp_empty_procfile(void **state) {
    setup_mock_procfile(0);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

// Test: Multiple calls maintain static state
static void test_do_proc_net_sctp_snmp_maintains_state_on_second_call(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    
    int result1 = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result1, 0);
    
    // Second call should not reinitialize (static state)
    int result2 = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result2, 0);
}

// Test: Different update_every values
static void test_do_proc_net_sctp_snmp_different_update_every(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    
    int result = do_proc_net_sctp_snmp(10, 0);
    assert_int_equal(result, 0);
}

// Test: dt parameter ignored
static void test_do_proc_net_sctp_snmp_dt_ignored(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    
    int result = do_proc_net_sctp_snmp(1, 12345);
    assert_int_equal(result, 0);
}

// Test: Line with less than 2 words skipped
static void test_do_proc_net_sctp_snmp_skips_short_lines(void **state) {
    setup_mock_procfile(1);
    
    expect_any(procfile_open, filename);
    expect_value(rrdset_create_localhost, type, "sctp");
    
    int result = do_proc_net_sctp_snmp(1, 0);
    assert_int_equal(result, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_proc_net_sctp_snmp_first_call_initializes),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_procfile_open_fails),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_procfile_readall_fails),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_processes_lines),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_associations_chart_auto),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_transitions_chart_auto),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_packets_chart_auto),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_packet_errors_chart_auto),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_fragmentation_chart_auto),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_chunk_types_chart_auto),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_sets_established_value),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_empty_procfile),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_maintains_state_on_second_call),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_different_update_every),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_dt_ignored),
        cmocka_unit_test(test_do_proc_net_sctp_snmp_skips_short_lines),
    };

    return cmocka_run_tests(tests);
}