#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/mount.h>

/* Forward declarations and mocks for dependencies */
typedef struct {
    char *name;
    uint32_t hash;
    size_t len;
    int configured;
    int enabled;
    int updated;
    int do_space;
    int do_inodes;
    size_t collected;
    void *st_space;
    void *rd_space_used;
    void *rd_space_avail;
    void *rd_space_reserved;
    void *st_inodes;
    void *rd_inodes_used;
    void *rd_inodes_avail;
    struct mount_point *next;
} mount_point_t;

/* Global state for tests */
static mount_point_t *mount_points_root = NULL;
static mount_point_t *mount_points_last_used = NULL;
static size_t mount_points_added = 0;
static size_t mount_points_found = 0;

/* Mock functions */
uint32_t simple_hash(const char *name) {
    uint32_t hash = 0;
    while (*name) {
        hash = ((hash << 5) + hash) + *name++;
    }
    return hash;
}

void* callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void* reallocz(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void freez(void *ptr) {
    free(ptr);
}

char* strdupz(const char *str) {
    return strdup(str);
}

void rrdset_is_obsolete___safe_from_collector_thread(void *st) {
    // Mock implementation
}

void rrddim_add(void *st, const char *name, const char *divisor, 
                long multiplier, long divisor_val, int algorithm) {
    // Mock implementation
}

void rrddim_set_by_pointer(void *st, void *rd, long value) {
    // Mock implementation
}

void rrddim_find_active(void *st, const char *name) {
    // Mock implementation
}

void* rrdset_create_localhost(const char *type, const char *id, const char *parent_id,
                             const char *title, const char *units, const char *family,
                             const char *context, const char *plugin_name,
                             const char *module_name, long priority, int update_every,
                             int type_val) {
    return malloc(sizeof(void*));
}

void rrdset_done(void *st) {
    // Mock implementation
}

void rrdlabels_add(void *labels, const char *name, const char *value, int source) {
    // Mock implementation
}

int inicfg_get_boolean_ondemand(void *config, const char *section, const char *name, int default_val) {
    return default_val;
}

const char* inicfg_get(void *config, const char *section, const char *name, const char *default_val) {
    return default_val;
}

void* simple_pattern_create(const char *patterns, const char *separator, int type, int case_sensitive) {
    return malloc(sizeof(void*));
}

int simple_pattern_matches(void *pattern, const char *str) {
    return 0;
}

void collector_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void collector_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

int getmntinfo(struct statfs **mntbufp, int flags) {
    return 0;
}

/* Actual implementation from the source file */
static void mount_point_free(mount_point_t *m) {
    if (m->st_space)
        rrdset_is_obsolete___safe_from_collector_thread(m->st_space);
    if (m->st_inodes)
        rrdset_is_obsolete___safe_from_collector_thread(m->st_inodes);

    mount_points_added--;
    freez(m->name);
    freez(m);
}

static void mount_points_cleanup() {
    if (mount_points_found == mount_points_added) return;

    mount_point_t *m = mount_points_root, *last = NULL;
    while(m) {
        if (!m->updated) {
            if (mount_points_last_used == m)
                mount_points_last_used = last;

            mount_point_t *t = m;

            if (m == mount_points_root || !last)
                mount_points_root = m = m->next;
            else
                last->next = m = m->next;

            t->next = NULL;
            mount_point_free(t);
        }
        else {
            last = m;
            m->updated = 0;
            m = m->next;
        }
    }
}

static mount_point_t *get_mount_point(const char *name) {
    mount_point_t *m;
    uint32_t hash = simple_hash(name);

    // search from last position to end
    for(m = mount_points_last_used ; m ; m = m->next) {
        if (hash == m->hash && !strcmp(name, m->name)) {
            mount_points_last_used = m->next;
            return m;
        }
    }

    // search from beginning to last position
    for(m = mount_points_root ; m != mount_points_last_used ; m = m->next) {
        if (hash == m->hash && !strcmp(name, m->name)) {
            mount_points_last_used = m->next;
            return m;
        }
    }

    // create new one
    m = callocz(1, sizeof(mount_point_t));
    m->name = strdupz(name);
    m->hash = simple_hash(m->name);
    m->len = strlen(m->name);
    mount_points_added++;

    if (mount_points_root) {
        mount_point_t *e;
        for(e = mount_points_root; e->next ; e = e->next) ;
        e->next = m;
    }
    else
        mount_points_root = m;

    return m;
}

/* TEST SUITE */

/* Setup and Teardown */
static int setup(void **state) {
    mount_points_root = NULL;
    mount_points_last_used = NULL;
    mount_points_added = 0;
    mount_points_found = 0;
    return 0;
}

static int teardown(void **state) {
    mount_point_t *m = mount_points_root;
    while (m) {
        mount_point_t *next = m->next;
        freez(m->name);
        freez(m);
        m = next;
    }
    mount_points_root = NULL;
    mount_points_last_used = NULL;
    mount_points_added = 0;
    mount_points_found = 0;
    return 0;
}

/* Tests for mount_point_free */
static void test_mount_point_free_with_space_and_inodes(void **state) {
    mount_point_t *m = callocz(1, sizeof(mount_point_t));
    m->name = strdupz("/mnt/test");
    m->st_space = malloc(sizeof(void*));
    m->st_inodes = malloc(sizeof(void*));
    mount_points_added = 1;

    mount_point_free(m);

    assert_int_equal(mount_points_added, 0);
}

static void test_mount_point_free_with_null_pointers(void **state) {
    mount_point_t *m = callocz(1, sizeof(mount_point_t));
    m->name = strdupz("/mnt/test");
    m->st_space = NULL;
    m->st_inodes = NULL;
    mount_points_added = 1;

    mount_point_free(m);

    assert_int_equal(mount_points_added, 0);
}

/* Tests for get_mount_point */
static void test_get_mount_point_create_new(void **state) {
    mount_point_t *m = get_mount_point("/mnt/test");

    assert_non_null(m);
    assert_string_equal(m->name, "/mnt/test");
    assert_int_equal(mount_points_added, 1);
    assert_ptr_equal(mount_points_root, m);
}

static void test_get_mount_point_search_from_last_used(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    mount_points_last_used = m2;

    mount_point_t *found = get_mount_point("/mnt/second");

    assert_ptr_equal(found, m2);
}

static void test_get_mount_point_search_from_root(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    mount_points_last_used = NULL;

    mount_point_t *found = get_mount_point("/mnt/first");

    assert_ptr_equal(found, m1);
}

static void test_get_mount_point_multiple_additions(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    mount_point_t *m3 = get_mount_point("/mnt/third");

    assert_int_equal(mount_points_added, 3);
    assert_ptr_equal(mount_points_root, m1);
    assert_ptr_equal(m1->next, m2);
    assert_ptr_equal(m2->next, m3);
}

static void test_get_mount_point_hash_consistency(void **state) {
    mount_point_t *m = get_mount_point("/mnt/test");
    uint32_t expected_hash = simple_hash("/mnt/test");

    assert_int_equal(m->hash, expected_hash);
}

/* Tests for mount_points_cleanup */
static void test_mount_points_cleanup_no_removals(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    m1->updated = 1;
    m2->updated = 1;
    mount_points_found = 2;

    mount_points_cleanup();

    assert_ptr_equal(mount_points_root, m1);
    assert_ptr_equal(m1->next, m2);
    assert_int_equal(m1->updated, 0);
    assert_int_equal(m2->updated, 0);
}

static void test_mount_points_cleanup_remove_first(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    m1->updated = 0;
    m2->updated = 1;
    mount_points_found = 2;

    mount_points_cleanup();

    assert_ptr_equal(mount_points_root, m2);
    assert_null(m2->next);
    assert_int_equal(mount_points_added, 1);
}

static void test_mount_points_cleanup_remove_middle(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    mount_point_t *m3 = get_mount_point("/mnt/third");
    m1->updated = 1;
    m2->updated = 0;
    m3->updated = 1;
    mount_points_found = 3;

    mount_points_cleanup();

    assert_ptr_equal(mount_points_root, m1);
    assert_ptr_equal(m1->next, m3);
    assert_null(m3->next);
}

static void test_mount_points_cleanup_remove_last(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    m1->updated = 1;
    m2->updated = 0;
    mount_points_found = 2;

    mount_points_cleanup();

    assert_ptr_equal(mount_points_root, m1);
    assert_null(m1->next);
}

static void test_mount_points_cleanup_all_removed(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    m1->updated = 0;
    m2->updated = 0;
    mount_points_found = 2;

    mount_points_cleanup();

    assert_null(mount_points_root);
    assert_int_equal(mount_points_added, 0);
}

static void test_mount_points_cleanup_early_exit_when_same(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_points_found = 1;
    mount_points_added = 1;

    mount_points_cleanup();

    // Should return early without processing
    assert_ptr_equal(mount_points_root, m1);
}

static void test_mount_points_cleanup_last_used_update(void **state) {
    mount_point_t *m1 = get_mount_point("/mnt/first");
    mount_point_t *m2 = get_mount_point("/mnt/second");
    mount_point_t *m3 = get_mount_point("/mnt/third");
    
    m1->updated = 1;
    m2->updated = 0;
    m3->updated = 1;
    mount_points_last_used = m2;
    mount_points_found = 3;

    mount_points_cleanup();

    assert_ptr_equal(mount_points_last_used, m1);
}

/* Tests for edge cases */
static void test_get_mount_point_empty_string(void **state) {
    mount_point_t *m = get_mount_point("");

    assert_non_null(m);
    assert_string_equal(m->name, "");
}

static void test_get_mount_point_special_characters(void **state) {
    mount_point_t *m = get_mount_point("/mnt/test-123_abc");

    assert_non_null(m);
    assert_string_equal(m->name, "/mnt/test-123_abc");
}

static void test_get_mount_point_long_path(void **state) {
    char long_path[4096];
    snprintf(long_path, sizeof(long_path), "/mnt/very/long/path/with/many/components");
    
    mount_point_t *m = get_mount_point(long_path);

    assert_non_null(m);
    assert_string_equal(m->name, long_path);
}

static void test_mount_point_free_only_space(void **state) {
    mount_point_t *m = callocz(1, sizeof(mount_point_t));
    m->name = strdupz("/mnt/test");
    m->st_space = malloc(sizeof(void*));
    m->st_inodes = NULL;
    mount_points_added = 1;

    mount_point_free(m);

    assert_int_equal(mount_points_added, 0);
}

static void test_mount_point_free_only_inodes(void **state) {
    mount_point_t *m = callocz(1, sizeof(mount_point_t));
    m->name = strdupz("/mnt/test");
    m->st_space = NULL;
    m->st_inodes = malloc(sizeof(void*));
    mount_points_added = 1;

    mount_point_free(m);

    assert_int_equal(mount_points_added, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        /* mount_point_free tests */
        cmocka_unit_test(test_mount_point_free_with_space_and_inodes),
        cmocka_unit_test(test_mount_point_free_with_null_pointers),
        cmocka_unit_test(test_mount_point_free_only_space),
        cmocka_unit_test(test_mount_point_free_only_inodes),

        /* get_mount_point tests */
        cmocka_unit_test_setup_teardown(test_get_mount_point_create_new, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_mount_point_search_from_last_used, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_mount_point_search_from_root, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_mount_point_multiple_additions, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_mount_point_hash_consistency, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_mount_point_empty_string, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_mount_point_special_characters, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_mount_point_long_path, setup, teardown),

        /* mount_points_cleanup tests */
        cmocka_unit_test_setup_teardown(test_mount_points_cleanup_no_removals, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mount_points_cleanup_remove_first, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mount_points_cleanup_remove_middle, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mount_points_cleanup_remove_last, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mount_points_cleanup_all_removed, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mount_points_cleanup_early_exit_when_same, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mount_points_cleanup_last_used_update, setup, teardown),
    };

    return cmocka_run_tests(tests);
}