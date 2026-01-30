/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Test suite for aclk_proxy.h - header file verification
 * This header file contains only function declarations tested in test_aclk_proxy.c
 */

#include <stdio.h>
#include <assert.h>

/* Verify that all expected types are defined */
void test_aclk_proxy_types_defined(void) {
    /* Verify ACLK_PROXY_TYPE enum exists */
    printf("Testing ACLK_PROXY_TYPE enum definition...\n");
    
    /* Expected enum values:
     * - PROXY_TYPE_UNKNOWN = 0
     * - PROXY_TYPE_SOCKS5
     * - PROXY_TYPE_HTTP
     * - PROXY_DISABLED
     * - PROXY_NOT_SET
     */
    assert(PROXY_TYPE_UNKNOWN == 0);
    assert(PROXY_TYPE_SOCKS5 > PROXY_TYPE_UNKNOWN);
    assert(PROXY_TYPE_HTTP > PROXY_TYPE_SOCKS5);
    assert(PROXY_DISABLED > PROXY_TYPE_HTTP);
    assert(PROXY_NOT_SET > PROXY_DISABLED);
    
    printf("ACLK_PROXY_TYPE enum definition: PASS\n");
}

/* Verify that all expected functions are declared */
void test_aclk_proxy_functions_declared(void) {
    printf("Testing aclk_proxy.h function declarations...\n");
    
    /* Verify function pointers exist */
    assert(aclk_verify_proxy != NULL);
    assert(aclk_lws_wss_get_proxy_setting != NULL);
    assert(safe_log_proxy_censor != NULL);
    assert(aclk_get_proxy != NULL);
    
    printf("aclk_proxy.h function declarations: PASS\n");
}

/* Verify ACLK_PROXY_PROTO_ADDR_SEPARATOR macro */
void test_aclk_proxy_macro_separator(void) {
    printf("Testing ACLK_PROXY_PROTO_ADDR_SEPARATOR macro...\n");
    
    const char *sep = ACLK_PROXY_PROTO_ADDR_SEPARATOR;
    assert(sep != NULL);
    assert(strcmp(sep, "://") == 0);
    
    printf("ACLK_PROXY_PROTO_ADDR_SEPARATOR macro: PASS\n");
}

int main(void) {
    printf("Running aclk_proxy.h header verification tests...\n\n");
    
    test_aclk_proxy_types_defined();
    test_aclk_proxy_functions_declared();
    test_aclk_proxy_macro_separator();
    
    printf("\nAll aclk_proxy.h header tests passed!\n");
    return 0;
}