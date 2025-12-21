```c
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "src/web/api/v2/api_v2_calls.h"
#include "src/web/api/v2/api_v2_info.h"

// Mock implementations for dependencies
RRDHOST* mock_rrdhost_create() {
    return (RRDHOST*)mock_ptr_type(RRDHOST*);
}

struct web_client* mock_web_client_create() {
    return (struct web_client*)mock_ptr_type(struct web_client*);
}

// Test successful info API call
static void test_api_v2_info_success(void **state) {
    (void) state; // unused

    RRDHOST *host = mock_rrdhost_create();
    struct web_client *w = mock_web_client_create();
    char *url = "/info";

    will_return(api_v2_contexts_internal, 0);
    expect_value(api_v2_contexts_internal, host, host);
    expect_value(api_v2_contexts_internal, w, w);
    expect_string(api_v2_contexts_internal, url, url);
    expect_value(api_v2_contexts_internal, flags, CONTEXTS_V2_AGENTS | CONTEXTS_V2_AGENTS_INFO);
    expect_value(api_v2_contexts_internal, hash, 0);

    int result = api_v2_info(host, w, url);
    assert_int_equal(result, 0);
}

// Test with NULL inputs
static void test_api_v2_info_null_inputs(void **state) {
    (void) state; // unused

    will_return(api_v2_contexts_internal, -1);
    expect_value(api_v2_contexts_internal, host, NULL);
    expect_value(api_v2_contexts_internal, w, NULL);
    expect_string(api_v2_contexts_internal, url, NULL);
    expect_value(api_v2_contexts_internal, flags, CONTEXTS_V2_AGENTS | CONTEXTS_V2_AGENTS_INFO);
    expect_value(api_v2_contexts_internal, hash, 0);

    int result = api_v2_info(NULL, NULL, NULL);
    assert_int_equal(result, -1);
}

// Test suite setup
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_api_v2_info_success),
        cmocka_unit_test(test_api_v2_info_null_inputs),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
```