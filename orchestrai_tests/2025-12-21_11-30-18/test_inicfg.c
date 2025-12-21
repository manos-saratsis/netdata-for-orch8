#include "test_inicfg.h"
#include <string.h>

// Helper function to set up a test config
struct config* setup_test_config(void) {
    struct config *cfg = malloc(sizeof(struct config));
    *cfg = APPCONFIG_INITIALIZER;
    return cfg;
}

// Helper function to tear down a test config
void teardown_test_config(struct config* cfg) {
    inicfg_free(cfg);
    free(cfg);
}

TEST(test_inicfg_load) {
    struct config *cfg = setup_test_config();

    // Test successful load
    const char *test_file = "/tmp/test_config.conf";
    FILE *fp = fopen(test_file, "w");
    fprintf(fp, "[global]\ntest_option = value\n");
    fclose(fp);

    int result = inicfg_load(cfg, (char*)test_file, 0, NULL);
    assert_int_equal(result, 0);

    // Test invalid file
    result = inicfg_load(cfg, "/path/to/nonexistent/file", 0, NULL);
    assert_int_not_equal(result, 0);

    // Clean up
    unlink(test_file);
    teardown_test_config(cfg);
}

TEST(test_inicfg_foreach_value_in_section) {
    struct config *cfg = setup_test_config();
    
    // Callback function for testing
    bool test_callback(void *data, const char *name, const char *value) {
        int *count = (int*)data;
        (*count)++;
        return true;
    }
    
    // Add some test data
    inicfg_set(cfg, "test_section", "key1", "value1");
    inicfg_set(cfg, "test_section", "key2", "value2");
    
    int count = 0;
    size_t result = inicfg_foreach_value_in_section(cfg, "test_section", test_callback, &count);
    
    assert_int_equal(result, 2);
    assert_int_equal(count, 2);
    
    teardown_test_config(cfg);
}

// Additional test functions for the remaining prototypes...

TEST(test_inicfg_test_boolean_value) {
    assert_true(inicfg_test_boolean_value("true"));
    assert_true(inicfg_test_boolean_value("yes"));
    assert_true(inicfg_test_boolean_value("on"));
    
    assert_false(inicfg_test_boolean_value("false"));
    assert_false(inicfg_test_boolean_value("no"));
    assert_false(inicfg_test_boolean_value("off"));
    
    assert_false(inicfg_test_boolean_value(NULL));
    assert_false(inicfg_test_boolean_value("invalid"));
}

// More comprehensive test cases can be added for each function
// This is a sample/template approach

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_inicfg_load),
        cmocka_unit_test(test_inicfg_foreach_value_in_section),
        cmocka_unit_test(test_inicfg_test_boolean_value),
        // Add more tests here
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}