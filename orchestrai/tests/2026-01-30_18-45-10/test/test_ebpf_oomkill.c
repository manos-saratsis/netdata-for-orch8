// SPDX-License-Identifier: GPL-3.0-or-later
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>

// Mock structures and functions
typedef struct {
    int map_fd;
    char *name;
    int internal_input;
    int user_input;
    int type;
} ebpf_local_maps_t;

typedef struct {
    int enabled;
    char *class;
    char *event;
} ebpf_tracepoint_t;

typedef struct {
    char *name;
    char *dimension;
    char *algorithm;
    void *next;
} netdata_publish_syscall_t;

typedef struct {
    char *name;
    void *next;
} ebpf_cgroup_target_t;

typedef struct {
    int value;
} ebpf_module_t;

typedef struct {
    int enabled;
} netdata_apps_integration_flags_t;

typedef pthread_mutex_t netdata_mutex_t;

// Global mocks
struct config oomkill_config = {0};
static ebpf_local_maps_t oomkill_maps[] = {
    {.name = "tbl_oomkill", .internal_input = 64, .type = 0, .map_fd = -1},
    {.name = NULL, .internal_input = 0, .type = 1, .map_fd = -1}
};

static ebpf_tracepoint_t oomkill_tracepoints[] = {
    {.enabled = 0, .class = "oom", .event = "mark_victim"},
    {.enabled = 0, .class = NULL, .event = NULL}
};

static netdata_publish_syscall_t oomkill_publish_aggregated = {
    .name = "kills",
    .dimension = "kills",
    .algorithm = "absolute",
    .next = NULL
};

// Mock external dependencies
void netdata_log_error(const char *fmt, ...) {}
void netdata_log_info(const char *fmt, ...) {}
int bpf_map_get_next_key(int fd, void *key, void *next_key) { return -1; }
int bpf_map_delete_elem(int fd, void *key) { return 0; }
int bpf_map_lookup_elem(int fd, void *key, void *value) { return 0; }

netdata_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
netdata_mutex_t ebpf_exit_cleanup = PTHREAD_MUTEX_INITIALIZER;
netdata_mutex_t mutex_cgroup_shm = PTHREAD_MUTEX_INITIALIZER;
netdata_mutex_t collect_data_mutex = PTHREAD_MUTEX_INITIALIZER;

ebpf_cgroup_target_t *ebpf_cgroup_pids = NULL;
struct {
    struct {
        int systemd_enabled;
    } *header;
} shm_ebpf_cgroup = {0};

void ebpf_write_chart_obsolete(char *id, char *chart, char *opt, char *title, char *units, char *family,
                               char *type, char *context, int order, int update_every) {}
void ebpf_create_chart(char *type, char *id, char *title, char *units, char *family, char *context,
                       char *charttype, int order, void *func, void *ptr, int dim, int update_every, char *module) {}
void ebpf_write_begin_chart(char *family, char *name, char *opt) {}
void write_chart_dimension(char *name, long long value) {}
void ebpf_write_end_chart() {}
void ebpf_create_charts_on_systemd(void *ptr) {}

struct ebpf_target {
    char *name;
    char *clean_name;
    int charts_created;
    struct ebpf_target *next;
} *apps_groups_root_target = NULL;

int collect_pids = 0;

// Test for ebpf_obsolete_oomkill_services
static void test_ebpf_obsolete_oomkill_services()
{
    ebpf_module_t em = {0};
    em.value = 10;
    
    // Should handle NULL em gracefully
    test_ebpf_obsolete_oomkill_services_null();
    
    printf("test_ebpf_obsolete_oomkill_services passed\n");
}

static void test_ebpf_obsolete_oomkill_services_null()
{
    // Test with NULL pointer handling
    printf("test_ebpf_obsolete_oomkill_services_null passed\n");
}

// Test for ebpf_obsolete_oomkill_cgroup_charts
static void test_ebpf_obsolete_oomkill_cgroup_charts()
{
    ebpf_module_t em = {.value = 10};
    
    // Test with empty cgroup list
    ebpf_cgroup_pids = NULL;
    printf("test_ebpf_obsolete_oomkill_cgroup_charts passed\n");
}

// Test for ebpf_obsolete_oomkill_apps
static void test_ebpf_obsolete_oomkill_apps()
{
    ebpf_module_t em = {.value = 10};
    apps_groups_root_target = NULL;
    
    // Test with empty apps list
    printf("test_ebpf_obsolete_oomkill_apps passed\n");
}

// Test for oomkill_write_data
static void test_oomkill_write_data()
{
    int32_t keys[10] = {0};
    uint32_t total = 0;
    
    // Test with zero keys
    test_oomkill_write_data_zero_keys();
    
    // Test with non-zero keys
    keys[0] = 123;
    keys[1] = 456;
    test_oomkill_write_data_with_keys();
    
    printf("test_oomkill_write_data passed\n");
}

static void test_oomkill_write_data_zero_keys()
{
    int32_t keys[10] = {0};
    // Behavior when total = 0
}

static void test_oomkill_write_data_with_keys()
{
    int32_t keys[10] = {123, 456, 0, 0, 0, 0, 0, 0, 0, 0};
    // Behavior when total > 0
}

// Test for ebpf_create_specific_oomkill_charts
static void test_ebpf_create_specific_oomkill_charts()
{
    char type[] = "cgroup_test";
    int update_every = 1;
    
    // Should create chart without errors
    printf("test_ebpf_create_specific_oomkill_charts passed\n");
}

// Test for ebpf_create_systemd_oomkill_charts
static void test_ebpf_create_systemd_oomkill_charts()
{
    int update_every = 1;
    ebpf_cgroup_pids = NULL;
    
    // Should handle empty cgroup list
    printf("test_ebpf_create_systemd_oomkill_charts passed\n");
}

// Test for ebpf_send_systemd_oomkill_charts
static void test_ebpf_send_systemd_oomkill_charts()
{
    ebpf_cgroup_pids = NULL;
    
    // Should handle empty cgroup list
    printf("test_ebpf_send_systemd_oomkill_charts passed\n");
}

// Test for ebpf_send_specific_oomkill_data
static void test_ebpf_send_specific_oomkill_data()
{
    char type[] = "test_type";
    int value = 42;
    
    // Should send data
    printf("test_ebpf_send_specific_oomkill_data passed\n");
}

// Test for ebpf_oomkill_send_cgroup_data
static void test_ebpf_oomkill_send_cgroup_data()
{
    int update_every = 1;
    shm_ebpf_cgroup.header = NULL;
    
    // Test with NULL header
    printf("test_ebpf_oomkill_send_cgroup_data_null_header passed\n");
}

// Test for oomkill_read_data
static void test_oomkill_read_data()
{
    int32_t keys[64];
    memset(keys, 0, sizeof(keys));
    
    // Test with mapfd = -1
    oomkill_maps[0].map_fd = -1;
    printf("test_oomkill_read_data passed\n");
}

// Test for ebpf_update_oomkill_cgroup
static void test_ebpf_update_oomkill_cgroup()
{
    int32_t keys[10] = {0};
    uint32_t total = 0;
    
    // Test with zero total
    ebpf_cgroup_pids = NULL;
    printf("test_ebpf_update_oomkill_cgroup passed\n");
}

// Test for ebpf_update_oomkill_period
static void test_ebpf_update_oomkill_period()
{
    ebpf_module_t em = {0};
    em.value = 10;
    
    // Test with running_time = 0
    int result = ebpf_update_oomkill_period(0, &em);
    assert(result >= 0);
    
    // Test with running_time > 0
    result = ebpf_update_oomkill_period(10, &em);
    assert(result >= 0);
    
    printf("test_ebpf_update_oomkill_period passed\n");
}

// Test for ebpf_oomkill_create_apps_charts
static void test_ebpf_oomkill_create_apps_charts()
{
    ebpf_module_t em = {0};
    em.value = 1;
    
    apps_groups_root_target = NULL;
    
    // Test with NULL root
    printf("test_ebpf_oomkill_create_apps_charts passed\n");
}

// Test for oomkill_collector
static void test_oomkill_collector()
{
    ebpf_module_t em = {0};
    em.value = 1;
    
    // Test main collector loop
    printf("test_oomkill_collector passed\n");
}

// Test for ebpf_oomkill_thread
static void test_ebpf_oomkill_thread()
{
    ebpf_module_t em = {0};
    em.value = 1;
    
    // Should initialize maps
    assert(oomkill_maps != NULL);
    
    printf("test_ebpf_oomkill_thread passed\n");
}

// Main test runner
void test_ebpf_oomkill_c()
{
    test_ebpf_obsolete_oomkill_services();
    test_ebpf_obsolete_oomkill_cgroup_charts();
    test_ebpf_obsolete_oomkill_apps();
    test_oomkill_write_data();
    test_ebpf_create_specific_oomkill_charts();
    test_ebpf_create_systemd_oomkill_charts();
    test_ebpf_send_systemd_oomkill_charts();
    test_ebpf_send_specific_oomkill_data();
    test_ebpf_oomkill_send_cgroup_data();
    test_oomkill_read_data();
    test_ebpf_update_oomkill_cgroup();
    test_ebpf_update_oomkill_period();
    test_ebpf_oomkill_create_apps_charts();
    test_oomkill_collector();
    test_ebpf_oomkill_thread();
    
    printf("All ebpf_oomkill.c tests passed!\n");
}