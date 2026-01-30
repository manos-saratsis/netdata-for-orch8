#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Mock structures and functions
typedef struct {
    char *key;
    uint32_t len;
    int flags;
} HASHED_KEY;

typedef struct {
    const char *pattern;
} REPLACE_PATTERN;

typedef struct {
    HASHED_KEY key;
    REPLACE_PATTERN value;
} INJECTION;

typedef struct {
    uint32_t used;
    INJECTION keys[512];
} INJECTION_ARRAY;

typedef struct {
    HASHED_KEY key;
    INJECTION_ARRAY injections;
} UNMATCHED_INJECTIONS;

typedef struct {
    INJECTION_ARRAY injections;
    UNMATCHED_INJECTIONS unmatched;
} LOG_JOB;

// Mock implementations
void hashed_key_cleanup(HASHED_KEY *k) {
    if (k && k->key) {
        free(k->key);
        k->key = NULL;
        k->len = 0;
    }
}

void hashed_key_set(HASHED_KEY *k, const char *name, int32_t len) {
    hashed_key_cleanup(k);
    if (!name) return;
    
    if (len == -1) {
        len = strlen(name);
    }
    
    k->key = malloc(len + 1);
    memcpy(k->key, name, len);
    k->key[len] = '\0';
    k->len = len;
}

void replace_pattern_cleanup(REPLACE_PATTERN *rp) {
    if (rp) {
        rp->pattern = NULL;
    }
}

bool replace_pattern_set(REPLACE_PATTERN *rp, const char *pattern) {
    if (!rp || !pattern) return false;
    rp->pattern = pattern;
    return true;
}

char *strndupz(const char *s, size_t len) {
    if (!s) return calloc(1, 1);
    char *result = malloc(len + 1);
    memcpy(result, s, len);
    result[len] = '\0';
    return result;
}

void freez(void *ptr) {
    free(ptr);
}

void l2j_log(const char *format, ...) {
    // No-op for testing
}

// Source code
void injection_cleanup(INJECTION *inj) {
    hashed_key_cleanup(&inj->key);
    replace_pattern_cleanup(&inj->value);
}

static inline bool log_job_injection_replace(INJECTION *inj, const char *key, size_t key_len, const char *value, size_t value_len) {
    if(key_len > 64)
        l2j_log("WARNING: injection key '%.*s' is too long for journal. Will be truncated.", (int)key_len, key);

    if(value_len > 48 * 1024)
        l2j_log(
            "WARNING: injection value of key '%.*s' is too long for journal. Will be truncated.", (int)key_len, key);

    hashed_key_set(&inj->key, key, key_len);
    char *v = strndupz(value, value_len);
    bool ret = replace_pattern_set(&inj->value, v);
    freez(v);

    return ret;
}

bool log_job_injection_add(LOG_JOB *jb, const char *key, size_t key_len, const char *value, size_t value_len, bool unmatched) {
    if (unmatched) {
        if (jb->unmatched.injections.used >= 256) {
            l2j_log("Error: too many unmatched injections. You can inject up to %d lines.", 256);
            return false;
        }
    }
    else {
        if (jb->injections.used >= 256) {
            l2j_log("Error: too many injections. You can inject up to %d lines.", 256);
            return false;
        }
    }

    bool ret;
    if (unmatched) {
        ret = log_job_injection_replace(&jb->unmatched.injections.keys[jb->unmatched.injections.used++],
                                        key, key_len, value, value_len);
    } else {
        ret = log_job_injection_replace(&jb->injections.keys[jb->injections.used++],
                                        key, key_len, value, value_len);
    }

    return ret;
}

// Tests
static void test_injection_cleanup_null(void **state) {
    INJECTION inj = {0};
    injection_cleanup(&inj);
    assert_null(inj.key.key);
}

static void test_injection_cleanup_with_key(void **state) {
    INJECTION inj = {0};
    hashed_key_set(&inj.key, "TEST_KEY", -1);
    assert_non_null(inj.key.key);
    
    injection_cleanup(&inj);
    assert_null(inj.key.key);
}

static void test_log_job_injection_add_matched_normal(void **state) {
    LOG_JOB jb = {0};
    
    bool result = log_job_injection_add(&jb, "key1", 4, "value1", 6, false);
    assert_true(result);
    assert_int_equal(jb.injections.used, 1);
    assert_non_null(jb.injections.keys[0].key.key);
}

static void test_log_job_injection_add_matched_multiple(void **state) {
    LOG_JOB jb = {0};
    
    bool result1 = log_job_injection_add(&jb, "key1", 4, "value1", 6, false);
    bool result2 = log_job_injection_add(&jb, "key2", 4, "value2", 6, false);
    
    assert_true(result1);
    assert_true(result2);
    assert_int_equal(jb.injections.used, 2);
}

static void test_log_job_injection_add_matched_max_limit(void **state) {
    LOG_JOB jb = {0};
    jb.injections.used = 256;
    
    bool result = log_job_injection_add(&jb, "key1", 4, "value1", 6, false);
    assert_false(result);
    assert_int_equal(jb.injections.used, 256);
}

static void test_log_job_injection_add_unmatched_normal(void **state) {
    LOG_JOB jb = {0};
    
    bool result = log_job_injection_add(&jb, "key1", 4, "value1", 6, true);
    assert_true(result);
    assert_int_equal(jb.unmatched.injections.used, 1);
}

static void test_log_job_injection_add_unmatched_multiple(void **state) {
    LOG_JOB jb = {0};
    
    bool result1 = log_job_injection_add(&jb, "key1", 4, "value1", 6, true);
    bool result2 = log_job_injection_add(&jb, "key2", 4, "value2", 6, true);
    
    assert_true(result1);
    assert_true(result2);
    assert_int_equal(jb.unmatched.injections.used, 2);
}

static void test_log_job_injection_add_unmatched_max_limit(void **state) {
    LOG_JOB jb = {0};
    jb.unmatched.injections.used = 256;
    
    bool result = log_job_injection_add(&jb, "key1", 4, "value1", 6, true);
    assert_false(result);
    assert_int_equal(jb.unmatched.injections.used, 256);
}

static void test_log_job_injection_add_empty_key(void **state) {
    LOG_JOB jb = {0};
    
    bool result = log_job_injection_add(&jb, "", 0, "value", 5, false);
    assert_true(result);
    assert_int_equal(jb.injections.used, 1);
}

static void test_log_job_injection_add_empty_value(void **state) {
    LOG_JOB jb = {0};
    
    bool result = log_job_injection_add(&jb, "key", 3, "", 0, false);
    assert_true(result);
    assert_int_equal(jb.injections.used, 1);
}

static void test_log_job_injection_add_long_key(void **state) {
    LOG_JOB jb = {0};
    char long_key[100];
    memset(long_key, 'a', 99);
    long_key[99] = '\0';
    
    bool result = log_job_injection_add(&jb, long_key, 99, "value", 5, false);
    assert_true(result);
    assert_int_equal(jb.injections.used, 1);
}

static void test_log_job_injection_add_long_value(void **state) {
    LOG_JOB jb = {0};
    char long_value[1000];
    memset(long_value, 'v', 999);
    long_value[999] = '\0';
    
    bool result = log_job_injection_add(&jb, "key", 3, long_value, 999, false);
    assert_true(result);
    assert_int_equal(jb.injections.used, 1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_injection_cleanup_null),
        cmocka_unit_test(test_injection_cleanup_with_key),
        cmocka_unit_test(test_log_job_injection_add_matched_normal),
        cmocka_unit_test(test_log_job_injection_add_matched_multiple),
        cmocka_unit_test(test_log_job_injection_add_matched_max_limit),
        cmocka_unit_test(test_log_job_injection_add_unmatched_normal),
        cmocka_unit_test(test_log_job_injection_add_unmatched_multiple),
        cmocka_unit_test(test_log_job_injection_add_unmatched_max_limit),
        cmocka_unit_test(test_log_job_injection_add_empty_key),
        cmocka_unit_test(test_log_job_injection_add_empty_value),
        cmocka_unit_test(test_log_job_injection_add_long_key),
        cmocka_unit_test(test_log_job_injection_add_long_value),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}