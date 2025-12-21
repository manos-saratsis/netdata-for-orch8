```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stream-receiver-internals.h"
#include "compression.h"

void test_stream_parse_compression_order_basic() {
    struct stream_receiver_config config = {0};
    
    // Test default order
    stream_parse_compression_order(&config, STREAM_COMPRESSION_ALGORITHMS_ORDER);
    
    // Check that algorithms are parsed in the right order
    assert(config.compression.priorities[0] == STREAM_CAP_ZSTD);
    assert(config.compression.priorities[1] == STREAM_CAP_LZ4);
    assert(config.compression.priorities[2] == STREAM_CAP_BROTLI);
    assert(config.compression.priorities[3] == STREAM_CAP_GZIP);
}

void test_stream_parse_compression_order_custom() {
    struct stream_receiver_config config = {0};
    
    // Test custom order
    stream_parse_compression_order(&config, "lz4 gzip zstd");
    
    // Check that algorithms are parsed in the specified order
    assert(config.compression.priorities[0] == STREAM_CAP_LZ4);
    assert(config.compression.priorities[1] == STREAM_CAP_GZIP);
    assert(config.compression.priorities[2] == STREAM_CAP_ZSTD);
}

void test_stream_parse_compression_order_duplicates() {
    struct stream_receiver_config config = {0};
    
    // Test order with duplicates
    stream_parse_compression_order(&config, "zstd lz4 zstd brotli lz4");
    
    // Check that duplicates are handled and only added once
    size_t unique_entries = 0;
    for (size_t i = 0; i < COMPRESSION_ALGORITHM_MAX; i++) {
        if (config.compression.priorities[i] != STREAM_CAP_NONE) {
            unique_entries++;
        }
    }
    assert(unique_entries == 4);
}

void test_stream_parse_compression_order_invalid() {
    struct stream_receiver_config config = {0};
    
    // Test order with invalid algorithm names
    stream_parse_compression_order(&config, "invalid compression algo");
    
    // Check that default order is applied
    assert(config.compression.priorities[0] != STREAM_CAP_NONE);
}

int main() {
    test_stream_parse_compression_order_basic();
    test_stream_parse_compression_order_custom();
    test_stream_parse_compression_order_duplicates();
    test_stream_parse_compression_order_invalid();

    printf("All stream compression order parsing tests passed successfully!\n");
    return 0;
}
```