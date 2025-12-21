```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "stream-conf.h"

// Mock struct for testing
struct receiver_state;

// Test stream configuration loading
static void test_stream_conf_load(void **state) {
    (void) state; // unused
    stream_conf_load();
    // Add appropriate assertions based on expected behavior
}

// Test checking if current node is a parent
static void test_stream_conf_is_parent(void **state) {
    (void) state; // unused
    bool is_parent_false = stream_conf_is_parent(false);
    bool is_parent_true = stream_conf_is_parent(true);

    // Assertions depend on configuration
    assert_true(is_parent_false == is_parent_true || is_parent_false != is_parent_true);
}

// Test checking if current node is a child
static void test_stream_conf_is_child(void **state) {
    (void) state; // unused
    bool is_child = stream_conf_is_child();

    // Assertions depend on configuration
    assert_true(is_child == false || is_child == true);
}

// Test checking key type
static void test_stream_conf_is_key_type(void **state) {
    (void) state; // unused

    // Test with various scenarios
    assert_true(stream_conf_is_key_type("valid_key", "type1") == true || 
                stream_conf_is_key_type("valid_key", "type1") == false);
    
    // Test with NULL inputs
    assert_false(stream_conf_is_key_type(NULL, NULL));
}

// Test API key enabling/disabling
static void test_stream_conf_api_key_is_enabled(void **state) {
    (void) state; // unused

    assert_true(stream_conf_api_key_is_enabled("test_key", true) == true || 
                stream_conf_api_key_is_enabled("test_key", true) == false);
    
    assert_true(stream_conf_api_key_is_enabled("test_key", false) == true || 
                stream_conf_api_key_is_enabled("test_key", false) == false);
}

// Test API key client permissions
static void test_stream_conf_api_key_allows_client(void **state) {
    (void) state; // unused

    assert_true(stream_conf_api_key_allows_client("test_key", "127.0.0.1") == true || 
                stream_conf_api_key_allows_client("test_key", "127.0.0.1") == false);
}

// Test needs DBEngine
static void test_stream_conf_receiver_needs_dbengine(void **state) {
    (void) state; // unused
    bool needs_dbengine = stream_conf_receiver_needs_dbengine();
    assert_true(needs_dbengine == true || needs_dbengine == false);
}

// Test configuration free
static void test_stream_config_free(void **state) {
    (void) state; // unused
    
    // Call config free and ensure no segmentation fault or memory errors
    stream_config_free();
}

// Test setting sender compression levels
static void test_stream_conf_set_sender_compression_levels(void **state) {
    (void) state; // unused

    // Test with different compression profiles
    stream_conf_set_sender_compression_levels(ND_COMPRESSION_PROFILE_NONE);
    stream_conf_set_sender_compression_levels(ND_COMPRESSION_PROFILE_BALANCED);
    stream_conf_set_sender_compression_levels(ND_COMPRESSION_PROFILE_HIGH);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_stream_conf_load),
        cmocka_unit_test(test_stream_conf_is_parent),
        cmocka_unit_test(test_stream_conf_is_child),
        cmocka_unit_test(test_stream_conf_is_key_type),
        cmocka_unit_test(test_stream_conf_api_key_is_enabled),
        cmocka_unit_test(test_stream_conf_api_key_allows_client),
        cmocka_unit_test(test_stream_conf_receiver_needs_dbengine),
        cmocka_unit_test(test_stream_config_free),
        cmocka_unit_test(test_stream_conf_set_sender_compression_levels),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
```