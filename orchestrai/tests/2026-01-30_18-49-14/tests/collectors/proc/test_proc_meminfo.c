#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

// Mock declarations and externs
extern int do_proc_meminfo(int update_every, usec_t dt);

// Mock variables
static procfile *mock_procfile = NULL;
static int mock_procfile_readall_return = 0;
static size_t mock_procfile_lines = 0;
static char **mock_procfile_words[256];
static size_t mock_procfile_word_counts[256];

// Mock structures for testing
typedef struct {
    char *name;
    unsigned long long value;
} meminfo_entry;

// Mock implementations
static procfile* __wrap_procfile_open(const char *filename, const char *separators, uint32_t flags) {
    check_expected(filename);
    check_expected(separators);
    check_expected(flags);
    
    if (mock_procfile) {
        return mock_procfile;
    }
    
    return NULL;
}

static procfile* __wrap_procfile_readall(procfile *ff) {
    if (ff == NULL)
        return NULL;
    
    if (mock_procfile_readall_return == 0) {
        return ff;
    }
    
    return NULL;
}

static size_t __wrap_procfile_lines(procfile *ff) {
    return mock_procfile_lines;
}

static size_t __wrap_procfile_linewords(procfile *ff, size_t line) {
    if (line < 256)
        return mock_procfile_word_counts[line];
    return 0;
}

static char* __wrap_procfile_lineword(procfile *ff, size_t line, size_t word) {
    if (line < 256 && word < 2)
        return mock_procfile_words[line][word];
    return NULL;
}

// Helper function to setup meminfo mock data
static void setup_meminfo_mockdata(meminfo_entry *entries, size_t count) {
    mock_procfile_lines = count + 1; // +1 for header
    
    for (size_t i = 0; i < count; i++) {
        mock_procfile_words[i + 1][0] = entries[i].name;
        
        // Convert value to string
        static char value_str[64][256];
        snprintf(value_str[i], sizeof(value_str[i]), "%llu", entries[i].value);
        mock_procfile_words[i + 1][1] = value_str[i];
        
        mock_procfile_word_counts[i + 1] = 2;
    }
}

// Test: do_proc_meminfo with NULL procfile returns 1
static void test_do_proc_meminfo_file_open_failure(void **state) {
    mock_procfile = NULL;
    
    int result = do_proc_meminfo(10, 0);
    assert_int_equal(result, 1);
}

// Test: do_proc_meminfo with successful file open and read
static void test_do_proc_meminfo_basic_success(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    mock_procfile_lines = 1;
    mock_procfile_word_counts[0] = 0;
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with procfile_readall returning NULL
static void test_do_proc_meminfo_file_read_failure(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 1; // Simulate read failure
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0); // Returns 0 to retry
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with memory metrics
static void test_do_proc_meminfo_with_memory_metrics(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 8000000},
        {"MemFree", 2000000},
        {"MemAvailable", 3000000},
        {"Buffers", 100000},
        {"Cached", 500000},
        {"SwapTotal", 2000000},
        {"SwapFree", 1500000},
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with line having less than 2 words
static void test_do_proc_meminfo_invalid_line_format(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    mock_procfile_lines = 2;
    
    // Line 1: only 1 word
    mock_procfile_word_counts[1] = 1;
    mock_procfile_words[1][0] = "MemTotal";
    
    // Line 0: header (no words)
    mock_procfile_word_counts[0] = 0;
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with swap enabled
static void test_do_proc_meminfo_with_swap(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 8000000},
        {"MemFree", 2000000},
        {"SwapTotal", 4000000},
        {"SwapFree", 3000000},
        {"SwapCached", 100000},
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with HardwareCorrupted
static void test_do_proc_meminfo_with_hwcorrupt(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 8000000},
        {"MemFree", 2000000},
        {"HardwareCorrupted", 10000},
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with hugepages
static void test_do_proc_meminfo_with_hugepages(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 8000000},
        {"MemFree", 2000000},
        {"HugePages_Total", 1000},
        {"HugePages_Free", 500},
        {"HugePages_Rsvd", 100},
        {"HugePages_Surp", 10},
        {"Hugepagesize", 2048},
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with CMA memory
static void test_do_proc_meminfo_with_cma(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 8000000},
        {"MemFree", 2000000},
        {"CmaTotal", 100000},
        {"CmaFree", 50000},
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with DirectMap entries
static void test_do_proc_meminfo_with_directmap(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 8000000},
        {"MemFree", 2000000},
        {"DirectMap4k", 1000000},
        {"DirectMap2M", 4000000},
        {"DirectMap4M", 0},
        {"DirectMap1G", 2000000},
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with zero values
static void test_do_proc_meminfo_with_zero_values(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 0},
        {"MemFree", 0},
        {"Buffers", 0},
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with large values
static void test_do_proc_meminfo_with_large_values(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 274877906944ULL}, // 256 GB
        {"MemFree", 137438953472ULL},   // 128 GB
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with update_every = 1
static void test_do_proc_meminfo_update_every_one(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    mock_procfile_lines = 1;
    
    int result = do_proc_meminfo(1, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with dt parameter
static void test_do_proc_meminfo_with_dt(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    mock_procfile_lines = 1;
    
    usec_t dt = 1000000; // 1 second in microseconds
    int result = do_proc_meminfo(10, dt);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

// Test: do_proc_meminfo with all memory categories
static void test_do_proc_meminfo_all_categories(void **state) {
    mock_procfile = (procfile *)malloc(sizeof(procfile));
    mock_procfile_readall_return = 0;
    
    meminfo_entry entries[] = {
        {"MemTotal", 16000000},
        {"MemFree", 4000000},
        {"MemAvailable", 8000000},
        {"Buffers", 200000},
        {"Cached", 2000000},
        {"SwapCached", 100000},
        {"Active", 6000000},
        {"Inactive", 3000000},
        {"Dirty", 50000},
        {"Writeback", 10000},
        {"AnonPages", 3000000},
        {"Slab", 200000},
        {"SReclaimable", 100000},
        {"SUnreclaim", 100000},
        {"KernelStack", 50000},
        {"PageTables", 30000},
    };
    
    setup_meminfo_mockdata(entries, sizeof(entries) / sizeof(entries[0]));
    
    int result = do_proc_meminfo(10, 0);
    
    assert_int_equal(result, 0);
    
    free(mock_procfile);
    mock_procfile = NULL;
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_proc_meminfo_file_open_failure),
        cmocka_unit_test(test_do_proc_meminfo_basic_success),
        cmocka_unit_test(test_do_proc_meminfo_file_read_failure),
        cmocka_unit_test(test_do_proc_meminfo_with_memory_metrics),
        cmocka_unit_test(test_do_proc_meminfo_invalid_line_format),
        cmocka_unit_test(test_do_proc_meminfo_with_swap),
        cmocka_unit_test(test_do_proc_meminfo_with_hwcorrupt),
        cmocka_unit_test(test_do_proc_meminfo_with_hugepages),
        cmocka_unit_test(test_do_proc_meminfo_with_cma),
        cmocka_unit_test(test_do_proc_meminfo_with_directmap),
        cmocka_unit_test(test_do_proc_meminfo_with_zero_values),
        cmocka_unit_test(test_do_proc_meminfo_with_large_values),
        cmocka_unit_test(test_do_proc_meminfo_update_every_one),
        cmocka_unit_test(test_do_proc_meminfo_with_dt),
        cmocka_unit_test(test_do_proc_meminfo_all_categories),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}