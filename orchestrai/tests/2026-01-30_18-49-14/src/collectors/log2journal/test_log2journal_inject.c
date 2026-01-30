#include <cmocka.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Mock structures and functions
typedef struct {
    char *key;
    uint32_t len;
    uint64_t hash;
} HASHED_KEY;

typedef struct {
    HASHED_KEY name;
    bool is_variable;
    bool logged_error;
    void *next;
} REPLACE_NODE;

typedef struct {
    const char *pattern;
    REPLACE_NODE *nodes;
    bool has_variables;
} REPLACE_PATTERN;

typedef struct {
    HASHED_KEY key;
    REPLACE_PATTERN value;
} INJECTION;

typedef struct {
    struct {
        uint32_t used;
        INJECTION keys[512];
    } injections;
    
    struct {
        HASHED_KEY key;
        struct {
            uint32_t used;
            INJECTION keys[512];
        } injections;
    } unmatched;
} LOG_JOB;

// Mock implementations
void hashed_key_cleanup(HASHED_KEY *k) {
    if(k) {
        free((void *)k->key);
        k->key = NULL;
        k->len = 0;
        k->hash = 0;
    }
}

void hashed_key_set(HASHED_KEY *k, const char *name, int32_t len) {
    hashed_key_cleanup(k);
    if(len == -1) {
        k->key = strdup(name);
        k->len = strlen(k->key);
    } else {
        k->key = malloc(len + 1);
        memcpy(k->key, name, len);
        k->key[len] = '\0';
        k->len = len;
    }
}

void replace_pattern_cleanup(REPLACE_PATTERN *rp) {
    if(rp && rp->pattern) {
        free((void *)rp->pattern);
        rp->pattern = NULL;
    }
}

bool replace_pattern_set(REPLACE_PATTERN *rp, const char *pattern) {
    check_expected_ptr(pattern);
    return (bool)mock_type(int);
}

char *strndupz(const char *s, size_t n) {
    char *result = malloc(n + 1);
    if(result) {
        memcpy(result, s, n);
        result[n] = '\0';
    }
    return result;
}

void freez(void *ptr) {
    free(ptr);
}

void l2j_log(const char *format, ...) {
    (void)format;
}

// Include the actual source file
#include "../log2journal-inject.c"

// Test: injection_cleanup with valid injection
static void test_injection_cleanup_valid(void **state) {
    (void)state;
    
    INJECTION inj = {0};
    inj.key.key = strdup("test_key");
    inj.key.len = 8;
    inj.value.pattern = strdup("test_pattern");
    
    injection_cleanup(&inj);
    
    assert_null(inj.key.key);
    assert_null(inj.value.pattern);
    assert_int_equal(inj.key.len, 0);
}

// Test: injection_cleanup with NULL injection
static void test_injection_cleanup_null_injection(void **state) {
    (void)state;
    
    INJECTION inj = {0};
    memset(&inj, 0, sizeof(INJECTION));
    
    injection_cleanup(&inj);
    
    assert_null(inj.key.key);
}

// Test: log_job_injection_replace success path
static void test_log_job_injection_replace_success(void **state) {
    (void)state;
    
    INJECTION inj = {0};
    const char *key = "testkey";
    size_t key_len = 7;
    const char *value = "testvalue";
    size_t value_len = 9;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_replace(&inj, key, key_len, value, value_len);
    
    assert_true(result);
    assert_non_null(inj.key.key);
    assert_int_equal(inj.key.len, 7);
}

// Test: log_job_injection_replace with long key
static void test_log_job_injection_replace_long_key(void **state) {
    (void)state;
    
    INJECTION inj = {0};
    const char *long_key = "this_is_a_very_long_key_that_exceeds_the_journal_max_key_length";
    size_t long_key_len = strlen(long_key);
    const char *value = "value";
    size_t value_len = 5;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_replace(&inj, long_key, long_key_len, value, value_len);
    
    assert_true(result);
}

// Test: log_job_injection_replace with long value
static void test_log_job_injection_replace_long_value(void **state) {
    (void)state;
    
    INJECTION inj = {0};
    const char *key = "key";
    size_t key_len = 3;
    char long_value[50000];
    memset(long_value, 'x', sizeof(long_value) - 1);
    long_value[sizeof(long_value) - 1] = '\0';
    size_t value_len = sizeof(long_value) - 1;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_replace(&inj, key, key_len, long_value, value_len);
    
    assert_true(result);
}

// Test: log_job_injection_replace pattern set failure
static void test_log_job_injection_replace_pattern_failure(void **state) {
    (void)state;
    
    INJECTION inj = {0};
    const char *key = "key";
    size_t key_len = 3;
    const char *value = "value";
    size_t value_len = 5;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 0);
    
    bool result = log_job_injection_replace(&inj, key, key_len, value, value_len);
    
    assert_false(result);
}

// Test: log_job_injection_add to matched injections when under limit
static void test_log_job_injection_add_matched_under_limit(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.injections.used = 10;
    
    const char *key = "key";
    size_t key_len = 3;
    const char *value = "value";
    size_t value_len = 5;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_add(&jb, key, key_len, value, value_len, false);
    
    assert_true(result);
    assert_int_equal(jb.injections.used, 11);
}

// Test: log_job_injection_add to matched injections at max limit
static void test_log_job_injection_add_matched_at_max(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.injections.used = 256; // MAX_INJECTIONS = 512/2 = 256
    
    const char *key = "key";
    size_t key_len = 3;
    const char *value = "value";
    size_t value_len = 5;
    
    bool result = log_job_injection_add(&jb, key, key_len, value, value_len, false);
    
    assert_false(result);
    assert_int_equal(jb.injections.used, 256);
}

// Test: log_job_injection_add to unmatched injections when under limit
static void test_log_job_injection_add_unmatched_under_limit(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.unmatched.injections.used = 10;
    
    const char *key = "key";
    size_t key_len = 3;
    const char *value = "value";
    size_t value_len = 5;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_add(&jb, key, key_len, value, value_len, true);
    
    assert_true(result);
    assert_int_equal(jb.unmatched.injections.used, 11);
}

// Test: log_job_injection_add to unmatched injections at max limit
static void test_log_job_injection_add_unmatched_at_max(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.unmatched.injections.used = 256;
    
    const char *key = "key";
    size_t key_len = 3;
    const char *value = "value";
    size_t value_len = 5;
    
    bool result = log_job_injection_add(&jb, key, key_len, value, value_len, true);
    
    assert_false(result);
    assert_int_equal(jb.unmatched.injections.used, 256);
}

// Test: log_job_injection_add with empty key
static void test_log_job_injection_add_empty_key(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.injections.used = 0;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_add(&jb, "", 0, "value", 5, false);
    
    assert_true(result);
    assert_int_equal(jb.injections.used, 1);
}

// Test: log_job_injection_add with empty value
static void test_log_job_injection_add_empty_value(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.injections.used = 0;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_add(&jb, "key", 3, "", 0, false);
    
    assert_true(result);
    assert_int_equal(jb.injections.used, 1);
}

// Test: log_job_injection_add matched with pattern failure
static void test_log_job_injection_add_matched_pattern_failure(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.injections.used = 5;
    
    const char *key = "key";
    size_t key_len = 3;
    const char *value = "value";
    size_t value_len = 5;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 0);
    
    bool result = log_job_injection_add(&jb, key, key_len, value, value_len, false);
    
    assert_false(result);
    assert_int_equal(jb.injections.used, 6);
}

// Test: log_job_injection_add unmatched with pattern failure
static void test_log_job_injection_add_unmatched_pattern_failure(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.unmatched.injections.used = 5;
    
    const char *key = "key";
    size_t key_len = 3;
    const char *value = "value";
    size_t value_len = 5;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 0);
    
    bool result = log_job_injection_add(&jb, key, key_len, value, value_len, true);
    
    assert_false(result);
    assert_int_equal(jb.unmatched.injections.used, 6);
}

// Test: log_job_injection_add large key and value
static void test_log_job_injection_add_large_values(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.injections.used = 100;
    
    char large_key[100];
    memset(large_key, 'k', sizeof(large_key) - 1);
    large_key[sizeof(large_key) - 1] = '\0';
    
    char large_value[10000];
    memset(large_value, 'v', sizeof(large_value) - 1);
    large_value[sizeof(large_value) - 1] = '\0';
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_add(&jb, large_key, sizeof(large_key) - 1, 
                                        large_value, sizeof(large_value) - 1, false);
    
    assert_true(result);
    assert_int_equal(jb.injections.used, 101);
}

// Test: log_job_injection_add single char key and value
static void test_log_job_injection_add_single_char(void **state) {
    (void)state;
    
    LOG_JOB jb = {0};
    jb.injections.used = 0;
    
    expect_any(replace_pattern_set, pattern);
    will_return(replace_pattern_set, 1);
    
    bool result = log_job_injection_add(&jb, "k", 1, "v", 1, false);
    
    assert_true(result);
    assert_int_equal(jb.injections.used, 1);
}

// Test: injection_cleanup with NULL key pointer
static void test_injection_cleanup_null_key_pointer(void **state) {
    (void)state;
    
    INJECTION inj = {0};
    inj.key.key = NULL;
    inj.value.pattern = NULL;
    
    injection_cleanup(&inj);
    
    assert_null(inj.key.key);
}

// Test: injection_cleanup with valid pattern
static void test_injection_cleanup_with_pattern(void **state) {
    (void)state;
    
    INJECTION inj = {0};
    inj.key.key = strdup("mykey");
    inj.key.len = 5;
    inj.value.pattern = strdup("mypattern");
    
    injection_cleanup(&inj);
    
    assert_null(inj.key.key);
    assert_null(inj.value.pattern);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_injection_cleanup_valid),
        cmocka_unit_test(test_injection_cleanup_null_injection),
        cmocka_unit_test(test_log_job_injection_replace_success),
        cmocka_unit_test(test_log_job_injection_replace_long_key),
        cmocka_unit_test(test_log_job_injection_replace_long_value),
        cmocka_unit_test(test_log_job_injection_replace_pattern_failure),
        cmocka_unit_test(test_log_job_injection_add_matched_under_limit),
        cmocka_unit_test(test_log_job_injection_add_matched_at_max),
        cmocka_unit_test(test_log_job_injection_add_unmatched_under_limit),
        cmocka_unit_test(test_log_job_injection_add_unmatched_at_max),
        cmocka_unit_test(test_log_job_injection_add_empty_key),
        cmocka_unit_test(test_log_job_injection_add_empty_value),
        cmocka_unit_test(test_log_job_injection_add_matched_pattern_failure),
        cmocka_unit_test(test_log_job_injection_add_unmatched_pattern_failure),
        cmocka_unit_test(test_log_job_injection_add_large_values),
        cmocka_unit_test(test_log_job_injection_add_single_char),
        cmocka_unit_test(test_injection_cleanup_null_key_pointer),
        cmocka_unit_test(test_injection_cleanup_with_pattern),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}