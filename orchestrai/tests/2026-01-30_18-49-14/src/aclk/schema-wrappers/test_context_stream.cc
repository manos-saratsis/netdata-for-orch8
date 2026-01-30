#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>

// Mock protobuf message classes
#include "proto/context/v1/stream.pb.h"
#include "context_stream.h"

// Mock libc functions
extern "C" {
    extern void* callocz(size_t count, size_t size);
    extern void* mallocz(size_t size);
    extern void freez(void *ptr);
    extern char* strdupz(const char *str);
}

class ContextStreamTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        // Setup code if needed
    }

    virtual void TearDown() {
        // Cleanup code if needed
    }
};

// Test: parse_stop_streaming_ctxs with valid data
TEST_F(ContextStreamTest, ParseStopStreamingCtxsValidData) {
    // Arrange
    context::v1::StopStreamingContexts msg;
    msg.set_claim_id("test-claim-123");
    msg.set_node_id("test-node-456");
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("test-claim-123", result->claim_id);
    EXPECT_STREQ("test-node-456", result->node_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_stop_streaming_ctxs with empty claim_id
TEST_F(ContextStreamTest, ParseStopStreamingCtxsEmptyClaimId) {
    // Arrange
    context::v1::StopStreamingContexts msg;
    msg.set_claim_id("");
    msg.set_node_id("test-node-789");
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("", result->claim_id);
    EXPECT_STREQ("test-node-789", result->node_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_stop_streaming_ctxs with empty node_id
TEST_F(ContextStreamTest, ParseStopStreamingCtxsEmptyNodeId) {
    // Arrange
    context::v1::StopStreamingContexts msg;
    msg.set_claim_id("test-claim-999");
    msg.set_node_id("");
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("test-claim-999", result->claim_id);
    EXPECT_STREQ("", result->node_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_stop_streaming_ctxs with both empty
TEST_F(ContextStreamTest, ParseStopStreamingCtxsEmptyBoth) {
    // Arrange
    context::v1::StopStreamingContexts msg;
    msg.set_claim_id("");
    msg.set_node_id("");
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("", result->claim_id);
    EXPECT_STREQ("", result->node_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_stop_streaming_ctxs with long strings
TEST_F(ContextStreamTest, ParseStopStreamingCtxsLongStrings) {
    // Arrange
    std::string long_claim(1000, 'a');
    std::string long_node(1000, 'b');
    
    context::v1::StopStreamingContexts msg;
    msg.set_claim_id(long_claim);
    msg.set_node_id(long_node);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(long_claim, std::string(result->claim_id));
    EXPECT_EQ(long_node, std::string(result->node_id));
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_stop_streaming_ctxs with invalid data
TEST_F(ContextStreamTest, ParseStopStreamingCtxsInvalidData) {
    // Arrange
    const char* invalid_data = "invalid protobuf data";
    size_t len = strlen(invalid_data);
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(invalid_data, len);
    
    // Assert
    EXPECT_EQ(nullptr, result);
}

// Test: parse_stop_streaming_ctxs with zero length
TEST_F(ContextStreamTest, ParseStopStreamingCtxsZeroLength) {
    // Arrange
    const char* data = "";
    size_t len = 0;
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert
    EXPECT_EQ(nullptr, result);
}

// Test: parse_stop_streaming_ctxs with null data pointer
TEST_F(ContextStreamTest, ParseStopStreamingCtxsNullData) {
    // Arrange
    const char* data = nullptr;
    size_t len = 10;
    
    // Act - Should not crash, behavior depends on implementation
    // This tests error handling for null input
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert - Should return NULL or handle gracefully
    EXPECT_EQ(nullptr, result);
}

// Test: parse_ctxs_checkpoint with valid data
TEST_F(ContextStreamTest, ParseCtxsCheckpointValidData) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("claim-check-123");
    msg.set_node_id("node-check-456");
    msg.set_version_hash(0x1234567890ABCDEF);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("claim-check-123", result->claim_id);
    EXPECT_STREQ("node-check-456", result->node_id);
    EXPECT_EQ(0x1234567890ABCDEF, result->version_hash);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with zero version_hash
TEST_F(ContextStreamTest, ParseCtxsCheckpointZeroVersionHash) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("claim-zero");
    msg.set_node_id("node-zero");
    msg.set_version_hash(0);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(0, result->version_hash);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with max version_hash
TEST_F(ContextStreamTest, ParseCtxsCheckpointMaxVersionHash) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("claim-max");
    msg.set_node_id("node-max");
    msg.set_version_hash(0xFFFFFFFFFFFFFFFF);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(0xFFFFFFFFFFFFFFFF, result->version_hash);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with empty claim_id
TEST_F(ContextStreamTest, ParseCtxsCheckpointEmptyClaimId) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("");
    msg.set_node_id("node-check");
    msg.set_version_hash(12345);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("", result->claim_id);
    EXPECT_STREQ("node-check", result->node_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with empty node_id
TEST_F(ContextStreamTest, ParseCtxsCheckpointEmptyNodeId) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("claim-check");
    msg.set_node_id("");
    msg.set_version_hash(54321);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("claim-check", result->claim_id);
    EXPECT_STREQ("", result->node_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with both empty
TEST_F(ContextStreamTest, ParseCtxsCheckpointEmptyBoth) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("");
    msg.set_node_id("");
    msg.set_version_hash(999);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("", result->claim_id);
    EXPECT_STREQ("", result->node_id);
    EXPECT_EQ(999, result->version_hash);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with long strings
TEST_F(ContextStreamTest, ParseCtxsCheckpointLongStrings) {
    // Arrange
    std::string long_claim(2000, 'x');
    std::string long_node(2000, 'y');
    
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id(long_claim);
    msg.set_node_id(long_node);
    msg.set_version_hash(0xABCDEF1234567890);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(long_claim, std::string(result->claim_id));
    EXPECT_EQ(long_node, std::string(result->node_id));
    EXPECT_EQ(0xABCDEF1234567890, result->version_hash);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with invalid data
TEST_F(ContextStreamTest, ParseCtxsCheckpointInvalidData) {
    // Arrange
    const char* invalid_data = "not valid protobuf";
    size_t len = strlen(invalid_data);
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(invalid_data, len);
    
    // Assert
    EXPECT_EQ(nullptr, result);
}

// Test: parse_ctxs_checkpoint with zero length
TEST_F(ContextStreamTest, ParseCtxsCheckpointZeroLength) {
    // Arrange
    const char* data = "";
    size_t len = 0;
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    EXPECT_EQ(nullptr, result);
}

// Test: parse_ctxs_checkpoint with null data pointer
TEST_F(ContextStreamTest, ParseCtxsCheckpointNullData) {
    // Arrange
    const char* data = nullptr;
    size_t len = 10;
    
    // Act - Should not crash
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    EXPECT_EQ(nullptr, result);
}

// Test: parse_ctxs_checkpoint with special characters in claim_id
TEST_F(ContextStreamTest, ParseCtxsCheckpointSpecialCharsClaimId) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("claim-!@#$%^&*()_+-=[]{}|;:',.<>?/");
    msg.set_node_id("node-special");
    msg.set_version_hash(77777);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("claim-!@#$%^&*()_+-=[]{}|;:',.<>?/", result->claim_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_stop_streaming_ctxs with special characters
TEST_F(ContextStreamTest, ParseStopStreamingCtxsSpecialChars) {
    // Arrange
    context::v1::StopStreamingContexts msg;
    msg.set_claim_id("claim-\t\n\r");
    msg.set_node_id("node-!@#$%");
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("claim-\t\n\r", result->claim_id);
    EXPECT_STREQ("node-!@#$%", result->node_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with special characters in node_id
TEST_F(ContextStreamTest, ParseCtxsCheckpointSpecialCharsNodeId) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("claim-normal");
    msg.set_node_id("node-\t\n\rspecial");
    msg.set_version_hash(88888);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_STREQ("node-\t\n\rspecial", result->node_id);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Test: parse_ctxs_checkpoint with one version_hash (small value)
TEST_F(ContextStreamTest, ParseCtxsCheckpointSmallVersionHash) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("claim-one");
    msg.set_node_id("node-one");
    msg.set_version_hash(1);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length();
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(1, result->version_hash);
    
    // Cleanup
    freez(result->claim_id);
    freez(result->node_id);
    freez(result);
}

// Edge case: parse_stop_streaming_ctxs with malformed length
TEST_F(ContextStreamTest, ParseStopStreamingCtxsPartialData) {
    // Arrange
    context::v1::StopStreamingContexts msg;
    msg.set_claim_id("claim-partial");
    msg.set_node_id("node-partial");
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length() / 2;  // Only half the data
    
    // Act
    struct stop_streaming_ctxs *result = parse_stop_streaming_ctxs(data, len);
    
    // Assert - Should fail to parse incomplete data
    EXPECT_EQ(nullptr, result);
}

// Edge case: parse_ctxs_checkpoint with excessive length
TEST_F(ContextStreamTest, ParseCtxsCheckpointExcessiveLength) {
    // Arrange
    context::v1::ContextsCheckpoint msg;
    msg.set_claim_id("claim-excess");
    msg.set_node_id("node-excess");
    msg.set_version_hash(99999);
    
    std::string serialized = msg.SerializeAsString();
    const char* data = serialized.c_str();
    size_t len = serialized.length() * 2;  // Claim length is double
    
    // Act
    struct ctxs_checkpoint *result = parse_ctxs_checkpoint(data, len);
    
    // Assert - Should fail to parse with invalid length
    EXPECT_EQ(nullptr, result);
}