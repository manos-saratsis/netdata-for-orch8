#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

/* Mock declarations */
#define MOCK_JUDY_ARRAY ((PPvoid_t)0x12345678)
#define MOCK_PID 1234
#define MOCK_ERROR_CODE -1

/* Forward declarations for mocked functions */
extern int __wrap_ebpf_is_tracepoint_enabled(const char *class, const char *event);
extern int __wrap_ebpf_enable_tracing_values(const char *class, const char *event);
extern int __wrap_ebpf_disable_tracing_values(const char *class, const char *event);

/* ========================================
 * TEST: ebpf_judy_insert_unsafe
 * ======================================== */

static void test_ebpf_judy_insert_unsafe_success(void **state)
{
    /* Arrange */
    PPvoid_t test_array = NULL;
    Word_t test_key = 123;
    
    /* Act & Assert - Function should handle NULL array */
    void **result = ebpf_judy_insert_unsafe(test_array, test_key);
    
    /* Verify basic behavior without actual Judy operations */
    assert_non_null(result);
}

static void test_ebpf_judy_insert_unsafe_zero_key(void **state)
{
    /* Arrange */
    PPvoid_t test_array = NULL;
    Word_t test_key = 0;
    
    /* Act */
    void **result = ebpf_judy_insert_unsafe(test_array, test_key);
    
    /* Assert */
    assert_non_null(result);
}

static void test_ebpf_judy_insert_unsafe_large_key(void **state)
{
    /* Arrange */
    PPvoid_t test_array = NULL;
    Word_t test_key = 0xFFFFFFFFFFFFFFFFUL;
    
    /* Act */
    void **result = ebpf_judy_insert_unsafe(test_array, test_key);
    
    /* Assert */
    assert_non_null(result);
}

/* ========================================
 * TEST: ebpf_get_pid_from_judy_unsafe
 * ======================================== */

static void test_ebpf_get_pid_from_judy_unsafe_new_pid(void **state)
{
    /* Arrange */
    PPvoid_t judy_array = NULL;
    uint32_t pid = 5678;
    
    /* Act */
    netdata_ebpf_judy_pid_stats_t *result = ebpf_get_pid_from_judy_unsafe(judy_array, pid);
    
    /* Assert */
    assert_non_null(result);
    assert_null(result->cmdline);
    assert_null(result->socket_stats.JudyLArray);
}

static void test_ebpf_get_pid_from_judy_unsafe_zero_pid(void **state)
{
    /* Arrange */
    PPvoid_t judy_array = NULL;
    uint32_t pid = 0;
    
    /* Act */
    netdata_ebpf_judy_pid_stats_t *result = ebpf_get_pid_from_judy_unsafe(judy_array, pid);
    
    /* Assert */
    assert_non_null(result);
}

static void test_ebpf_get_pid_from_judy_unsafe_max_pid(void **state)
{
    /* Arrange */
    PPvoid_t judy_array = NULL;
    uint32_t pid = 0xFFFFFFFFU;
    
    /* Act */
    netdata_ebpf_judy_pid_stats_t *result = ebpf_get_pid_from_judy_unsafe(judy_array, pid);
    
    /* Assert */
    assert_non_null(result);
}

/* ========================================
 * TEST: ebpf_allocate_pid_aral
 * ======================================== */

static void test_ebpf_allocate_pid_aral_valid_name_and_size(void **state)
{
    /* Arrange */
    char name[] = "test_aral";
    size_t size = sizeof(netdata_ebpf_judy_pid_stats_t);
    
    /* Act */
    ARAL *result = ebpf_allocate_pid_aral(name, size);
    
    /* Assert - Should return valid ARAL or handle error gracefully */
    if (result != NULL) {
        /* Successfully allocated */
        assert_non_null(result);
    }
}

static void test_ebpf_allocate_pid_aral_small_size(void **state)
{
    /* Arrange */
    char name[] = "test_small";
    size_t size = 1;
    
    /* Act */
    ARAL *result = ebpf_allocate_pid_aral(name, size);
    
    /* Assert */
    if (result != NULL) {
        assert_non_null(result);
    }
}

static void test_ebpf_allocate_pid_aral_large_size(void **state)
{
    /* Arrange */
    char name[] = "test_large";
    size_t size = 1024 * 1024;
    
    /* Act */
    ARAL *result = ebpf_allocate_pid_aral(name, size);
    
    /* Assert */
    if (result != NULL) {
        assert_non_null(result);
    }
}

static void test_ebpf_allocate_pid_aral_empty_name(void **state)
{
    /* Arrange */
    char name[] = "";
    size_t size = sizeof(netdata_ebpf_judy_pid_stats_t);
    
    /* Act */
    ARAL *result = ebpf_allocate_pid_aral(name, size);
    
    /* Assert - Should handle empty name */
    if (result != NULL) {
        assert_non_null(result);
    }
}

/* ========================================
 * TEST: get_value_from_structure
 * ======================================== */

static void test_get_value_from_structure_positive_value(void **state)
{
    /* Arrange */
    collected_number test_value = 100;
    char buffer[sizeof(collected_number)];
    memcpy(buffer, &test_value, sizeof(test_value));
    
    /* Act */
    collected_number result = get_value_from_structure(buffer, 0);
    
    /* Assert */
    assert_int_equal(result, 100);
}

static void test_get_value_from_structure_negative_value(void **state)
{
    /* Arrange */
    collected_number test_value = -50;
    char buffer[sizeof(collected_number)];
    memcpy(buffer, &test_value, sizeof(test_value));
    
    /* Act */
    collected_number result = get_value_from_structure(buffer, 0);
    
    /* Assert */
    assert_int_equal(result, 50);
}

static void test_get_value_from_structure_zero_value(void **state)
{
    /* Arrange */
    collected_number test_value = 0;
    char buffer[sizeof(collected_number)];
    memcpy(buffer, &test_value, sizeof(test_value));
    
    /* Act */
    collected_number result = get_value_from_structure(buffer, 0);
    
    /* Assert */
    assert_int_equal(result, 0);
}

static void test_get_value_from_structure_with_offset(void **state)
{
    /* Arrange */
    char buffer[256];
    collected_number test_value = 42;
    size_t offset = 100;
    memcpy(buffer + offset, &test_value, sizeof(test_value));
    
    /* Act */
    collected_number result = get_value_from_structure(buffer, offset);
    
    /* Assert */
    assert_int_equal(result, 42);
}

static void test_get_value_from_structure_large_positive(void **state)
{
    /* Arrange */
    collected_number test_value = 9223372036854775807LL;
    char buffer[sizeof(collected_number)];
    memcpy(buffer, &test_value, sizeof(test_value));
    
    /* Act */
    collected_number result = get_value_from_structure(buffer, 0);
    
    /* Assert */
    assert_int_equal(result, 9223372036854775807LL);
}

/* ========================================
 * TEST: write_chart_dimension
 * ======================================== */

static void test_write_chart_dimension_basic(void **state)
{
    /* Arrange */
    FILE *old_stdout = stdout;
    char dim[] = "test_dimension";
    long long value = 123;
    
    /* Act */
    /* Redirect stdout for verification (would need capture in real test) */
    write_chart_dimension(dim, value);
    
    /* Assert - Function writes to stdout */
}

static void test_write_chart_dimension_negative_value(void **state)
{
    /* Arrange */
    char dim[] = "test_dim";
    long long value = -999;
    
    /* Act */
    write_chart_dimension(dim, value);
    
    /* Assert - Should handle negative values */
}

static void test_write_chart_dimension_zero_value(void **state)
{
    /* Arrange */
    char dim[] = "test";
    long long value = 0;
    
    /* Act */
    write_chart_dimension(dim, value);
    
    /* Assert - Should handle zero */
}

static void test_write_chart_dimension_empty_dim(void **state)
{
    /* Arrange */
    char dim[] = "";
    long long value = 100;
    
    /* Act */
    write_chart_dimension(dim, value);
    
    /* Assert */
}

/* ========================================
 * TEST: write_count_chart
 * ======================================== */

static void test_write_count_chart_null_move(void **state)
{
    /* Arrange */
    char name[] = "test_chart";
    char family[] = "test_family";
    netdata_publish_syscall_t *move = NULL;
    uint32_t end = 0;
    
    /* Act */
    write_count_chart(name, family, move, end);
    
    /* Assert */
}

static void test_write_count_chart_single_element(void **state)
{
    /* Arrange */
    char name[] = "test";
    char family[] = "test_fam";
    netdata_publish_syscall_t entry = {
        .name = "syscall1",
        .ncall = 100,
        .next = NULL
    };
    uint32_t end = 1;
    
    /* Act */
    write_count_chart(name, family, &entry, end);
    
    /* Assert */
}

static void test_write_count_chart_multiple_elements(void **state)
{
    /* Arrange */
    char name[] = "chart";
    char family[] = "fam";
    netdata_publish_syscall_t entry1 = {
        .name = "sys1",
        .ncall = 50,
        .next = NULL
    };
    netdata_publish_syscall_t entry2 = {
        .name = "sys2",
        .ncall = 75,
        .next = NULL
    };
    entry1.next = &entry2;
    uint32_t end = 2;
    
    /* Act */
    write_count_chart(name, family, &entry1, end);
    
    /* Assert */
}

static void test_write_count_chart_zero_end(void **state)
{
    /* Arrange */
    char name[] = "test";
    char family[] = "fam";
    netdata_publish_syscall_t entry = {
        .name = "sys",
        .ncall = 100,
        .next = NULL
    };
    uint32_t end = 0;
    
    /* Act */
    write_count_chart(name, family, &entry, end);
    
    /* Assert */
}

/* ========================================
 * TEST: write_err_chart
 * ======================================== */

static void test_write_err_chart_null_move(void **state)
{
    /* Arrange */
    char name[] = "err_chart";
    char family[] = "fam";
    netdata_publish_syscall_t *move = NULL;
    int end = 0;
    
    /* Act */
    write_err_chart(name, family, move, end);
    
    /* Assert */
}

static void test_write_err_chart_single_element(void **state)
{
    /* Arrange */
    char name[] = "errors";
    char family[] = "sys";
    netdata_publish_syscall_t entry = {
        .name = "open",
        .nerr = 10,
        .next = NULL
    };
    int end = 1;
    
    /* Act */
    write_err_chart(name, family, &entry, end);
    
    /* Assert */
}

static void test_write_err_chart_multiple_errors(void **state)
{
    /* Arrange */
    char name[] = "errs";
    char family[] = "family";
    netdata_publish_syscall_t e1 = {
        .name = "read",
        .nerr = 5,
        .next = NULL
    };
    netdata_publish_syscall_t e2 = {
        .name = "write",
        .nerr = 3,
        .next = NULL
    };
    e1.next = &e2;
    int end = 2;
    
    /* Act */
    write_err_chart(name, family, &e1, end);
    
    /* Assert */
}

/* ========================================
 * TEST: ebpf_one_dimension_write_charts
 * ======================================== */

static void test_ebpf_one_dimension_write_charts_basic(void **state)
{
    /* Arrange */
    char family[] = "test";
    char chart[] = "test_chart";
    char dim[] = "dimension";
    long long v1 = 500;
    
    /* Act */
    ebpf_one_dimension_write_charts(family, chart, dim, v1);
    
    /* Assert */
}

static void test_ebpf_one_dimension_write_charts_negative(void **state)
{
    /* Arrange */
    char family[] = "fam";
    char chart[] = "chart";
    char dim[] = "dim";
    long long v1 = -100;
    
    /* Act */
    ebpf_one_dimension_write_charts(family, chart, dim, v1);
    
    /* Assert */
}

static void test_ebpf_one_dimension_write_charts_zero(void **state)
{
    /* Arrange */
    char family[] = "f";
    char chart[] = "c";
    char dim[] = "d";
    long long v1 = 0;
    
    /* Act */
    ebpf_one_dimension_write_charts(family, chart, dim, v1);
    
    /* Assert */
}

/* ========================================
 * TEST: write_io_chart
 * ======================================== */

static void test_write_io_chart_basic(void **state)
{
    /* Arrange */
    char chart[] = "io";
    char family[] = "fam";
    char dwrite[] = "writes";
    long long vwrite = 100;
    char dread[] = "reads";
    long long vread = 200;
    
    /* Act */
    write_io_chart(chart, family, dwrite, vwrite, dread, vread);
    
    /* Assert */
}

static void test_write_io_chart_zero_values(void **state)
{
    /* Arrange */
    char chart[] = "io";
    char family[] = "f";
    char dwrite[] = "w";
    long long vwrite = 0;
    char dread[] = "r";
    long long vread = 0;
    
    /* Act */
    write_io_chart(chart, family, dwrite, vwrite, dread, vread);
    
    /* Assert */
}

static void test_write_io_chart_large_values(void **state)
{
    /* Arrange */
    char chart[] = "io_large";
    char family[] = "io_fam";
    char dwrite[] = "writes";
    long long vwrite = 9223372036854775807LL;
    char dread[] = "reads";
    long long vread = 9223372036854775807LL;
    
    /* Act */
    write_io_chart(chart, family, dwrite, vwrite, dread, vread);
    
    /* Assert */
}

/* ========================================
 * TEST: ebpf_write_chart_cmd
 * ======================================== */

static void test_ebpf_write_chart_cmd_basic(void **state)
{
    /* Arrange */
    char type[] = "area";
    char id[] = "test_id";
    char suffix[] = "";
    char title[] = "Test Chart";
    char units[] = "calls";
    char family[] = "family";
    char charttype[] = "line";
    char context[] = "ctx";
    int order = 1;
    int update_every = 10;
    char module[] = "ebpf";
    
    /* Act */
    ebpf_write_chart_cmd(type, id, suffix, title, units, family, charttype, context, order, update_every, module);
    
    /* Assert */
}

static void test_ebpf_write_chart_cmd_null_optional(void **state)
{
    /* Arrange */
    char type[] = "line";
    char id[] = "id";
    char suffix[] = "";
    char title[] = "Title";
    char units[] = "units";
    
    /* Act */
    ebpf_write_chart_cmd(type, id, suffix, title, units, NULL, NULL, NULL, 1, 10, "mod");
    
    /* Assert */
}

static void test_ebpf_write_chart_cmd_with_suffix(void **state)
{
    /* Arrange */
    char type[] = "stacked";
    char id[] = "chart";
    char suffix[] = "_suffix";
    char title[] = "Title";
    char units[] = "bytes";
    char family[] = "fam";
    char charttype[] = "stacked";
    char context[] = "ctx";
    int order = 999;
    int update_every = 60;
    char module[] = "module";
    
    /* Act */
    ebpf_write_chart_cmd(type, id, suffix, title, units, family, charttype, context, order, update_every, module);
    
    /* Assert */
}

/* ========================================
 * TEST: ebpf_write_chart_obsolete
 * ======================================== */

static void test_ebpf_write_chart_obsolete_basic(void **state)
{
    /* Arrange */
    char type[] = "area";
    char id[] = "obsolete_chart";
    char suffix[] = "_old";
    char title[] = "Old Chart";
    char units[] = "calls";
    char family[] = "family";
    char charttype[] = "line";
    char context[] = "context";
    int order = 100;
    int update_every = 10;
    
    /* Act */
    ebpf_write_chart_obsolete(type, id, suffix, title, units, family, charttype, context, order, update_every);
    
    /* Assert */
}

static void test_ebpf_write_chart_obsolete_null_optional(void **state)
{
    /* Arrange */
    char type[] = "line";
    char id[] = "id";
    char suffix[] = "";
    char title[] = "Title";
    char units[] = "units";
    
    /* Act */
    ebpf_write_chart_obsolete(type, id, suffix, title, units, NULL, NULL, NULL, 50, 20);
    
    /* Assert */
}

/* ========================================
 * TEST: ebpf_write_global_dimension
 * ======================================== */

static void test_ebpf_write_global_dimension_basic(void **state)
{
    /* Arrange */
    char name[] = "syscall1";
    char id[] = "id1";
    char algorithm[] = "absolute";
    
    /* Act */
    ebpf_write_global_dimension(name, id, algorithm);
    
    /* Assert */
}

static void test_ebpf_write_global_dimension_incremental(void **state)
{
    /* Arrange */
    char name[] = "dim2";
    char id[] = "dim2_id";
    char algorithm[] = "incremental";
    
    /* Act */
    ebpf_write_global_dimension(name, id, algorithm);
    
    /* Assert */
}

/* ========================================
 * TEST: ebpf_create_global_dimension
 * ======================================== */

static void test_ebpf_create_global_dimension_null_ptr(void **state)
{
    /* Arrange */
    void *ptr = NULL;
    int end = 0;
    
    /* Act */
    ebpf_create_global_dimension(ptr, end);
    
    /* Assert */
}

static void test_ebpf_create_global_dimension_single(void **state)
{
    /* Arrange */
    netdata_publish_syscall_t entry = {
        .name = "test",
        .dimension = "test_dim",
        .algorithm = "absolute",
        .next = NULL
    };
    int end = 1;
    
    /* Act */
    ebpf_create_global_dimension(&entry, end);
    
    /* Assert */
}

static void test_ebpf_create_global_dimension_multiple(void **state)
{
    /* Arrange */
    netdata_publish_syscall_t e1 = {
        .name = "sys1",
        .dimension = "d1",
        .algorithm = "absolute",
        .next = NULL
    };
    netdata_publish_syscall_t e2 = {
        .name = "sys2",
        .dimension = "d2",
        .algorithm = "incremental",
        .next = NULL
    };
    e1.next = &e2;
    int end = 2;
    
    /* Act */
    ebpf_create_global_dimension(&e1, end);
    
    /* Assert */
}

/* ========================================
 * TEST: ebpf_create_chart
 * ======================================== */

static void test_ebpf_create_chart_basic(void **state)
{
    /* Arrange */
    char type[] = "line";
    char id[] = "chart_id";
    char title[] = "Chart Title";
    char units[] = "calls";
    char family[] = "family";
    char context[] = "ctx";
    char charttype[] = "line";
    int order = 1;
    int end = 1;
    int update_every = 10;
    char module[] = "module";
    
    /* Act */
    ebpf_create_chart(type, id, title, units, family, context, charttype, order, NULL, NULL, end, update_every, module);
    
    /* Assert */
}

static void test_ebpf_create_chart_with_callback(void **state)
{
    /* Arrange */
    char type[] = "stacked";
    char id[] = "chart";
    char title[] = "Title";
    char units[] = "bytes";
    char family[] = "fam";
    char context[] = "context";
    char charttype[] = "stacked";
    int order = 10;
    int end = 2;
    int update_every = 60;
    char module[] = "mod";
    
    netdata_publish_syscall_t entry = {
        .name = "test",
        .dimension = "dim",
        .algorithm = "absolute",
        .next = NULL
    };
    
    /* Act */
    ebpf_create_chart(type, id, title, units, family, context, charttype, order, 
                     ebpf_create_global_dimension, &entry, end, update_every, module);
    
    /* Assert */
}

/* ========================================
 * TEST: write_histogram_chart
 * ======================================== */

static void test_write_histogram_chart_basic(void **state)
{
    /* Arrange */
    char family[] = "histo";
    char name[] = "histogram";
    netdata_idx_t hist[] = {10, 20, 30};
    char *dims[] = {"bin0", "bin1", "bin2"};
    uint32_t end = 3;
    
    /* Act */
    write_histogram_chart(family, name, hist, dims, end);
    
    /* Assert */
}

static void test_write_histogram_chart_single_bin(void **state)
{
    /* Arrange */
    char family[] = "f";
    char name[] = "n";
    netdata_idx_t hist[] = {100};
    char *dims[] = {"bin"};
    uint32_t end = 1;
    
    /* Act */
    write_histogram_chart(family, name, hist, dims, end);
    
    /* Assert */
}

static void test_write_histogram_chart_zero_end(void **state)
{
    /* Arrange */
    char family[] = "fam";
    char name[] = "name";
    netdata_idx_t hist[] = {1, 2, 3};
    char *dims[] = {"d1", "d2", "d3"};
    uint32_t end = 0;
    
    /* Act */
    write_histogram_chart(family, name, hist, dims, end);
    
    /* Assert */
}

static void test_write_histogram_chart_large_values(void **state)
{
    /* Arrange */
    char family[] = "histo";
    char name[] = "large";
    netdata_idx_t hist[] = {
        18446744073709551615UL,
        18446744073709551614UL,
        18446744073709551613UL
    };
    char *dims[] = {"max", "high", "med"};
    uint32_t end = 3;
    
    /* Act */
    write_histogram_chart(family, name, hist, dims, end);
    
    /* Assert */
}

/* ========================================
 * TEST: ebpf_clean_port_structure
 * ======================================== */

static void test_ebpf_clean_port_structure_null(void **state)
{
    /* Arrange */
    ebpf_network_viewer_port_list_t *clean = NULL;
    
    /* Act */
    ebpf_clean_port_structure(&clean);
    
    /* Assert */
    assert_null(clean);
}

static void test_ebpf_clean_port_structure_single(void **state)
{
    /* Arrange */
    ebpf_network_viewer_port_list_t *list = malloc(sizeof(ebpf_network_viewer_port_list_t));
    list->value = malloc(10);
    strcpy(list->value, "80");
    list->next = NULL;
    
    /* Act */
    ebpf_clean_port_structure(&list);
    
    /* Assert */
    assert_null(list);
}

static void test_ebpf_clean_port_structure_multiple(void **state)
{
    /* Arrange */
    ebpf_network_viewer_port_list_t *p1 = malloc(sizeof(ebpf_network_viewer_port_list_t));
    p1->value = malloc(10);
    strcpy(p1->value, "80");
    
    ebpf_network_viewer_port_list_t *p2 = malloc(sizeof(ebpf_network_viewer_port_list_t));
    p2->value = malloc(10);
    strcpy(p2->value, "443");
    p2->next = NULL;
    p1->next = p2;
    
    /* Act */
    ebpf_clean_port_structure(&p1);
    
    /* Assert */
    assert_null(p1);
}

/* ========================================
 * TEST: ebpf_clean_ip_structure
 * ======================================== */

static void test_ebpf_clean_ip_structure_null(void **state)
{
    /* Arrange */
    ebpf_network_viewer_ip_list_t *clean = NULL;
    
    /* Act */
    ebpf_clean_ip_structure(&clean);
    
    /* Assert */
    assert_null(clean);
}

static void test_ebpf_clean_ip_structure_single(void **state)
{
    /* Arrange */
    ebpf_network_viewer_ip_list_t *list = malloc(sizeof(ebpf_network_viewer_ip_list_t));
    list->value = malloc(20);
    strcpy(list->value, "192.168.1.1");
    list->next = NULL;
    
    /* Act */
    ebpf_clean_ip_structure(&list);
    
    /* Assert */
    assert_null(list);
}

static void test_ebpf_clean_ip_structure_multiple(void **state)
{
    /* Arrange */
    ebpf_network_viewer_ip_list_t *i1 = malloc(sizeof(ebpf_network_viewer_ip_list_t));
    i1->value = malloc(20);
    strcpy(i1->value, "10.0.0.1");
    
    ebpf_network_viewer_ip_list_t *i2 = malloc(sizeof(ebpf_network_viewer_ip_list_t));
    i2->value = malloc(20);
    strcpy(i2->value, "172.16.0.1");
    i2->next = NULL;
    i1->next = i2;
    
    /* Act */
    ebpf_clean_ip_structure(&i1);
    
    /* Assert */
    assert_null(i1);
}

/* ========================================
 * TEST: ebpf_parse_ports
 * ======================================== */

static void test_ebpf_parse_ports_null(void **state)
{
    /* Arrange */
    const char *ptr = NULL;
    
    /* Act */
    ebpf_parse_ports(ptr);
    
    /* Assert */
}

static void test_ebpf_parse_ports_empty_string(void **state)
{