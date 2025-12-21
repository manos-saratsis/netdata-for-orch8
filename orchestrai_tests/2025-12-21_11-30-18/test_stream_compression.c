```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "compression.h"

// Utility function to create a test buffer
void create_test_buffer(char *buffer, size_t size, char fill_char) {
    memset(buffer, fill_char, size);
}

// Test stream compression signature encoding
void test_stream_compress_encode_signature() {
    // Test normal case
    stream_compression_signature_t sig1 = stream_compress_encode_signature(100);
    assert(sig1 != 0);

    // Test max size
    stream_compression_signature_t sig2 = stream_compress_encode_signature(COMPRESSION_MAX_CHUNK - 1);
    assert(sig2 != 0);

    // Test zero size
    stream_compression_signature_t sig3 = stream_compress_encode_signature(0);
    assert(sig3 != 0);
}

// Test signature decoding 
void test_stream_decompress_decode_signature() {
    // Test valid signature
    char valid_sig[STREAM_COMPRESSION_SIGNATURE_SIZE];
    stream_compression_signature_t sig = stream_compress_encode_signature(100);
    memcpy(valid_sig, &sig, STREAM_COMPRESSION_SIGNATURE_SIZE);
    
    size_t decoded_size = stream_decompress_decode_signature(valid_sig, STREAM_COMPRESSION_SIGNATURE_SIZE);
    assert(decoded_size == 100);

    // Test invalid signature
    char invalid_sig[STREAM_COMPRESSION_SIGNATURE_SIZE] = {0};
    size_t invalid_decoded = stream_decompress_decode_signature(invalid_sig, STREAM_COMPRESSION_SIGNATURE_SIZE);
    assert(invalid_decoded == 0);
}

// Test compression/decompression for each algorithm
void test_stream_compression_algorithms() {
    compression_algorithm_t algorithms[] = {
        COMPRESSION_ALGORITHM_ZSTD,
        COMPRESSION_ALGORITHM_LZ4,
        COMPRESSION_ALGORITHM_BROTLI,
        COMPRESSION_ALGORITHM_GZIP
    };

    for (size_t algo_idx = 0; algo_idx < sizeof(algorithms) / sizeof(algorithms[0]); algo_idx++) {
        struct compressor_state cctx = {
            .initialized = false,
            .algorithm = algorithms[algo_idx],
        };
        struct decompressor_state dctx = {
            .initialized = false,
            .algorithm = algorithms[algo_idx],
        };

        // Initialize compressor and decompressor
        stream_compressor_init(&cctx);
        stream_decompressor_init(&dctx);

        // Test data
        char test_data[1024];
        create_test_buffer(test_data, sizeof(test_data), 'A');

        // Compress data
        const char *compressed_data;
        size_t compressed_size = stream_compress(&cctx, test_data, sizeof(test_data), &compressed_data);
        assert(compressed_size > 0);
        assert(compressed_size < COMPRESSION_MAX_CHUNK);

        // Decompress data
        size_t decompressed_size = stream_decompress(&dctx, compressed_data, compressed_size);
        assert(decompressed_size == sizeof(test_data));

        // Verify decompressed data
        char *decompressed_data = (char *) &dctx.output.data[dctx.output.read_pos];
        assert(memcmp(test_data, decompressed_data, sizeof(test_data)) == 0);

        // Cleanup
        stream_compressor_destroy(&cctx);
        stream_decompressor_destroy(&dctx);
    }
}

// Test edge cases and error handling
void test_stream_compression_edge_cases() {
    struct compressor_state cctx = {
        .initialized = false,
        .algorithm = COMPRESSION_ALGORITHM_GZIP,
    };
    struct decompressor_state dctx = {
        .initialized = false,
        .algorithm = COMPRESSION_ALGORITHM_GZIP,
    };

    stream_compressor_init(&cctx);
    stream_decompressor_init(&dctx);

    // Test zero-length data
    const char *compressed_zero;
    size_t compressed_zero_size = stream_compress(&cctx, "", 0, &compressed_zero);
    assert(compressed_zero_size == 0);

    // Attempt to decompress invalid data
    char invalid_data[10] = {0};
    size_t invalid_decomp_size = stream_decompress(&dctx, invalid_data, sizeof(invalid_data));
    assert(invalid_decomp_size == 0);

    stream_compressor_destroy(&cctx);
    stream_decompressor_destroy(&dctx);
}

int main() {
    test_stream_compress_encode_signature();
    test_stream_decompress_decode_signature();
    test_stream_compression_algorithms();
    test_stream_compression_edge_cases();

    printf("All stream compression tests passed successfully!\n");
    return 0;
}
```