```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stream-receiver-internals.h"
#include "compression.h"

void test_stream_decompression_initialize_available_algorithms() {
    struct receiver_state rpt = {0};
    
    // Test different algorithm capabilities
    rpt.capabilities = STREAM_CAP_ZSTD | STREAM_CAP_LZ4 | STREAM_CAP_BROTLI | STREAM_CAP_GZIP;
    
    bool initialized = stream_decompression_initialize(&rpt);
    
    assert(initialized == true);
    assert(rpt.thread.compressed.decompressor.algorithm == COMPRESSION_ALGORITHM_ZSTD);
}

void test_stream_decompression_initialize_no_algorithms() {
    struct receiver_state rpt = {0};
    
    // Test no available algorithms
    rpt.capabilities = STREAM_CAP_NONE;
    
    bool initialized = stream_decompression_initialize(&rpt);
    
    assert(initialized == false);
    assert(rpt.thread.compressed.decompressor.algorithm == COMPRESSION_ALGORITHM_NONE);
}

void test_stream_select_receiver_compression_algorithm() {
    struct receiver_state rpt = {0};
    
    // Test multiple compression capabilities
    rpt.capabilities = STREAM_CAP_ZSTD | STREAM_CAP_LZ4 | STREAM_CAP_BROTLI | STREAM_CAP_GZIP;
    rpt.config.compression.enabled = true;
    rpt.config.compression.priorities[0] = STREAM_CAP_LZ4;
    rpt.config.compression.priorities[1] = STREAM_CAP_ZSTD;
    
    stream_select_receiver_compression_algorithm(&rpt);
    
    // Verify that LZ4 is selected first
    assert((rpt.capabilities & STREAM_CAP_LZ4) != 0);
    assert((rpt.capabilities & ~STREAM_CAP_LZ4) == 0);
}

void test_stream_select_receiver_compression_algorithm_disabled() {
    struct receiver_state rpt = {0};
    
    // Test compression disabled
    rpt.capabilities = STREAM_CAP_ZSTD | STREAM_CAP_LZ4;
    rpt.config.compression.enabled = false;
    
    stream_select_receiver_compression_algorithm(&rpt);
    
    // Verify compression capabilities are removed
    assert((rpt.capabilities & STREAM_CAP_COMPRESSIONS_AVAILABLE) == 0);
}

int main() {
    test_stream_decompression_initialize_available_algorithms();
    test_stream_decompression_initialize_no_algorithms();
    test_stream_select_receiver_compression_algorithm();
    test_stream_select_receiver_compression_algorithm_disabled();

    printf("All stream receiver compression tests passed successfully!\n");
    return 0;
}
```