#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

/* Mock definitions and structures */
#define DICTIONARY_ITEM struct dictionary_item_mock
struct dictionary_item_mock {
    void *data;
};

typedef int SPINLOCK;
#define SPINLOCK_INITIALIZER 0
#define spinlock_lock(x) do { (void)(x); } while(0)
#define spinlock_unlock(x) do { (void)(x); } while(0)

typedef struct {
    char *labels;
} RRDLABELS;

#define SWAP(a, b) do { typeof(a) tmp = a; a = b; b = tmp; } while(0)

/* Mock functions */
static RRDLABELS* rrdlabels_create(void) {
    return (RRDLABELS*)malloc(sizeof(RRDLABELS));
}

static void rrdlabels_destroy(RRDLABELS *labels) {
    if(labels) free(labels);
}

static void rrdlabels_migrate_to_these(RRDLABELS *dst, RRDLABELS *src) {
    (void)dst;
    (void)src;
}

static char* strdupz(const char *s) {
    if(!s) return NULL;
    char *dup = (char*)malloc(strlen(s) + 1);
    if(dup) strcpy(dup, s);
    return dup;
}

static void freez(void *p) {
    if(p) free(p);
}

static void* cgroup_netdev_release_mock = NULL;
static void cgroup_netdev_release(const DICTIONARY_ITEM *link) {
    (void)link;
}

typedef struct {
    int (*on_item_delete)(void);
    int (*on_conflict)(void);
} DICTIONARY;

static DICTIONARY* dictionary_create_advanced(int options, void *x, size_t size) {
    (void)options;
    (void)x;
    (void)size;
    return (DICTIONARY*)malloc(sizeof(DICTIONARY));
}

static void dictionary_register_conflict_callback(DICTIONARY *dict, void *cb, void *data) {
    (void)dict;
    (void)cb;
    (void)data;
}

static void dictionary_register_delete_callback(DICTIONARY *dict, void *cb, void *data) {
    (void)dict;
    (void)cb;
    (void)data;
}

static void dictionary_destroy(DICTIONARY *dict) {
    if(dict) free(dict);
}

static void dictionary_set(DICTIONARY *dict, const char *key, void *value, size_t size) {
    (void)dict;
    (void)key;
    (void)value;
    (void)size;
}

static void dictionary_del(DICTIONARY *dict, const char *key) {
    (void)dict;
    (void)key;
}

/* Include the actual source code */
#include "proc_net_dev_renames.c"

/* Test functions */

void test_netdev_rename_task_cleanup_unsafe(void) {
    printf("TEST: netdev_rename_task_cleanup_unsafe\n");
    
    struct rename_task task = {
        .spinlock = SPINLOCK_INITIALIZER,
        .cgroup_netdev_link = (DICTIONARY_ITEM*)malloc(sizeof(DICTIONARY_ITEM)),
        .chart_labels = rrdlabels_create(),
        .container_name = strdupz("container1"),
        .container_device = strdupz("eth0"),
        .ctx_prefix = strdupz("ctx"),
    };
    
    netdev_rename_task_cleanup_unsafe(&task);
    
    assert(task.cgroup_netdev_link == NULL);
    assert(task.chart_labels == NULL);
    assert(task.container_name == NULL);
    assert(task.container_device == NULL);
    assert(task.ctx_prefix == NULL);
    
    printf("  PASSED\n");
}

void test_netdev_rename_task_cleanup_unsafe_null_pointers(void) {
    printf("TEST: netdev_rename_task_cleanup_unsafe with NULL pointers\n");
    
    struct rename_task task = {
        .spinlock = SPINLOCK_INITIALIZER,
        .cgroup_netdev_link = NULL,
        .chart_labels = NULL,
        .container_name = NULL,
        .container_device = NULL,
        .ctx_prefix = NULL,
    };
    
    netdev_rename_task_cleanup_unsafe(&task);
    
    assert(task.cgroup_netdev_link == NULL);
    assert(task.chart_labels == NULL);
    
    printf("  PASSED\n");
}

void test_netdev_renames_init_first_time(void) {
    printf("TEST: netdev_renames_init - first time\n");
    
    if(netdev_renames) {
        dictionary_destroy(netdev_renames);
        netdev_renames = NULL;
    }
    
    netdev_renames_init();
    
    assert(netdev_renames != NULL);
    
    printf("  PASSED\n");
}

void test_netdev_renames_init_already_initialized(void) {
    printf("TEST: netdev_renames_init - already initialized\n");
    
    if(netdev_renames) {
        dictionary_destroy(netdev_renames);
        netdev_renames = NULL;
    }
    
    netdev_renames_init();
    DICTIONARY *first_ptr = netdev_renames;
    
    netdev_renames_init();
    DICTIONARY *second_ptr = netdev_renames;
    
    assert(first_ptr == second_ptr);
    
    printf("  PASSED\n");
}

void test_netdev_renames_destroy(void) {
    printf("TEST: netdev_renames_destroy\n");
    
    netdev_renames_init();
    assert(netdev_renames != NULL);
    
    netdev_renames_destroy();
    assert(netdev_renames == NULL);
    
    printf("  PASSED\n");
}

void test_netdev_renames_destroy_already_null(void) {
    printf("TEST: netdev_renames_destroy - already NULL\n");
    
    if(netdev_renames) {
        dictionary_destroy(netdev_renames);
        netdev_renames = NULL;
    }
    
    netdev_renames_destroy();
    assert(netdev_renames == NULL);
    
    printf("  PASSED\n");
}

void test_cgroup_rename_task_add(void) {
    printf("TEST: cgroup_rename_task_add\n");
    
    const char *host_device = "eth0";
    const char *container_device = "veth123";
    const char *container_name = "mycontainer";
    RRDLABELS *labels = rrdlabels_create();
    const char *ctx_prefix = "ctx1";
    DICTIONARY_ITEM *cgroup_link = NULL;
    
    cgroup_rename_task_add(host_device, container_device, container_name, 
                          labels, ctx_prefix, cgroup_link);
    
    rrdlabels_destroy(labels);
    
    printf("  PASSED\n");
}

void test_cgroup_rename_task_add_empty_strings(void) {
    printf("TEST: cgroup_rename_task_add with empty strings\n");
    
    const char *host_device = "";
    const char *container_device = "";
    const char *container_name = "";
    RRDLABELS *labels = rrdlabels_create();
    const char *ctx_prefix = "";
    DICTIONARY_ITEM *cgroup_link = NULL;
    
    cgroup_rename_task_add(host_device, container_device, container_name, 
                          labels, ctx_prefix, cgroup_link);
    
    rrdlabels_destroy(labels);
    
    printf("  PASSED\n");
}

void test_cgroup_rename_task_device_del(void) {
    printf("TEST: cgroup_rename_task_device_del\n");
    
    cgroup_rename_task_device_del("eth0");
    
    printf("  PASSED\n");
}

void test_cgroup_rename_task_device_del_nonexistent(void) {
    printf("TEST: cgroup_rename_task_device_del - nonexistent device\n");
    
    cgroup_rename_task_device_del("nonexistent_device");
    
    printf("  PASSED\n");
}

void test_dictionary_netdev_rename_delete_cb(void) {
    printf("TEST: dictionary_netdev_rename_delete_cb\n");
    
    struct rename_task task = {
        .spinlock = SPINLOCK_INITIALIZER,
        .cgroup_netdev_link = (DICTIONARY_ITEM*)malloc(sizeof(DICTIONARY_ITEM)),
        .chart_labels = rrdlabels_create(),
        .container_name = strdupz("container"),
        .container_device = strdupz("eth0"),
        .ctx_prefix = strdupz("ctx"),
    };
    
    dictionary_netdev_rename_delete_cb(NULL, &task, NULL);
    
    assert(task.cgroup_netdev_link == NULL);
    assert(task.chart_labels == NULL);
    assert(task.container_name == NULL);
    assert(task.container_device == NULL);
    assert(task.ctx_prefix == NULL);
    
    printf("  PASSED\n");
}

void test_dictionary_netdev_rename_conflict_cb(void) {
    printf("TEST: dictionary_netdev_rename_conflict_cb\n");
    
    struct rename_task old_task = {
        .spinlock = SPINLOCK_INITIALIZER,
        .cgroup_netdev_link = NULL,
        .chart_labels = rrdlabels_create(),
        .container_name = strdupz("old_container"),
        .container_device = strdupz("eth0"),
        .ctx_prefix = strdupz("old_ctx"),
    };
    
    struct rename_task new_task = {
        .spinlock = SPINLOCK_INITIALIZER,
        .cgroup_netdev_link = (DICTIONARY_ITEM*)malloc(sizeof(DICTIONARY_ITEM)),
        .chart_labels = rrdlabels_create(),
        .container_name = strdupz("new_container"),
        .container_device = strdupz("eth1"),
        .ctx_prefix = strdupz("new_ctx"),
    };
    
    bool result = dictionary_netdev_rename_conflict_cb(NULL, &old_task, &new_task, NULL);
    
    assert(result == true);
    
    printf("  PASSED\n");
}

void test_dictionary_netdev_rename_conflict_cb_with_all_fields(void) {
    printf("TEST: dictionary_netdev_rename_conflict_cb with all fields\n");
    
    struct rename_task old_task = {
        .spinlock = SPINLOCK_INITIALIZER,
        .cgroup_netdev_link = (DICTIONARY_ITEM*)malloc(sizeof(DICTIONARY_ITEM)),
        .chart_labels = rrdlabels_create(),
        .container_name = strdupz("container1"),
        .container_device = strdupz("eth0"),
        .ctx_prefix = strdupz("ctx1"),
    };
    
    struct rename_task new_task = {
        .spinlock = SPINLOCK_INITIALIZER,
        .cgroup_netdev_link = (DICTIONARY_ITEM*)malloc(sizeof(DICTIONARY_ITEM)),
        .chart_labels = rrdlabels_create(),
        .container_name = strdupz("container2"),
        .container_device = strdupz("eth1"),
        .ctx_prefix = strdupz("ctx2"),
    };
    
    bool result = dictionary_netdev_rename_conflict_cb(NULL, &old_task, &new_task, NULL);
    
    assert(result == true);
    assert(new_task.cgroup_netdev_link == NULL);
    assert(new_task.chart_labels == NULL);
    assert(new_task.container_name == NULL);
    assert(new_task.container_device == NULL);
    assert(new_task.ctx_prefix == NULL);
    
    printf("  PASSED\n");
}

void test_freez_and_set_to_null_macro(void) {
    printf("TEST: freez_and_set_to_null macro\n");
    
    char *ptr = (char*)malloc(10);
    assert(ptr != NULL);
    
    freez_and_set_to_null(ptr);
    assert(ptr == NULL);
    
    printf("  PASSED\n");
}

int main(void) {
    printf("\n=== Testing proc_net_dev_renames ===\n\n");
    
    test_netdev_rename_task_cleanup_unsafe();
    test_netdev_rename_task_cleanup_unsafe_null_pointers();
    test_netdev_renames_init_first_time();
    test_netdev_renames_init_already_initialized();
    test_netdev_renames_destroy();
    test_netdev_renames_destroy_already_null();
    test_cgroup_rename_task_add();
    test_cgroup_rename_task_add_empty_strings();
    test_cgroup_rename_task_device_del();
    test_cgroup_rename_task_device_del_nonexistent();
    test_dictionary_netdev_rename_delete_cb();
    test_dictionary_netdev_rename_conflict_cb();
    test_dictionary_netdev_rename_conflict_cb_with_all_fields();
    test_freez_and_set_to_null_macro();
    
    printf("\n=== All tests passed! ===\n\n");
    
    return 0;
}