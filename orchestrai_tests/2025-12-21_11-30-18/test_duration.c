```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "duration.h"
#include "libnetdata/libnetdata.h"

// Helper macro for test assertions
#define ASSERT(condition, message, ...) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "ASSERTION FAILED: " message "\n", ##__VA_ARGS__); \
            return false; \
        } \
    } while(0)

// Test function for duration_find_unit
bool test_duration_find_unit(void) {
    const struct duration_unit *du;

    // Test valid units with different cases
    du = duration_find_unit("s");
    ASSERT(du != NULL && strcmp(du->unit, "s") == 0, "Failed to find 's' unit");

    du = duration_find_unit("SECONDS");
    ASSERT(du != NULL && strcmp(du->unit, "seconds") == 0, "Failed to find 'SECONDS' unit case-insensitively");

    du = duration_find_unit("days");
    ASSERT(du != NULL && strcmp(du->unit, "days") == 0, "Failed to find 'days' unit");

    // Test default behavior when no unit provided
    du = duration_find_unit(NULL);
    ASSERT(du != NULL && strcmp(du->unit, "ns") == 0, "Failed to use default 'ns' unit");

    // Test unknown unit
    du = duration_find_unit("xyz");
    ASSERT(du == NULL, "Should return NULL for unknown unit");

    return true;
}

// Test function for duration_round_to_resolution
bool test_duration_round_to_resolution(void) {
    // Positive rounding
    ASSERT(duration_round_to_resolution(10, 3) == 3, "Failed to round up 10 with resolution 3");
    ASSERT(duration_round_to_resolution(11, 3) == 4, "Failed to round up 11 with resolution 3");
    ASSERT(duration_round_to_resolution(9, 3) == 3, "Failed to round down 9 with resolution 3");

    // Negative rounding
    ASSERT(duration_round_to_resolution(-10, 3) == -3, "Failed to round up -10 with resolution 3");
    ASSERT(duration_round_to_resolution(-11, 3) == -4, "Failed to round down -11 with resolution 3");
    ASSERT(duration_round_to_resolution(-9, 3) == -3, "Failed to round down -9 with resolution 3");

    // Zero handling
    ASSERT(duration_round_to_resolution(0, 3) == 0, "Failed to handle zero with resolution 3");

    return true;
}

// Comprehensive test for duration_parse
bool test_duration_parse(void) {
    int64_t result;
    bool success;

    // Basic positive parsing
    success = duration_parse("5m", &result, "s", "s");
    ASSERT(success && result == 300, "Failed to parse '5m' to seconds");

    // Parsing with full unit names
    success = duration_parse("2 hours", &result, "s", "s");
    ASSERT(success && result == 7200, "Failed to parse '2 hours' to seconds");

    // Parsing with case variations
    success = duration_parse("30 SECONDS", &result, "s", "s");
    ASSERT(success && result == 30, "Failed to parse case-varied '30 SECONDS'");

    // Negative duration
    success = duration_parse("-5 minutes", &result, "s", "s");
    ASSERT(success && result == -300, "Failed to parse negative duration");

    // Special keywords
    success = duration_parse("never", &result, "s", "s");
    ASSERT(success && result == 0, "Failed to parse 'never' keyword");

    // Decimal values
    success = duration_parse("1.5 days", &result, "h", "h");
    ASSERT(success && result == 36, "Failed to parse decimal duration");

    // "Ago" suffix
    success = duration_parse("7 days ago", &result, "s", "s");
    ASSERT(success && result == -604800, "Failed to parse 'ago' suffix");

    // Complex duration
    success = duration_parse("2 hours 30 minutes", &result, "s", "s");
    ASSERT(success && result == 9000, "Failed to parse complex duration");

    // Error cases
    success = duration_parse("invalid", &result, "s", "s");
    ASSERT(!success, "Should fail for invalid duration");

    success = duration_parse("", &result, "s", "s");
    ASSERT(!success, "Should fail for empty string");

    return true;
}

// Test for duration_parse_seconds (compatibility function)
bool test_duration_parse_seconds(void) {
    int result;

    // Valid parsing
    ASSERT(duration_parse_seconds("60", &result) && result == 60, "Failed to parse plain '60'");
    ASSERT(duration_parse_seconds("3600", &result) && result == 3600, "Failed to parse plain '3600'");
    
    // Invalid cases
    ASSERT(!duration_parse_seconds("invalid", &result), "Should fail for non-numeric input");
    ASSERT(!duration_parse_seconds("", &result), "Should fail for empty string");

    return true;
}

// Comprehensive test for duration_snprintf
bool test_duration_snprintf(void) {
    char buffer[256];
    ssize_t len;

    // Basic positive formatting
    len = duration_snprintf(buffer, sizeof(buffer), 300, "s", false);
    ASSERT(len > 0 && strcmp(buffer, "5m") == 0, "Failed to format 300 seconds to '5m'");

    // Negative formatting
    len = duration_snprintf(buffer, sizeof(buffer), -300, "s", false);
    ASSERT(len > 0 && strcmp(buffer, "-5m") == 0, "Failed to format -300 seconds to '-5m'");

    // Zero value
    len = duration_snprintf(buffer, sizeof(buffer), 0, "s", false);
    ASSERT(len > 0 && strcmp(buffer, "off") == 0, "Failed to format 0 to 'off'");

    // Complex duration
    len = duration_snprintf(buffer, sizeof(buffer), 9000, "s", false);
    ASSERT(len > 0 && strcmp(buffer, "2h30m") == 0, "Failed to format 9000 seconds to '2h30m'");

    // Buffer size limit tests
    len = duration_snprintf(buffer, 2, 300, "s", false);
    ASSERT(len == -2, "Should return -2 when buffer is too small");

    // Null buffer
    len = duration_snprintf(NULL, 0, 300, "s", false);
    ASSERT(len == -1, "Should return -1 for null buffer");

    return true;
}

int main(void) {
    int failed_tests = 0;

    printf("Running Duration Parser Unit Tests\n");
    printf("==================================\n");

    #define RUN_TEST(test_func) \
        do { \
            printf("Running %s... ", #test_func); \
            if (test_func()) { \
                printf("PASS\n"); \
            } else { \
                printf("FAIL\n"); \
                failed_tests++; \
            } \
        } while(0)

    RUN_TEST(test_duration_find_unit);
    RUN_TEST(test_duration_round_to_resolution);
    RUN_TEST(test_duration_parse);
    RUN_TEST(test_duration_parse_seconds);
    RUN_TEST(test_duration_snprintf);

    printf("\nTest Summary\n");
    printf("============\n");
    printf("Total Failed Tests: %d\n", failed_tests);

    return failed_tests;
}
```

I have created a comprehensive test suite for the duration parsing and formatting functions, covering:
1. Corner cases for unit finding
2. Rounding logic for different value types
3. Parsing variations (full names, case sensitivity, keywords)
4. Formatting for various duration scenarios
5. Error handling and edge cases

The test suite:
- Covers every exported function
- Tests both success and failure paths
- Handles various input types (positive, negative, decimal, special keywords)
- Checks error handling for invalid inputs
- Verifies formatting for different durations

Key testing strategies:
- Macro-based assertion for clear test reporting
- Systematic testing of functions with multiple scenarios
- Emphasis on edge cases and potential failure modes
- Reporting of failed tests with count

The test can be compiled and run to validate the duration parsing implementation.