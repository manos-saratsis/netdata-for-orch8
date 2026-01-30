/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Comprehensive unit tests for ebpf_hardirq.c
 * Target: 100% code coverage
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <unistd.h>

/* Mock data structures and functions */

/* Mock types matching the real headers */
typedef struct {
    int dummy;
} ebpf_module_t;

typedef struct {
    int dummy;
} avl_t;

typedef struct {
    avl_t avl;
    int irq;
    bool dim_exists;
    uint64_t latency;
    char name[32];
} hardirq_val_t;

typedef struct {
    uint32_t idx;
    char *name;
    uint64_t latency;
} hardirq_static_val_t;

typedef struct {
    int irq;
} hardirq_ebpf_key_t;

typedef struct {
    uint64_t latency;
    uint64_t ts;
} hardirq_ebpf_static_val_t;

typedef struct {
    char *name;
    size_t internal_input;
    size_t user_input;
    int type;
    int map_fd;
    int map_type;
} ebpf_local_maps_t;

typedef struct {
    int dummy;
} ARAL;

typedef struct {
    int dummy;
} procfile;

typedef struct {
    int dummy;
} heartbeat_t;

typedef struct {
    int dummy;
} avl_tree_lock;

typedef struct {
    int dummy;
} hardirq_bpf;

typedef struct {
    hardirq_bpf *maps;
} hardirq_bpf_maps;

struct config {
    int dummy;
};

/* Global mock variables */
ARAL *ebpf_aral_hardirq = NULL;
avl_tree_lock hardirq_pub;
struct config hardirq_config;
hardirq_bpf *hardirq_bpf_obj = NULL;
int ebpf_nprocs = 4;
int running_on_kernel = 0x041500;
int isrh = 0;
const char *netdata_configured_host_prefix = "";
const char *ebpf_plugin_dir = "/path/to/plugins";
int default_btf = 0;

/* Mock function declarations */
ARAL *ebpf_allocate_pid_aral(const char *name, size_t item_size);
hardirq_val_t *aral_mallocz(ARAL *aral);
void aral_freez(ARAL *aral, void *ptr);
int bpf_map_get_next_key(int fd, void *key, void *next_key);
int bpf_map_lookup_elem(int fd, void *key, void *value);
void ebpf_write_chart_obsolete(const char *group, const char *name, const char *context,
                                const char *title, const char *units, const char *family,
                                const char *chart_type, const char *ctx, int prio, int update_every);
void netdata_mutex_lock(void *mutex);
void netdata_mutex_unlock(void *mutex);
void ebpf_unload_legacy_code(void *objects, void *probe_links);
void ebpf_disable_tracepoint(void *tp);
void ebpf_update_stats(void *stats, ebpf_module_t *em);
void ebpf_update_kernel_memory_with_vector(void *stats, void *maps, int action);
int ebpf_plugin_stop(void);
procfile *procfile_open(const char *filename, const char *separators, int flags);
procfile *procfile_readall(procfile *ff);
size_t procfile_linewords(procfile *ff, size_t l);
size_t procfile_lines(procfile *ff);
const char *procfile_lineword(procfile *ff, size_t l, size_t w);
int str2i(const char *s);
void ebpf_create_chart(const char *group, const char *name, const char *title,
                       const char *units, const char *family, const char *ctx,
                       const char *chart_type, int prio, void *a, void *b, int c,
                       int update_every, const char *module);
void ebpf_write_global_dimension(const char *name, const char *label, const char *algorithm);
void write_chart_dimension(const char *name, uint64_t value);
void ebpf_write_begin_chart(const char *group, const char *name, const char *context);
void ebpf_write_end_chart(void);
void avl_init_lock(avl_tree_lock *tree, int (*cmp)(void *a, void *b));
avl_t *avl_search_lock(avl_tree_lock *tree, avl_t *item);
avl_t *avl_insert_lock(avl_tree_lock *tree, avl_t *item);
int avl_traverse_lock(avl_tree_lock *tree, int (*callback)(void *entry, void *data), void *data);
void ebpf_enable_tracepoints(void *tp);
void ebpf_define_map_type(void *maps, int per_core, int kernel);
void ebpf_adjust_thread_load(ebpf_module_t *em, int btf);
hardirq_bpf *hardirq_bpf__open(void);
void *ebpf_load_program(const char *dir, ebpf_module_t *em, int kernel, int rh, void **objects);
int hardirq_bpf__load(hardirq_bpf *obj);
int hardirq_bpf__attach(hardirq_bpf *obj);
int bpf_map__fd(void *map);
void ebpf_update_disabled_plugin_stats(ebpf_module_t *em);
void *callocz(size_t count, size_t size);
void collector_error(const char *fmt, ...);
void netdata_log_error(const char *fmt, ...);
void snprintfz(char *dst, size_t n, const char *fmt, ...);
void heartbeat_init(heartbeat_t *hb, int period);
void heartbeat_next(heartbeat_t *hb);
void fflush(FILE *stream);

/* ============================================ */
/*     TEST: ebpf_hardirq_get()                 */
/* ============================================ */

static int setup_aral_test(void **state)
{
    ebpf_aral_hardirq = ebpf_allocate_pid_aral("test", sizeof(hardirq_val_t));
    assert_non_null(ebpf_aral_hardirq);
    return 0;
}

static int teardown_aral_test(void **state)
{
    ebpf_aral_hardirq = NULL;
    return 0;
}

void test_ebpf_hardirq_get_allocates_memory(void **state)
{
    /* Arrange */
    hardirq_val_t *result = NULL;
    
    /* Act */
    result = ebpf_hardirq_get();
    
    /* Assert */
    assert_non_null(result);
}

void test_ebpf_hardirq_get_initializes_to_zero(void **state)
{
    /* Arrange */
    hardirq_val_t *result = NULL;
    
    /* Act */
    result = ebpf_hardirq_get();
    
    /* Assert */
    assert_non_null(result);
    assert_int_equal(result->irq, 0);
    assert_false(result->dim_exists);
    assert_int_equal(result->latency, 0);
    assert_int_equal(result->name[0], 0);
}

/* ============================================ */
/*     TEST: ebpf_hardirq_release()             */
/* ============================================ */

void test_ebpf_hardirq_release_frees_memory(void **state)
{
    /* Arrange */
    hardirq_val_t *val = ebpf_hardirq_get();
    assert_non_null(val);
    val->irq = 42;
    
    /* Act */
    ebpf_hardirq_release(val);
    
    /* Assert - no crash indicates success */
}

void test_ebpf_hardirq_release_with_populated_value(void **state)
{
    /* Arrange */
    hardirq_val_t *val = ebpf_hardirq_get();
    assert_non_null(val);
    val->irq = 255;
    val->dim_exists = true;
    val->latency = 12345;
    strcpy(val->name, "test_irq");
    
    /* Act */
    ebpf_hardirq_release(val);
    
    /* Assert - no crash indicates success */
}

/* ============================================ */
/*     TEST: ebpf_hardirq_aral_init()           */
/* ============================================ */

void test_ebpf_hardirq_aral_init_creates_allocator(void **state)
{
    /* Arrange */
    ebpf_aral_hardirq = NULL;
    
    /* Act */
    ebpf_hardirq_aral_init();
    
    /* Assert */
    assert_non_null(ebpf_aral_hardirq);
}

/* ============================================ */
/*     TEST: hardirq_val_cmp()                  */
/* ============================================ */

void test_hardirq_val_cmp_equal_irqs(void **state)
{
    /* Arrange */
    hardirq_val_t a = {.irq = 5, .latency = 100};
    hardirq_val_t b = {.irq = 5, .latency = 200};
    
    /* Act */
    int result = hardirq_val_cmp(&a, &b);
    
    /* Assert */
    assert_int_equal(result, 0);
}

void test_hardirq_val_cmp_first_greater(void **state)
{
    /* Arrange */
    hardirq_val_t a = {.irq = 10, .latency = 100};
    hardirq_val_t b = {.irq = 5, .latency = 100};
    
    /* Act */
    int result = hardirq_val_cmp(&a, &b);
    
    /* Assert */
    assert_int_equal(result, 1);
}

void test_hardirq_val_cmp_first_less(void **state)
{
    /* Arrange */
    hardirq_val_t a = {.irq = 3, .latency = 100};
    hardirq_val_t b = {.irq = 7, .latency = 100};
    
    /* Act */
    int result = hardirq_val_cmp(&a, &b);
    
    /* Assert */
    assert_int_equal(result, -1);
}

void test_hardirq_val_cmp_zero_irqs(void **state)
{
    /* Arrange */
    hardirq_val_t a = {.irq = 0, .latency = 100};
    hardirq_val_t b = {.irq = 0, .latency = 200};
    
    /* Act */
    int result = hardirq_val_cmp(&a, &b);
    
    /* Assert */
    assert_int_equal(result, 0);
}

void test_hardirq_val_cmp_negative_irqs(void **state)
{
    /* Arrange */
    hardirq_val_t a = {.irq = -5, .latency = 100};
    hardirq_val_t b = {.irq = -10, .latency = 100};
    
    /* Act */
    int result = hardirq_val_cmp(&a, &b);
    
    /* Assert */
    assert_int_equal(result, 1);
}

void test_hardirq_val_cmp_large_irq_numbers(void **state)
{
    /* Arrange */
    hardirq_val_t a = {.irq = 1024, .latency = 100};
    hardirq_val_t b = {.irq = 1023, .latency = 100};
    
    /* Act */
    int result = hardirq_val_cmp(&a, &b);
    
    /* Assert */
    assert_int_equal(result, 1);
}

/* ============================================ */
/*     TEST: hardirq_parse_interrupts()         */
/* ============================================ */

void test_hardirq_parse_interrupts_procfile_open_fails(void **state)
{
    /* Arrange */
    char irq_name[32] = {0};
    will_return(procfile_open, NULL);
    
    /* Act */
    int result = hardirq_parse_interrupts(irq_name, 5);
    
    /* Assert */
    assert_int_equal(result, -1);
}

void test_hardirq_parse_interrupts_procfile_readall_fails(void **state)
{
    /* Arrange */
    char irq_name[32] = {0};
    procfile *fake_ff = (procfile *)0x1234;
    will_return(procfile_open, fake_ff);
    will_return(procfile_readall, NULL);
    
    /* Act */
    int result = hardirq_parse_interrupts(irq_name, 5);
    
    /* Assert */
    assert_int_equal(result, -1);
}

void test_hardirq_parse_interrupts_zero_lines(void **state)
{
    /* Arrange */
    char irq_name[32] = {0};
    procfile *fake_ff = (procfile *)0x1234;
    will_return(procfile_open, fake_ff);
    will_return(procfile_readall, fake_ff);
    will_return(procfile_linewords, 2);
    will_return(procfile_lineword, "CPU0");
    will_return(procfile_lineword, "CPU1");
    will_return(procfile_lines, 0);
    
    /* Act */
    int result = hardirq_parse_interrupts(irq_name, 5);
    
    /* Assert */
    assert_int_equal(result, -1);
}

void test_hardirq_parse_interrupts_irq_not_found(void **state)
{
    /* Arrange */
    char irq_name[32] = {0};
    procfile *fake_ff = (procfile *)0x1234;
    will_return(procfile_open, fake_ff);
    will_return(procfile_readall, fake_ff);
    will_return(procfile_linewords, 3);
    will_return(procfile_lineword, "CPU0");
    will_return(procfile_lineword, "CPU1");
    will_return(procfile_lineword, "CPU2");
    will_return(procfile_lines, 5);
    
    /* Line 1: IRQ 10 (not matching 5) */
    will_return(procfile_linewords, 4);
    will_return(procfile_lineword, "10");
    will_return(str2i, 10);
    
    /* No more lines to process */
    
    /* Act */
    int result = hardirq_parse_interrupts(irq_name, 5);
    
    /* Assert */
    assert_int_equal(result, 0);
}

void test_hardirq_parse_interrupts_irq_found(void **state)
{
    /* Arrange */
    char irq_name[32] = {0};
    procfile *fake_ff = (procfile *)0x1234;
    will_return(procfile_open, fake_ff);
    will_return(procfile_readall, fake_ff);
    will_return(procfile_linewords, 3);
    will_return(procfile_lineword, "CPU0");
    will_return(procfile_lineword, "CPU1");
    will_return(procfile_lineword, "CPU2");
    will_return(procfile_lines, 3);
    
    /* Line 1: IRQ 5 (matching) */
    will_return(procfile_linewords, 6);
    will_return(procfile_lineword, "5");
    will_return(str2i, 5);
    will_return(procfile_lineword, "timer");
    
    /* Act */
    int result = hardirq_parse_interrupts(irq_name, 5);
    
    /* Assert */
    assert_int_equal(result, 0);
}

void test_hardirq_parse_interrupts_non_digit_first_char(void **state)
{
    /* Arrange */
    char irq_name[32] = {0};
    procfile *fake_ff = (procfile *)0x1234;
    will_return(procfile_open, fake_ff);
    will_return(procfile_readall, fake_ff);
    will_return(procfile_linewords, 2);
    will_return(procfile_lineword, "CPU0");
    will_return(procfile_lineword, "CPU1");
    will_return(procfile_lines, 3);
    
    /* Line 1: Non-digit character */
    will_return(procfile_linewords, 1);
    will_return(procfile_lineword, "NMI");
    
    /* Act */
    int result = hardirq_parse_interrupts(irq_name, 5);
    
    /* Assert */
    assert_int_equal(result, 0);
}

void test_hardirq_parse_interrupts_empty_words_line(void **state)
{
    /* Arrange */
    char irq_name[32] = {0};
    procfile *fake_ff = (procfile *)0x1234;
    will_return(procfile_open, fake_ff);
    will_return(procfile_readall, fake_ff);
    will_return(procfile_linewords, 2);
    will_return(procfile_lineword, "CPU0");
    will_return(procfile_lineword, "CPU1");
    will_return(procfile_lines, 3);
    
    /* Line 1: Empty */
    will_return(procfile_linewords, 0);
    
    /* Act */
    int result = hardirq_parse_interrupts(irq_name, 5);
    
    /* Assert */
    assert_int_equal(result, 0);
}

/* ============================================ */
/*     TEST: hardirq_write_dims() callback      */
/* ============================================ */

void test_hardirq_write_dims_new_dimension(void **state)
{
    /* Arrange */
    hardirq_val_t v = {.irq = 5, .dim_exists = false, .latency = 1000};
    strcpy(v.name, "timer");
    
    /* Act */
    int result = hardirq_write_dims(&v, NULL);
    
    /* Assert */
    assert_int_equal(result, 1);
    assert_true(v.dim_exists);
}

void test_hardirq_write_dims_existing_dimension(void **state)
{
    /* Arrange */
    hardirq_val_t v = {.irq = 5, .dim_exists = true, .latency = 1000};
    strcpy(v.name, "timer");
    
    /* Act */
    int result = hardirq_write_dims(&v, NULL);
    
    /* Assert */
    assert_int_equal(result, 1);
    assert_true(v.dim_exists);
}

void test_hardirq_write_dims_with_high_latency(void **state)
{
    /* Arrange */
    hardirq_val_t v = {.irq = 255, .dim_exists = false, .latency = 0xFFFFFFFFFFFFFFFF};
    strcpy(v.name, "critical");
    
    /* Act */
    int result = hardirq_write_dims(&v, NULL);
    
    /* Assert */
    assert_int_equal(result, 1);
}

/* ============================================ */
/*     TEST: hardirq_exit()                     */
/* ============================================ */

void test_hardirq_exit_with_running_module(void **state)
{
    /* Arrange */
    ebpf_module_t em = {.dummy = 0};
    em.enabled = 1; /* NETDATA_THREAD_EBPF_FUNCTION_RUNNING */
    
    /* Mock the cleanup functions */
    expect_function_call(netdata_mutex_lock);
    expect_function_call(ebpf_write_chart_obsolete);
    expect_function_call(netdata_mutex_unlock);
    expect_function_call(ebpf_update_kernel_memory_with_vector);
    expect_function_call(ebpf_unload_legacy_code);
    expect_function_call(ebpf_disable_tracepoint);
    expect_function_call(netdata_mutex_lock);
    expect_function_call(ebpf_update_stats);
    expect_function_call(netdata_mutex_unlock);
    
    /* Act */
    hardirq_exit(&em);
    
    /* Assert - function completes without crash */
}

void test_hardirq_exit_with_null_module(void **state)
{
    /* Arrange */
    ebpf_module_t *em = NULL;
    
    /* Act */
    hardirq_exit(em);
    
    /* Assert - function handles NULL gracefully */
}

void test_hardirq_exit_with_stopped_module(void **state)
{
    /* Arrange */
    ebpf_module_t em = {.dummy = 0};
    em.enabled = 2; /* NETDATA_THREAD_EBPF_STOPPED */
    
    /* Mock the cleanup functions */
    expect_function_call(ebpf_update_kernel_memory_with_vector);
    expect_function_call(ebpf_unload_legacy_code);
    expect_function_call(ebpf_disable_tracepoint);
    expect_function_call(netdata_mutex_lock);
    expect_function_call(ebpf_update_stats);
    expect_function_call(netdata_mutex_unlock);
    
    /* Act */
    hardirq_exit(&em);
    
    /* Assert - function completes without crash */
}

/* ============================================ */
/*     TEST: ebpf_hardirq_load_bpf()            */
/* ============================================ */

void test_ebpf_hardirq_load_bpf_legacy_success(void **state)
{
    /* Arrange */
    ebpf_module_t em = {.dummy = 0};
    em.load = 0x01; /* EBPF_LOAD_LEGACY */
    em.probe_links = (void *)0x5678;
    em.objects = NULL;
    
    will_return(ebpf_load_program, (void *)0x1234);
    
    /* Act */
    int result = ebpf_hardirq_load_bpf(&em);
    
    /* Assert */
    assert_int_equal(result, 0);
}

void test_ebpf_hardirq_load_bpf_legacy_failure(void **state)
{
    /* Arrange */
    ebpf_module_t em = {.dummy = 0};
    em.load = 0x01; /* EBPF_LOAD_LEGACY */
    em.probe_links = NULL;
    em.objects = NULL;
    
    will_return(ebpf_load_program, NULL);
    
    /* Act */
    int result = ebpf_hardirq_load_bpf(&em);
    
    /* Assert */
    assert_int_equal(result, -1);
}

/* ============================================ */
/*     TEST: ebpf_hardirq_thread()              */
/* ============================================ */

void test_ebpf_hardirq_thread_early_exit(void **state)
{
    /* Arrange */
    ebpf_module_t em = {.dummy = 0};
    em.maps = NULL;
    em.update_every = 1;
    em.lifetime = 1;
    
    will_return(ebpf_enable_tracepoints, 0);
    expect_function_call(ebpf_update_disabled_plugin_stats);
    
    /* Act */
    ebpf_hardirq_thread(&em);
    
    /* Assert - function handles early exit */
}

/* ============================================ */
/*     Consolidate all test cases              */
/* ============================================ */

int main(void)
{
    const struct CMUnitTest tests[] = {
        /* ebpf_hardirq_get tests */
        cmocka_unit_test_setup_teardown(
            test_ebpf_hardirq_get_allocates_memory,
            setup_aral_test, teardown_aral_test),
        cmocka_unit_test_setup_teardown(
            test_ebpf_hardirq_get_initializes_to_zero,
            setup_aral_test, teardown_aral_test),

        /* ebpf_hardirq_release tests */
        cmocka_unit_test_setup_teardown(
            test_ebpf_hardirq_release_frees_memory,
            setup_aral_test, teardown_aral_test),
        cmocka_unit_test_setup_teardown(
            test_ebpf_hardirq_release_with_populated_value,
            setup_aral_test, teardown_aral_test),

        /* ebpf_hardirq_aral_init tests */
        cmocka_unit_test(test_ebpf_hardirq_aral_init_creates_allocator),

        /* hardirq_val_cmp tests */
        cmocka_unit_test(test_hardirq_val_cmp_equal_irqs),
        cmocka_unit_test(test_hardirq_val_cmp_first_greater),
        cmocka_unit_test(test_hardirq_val_cmp_first_less),
        cmocka_unit_test(test_hardirq_val_cmp_zero_irqs),
        cmocka_unit_test(test_hardirq_val_cmp_negative_irqs),
        cmocka_unit_test(test_hardirq_val_cmp_large_irq_numbers),

        /* hardirq_parse_interrupts tests */
        cmocka_unit_test(test_hardirq_parse_interrupts_procfile_open_fails),
        cmocka_unit_test(test_hardirq_parse_interrupts_procfile_readall_fails),
        cmocka_unit_test(test_hardirq_parse_interrupts_zero_lines),
        cmocka_unit_test(test_hardirq_parse_interrupts_irq_not_found),
        cmocka_unit_test(test_hardirq_parse_interrupts_irq_found),
        cmocka_unit_test(test_hardirq_parse_interrupts_non_digit_first_char),
        cmocka_unit_test(test_hardirq_parse_interrupts_empty_words_line),

        /* hardirq_write_dims tests */
        cmocka_unit_test(test_hardirq_write_dims_new_dimension),
        cmocka_unit_test(test_hardirq_write_dims_existing_dimension),
        cmocka_unit_test(test_hardirq_write_dims_with_high_latency),

        /* hardirq_exit tests */
        cmocka_unit_test(test_hardirq_exit_with_running_module),
        cmocka_unit_test(test_hardirq_exit_with_null_module),
        cmocka_unit_test(test_hardirq_exit_with_stopped_module),

        /* ebpf_hardirq_load_bpf tests */
        cmocka_unit_test(test_ebpf_hardirq_load_bpf_legacy_success),
        cmocka_unit_test(test_ebpf_hardirq_load_bpf_legacy_failure),

        /* ebpf_hardirq_thread tests */
        cmocka_unit_test(test_ebpf_hardirq_thread_early_exit),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}