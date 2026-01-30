// SPDX-License-Identifier: GPL-3.0-or-later
// Comprehensive tests for apps_pid_match.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock the required structures and functions
#define SIMPLE_PATTERN_NO_SEPARATORS 1
#define SIMPLE_PATTERN_EXACT 2

typedef struct {
    int dummy;
} SIMPLE_PATTERN;

typedef struct {
    char *str;
} STRING;

typedef struct pid_stat {
    int pid;
    STRING *comm;
    STRING *comm_orig;
    STRING *name;
    STRING *cmdline;
} pid_stat;

typedef struct apps_match {
    bool starts_with;
    bool ends_with;
    STRING *compare;
    SIMPLE_PATTERN *pattern;
} APPS_MATCH;

// Mock functions
bool simple_pattern_matches_string(SIMPLE_PATTERN *pattern, STRING *string) {
    if (!pattern || !string || !string->str) return false;
    return strstr(string->str, "match") != NULL;
}

bool string_equals_string_nocase(STRING *s1, STRING *s2) {
    if (!s1 || !s2 || !s1->str || !s2->str) return false;
    return strcasecmp(s1->str, s2->str) == 0;
}

bool string_starts_with_string_nocase(STRING *s1, STRING *s2) {
    if (!s1 || !s2 || !s1->str || !s2->str) return false;
    size_t len = strlen(s2->str);
    return strncasecmp(s1->str, s2->str, len) == 0;
}

bool string_ends_with_string_nocase(STRING *s1, STRING *s2) {
    if (!s1 || !s2 || !s1->str || !s2->str) return false;
    size_t len1 = strlen(s1->str);
    size_t len2 = strlen(s2->str);
    if (len2 > len1) return false;
    return strcasecmp(s1->str + len1 - len2, s2->str) == 0;
}

const char *string2str(STRING *s) {
    return s && s->str ? s->str : "";
}

char *strcasestr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    // Simple implementation for testing
    size_t needle_len = strlen(needle);
    for (const char *p = haystack; *p; p++) {
        if (strncasecmp(p, needle, needle_len) == 0) {
            return (char *)p;
        }
    }
    return NULL;
}

SIMPLE_PATTERN *simple_pattern_create(const char *str, int a, int b, bool c) {
    SIMPLE_PATTERN *p = malloc(sizeof(SIMPLE_PATTERN));
    return p;
}

void simple_pattern_free(SIMPLE_PATTERN *p) {
    if (p) free(p);
}

STRING *string_strdupz(const char *str) {
    STRING *s = malloc(sizeof(STRING));
    s->str = str ? strdup(str) : NULL;
    return s;
}

void string_freez(STRING *s) {
    if (s) {
        free(s->str);
        free(s);
    }
}

void trim_all(char *buf) {
    // Simple trim implementation
    if (!buf) return;
    size_t len = strlen(buf);
    while (len > 0 && (buf[len-1] == ' ' || buf[len-1] == '\t' || buf[len-1] == '\n')) {
        buf[--len] = '\0';
    }
    size_t i = 0;
    while (buf[i] && (buf[i] == ' ' || buf[i] == '\t')) i++;
    if (i > 0) {
        memmove(buf, buf + i, len - i + 1);
    }
}

// Include the actual function under test
#define PROCESSES_HAVE_COMM_AND_NAME 0

bool pid_match_check(struct pid_stat *p, APPS_MATCH *match);
APPS_MATCH pid_match_create(const char *comm);
void pid_match_cleanup(APPS_MATCH *m);

// ===== TEST CASES FOR pid_match_check =====

static void test_pid_match_check_exact_match_comm(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "nginx";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "nginx";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = false;
    match.ends_with = false;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "nginx";

    // Test exact match on comm
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_exact_match_comm_orig(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "nginx_modified";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "nginx";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = false;
    match.ends_with = false;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "nginx";

    // Test exact match on comm_orig
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_exact_match_case_insensitive(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "NGINX";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "NGINX";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = false;
    match.ends_with = false;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "nginx";

    // Test case insensitive exact match
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_exact_no_match(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "apache";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "apache";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = false;
    match.ends_with = false;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "nginx";

    // Test exact match that should fail
    bool result = pid_match_check(&p, &match);
    assert_false(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_prefix_match(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "python3";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "python3";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = true;
    match.ends_with = false;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "python";

    // Test prefix match
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_prefix_match_case_insensitive(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "PYTHON3";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "PYTHON3";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = true;
    match.ends_with = false;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "python";

    // Test case insensitive prefix match
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_prefix_no_match(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "ruby";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "ruby";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = true;
    match.ends_with = false;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "python";

    // Test prefix match that should fail
    bool result = pid_match_check(&p, &match);
    assert_false(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_suffix_match(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "java.exe";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "java.exe";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = false;
    match.ends_with = true;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = ".exe";

    // Test suffix match
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_suffix_match_case_insensitive(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "JAVA.EXE";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "JAVA.EXE";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = false;
    match.ends_with = true;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = ".exe";

    // Test case insensitive suffix match
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_suffix_no_match(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "java.so";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "java.so";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = false;
    match.ends_with = true;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = ".exe";

    // Test suffix match that should fail
    bool result = pid_match_check(&p, &match);
    assert_false(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.compare);
}

static void test_pid_match_check_substring_match(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "cmd";
    p.cmdline = malloc(sizeof(STRING));
    p.cmdline->str = "python script.py arg1";

    APPS_MATCH match;
    match.starts_with = true;
    match.ends_with = true;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "script.py";

    // Test substring match in cmdline
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.cmdline);
    free(match.compare);
}

static void test_pid_match_check_substring_match_case_insensitive(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "cmd";
    p.cmdline = malloc(sizeof(STRING));
    p.cmdline->str = "PYTHON SCRIPT.PY ARG1";

    APPS_MATCH match;
    match.starts_with = true;
    match.ends_with = true;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "script.py";

    // Test case insensitive substring match
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.cmdline);
    free(match.compare);
}

static void test_pid_match_check_substring_no_match(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "cmd";
    p.cmdline = malloc(sizeof(STRING));
    p.cmdline->str = "python otherscript.py arg1";

    APPS_MATCH match;
    match.starts_with = true;
    match.ends_with = true;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "script.py";

    // Test substring match that should fail
    bool result = pid_match_check(&p, &match);
    assert_false(result);

    // Cleanup
    free(p.comm);
    free(p.cmdline);
    free(match.compare);
}

static void test_pid_match_check_null_cmdline(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "cmd";
    p.cmdline = NULL;

    APPS_MATCH match;
    match.starts_with = true;
    match.ends_with = true;
    match.pattern = NULL;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "script.py";

    // Test substring match with null cmdline
    bool result = pid_match_check(&p, &match);
    assert_false(result);

    // Cleanup
    free(p.comm);
    free(match.compare);
}

static void test_pid_match_check_pattern_exact(void **state) {
    // Setup
    struct pid_stat p;
    p.comm = malloc(sizeof(STRING));
    p.comm->str = "match";
    p.comm_orig = malloc(sizeof(STRING));
    p.comm_orig->str = "match";
    p.name = NULL;

    APPS_MATCH match;
    match.starts_with = false;
    match.ends_with = false;
    match.pattern = malloc(sizeof(SIMPLE_PATTERN));
    match.pattern->dummy = 1;
    match.compare = malloc(sizeof(STRING));
    match.compare->str = "match";

    // Test pattern match (exact)
    bool result = pid_match_check(&p, &match);
    assert_true(result);

    // Cleanup
    free(p.comm);
    free(p.comm_orig);
    free(match.pattern);
    free(match.compare);
}

// ===== TEST CASES FOR pid_match_create =====

static void test_pid_match_create_exact_match(void **state) {
    // Test exact match pattern
    APPS_MATCH m = pid_match_create("nginx");
    
    assert_false(m.starts_with);
    assert_false(m.ends_with);
    assert_non_null(m.compare);
    assert_null(m.pattern);
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

static void test_pid_match_create_prefix_match(void **state) {
    // Test prefix pattern
    APPS_MATCH m = pid_match_create("python*");
    
    assert_true(m.starts_with);
    assert_false(m.ends_with);
    assert_non_null(m.compare);
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

static void test_pid_match_create_suffix_match(void **state) {
    // Test suffix pattern
    APPS_MATCH m = pid_match_create("*.exe");
    
    assert_false(m.starts_with);
    assert_true(m.ends_with);
    assert_non_null(m.compare);
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

static void test_pid_match_create_substring_match(void **state) {
    // Test substring pattern
    APPS_MATCH m = pid_match_create("*script*");
    
    assert_true(m.starts_with);
    assert_true(m.ends_with);
    assert_non_null(m.compare);
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

static void test_pid_match_create_with_spaces(void **state) {
    // Test pattern with spaces (should be trimmed)
    APPS_MATCH m = pid_match_create("  nginx  *");
    
    assert_true(m.starts_with);
    assert_false(m.ends_with);
    assert_non_null(m.compare);
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

static void test_pid_match_create_complex_pattern(void **state) {
    // Test pattern with wildcards in middle
    APPS_MATCH m = pid_match_create("*py*");
    
    assert_true(m.starts_with);
    assert_true(m.ends_with);
    assert_non_null(m.compare);
    assert_non_null(m.pattern);
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

static void test_pid_match_create_empty_after_wildcards(void **state) {
    // Test pattern that becomes empty after wildcard removal
    APPS_MATCH m = pid_match_create("**");
    
    assert_true(m.starts_with);
    assert_true(m.ends_with);
    // compare should be empty string
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

static void test_pid_match_create_single_wildcard_prefix(void **state) {
    // Test single character before wildcard
    APPS_MATCH m = pid_match_create("p*");
    
    assert_true(m.starts_with);
    assert_false(m.ends_with);
    assert_non_null(m.compare);
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

static void test_pid_match_create_single_wildcard_suffix(void **state) {
    // Test single character after wildcard
    APPS_MATCH m = pid_match_create("*e");
    
    assert_false(m.starts_with);
    assert_true(m.ends_with);
    assert_non_null(m.compare);
    
    // Cleanup
    string_freez(m.compare);
    simple_pattern_free(m.pattern);
}

// ===== TEST CASES FOR pid_match_cleanup =====

static void test_pid_match_cleanup(void **state) {
    // Setup
    APPS_MATCH m;
    m.compare = malloc(sizeof(STRING));
    m.compare->str = malloc(10);
    strcpy(m.compare->str, "test");
    m.pattern = malloc(sizeof(SIMPLE_PATTERN));
    
    // Test cleanup (should not crash)
    pid_match_cleanup(&m);
    
    // After cleanup, pointers should be handled by cleanup function
    assert_true(true); // Just verify it didn't crash
}

static void test_pid_match_cleanup_null_pattern(void **state) {
    // Setup
    APPS_MATCH m;
    m.compare = malloc(sizeof(STRING));
    m.compare->str = malloc(10);
    strcpy(m.compare->str, "test");
    m.pattern = NULL;
    
    // Test cleanup with null pattern
    pid_match_cleanup(&m);
    
    assert_true(true); // Just verify it didn't crash
}

// ===== MAIN TEST SUITE =====

int main(void) {
    const struct CMUnitTest tests[] = {
        // pid_match_check tests
        cmocka_unit_test(test_pid_match_check_exact_match_comm),
        cmocka_unit_test(test_pid_match_check_exact_match_comm_orig),
        cmocka_unit_test(test_pid_match_check_exact_match_case_insensitive),
        cmocka_unit_test(test_pid_match_check_exact_no_match),
        cmocka_unit_test(test_pid_match_check_prefix_match),
        cmocka_unit_test(test_pid_match_check_prefix_match_case_insensitive),
        cmocka_unit_test(test_pid_match_check_prefix_no_match),
        cmocka_unit_test(test_pid_match_check_suffix_match),
        cmocka_unit_test(test_pid_match_check_suffix_match_case_insensitive),
        cmocka_unit_test(test_pid_match_check_suffix_no_match),
        cmocka_unit_test(test_pid_match_check_substring_match),
        cmocka_unit_test(test_pid_match_check_substring_match_case_insensitive),
        cmocka_unit_test(test_pid_match_check_substring_no_match),
        cmocka_unit_test(test_pid_match_check_null_cmdline),
        cmocka_unit_test(test_pid_match_check_pattern_exact),

        // pid_match_create tests
        cmocka_unit_test(test_pid_match_create_exact_match),
        cmocka_unit_test(test_pid_match_create_prefix_match),
        cmocka_unit_test(test_pid_match_create_suffix_match),
        cmocka_unit_test(test_pid_match_create_substring_match),
        cmocka_unit_test(test_pid_match_create_with_spaces),
        cmocka_unit_test(test_pid_match_create_complex_pattern),
        cmocka_unit_test(test_pid_match_create_empty_after_wildcards),
        cmocka_unit_test(test_pid_match_create_single_wildcard_prefix),
        cmocka_unit_test(test_pid_match_create_single_wildcard_suffix),

        // pid_match_cleanup tests
        cmocka_unit_test(test_pid_match_cleanup),
        cmocka_unit_test(test_pid_match_cleanup_null_pattern),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}