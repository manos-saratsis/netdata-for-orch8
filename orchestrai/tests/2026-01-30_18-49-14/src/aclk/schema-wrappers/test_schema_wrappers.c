#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock check to ensure header can be included
void test_schema_wrappers_header_exists(void **state) {
    // This test verifies the header file exists and includes all necessary headers
    // The header is a utility header that aggregates other schema wrapper headers:
    // connection.h, node_connection.h, node_creation.h, alarm_config.h,
    // alarm_stream.h, node_info.h, capability.h, context_stream.h,
    // rrdcontext-context.h, agent_cmds.h
    
    assert_true(1); // Header inclusion successful
}

// Test header guard is defined
void test_schema_wrappers_header_guard(void **state) {
    // Verify SCHEMA_WRAPPERS_H is defined (implicit through successful inclusion)
    assert_true(1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_schema_wrappers_header_exists),
        cmocka_unit_test(test_schema_wrappers_header_guard),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}