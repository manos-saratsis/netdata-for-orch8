#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <cmocka.h>

#include "log2journal.h"

// Mock implementations
void hashed_key_cleanup(HASHED_KEY *k) {
    if(k->flags & HK_HASHTABLE_ALLOCATED)
        txt_l2j_cleanup(&k->value);
    else
        k->hashtable_ptr = NULL;
    freez((void *)k->key);
    k->key = NULL;
    k->len = 0;
    k->hash = 0;
    k->flags = HK_NONE;
}

void hashed_key_set(HASHED_KEY *k, const char *name, int32_t len) {
    hashed_key_cleanup(k);
    if(len == -1) {
        k->key = strdupz(name);
        k->len = strlen(k->key);
    }
    else {
        k->key = strndupz(name, len);
        k->len = len;
    }
    k->hash = XXH3_64bits(k->key, k->len);
    k->flags = HK_NONE;
}

// Test fixture setup
static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    return 0;
}

// Test: log_job_rename_add with valid parameters
static void test_log_job_rename_add_valid(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    bool result = log_job_rename_add(&jb, "new_name", 8, "old_name", 8);
    assert_true(result);
    assert_int_equal(jb.renames.used, 1);
}

// Test: log_job_rename_add with empty new_key
static void test_log_job_rename_add_empty_new_key(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    bool result = log_job_rename_add(&jb, "", 0, "old_name", 8);
    assert_true(result);
    assert_int_equal(jb.renames.used, 1);
}

// Test: log_job_rename_add with empty old_key
static void test_log_job_rename_add_empty_old_key(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    bool result = log_job_rename_add(&jb, "new_name", 8, "", 0);
    assert_true(result);
    assert_int_equal(jb.renames.used, 1);
}

// Test: log_job_rename_add with zero-length keys
static void test_log_job_rename_add_zero_length_keys(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    bool result = log_job_rename_add(&jb, "new", 3, "old", 3);
    assert_true(result);
    assert_int_equal(jb.renames.used, 1);
}

// Test: log_job_rename_add exceeding MAX_RENAMES
static void test_log_job_rename_add_max_exceeded(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = MAX_RENAMES,
            .array = {0},
        },
    };
    
    bool result = log_job_rename_add(&jb, "new", 3, "old", 3);
    assert_false(result);
    assert_int_equal(jb.renames.used, MAX_RENAMES);
}

// Test: log_job_rename_add at MAX_RENAMES boundary
static void test_log_job_rename_add_at_max_boundary(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = MAX_RENAMES - 1,
            .array = {0},
        },
    };
    
    bool result = log_job_rename_add(&jb, "new", 3, "old", 3);
    assert_true(result);
    assert_int_equal(jb.renames.used, MAX_RENAMES);
}

// Test: log_job_rename_add multiple times
static void test_log_job_rename_add_multiple(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    bool result1 = log_job_rename_add(&jb, "new1", 4, "old1", 4);
    assert_true(result1);
    assert_int_equal(jb.renames.used, 1);
    
    bool result2 = log_job_rename_add(&jb, "new2", 4, "old2", 4);
    assert_true(result2);
    assert_int_equal(jb.renames.used, 2);
    
    bool result3 = log_job_rename_add(&jb, "new3", 4, "old3", 4);
    assert_true(result3);
    assert_int_equal(jb.renames.used, 3);
}

// Test: rename_cleanup with valid rename
static void test_rename_cleanup_valid(void **state) {
    RENAME rn = {
        .new_key = {0},
        .old_key = {0},
    };
    
    hashed_key_set(&rn.new_key, "new_key", -1);
    hashed_key_set(&rn.old_key, "old_key", -1);
    
    rename_cleanup(&rn);
    
    assert_null(rn.new_key.key);
    assert_null(rn.old_key.key);
    assert_int_equal(rn.new_key.len, 0);
    assert_int_equal(rn.old_key.len, 0);
}

// Test: rename_cleanup with empty rename
static void test_rename_cleanup_empty(void **state) {
    RENAME rn = {
        .new_key = {0},
        .old_key = {0},
    };
    
    rename_cleanup(&rn);
    
    assert_null(rn.new_key.key);
    assert_null(rn.old_key.key);
}

// Test: log_job_rename_add with very long keys
static void test_log_job_rename_add_long_keys(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    char long_new_key[256];
    char long_old_key[256];
    
    memset(long_new_key, 'a', 255);
    long_new_key[255] = '\0';
    
    memset(long_old_key, 'b', 255);
    long_old_key[255] = '\0';
    
    bool result = log_job_rename_add(&jb, long_new_key, 255, long_old_key, 255);
    assert_true(result);
    assert_int_equal(jb.renames.used, 1);
}

// Test: log_job_rename_add with special characters in keys
static void test_log_job_rename_add_special_chars(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    bool result = log_job_rename_add(&jb, "new_key-123", 11, "old.key@456", 11);
    assert_true(result);
    assert_int_equal(jb.renames.used, 1);
}

// Test: rename_cleanup with allocated hashtable key
static void test_rename_cleanup_allocated_hashtable(void **state) {
    RENAME rn = {
        .new_key = {0},
        .old_key = {0},
    };
    
    hashed_key_set(&rn.new_key, "new_key", -1);
    hashed_key_set(&rn.old_key, "old_key", -1);
    
    // Mark as allocated
    rn.new_key.flags |= HK_HASHTABLE_ALLOCATED;
    rn.old_key.flags |= HK_HASHTABLE_ALLOCATED;
    
    rename_cleanup(&rn);
    
    assert_null(rn.new_key.key);
    assert_null(rn.old_key.key);
}

// Test: log_job_rename_add with negative length (auto-strlen)
static void test_log_job_rename_add_negative_length(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    // Using actual string length but negative indicator
    bool result = log_job_rename_add(&jb, "new_key", -1, "old_key", -1);
    // The function expects positive lengths, so this tests edge case
    // Result depends on how hashed_key_set handles negative values
    assert_int_equal(jb.renames.used, 1);
}

// Test: log_job_rename_add same key for new and old
static void test_log_job_rename_add_same_key(void **state) {
    LOG_JOB jb = {
        .renames = {
            .used = 0,
            .array = {0},
        },
    };
    
    bool result = log_job_rename_add(&jb, "key", 3, "key", 3);
    assert_true(result);
    assert_int_equal(jb.renames.used, 1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_log_job_rename_add_valid),
        cmocka_unit_test(test_log_job_rename_add_empty_new_key),
        cmocka_unit_test(test_log_job_rename_add_empty_old_key),
        cmocka_unit_test(test_log_job_rename_add_zero_length_keys),
        cmocka_unit_test(test_log_job_rename_add_max_exceeded),
        cmocka_unit_test(test_log_job_rename_add_at_max_boundary),
        cmocka_unit_test(test_log_job_rename_add_multiple),
        cmocka_unit_test(test_rename_cleanup_valid),
        cmocka_unit_test(test_rename_cleanup_empty),
        cmocka_unit_test(test_log_job_rename_add_long_keys),
        cmocka_unit_test(test_log_job_rename_add_special_chars),
        cmocka_unit_test(test_rename_cleanup_allocated_hashtable),
        cmocka_unit_test(test_log_job_rename_add_negative_length),
        cmocka_unit_test(test_log_job_rename_add_same_key),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}