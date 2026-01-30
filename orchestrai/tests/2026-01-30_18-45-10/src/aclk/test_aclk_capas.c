#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "aclk_capas.c"

// Mock external functions
struct rrdhost *localhost = NULL;
int metric_correlations_version = 1;

int ml_capable(void) {
    return mock_type(int);
}

int ml_enabled(RRDHOST *host) {
    return mock_type(int);
}

int receiver_has_capability(RRDHOST *host, uint64_t cap) {
    return mock_type(int);
}

int dyncfg_available_for_rrdhost(RRDHOST *host) {
    return mock_type(int);
}

// Test: aclk_get_http_api_version should return HTTP_API_V2_VERSION
static void test_aclk_get_http_api_version_returns_constant(void **state) {
    size_t version = aclk_get_http_api_version();
    assert_int_equal(version, 7);
}

// Test: aclk_get_agent_capas should return valid capability structure
static void test_aclk_get_agent_capas_returns_valid_structure(void **state) {
    will_return(ml_capable, 1);
    will_return(ml_enabled, 1);
    
    const struct capability *caps = aclk_get_agent_capas();
    
    assert_non_null(caps);
    
    // Check json capability
    assert_string_equal(caps[0].name, "json");
    assert_int_equal(caps[0].version, 2);
    assert_int_equal(caps[0].enabled, 0);
    
    // Check proto capability
    assert_string_equal(caps[1].name, "proto");
    assert_int_equal(caps[1].version, 1);
    assert_int_equal(caps[1].enabled, 1);
    
    // Check ml capability - version and enabled should be set based on ml_capable() and ml_enabled()
    assert_string_equal(caps[2].name, "ml");
    assert_int_equal(caps[2].version, 1);
    assert_int_equal(caps[2].enabled, 1);
    
    // Check mc capability
    assert_string_equal(caps[3].name, "mc");
    assert_int_equal(caps[3].enabled, 1);
    
    // Check ctx capability
    assert_string_equal(caps[4].name, "ctx");
    assert_int_equal(caps[4].version, 1);
    assert_int_equal(caps[4].enabled, 1);
    
    // Check funcs capability
    assert_string_equal(caps[5].name, "funcs");
    assert_int_equal(caps[5].version, 1);
    assert_int_equal(caps[5].enabled, 1);
    
    // Check http_api_v2 capability
    assert_string_equal(caps[6].name, "http_api_v2");
    assert_int_equal(caps[6].version, 7);
    assert_int_equal(caps[6].enabled, 1);
    
    // Check health capability
    assert_string_equal(caps[7].name, "health");
    assert_int_equal(caps[7].version, 2);
    
    // Check req_cancel capability
    assert_string_equal(caps[8].name, "req_cancel");
    assert_int_equal(caps[8].version, 1);
    assert_int_equal(caps[8].enabled, 1);
    
    // Check dyncfg capability
    assert_string_equal(caps[9].name, "dyncfg");
    assert_int_equal(caps[9].version, 2);
    assert_int_equal(caps[9].enabled, 1);
    
    // Check NULL terminator
    assert_null(caps[10].name);
}

// Test: aclk_get_agent_capas with ml disabled
static void test_aclk_get_agent_capas_ml_disabled(void **state) {
    will_return(ml_capable, 0);
    will_return(ml_enabled, 0);
    
    const struct capability *caps = aclk_get_agent_capas();
    
    assert_non_null(caps);
    assert_string_equal(caps[2].name, "ml");
    assert_int_equal(caps[2].version, 0);
    assert_int_equal(caps[2].enabled, 0);
}

// Test: aclk_get_agent_capas with ml version check
static void test_aclk_get_agent_capas_ml_capable_enabled(void **state) {
    will_return(ml_capable, 1);
    will_return(ml_enabled, 0);
    
    const struct capability *caps = aclk_get_agent_capas();
    
    assert_non_null(caps);
    assert_string_equal(caps[2].name, "ml");
    assert_int_equal(caps[2].version, 1);
    assert_int_equal(caps[2].enabled, 0);
}

// Test: aclk_get_node_instance_capas with localhost
static void test_aclk_get_node_instance_capas_localhost(void **state) {
    struct rrdhost host;
    memset(&host, 0, sizeof(struct rrdhost));
    host.health.enabled = 1;
    
    localhost = &host;
    
    will_return(ml_capable, 1);
    will_return(ml_enabled, 1);
    
    struct capability *caps = aclk_get_node_instance_capas(&host);
    
    assert_non_null(caps);
    
    // Check proto capability
    assert_string_equal(caps[0].name, "proto");
    assert_int_equal(caps[0].version, 1);
    assert_int_equal(caps[0].enabled, 1);
    
    // Check ml capability
    assert_string_equal(caps[1].name, "ml");
    assert_int_equal(caps[1].enabled, 1);
    
    // Check mc capability
    assert_string_equal(caps[2].name, "mc");
    assert_int_equal(caps[2].enabled, 1);
    
    // Check ctx capability
    assert_string_equal(caps[3].name, "ctx");
    assert_int_equal(caps[3].enabled, 1);
    
    // Check funcs capability (localhost gets functions)
    assert_string_equal(caps[4].name, "funcs");
    assert_int_equal(caps[4].version, 1);
    assert_int_equal(caps[4].enabled, 1);
    
    // Check http_api_v2 capability
    assert_string_equal(caps[5].name, "http_api_v2");
    assert_int_equal(caps[5].version, 7);
    assert_int_equal(caps[5].enabled, 1);
    
    // Check health capability
    assert_string_equal(caps[6].name, "health");
    assert_int_equal(caps[6].enabled, 1);
    
    // Check req_cancel capability
    assert_string_equal(caps[7].name, "req_cancel");
    assert_int_equal(caps[7].enabled, 1);
    
    // Check dyncfg capability (localhost gets dyncfg)
    assert_string_equal(caps[8].name, "dyncfg");
    assert_int_equal(caps[8].enabled, 1);
    
    // Check NULL terminator
    assert_null(caps[9].name);
    
    freez(caps);
    localhost = NULL;
}

// Test: aclk_get_node_instance_capas with remote host with functions capability
static void test_aclk_get_node_instance_capas_remote_with_functions(void **state) {
    struct rrdhost host;
    struct rrdhost localhost_host;
    memset(&host, 0, sizeof(struct rrdhost));
    memset(&localhost_host, 0, sizeof(struct rrdhost));
    
    host.health.enabled = 0;
    localhost_host.health.enabled = 1;
    localhost = &localhost_host;
    
    will_return(ml_capable, 0);
    will_return(ml_enabled, 0);
    will_return(receiver_has_capability, 1); // has STREAM_CAP_FUNCTIONS
    will_return(dyncfg_available_for_rrdhost, 1);
    
    struct capability *caps = aclk_get_node_instance_capas(&host);
    
    assert_non_null(caps);
    
    // Check funcs capability (remote with capability)
    assert_string_equal(caps[4].name, "funcs");
    assert_int_equal(caps[4].version, 1);
    assert_int_equal(caps[4].enabled, 1);
    
    freez(caps);
    localhost = NULL;
}

// Test: aclk_get_node_instance_capas with remote host without functions capability
static void test_aclk_get_node_instance_capas_remote_without_functions(void **state) {
    struct rrdhost host;
    struct rrdhost localhost_host;
    memset(&host, 0, sizeof(struct rrdhost));
    memset(&localhost_host, 0, sizeof(struct rrdhost));
    
    host.health.enabled = 0;
    localhost_host.health.enabled = 1;
    localhost = &localhost_host;
    
    will_return(ml_capable, 0);
    will_return(ml_enabled, 0);
    will_return(receiver_has_capability, 0); // no functions capability
    will_return(dyncfg_available_for_rrdhost, 0);
    
    struct capability *caps = aclk_get_node_instance_capas(&host);
    
    assert_non_null(caps);
    
    // Check funcs capability (remote without capability)
    assert_string_equal(caps[4].name, "funcs");
    assert_int_equal(caps[4].version, 0);
    assert_int_equal(caps[4].enabled, 0);
    
    // Check dyncfg capability (not available)
    assert_string_equal(caps[8].name, "dyncfg");
    assert_int_equal(caps[8].enabled, 0);
    
    freez(caps);
    localhost = NULL;
}

// Test: aclk_get_node_instance_capas with health disabled
static void test_aclk_get_node_instance_capas_health_disabled(void **state) {
    struct rrdhost host;
    memset(&host, 0, sizeof(struct rrdhost));
    host.health.enabled = 0;
    
    localhost = &host;
    
    will_return(ml_capable, 0);
    will_return(ml_enabled, 0);
    will_return(receiver_has_capability, 0);
    will_return(dyncfg_available_for_rrdhost, 0);
    
    struct capability *caps = aclk_get_node_instance_capas(&host);
    
    assert_non_null(caps);
    
    // Check health capability (disabled)
    assert_string_equal(caps[6].name, "health");
    assert_int_equal(caps[6].enabled, 0);
    
    freez(caps);
    localhost = NULL;
}

// Test: aclk_get_node_instance_capas memory allocation
static void test_aclk_get_node_instance_capas_memory_allocation(void **state) {
    struct rrdhost host;
    memset(&host, 0, sizeof(struct rrdhost));
    host.health.enabled = 1;
    
    localhost = &host;
    
    will_return(ml_capable, 0);
    will_return(ml_enabled, 0);
    will_return(receiver_has_capability, 0);
    will_return(dyncfg_available_for_rrdhost, 0);
    
    struct capability *caps = aclk_get_node_instance_capas(&host);
    
    assert_non_null(caps);
    
    // Verify it's a newly allocated structure (not the same pointer as ni_caps)
    static struct capability ni_caps[] = {
        { .name = "proto", .version = 1, .enabled = 1 },
    };
    assert_ptr_not_equal(caps, ni_caps);
    
    freez(caps);
    localhost = NULL;
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_aclk_get_http_api_version_returns_constant),
        cmocka_unit_test(test_aclk_get_agent_capas_returns_valid_structure),
        cmocka_unit_test(test_aclk_get_agent_capas_ml_disabled),
        cmocka_unit_test(test_aclk_get_agent_capas_ml_capable_enabled),
        cmocka_unit_test(test_aclk_get_node_instance_capas_localhost),
        cmocka_unit_test(test_aclk_get_node_instance_capas_remote_with_functions),
        cmocka_unit_test(test_aclk_get_node_instance_capas_remote_without_functions),
        cmocka_unit_test(test_aclk_get_node_instance_capas_health_disabled),
        cmocka_unit_test(test_aclk_get_node_instance_capas_memory_allocation),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}