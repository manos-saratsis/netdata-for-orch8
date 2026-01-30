/*
 * Test suite for cgroup-internals.h
 * Tests static inline functions and macro definitions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Mocks and stubs */
static int test_matches_enabled_cgroup_paths_null_pattern(void **state) {
    (void) state;
    /* When pattern is NULL, should return 0 */
    int result = simple_pattern_matches(NULL, "test_id");
    assert_int_equal(result, 0);
    return 0;
}

static int test_matches_enabled_cgroup_paths_matching(void **state) {
    (void) state;
    /* Test matching enabled cgroup paths */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*docker*", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "/docker/container123");
    assert_int_not_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_enabled_cgroup_paths_non_matching(void **state) {
    (void) state;
    /* Test non-matching enabled cgroup paths */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*docker*", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "/system/other");
    assert_int_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_enabled_cgroup_names_matching(void **state) {
    (void) state;
    /* Test matching enabled cgroup names */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*service*", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "my-service");
    assert_int_not_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_enabled_cgroup_names_non_matching(void **state) {
    (void) state;
    /* Test non-matching enabled cgroup names */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*service*", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "unrelated");
    assert_int_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_enabled_cgroup_renames_matching(void **state) {
    (void) state;
    /* Test matching enabled cgroup renames */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*.scope", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "system-slice.scope");
    assert_int_not_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_enabled_cgroup_renames_non_matching(void **state) {
    (void) state;
    /* Test non-matching enabled cgroup renames */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*.scope", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "service.service");
    assert_int_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_systemd_services_cgroups_matching(void **state) {
    (void) state;
    /* Test matching systemd services cgroups */
    SIMPLE_PATTERN *pattern = simple_pattern_create("/system.slice/*.service", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "/system.slice/ssh.service");
    assert_int_not_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_systemd_services_cgroups_non_matching(void **state) {
    (void) state;
    /* Test non-matching systemd services cgroups */
    SIMPLE_PATTERN *pattern = simple_pattern_create("/system.slice/*.service", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "/user.slice/session.scope");
    assert_int_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_search_cgroup_paths_matching(void **state) {
    (void) state;
    /* Test matching search cgroup paths */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*/kubepods/*", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "/kubepods/pod123/container456");
    assert_int_not_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_search_cgroup_paths_non_matching(void **state) {
    (void) state;
    /* Test non-matching search cgroup paths */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*/kubepods/*", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "/docker/container");
    assert_int_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_entrypoint_parent_process_comm_matching(void **state) {
    (void) state;
    /* Test matching entrypoint parent process comm */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*bash*", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "bash");
    assert_int_not_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_matches_entrypoint_parent_process_comm_non_matching(void **state) {
    (void) state;
    /* Test non-matching entrypoint parent process comm */
    SIMPLE_PATTERN *pattern = simple_pattern_create("*bash*", NULL, SIMPLE_PATTERN_EXACT, true);
    assert_non_null(pattern);
    
    int result = simple_pattern_matches(pattern, "vim");
    assert_int_equal(result, 0);
    
    simple_pattern_free(pattern);
    return 0;
}

static int test_is_cgroup_systemd_service_true(void **state) {
    (void) state;
    /* Test is_cgroup_systemd_service when option is set */
    struct cgroup cg;
    cg.options = CGROUP_OPTIONS_SYSTEM_SLICE_SERVICE;
    
    int result = is_cgroup_systemd_service(&cg);
    assert_int_not_equal(result, 0);
    return 0;
}

static int test_is_cgroup_systemd_service_false(void **state) {
    (void) state;
    /* Test is_cgroup_systemd_service when option is not set */
    struct cgroup cg;
    cg.options = 0;
    
    int result = is_cgroup_systemd_service(&cg);
    assert_int_equal(result, 0);
    return 0;
}

static int test_k8s_is_kubepod_true(void **state) {
    (void) state;
    /* Test k8s_is_kubepod when orchestrator is k8s */
    struct cgroup cg;
    cg.container_orchestrator = CGROUPS_ORCHESTRATOR_K8S;
    
    int result = k8s_is_kubepod(&cg);
    assert_int_not_equal(result, 0);
    return 0;
}

static int test_k8s_is_kubepod_false_unset(void **state) {
    (void) state;
    /* Test k8s_is_kubepod when orchestrator is unset */
    struct cgroup cg;
    cg.container_orchestrator = CGROUPS_ORCHESTRATOR_UNSET;
    
    int result = k8s_is_kubepod(&cg);
    assert_int_equal(result, 0);
    return 0;
}

static int test_k8s_is_kubepod_false_unknown(void **state) {
    (void) state;
    /* Test k8s_is_kubepod when orchestrator is unknown */
    struct cgroup cg;
    cg.container_orchestrator = CGROUPS_ORCHESTRATOR_UNKNOWN;
    
    int result = k8s_is_kubepod(&cg);
    assert_int_equal(result, 0);
    return 0;
}

static int test_cgroup_chart_type_root(void **state) {
    (void) state;
    /* Test cgroup_chart_type for root cgroup */
    struct cgroup cg;
    cg.chart_id = "";
    cg.options = 0;
    
    char buffer[RRD_ID_LENGTH_MAX];
    char *result = cgroup_chart_type(buffer, &cg);
    
    assert_string_equal(result, "cgroup_root");
    return 0;
}

static int test_cgroup_chart_type_root_slash(void **state) {
    (void) state;
    /* Test cgroup_chart_type for root cgroup with / */
    struct cgroup cg;
    cg.chart_id = "/";
    cg.options = 0;
    
    char buffer[RRD_ID_LENGTH_MAX];
    char *result = cgroup_chart_type(buffer, &cg);
    
    assert_string_equal(result, "cgroup_root");
    return 0;
}

static int test_cgroup_chart_type_systemd_service(void **state) {
    (void) state;
    /* Test cgroup_chart_type for systemd service */
    struct cgroup cg;
    cg.chart_id = "ssh.service";
    cg.options = CGROUP_OPTIONS_SYSTEM_SLICE_SERVICE;
    
    char buffer[RRD_ID_LENGTH_MAX];
    snprintfz(buffer, RRD_ID_LENGTH_MAX, "%s%s", services_chart_id_prefix, cg.chart_id);
    char *result = cgroup_chart_type(buffer, &cg);
    
    assert_non_null(result);
    return 0;
}

static int test_cgroup_chart_type_normal_cgroup(void **state) {
    (void) state;
    /* Test cgroup_chart_type for normal cgroup */
    struct cgroup cg;
    cg.chart_id = "docker/container123";
    cg.options = 0;
    
    char buffer[RRD_ID_LENGTH_MAX];
    char *result = cgroup_chart_type(buffer, &cg);
    
    assert_non_null(result);
    assert_true(strstr(result, "docker") != NULL);
    return 0;
}

const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_matches_enabled_cgroup_paths_null_pattern),
    cmocka_unit_test(test_matches_enabled_cgroup_paths_matching),
    cmocka_unit_test(test_matches_enabled_cgroup_paths_non_matching),
    cmocka_unit_test(test_matches_enabled_cgroup_names_matching),
    cmocka_unit_test(test_matches_enabled_cgroup_names_non_matching),
    cmocka_unit_test(test_matches_enabled_cgroup_renames_matching),
    cmocka_unit_test(test_matches_enabled_cgroup_renames_non_matching),
    cmocka_unit_test(test_matches_systemd_services_cgroups_matching),
    cmocka_unit_test(test_matches_systemd_services_cgroups_non_matching),
    cmocka_unit_test(test_matches_search_cgroup_paths_matching),
    cmocka_unit_test(test_matches_search_cgroup_paths_non_matching),
    cmocka_unit_test(test_matches_entrypoint_parent_process_comm_matching),
    cmocka_unit_test(test_matches_entrypoint_parent_process_comm_non_matching),
    cmocka_unit_test(test_is_cgroup_systemd_service_true),
    cmocka_unit_test(test_is_cgroup_systemd_service_false),
    cmocka_unit_test(test_k8s_is_kubepod_true),
    cmocka_unit_test(test_k8s_is_kubepod_false_unset),
    cmocka_unit_test(test_k8s_is_kubepod_false_unknown),
    cmocka_unit_test(test_cgroup_chart_type_root),
    cmocka_unit_test(test_cgroup_chart_type_root_slash),
    cmocka_unit_test(test_cgroup_chart_type_systemd_service),
    cmocka_unit_test(test_cgroup_chart_type_normal_cgroup),
};

int main(void) {
    return cmocka_run_group_tests(tests, NULL, NULL);
}