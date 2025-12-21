```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "websocket-compression.h"
#include "websocket-internal.h"
#include <zlib.h>

using ::testing::Return;
using ::testing::_;

class WebsocketCompressionTest : public ::testing::Test {
protected:
    WS_CLIENT mock_wsc;
    
    void SetUp() override {
        // Initialize mock WebSocket client with default compression settings
        memset(&mock_wsc, 0, sizeof(WS_CLIENT));
        mock_wsc.compression = WEBSOCKET_COMPRESSION_DEFAULTS;
        mock_wsc.compression.enabled = true;
        mock_wsc.compression.type = WS_COMPRESS_DEFLATE;
    }

    void TearDown() override {
        // Cleanup any resources
        if (mock_wsc.compression.deflate_stream) {
            freez(mock_wsc.compression.deflate_stream);
            mock_wsc.compression.deflate_stream = NULL;
        }
        if (mock_wsc.compression.inflate_stream) {
            freez(mock_wsc.compression.inflate_stream);
            mock_wsc.compression.inflate_stream = NULL;
        }
    }
};

// Test compression initialization
TEST_F(WebsocketCompressionTest, CompressionInitSuccess) {
    // Setup thread context simulation
    pthread_t fake_tid = pthread_self();
    mock_wsc.wth = (WS_THREAD *)malloc(sizeof(WS_THREAD));
    mock_wsc.wth->tid = fake_tid;

    EXPECT_TRUE(websocket_compression_init(&mock_wsc));
    EXPECT_NE(mock_wsc.compression.deflate_stream, nullptr);

    free(mock_wsc.wth);
}

// Test compression initialization when disabled
TEST_F(WebsocketCompressionTest, CompressionInitDisabled) {
    mock_wsc.compression.enabled = false;

    EXPECT_FALSE(websocket_compression_init(&mock_wsc));
}

// Test message compression for small payloads
TEST_F(WebsocketCompressionTest, CompressSmallMessage) {
    const char* small_data = "small";
    
    websocket_compression_init(&mock_wsc);
    mock_wsc.c_payload = wsb_create();

    EXPECT_FALSE(websocket_client_compress_message(&mock_wsc, small_data, strlen(small_data)));
}

// Test message compression for compressible message
TEST_F(WebsocketCompressionTest, CompressLargeMessage) {
    char large_data[1024];
    memset(large_data, 'A', sizeof(large_data));
    large_data[sizeof(large_data)-1] = '\0';

    websocket_compression_init(&mock_wsc);
    mock_wsc.c_payload = wsb_create();

    EXPECT_TRUE(websocket_client_compress_message(&mock_wsc, large_data, strlen(large_data)));
    EXPECT_GT(wsb_length(&mock_wsc.c_payload), 0);
}

// Test compression cleanup
TEST_F(WebsocketCompressionTest, CompressionCleanup) {
    websocket_compression_init(&mock_wsc);
    ASSERT_NE(mock_wsc.compression.deflate_stream, nullptr);

    websocket_compression_cleanup(&mock_wsc);
    EXPECT_EQ(mock_wsc.compression.deflate_stream, nullptr);
}

// Test decompression initialization
TEST_F(WebsocketCompressionTest, DecompressionInitSuccess) {
    pthread_t fake_tid = pthread_self();
    mock_wsc.wth = (WS_THREAD *)malloc(sizeof(WS_THREAD));
    mock_wsc.wth->tid = fake_tid;

    EXPECT_TRUE(websocket_decompression_init(&mock_wsc));
    EXPECT_NE(mock_wsc.compression.inflate_stream, nullptr);

    free(mock_wsc.wth);
}

// Test decompression initialization when disabled
TEST_F(WebsocketCompressionTest, DecompressionInitDisabled) {
    mock_wsc.compression.enabled = false;
    EXPECT_FALSE(websocket_decompression_init(&mock_wsc));
}

// Test decompression cleanup
TEST_F(WebsocketCompressionTest, DecompressionCleanup) {
    websocket_decompression_init(&mock_wsc);
    ASSERT_NE(mock_wsc.compression.inflate_stream, nullptr);

    websocket_decompression_cleanup(&mock_wsc);
    EXPECT_EQ(mock_wsc.compression.inflate_stream, nullptr);
}

// Test compression and decompression reset
TEST_F(WebsocketCompressionTest, CompressionReset) {
    websocket_compression_init(&mock_wsc);
    ASSERT_NE(mock_wsc.compression.deflate_stream, nullptr);

    EXPECT_TRUE(websocket_compression_reset(&mock_wsc));
    EXPECT_NE(mock_wsc.compression.deflate_stream, nullptr);
}

// Test decompression reset
TEST_F(WebsocketCompressionTest, DecompressionReset) {
    websocket_decompression_init(&mock_wsc);
    ASSERT_NE(mock_wsc.compression.inflate_stream, nullptr);

    EXPECT_TRUE(websocket_decompression_reset(&mock_wsc));
    EXPECT_NE(mock_wsc.compression.inflate_stream, nullptr);
}
```