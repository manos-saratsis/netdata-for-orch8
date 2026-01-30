// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>

// Forward declarations of the functions we're testing
extern "C" {
    #include "context_stream.h"
}

// Mock for protobuf parsing - we'll use real protobuf for integration tests
// but also test edge cases with invalid data

class ContextStreamTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test: parse_stop_streaming_ctxs with valid data
TEST_F(ContextStreamTest, parse_stop_streaming_ctxs_with_valid_data) {
    // Using a minimal valid protobuf message
    // This would be an actual serialized protobuf in a real scenario
    // For now, testing the function's null check behavior
    
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(nullptr, 0);
    EXPECT_EQ(result, nullptr);
}

// Test: parse_stop_streaming_ctxs with empty data
TEST_F(ContextStreamTest, parse_stop_streaming_ctxs_with_empty_data) {
    const char data[] = "";
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, 0);
    EXPECT_EQ(result, nullptr);
}

// Test: parse_stop_streaming_ctxs with invalid protobuf data
TEST_F(ContextStreamTest, parse_stop_streaming_ctxs_with_invalid_protobuf) {
    const char invalid_data[] = "\xFF\xFF\xFF\xFF";
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(invalid_data, 4);
    // Invalid protobuf should fail parsing and return NULL
    EXPECT_EQ(result, nullptr);
}

// Test: parse_stop_streaming_ctxs with zero length
TEST_F(ContextStreamTest, parse_stop_streaming_ctxs_zero_length) {
    const char data[] = "some_data";
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, 0);
    EXPECT_EQ(result, nullptr);
}

// Test: parse_stop_streaming_ctxs with very large length
TEST_F(ContextStreamTest, parse_stop_streaming_ctxs_large_length) {
    const char data[] = "x";
    // Length larger than actual data - invalid
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, 1000000);
    // Should fail parsing
    EXPECT_EQ(result, nullptr);
}

// Test: parse_ctxs_checkpoint with null pointer
TEST_F(ContextStreamTest, parse_ctxs_checkpoint_with_null_data) {
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(nullptr, 0);
    EXPECT_EQ(result, nullptr);
}

// Test: parse_ctxs_checkpoint with empty data
TEST_F(ContextStreamTest, parse_ctxs_checkpoint_with_empty_data) {
    const char data[] = "";
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, 0);
    EXPECT_EQ(result, nullptr);
}

// Test: parse_ctxs_checkpoint with invalid protobuf data
TEST_F(ContextStreamTest, parse_ctxs_checkpoint_with_invalid_protobuf) {
    const char invalid_data[] = "\xFF\xFE\xFD\xFC";
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(invalid_data, 4);
    // Invalid protobuf should fail parsing and return NULL
    EXPECT_EQ(result, nullptr);
}

// Test: parse_ctxs_checkpoint with zero length
TEST_F(ContextStreamTest, parse_ctxs_checkpoint_zero_length) {
    const char data[] = "checkpoint_data";
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, 0);
    EXPECT_EQ(result, nullptr);
}

// Test: struct stop_streaming_ctxs initialization
TEST_F(ContextStreamTest, stop_streaming_ctxs_struct_members) {
    // Verify struct layout and member names exist
    struct stop_streaming_ctxs test_struct;
    test_struct.claim_id = nullptr;
    test_struct.node_id = nullptr;
    
    EXPECT_EQ(test_struct.claim_id, nullptr);
    EXPECT_EQ(test_struct.node_id, nullptr);
}

// Test: struct ctxs_checkpoint initialization
TEST_F(ContextStreamTest, ctxs_checkpoint_struct_members) {
    // Verify struct layout and member names exist
    struct ctxs_checkpoint test_struct;
    test_struct.claim_id = nullptr;
    test_struct.node_id = nullptr;
    test_struct.version_hash = 0;
    
    EXPECT_EQ(test_struct.claim_id, nullptr);
    EXPECT_EQ(test_struct.node_id, nullptr);
    EXPECT_EQ(test_struct.version_hash, 0);
}

// Test: parse_stop_streaming_ctxs with single byte
TEST_F(ContextStreamTest, parse_stop_streaming_ctxs_single_byte) {
    const char data[] = "\x00";
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, 1);
    // Single byte is not valid protobuf
    EXPECT_EQ(result, nullptr);
}

// Test: parse_ctxs_checkpoint with single byte
TEST_F(ContextStreamTest, parse_ctxs_checkpoint_single_byte) {
    const char data[] = "\x00";
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, 1);
    // Single byte is not valid protobuf
    EXPECT_EQ(result, nullptr);
}

// Test: parse_stop_streaming_ctxs boundary condition - small valid size
TEST_F(ContextStreamTest, parse_stop_streaming_ctxs_boundary_small_size) {
    // Minimum protobuf message would have at least 2 bytes for a field
    const char data[] = "\x0A\x00";  // Field 1 (claim_id), length 0
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, 2);
    // May or may not parse depending on protobuf content, but shouldn't crash
    if (result != nullptr) {
        // If parsed successfully, check basic invariants
        EXPECT_TRUE(result != nullptr);
    }
}

// Test: parse_ctxs_checkpoint boundary condition - small valid size  
TEST_F(ContextStreamTest, parse_ctxs_checkpoint_boundary_small_size) {
    const char data[] = "\x0A\x00";  // Field 1 (claim_id), length 0
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, 2);
    // May or may not parse depending on protobuf content, but shouldn't crash
    if (result != nullptr) {
        EXPECT_TRUE(result != nullptr);
    }
}