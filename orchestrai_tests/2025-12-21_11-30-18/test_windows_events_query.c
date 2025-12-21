```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "windows-events.h"

// Mock structures and functions to simulate query dependencies
typedef struct {
    LOGS_QUERY_SOURCE *sources;
    size_t source_count;
} MockQueryContext;

// Mock implementations
bool __wrap_wevt_sources_scan(void) {
    return true;
}

WEVT_LOG *__wrap_wevt_openlog6(int type) {
    return (WEVT_LOG *)1; // Dummy non-NULL pointer
}

void __wrap_wevt_sources_init(void) {
    // Do nothing
}

// Test helper to create a mock query context
static MockQueryContext* create_mock_query_context(void) {
    MockQueryContext *ctx = calloc(1, sizeof(MockQueryContext));
    ctx->sources = calloc(2, sizeof(LOGS_QUERY_SOURCE));
    ctx->source_count = 2;

    // Initialize first source
    ctx->sources[0].source_type = WEVTS_SYSTEM;
    ctx->sources[0].source = string_strdupz("System");
    ctx->sources[0].fullname = "System";
    ctx->sources[0].msg_first_ut = 1000000;
    ctx->sources[0].msg_last_ut = 2000000;
    ctx->sources[0].entries = 100;

    // Initialize second source
    ctx->sources[1].source_type = WEVTS_APPLICATION;
    ctx->sources[1].source = string_strdupz("Application");
    ctx->sources[1].fullname = "Application";
    ctx->sources[1].msg_first_ut = 1500000;
    ctx->sources[1].msg_last_ut = 2500000;
    ctx->sources[1].entries = 200;

    return ctx;
}

static void free_mock_query_context(MockQueryContext *ctx) {
    for (size_t i = 0; i < ctx->source_count; i++) {
        string_freez(ctx->sources[i].source);
    }
    free(ctx->sources);
    free(ctx);
}

// Test source_is_mine() function
static void test_source_is_mine_default(void **state) {
    (void) state; // unused

    MockQueryContext *ctx = create_mock_query_context();

    LOGS_QUERY_STATUS lqs = {
        .rq = {
            .source_type = WEVTS_NONE,
            .sources = NULL,
            .after_ut = 0,
            .before_ut = UINT64_MAX
        }
    };

    // Default settings should match all sources
    assert_true(source_is_mine(&ctx->sources[0], &lqs));
    assert_true(source_is_mine(&ctx->sources[1], &lqs));

    free_mock_query_context(ctx);
}

static void test_source_is_mine_type_filter(void **state) {
    (void) state; // unused

    MockQueryContext *ctx = create_mock_query_context();

    LOGS_QUERY_STATUS lqs = {
        .rq = {
            .source_type = WEVTS_SYSTEM,
            .sources = NULL,
            .after_ut = 0,
            .before_ut = UINT64_MAX
        }
    };

    // Only system source should match
    assert_true(source_is_mine(&ctx->sources[0], &lqs));
    assert_false(source_is_mine(&ctx->sources[1], &lqs));

    free_mock_query_context(ctx);
}

static void test_source_is_mine_timestamp_filter(void **state) {
    (void) state; // unused

    MockQueryContext *ctx = create_mock_query_context();

    LOGS_QUERY_STATUS lqs = {
        .rq = {
            .source_type = WEVTS_NONE,
            .sources = NULL,
            .after_ut = 1250000,  // Between first source's timestamps
            .before_ut = 1750000
        }
    };

    // Only first source should match
    assert_true(source_is_mine(&ctx->sources[0], &lqs));
    assert_false(source_is_mine(&ctx->sources[1], &lqs));

    free_mock_query_context(ctx);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_source_is_mine_default),
        cmocka_unit_test(test_source_is_mine_type_filter),
        cmocka_unit_test(test_source_is_mine_timestamp_filter),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
```