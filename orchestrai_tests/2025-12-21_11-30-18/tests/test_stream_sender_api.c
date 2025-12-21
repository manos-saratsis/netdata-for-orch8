```c
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include "src/streaming/stream-sender-internals.h"

// Mock structures and functions for testing
static struct rrdhost mock_host;
static struct rrdhost* mock_null_host = NULL;

static void test_stream_sender_has_capabilities(void **state) {
    (void) state; // unused

    // Setup
    mock_host.sender = calloc(1, sizeof(struct sender));
    mock_host.sender->capabilities = STREAM_CAP_COMPRESSION;

    // Test with valid host and capability
    assert_true(stream_sender_has_capabilities(&mock_host, STREAM_CAP_COMPRESSION));

    // Test with null host
    assert_false(stream_sender_has_capabilities(mock_null_host, STREAM_CAP_COMPRESSION));

    // Test with host but no capability
    assert_false(stream_sender_has_capabilities(&mock_host, STREAM_CAP_REPLICATION));

    free(mock_host.sender);
}

static void test_stream_sender_is_connected_with_ssl(void **state) {
    (void) state; // unused

    // Setup
    mock_host.sender = calloc(1, sizeof(struct sender));
    mock_host.flags = RRDHOST_FLAG_STREAM_SENDER_ENABLED;

    // Test with null host
    assert_false(stream_sender_is_connected_with_ssl(mock_null_host));

    // Test with valid SSL connection
    mock_host.flags |= RRDHOST_FLAG_STREAM_RECEIVER_REACHABLE;
    mock_host.sender->sock.flags = ND_SOCK_FLAGS_SSL_CONNECTED;
    assert_true(stream_sender_is_connected_with_ssl(&mock_host));

    // Test without SSL
    mock_host.sender->sock.flags = 0;
    assert_false(stream_sender_is_connected_with_ssl(&mock_host));

    free(mock_host.sender);
}

static void test_stream_sender_has_compression(void **state) {
    (void) state; // unused

    // Setup
    mock_host.sender = calloc(1, sizeof(struct sender));

    // Test with null host
    assert_false(stream_sender_has_compression(mock_null_host));

    // Test with uninitialized compressor
    assert_false(stream_sender_has_compression(&mock_host));

    // Test with initialized compressor
    mock_host.sender->thread.compressor.initialized = true;
    assert_true(stream_sender_has_compression(&mock_host));

    free(mock_host.sender);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_stream_sender_has_capabilities),
        cmocka_unit_test(test_stream_sender_is_connected_with_ssl),
        cmocka_unit_test(test_stream_sender_has_compression),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
```

I'll continue generating test files for the other source files using the same approach. Would you like me to proceed with the next file?