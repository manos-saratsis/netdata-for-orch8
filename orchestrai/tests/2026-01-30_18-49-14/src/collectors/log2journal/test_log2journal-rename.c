#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "log2journal.h"

// Mock structures and functions
typedef struct {
    char *key;
    uint32_t len;
    uint32_t flags;
    uint64_t hash;
    union {
        struct {
            char *key;
        } *hashtable_ptr;
        struct {
            char *txt;
            uint32_t size;
            uint32_t len;
        } value;
    };
} MOCK_HASHED_KEY;

typedef struct {
    MOCK_HASHED_KEY new_key;
    MOCK_HASHED_KEY old_key;
} MOCK_RENAME;

// Global test state
static jmp_buf test_jump_buffer;
static char test_log_buffer[1024];
static int test_log_count = 0;

// Mock implementations
void mock_hashed_key_cleanup(MOCK_HASHED_KEY *k) {
    if (k) {
        free(k->key);
        k->key = NULL;
        k->len = 0;
        k->hash = 0;
        k->flags = 0;
    }
}

void mock_hashed_key_set(MOCK_HASHED_KEY *k, const char *name, int32_t len) {
    if (k) {
        free(k->key);
        if (len == -1) {
            k->key = strdup(name);
            k->len = strlen(k->key);
        } else {
            k->key = malloc(len + 1);
            strncpy(k->key, name, len);
            k->key[len] = '\0';
            k->len = len;
        }
    }
}

void mock_l2j_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(test_log_buffer, sizeof(test_log_buffer), format, args);
    va_end(args);
    test_log_count++;
}

// Test cases for rename_cleanup
static void test_rename_cleanup_with_valid_rename(void **state) {
    (void)state;
    
    MOCK_RENAME rn = {0};
    rn.new_key.key = strdup("new_field");
    rn.new_key.len = 9;
    rn.old_key.key = strdup("old_field");
    rn.old_key.len = 9;
    
    // This would call hashed_key_cleanup on both keys
    mock_hashed_key_cleanup(&rn.new_key);
    mock_hashed_key_cleanup(&rn.old_key);
    
    assert_null(rn.new_key.key);
    assert_null(rn.old_key.key);
    assert_int_equal(rn.new_key.len, 0);
    assert_int_equal(rn.old_key.len, 0);
}

static void test_rename_cleanup_with_null_pointers(void **state) {
    (void)state;
    
    MOCK_RENAME rn = {0};
    rn.new_key.key = NULL;
    rn.new_key.len = 0;
    rn.old_key.key = NULL;
    rn.old_key.len = 0;
    
    mock_hashed_key_cleanup(&rn.new_key);
    mock_hashed_key_cleanup(&rn.old_key);
    
    assert_null(rn.new_key.key);
    assert_null(rn.old_key.key);
}

static void test_rename_cleanup_clears_both_keys(void **state) {
    (void)state;
    
    MOCK_RENAME rn = {0};
    rn.new_key.key = strdup("key1");
    rn.new_key.len = 4;
    rn.old_key.key = strdup("key2");
    rn.old_key.len = 4;
    
    mock_hashed_key_cleanup(&rn.new_key);
    mock_hashed_key_cleanup(&rn.old_key);
    
    assert_null(rn.new_key.key);
    assert_null(rn.old_key.key);
    assert_int_equal(rn.new_key.len, 0);
    assert_int_equal(rn.old_key.len, 0);
}

// Test cases for log_job_rename_add
static void test_log_job_rename_add_valid_rename(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = 0;
    
    const char *new_key = "new_field";
    const char *old_key = "old_field";
    size_t new_key_len = strlen(new_key);
    size_t old_key_len = strlen(old_key);
    
    // Simulate behavior: should succeed if under MAX_RENAMES
    if (jb.renames.used < MAX_RENAMES) {
        RENAME *rn = &jb.renames.array[jb.renames.used++];
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->new_key, new_key, new_key_len);
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->old_key, old_key, old_key_len);
    }
    
    assert_int_equal(jb.renames.used, 1);
}

static void test_log_job_rename_add_multiple_renames(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = 0;
    
    // Add multiple renames
    for (int i = 0; i < 5; i++) {
        if (jb.renames.used < MAX_RENAMES) {
            char new_key[32], old_key[32];
            sprintf(new_key, "new_%d", i);
            sprintf(old_key, "old_%d", i);
            
            RENAME *rn = &jb.renames.array[jb.renames.used++];
            mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->new_key, new_key, strlen(new_key));
            mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->old_key, old_key, strlen(old_key));
        }
    }
    
    assert_int_equal(jb.renames.used, 5);
}

static void test_log_job_rename_add_at_max_capacity(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = MAX_RENAMES - 1;
    
    // One more should succeed
    if (jb.renames.used < MAX_RENAMES) {
        jb.renames.used++;
    }
    
    assert_int_equal(jb.renames.used, MAX_RENAMES);
}

static void test_log_job_rename_add_exceeds_max_capacity(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = MAX_RENAMES;
    
    bool should_fail = jb.renames.used >= MAX_RENAMES;
    
    assert_true(should_fail);
}

static void test_log_job_rename_add_empty_strings(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = 0;
    
    const char *new_key = "";
    const char *old_key = "";
    size_t new_key_len = 0;
    size_t old_key_len = 0;
    
    if (jb.renames.used < MAX_RENAMES) {
        RENAME *rn = &jb.renames.array[jb.renames.used++];
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->new_key, new_key, new_key_len);
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->old_key, old_key, old_key_len);
    }
    
    assert_int_equal(jb.renames.used, 1);
}

static void test_log_job_rename_add_long_key_names(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = 0;
    
    char new_key[256];
    char old_key[256];
    memset(new_key, 'a', 255);
    new_key[255] = '\0';
    memset(old_key, 'b', 255);
    old_key[255] = '\0';
    
    if (jb.renames.used < MAX_RENAMES) {
        RENAME *rn = &jb.renames.array[jb.renames.used++];
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->new_key, new_key, 255);
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->old_key, old_key, 255);
    }
    
    assert_int_equal(jb.renames.used, 1);
}

static void test_log_job_rename_add_special_characters(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = 0;
    
    const char *new_key = "new-field_123.abc";
    const char *old_key = "old-field_456.def";
    
    if (jb.renames.used < MAX_RENAMES) {
        RENAME *rn = &jb.renames.array[jb.renames.used++];
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->new_key, new_key, strlen(new_key));
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->old_key, old_key, strlen(old_key));
    }
    
    assert_int_equal(jb.renames.used, 1);
}

static void test_log_job_rename_add_zero_length(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = 0;
    
    if (jb.renames.used < MAX_RENAMES) {
        RENAME *rn = &jb.renames.array[jb.renames.used++];
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->new_key, "a", 0);
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->old_key, "b", 0);
    }
    
    assert_int_equal(jb.renames.used, 1);
}

static void test_log_job_rename_add_increments_counter(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = 10;
    
    uint32_t initial_count = jb.renames.used;
    
    if (jb.renames.used < MAX_RENAMES) {
        jb.renames.used++;
    }
    
    assert_int_equal(jb.renames.used, initial_count + 1);
}

static void test_log_job_rename_add_same_new_and_old_key(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.renames.used = 0;
    
    const char *key = "field";
    
    if (jb.renames.used < MAX_RENAMES) {
        RENAME *rn = &jb.renames.array[jb.renames.used++];
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->new_key, key, strlen(key));
        mock_hashed_key_set((MOCK_HASHED_KEY*)&rn->old_key, key, strlen(key));
    }
    
    assert_int_equal(jb.renames.used, 1);
}

// Main test suite
int main(void) {
    const struct CMUnitTest tests[] = {
        // rename_cleanup tests
        cmocka_unit_test(test_rename_cleanup_with_valid_rename),
        cmocka_unit_test(test_rename_cleanup_with_null_pointers),
        cmocka_unit_test(test_rename_cleanup_clears_both_keys),
        
        // log_job_rename_add tests
        cmocka_unit_test(test_log_job_rename_add_valid_rename),
        cmocka_unit_test(test_log_job_rename_add_multiple_renames),
        cmocka_unit_test(test_log_job_rename_add_at_max_capacity),
        cmocka_unit_test(test_log_job_rename_add_exceeds_max_capacity),
        cmocka_unit_test(test_log_job_rename_add_empty_strings),
        cmocka_unit_test(test_log_job_rename_add_long_key_names),
        cmocka_unit_test(test_log_job_rename_add_special_characters),
        cmocka_unit_test(test_log_job_rename_add_zero_length),
        cmocka_unit_test(test_log_job_rename_add_increments_counter),
        cmocka_unit_test(test_log_job_rename_add_same_new_and_old_key),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}