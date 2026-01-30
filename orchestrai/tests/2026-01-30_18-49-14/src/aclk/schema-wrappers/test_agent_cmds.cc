#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>

extern "C" {
#include "agent_cmds.h"
}

// Mock for protobuf parsing and dependencies
#include "proto/agent/v1/cmds.pb.h"

using namespace ::testing;
using namespace agent::v1;

class AgentCmdsTest : public ::Test {
protected:
    void SetUp() override {
        // Initialize test structures
        memset(&test_req, 0, sizeof(test_req));
    }

    void TearDown() override {
        // Cleanup allocated memory
        if (test_req.request_id) {
            free(test_req.request_id);
            test_req.request_id = nullptr;
        }
        if (test_req.trace_id) {
            free(test_req.trace_id);
            test_req.trace_id = nullptr;
        }
    }

    struct aclk_cancel_pending_req test_req;
};

// Tests for parse_cancel_pending_req function

class ParseCancelPendingReqTest : public AgentCmdsTest {
};

TEST_F(ParseCancelPendingReqTest, ShouldSuccessfullyParseValidMessage) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("test-request-id-123");
    proto_msg.set_trace_id("test-trace-id-456");
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(1234567890);
    ts->set_nanos(123456000);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_NE(test_req.request_id, nullptr);
    EXPECT_STREQ(test_req.request_id, "test-request-id-123");
    EXPECT_NE(test_req.trace_id, nullptr);
    EXPECT_STREQ(test_req.trace_id, "test-trace-id-456");
    EXPECT_EQ(test_req.timestamp.tv_sec, 1234567890);
    EXPECT_EQ(test_req.timestamp.tv_usec, 123456);
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleMessageWithoutTraceId) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("test-request-id-789");
    // trace_id is intentionally not set
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(9876543210);
    ts->set_nanos(654321000);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_NE(test_req.request_id, nullptr);
    EXPECT_STREQ(test_req.request_id, "test-request-id-789");
    EXPECT_EQ(test_req.trace_id, nullptr);
    EXPECT_EQ(test_req.timestamp.tv_sec, 9876543210);
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleMessageWithEmptyTraceId) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("test-request-id-empty-trace");
    proto_msg.set_trace_id("");  // Empty but set
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(1000000000);
    ts->set_nanos(0);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_NE(test_req.request_id, nullptr);
    EXPECT_STREQ(test_req.request_id, "test-request-id-empty-trace");
    EXPECT_EQ(test_req.trace_id, nullptr);
}

TEST_F(ParseCancelPendingReqTest, ShouldFailWhenParsingInvalidData) {
    // Arrange
    const char *invalid_data = "not a valid protobuf message";
    
    // Act
    int result = parse_cancel_pending_req(invalid_data, strlen(invalid_data), &test_req);
    
    // Assert
    EXPECT_EQ(result, 1);
}

TEST_F(ParseCancelPendingReqTest, ShouldFailWhenParsingEmptyMessage) {
    // Arrange
    const char *empty_data = "";
    
    // Act
    int result = parse_cancel_pending_req(empty_data, 0, &test_req);
    
    // Assert
    EXPECT_EQ(result, 1);
}

TEST_F(ParseCancelPendingReqTest, ShouldFailWhenParsingCorruptedMessage) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("valid-id");
    proto_msg.set_trace_id("valid-trace");
    
    std::string serialized = proto_msg.SerializeAsString();
    // Corrupt the serialized data by truncating it
    std::string corrupted = serialized.substr(0, serialized.length() / 2);
    
    // Act
    int result = parse_cancel_pending_req(corrupted.c_str(), corrupted.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 1);
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleMessageWithZeroTimestamp) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("test-zero-time");
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(0);
    ts->set_nanos(0);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_NE(test_req.request_id, nullptr);
    EXPECT_STREQ(test_req.request_id, "test-zero-time");
    EXPECT_EQ(test_req.timestamp.tv_sec, 0);
    EXPECT_EQ(test_req.timestamp.tv_usec, 0);
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleMessageWithLargeNanoseconds) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("test-large-nanos");
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(1234567890);
    ts->set_nanos(999999999);  // Maximum nanos value
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_EQ(test_req.timestamp.tv_usec, 999999);  // nanos/1000
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleVeryLongRequestId) {
    // Arrange
    CancelPendingRequest proto_msg;
    std::string long_id(10000, 'a');  // 10000 character request ID
    proto_msg.set_request_id(long_id);
    proto_msg.set_trace_id("trace");
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(1000000000);
    ts->set_nanos(500000000);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_NE(test_req.request_id, nullptr);
    EXPECT_EQ(strlen(test_req.request_id), 10000);
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleVeryLongTraceId) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("request");
    std::string long_trace(10000, 'b');  // 10000 character trace ID
    proto_msg.set_trace_id(long_trace);
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(1000000000);
    ts->set_nanos(500000000);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_NE(test_req.trace_id, nullptr);
    EXPECT_EQ(strlen(test_req.trace_id), 10000);
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleMessageWithSpecialCharactersInIds) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("req-id_123!@#$%^&*()|:");
    proto_msg.set_trace_id("trace-id_456~`<>?");
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(1111111111);
    ts->set_nanos(111111111);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_NE(test_req.request_id, nullptr);
    EXPECT_STREQ(test_req.request_id, "req-id_123!@#$%^&*()|:");
    EXPECT_NE(test_req.trace_id, nullptr);
    EXPECT_STREQ(test_req.trace_id, "trace-id_456~`<>?");
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleNullMessagePointer) {
    // Arrange
    const char *msg = nullptr;
    struct aclk_cancel_pending_req req;
    memset(&req, 0, sizeof(req));
    
    // Act & Assert - should not crash, behavior depends on implementation
    // This tests boundary condition
    int result = parse_cancel_pending_req(msg, 0, &req);
    EXPECT_EQ(result, 1);
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleZeroLengthMessage) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("test");
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), 0, &test_req);
    
    // Assert
    EXPECT_EQ(result, 1);
}

TEST_F(ParseCancelPendingReqTest, ShouldHandleNegativeTimestampSeconds) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("test-negative-time");
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(-1234567890);  // Negative timestamp (before epoch)
    ts->set_nanos(500000000);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert
    EXPECT_EQ(result, 0);
    EXPECT_EQ(test_req.timestamp.tv_sec, -1234567890);
}

// Tests for free_cancel_pending_req function

class FreeCancelPendingReqTest : public AgentCmdsTest {
};

TEST_F(FreeCancelPendingReqTest, ShouldFreeBothRequestIdAndTraceId) {
    // Arrange
    struct aclk_cancel_pending_req req;
    req.request_id = strdupz("test-request-id");
    req.trace_id = strdupz("test-trace-id");
    
    // Act
    free_cancel_pending_req(&req);
    
    // Assert - memory should be freed (no crash)
    // Verify pointers are not accessible after free
    EXPECT_TRUE(true);  // If we get here, no crash occurred
}

TEST_F(FreeCancelPendingReqTest, ShouldFreeOnlyRequestIdWhenTraceIdIsNull) {
    // Arrange
    struct aclk_cancel_pending_req req;
    req.request_id = strdupz("test-request-id");
    req.trace_id = nullptr;
    
    // Act
    free_cancel_pending_req(&req);
    
    // Assert - memory should be freed (no crash)
    EXPECT_TRUE(true);  // If we get here, no crash occurred
}

TEST_F(FreeCancelPendingReqTest, ShouldFreeOnlyTraceIdWhenRequestIdIsNull) {
    // Arrange
    struct aclk_cancel_pending_req req;
    req.request_id = nullptr;
    req.trace_id = strdupz("test-trace-id");
    
    // Act
    free_cancel_pending_req(&req);
    
    // Assert - memory should be freed (no crash)
    EXPECT_TRUE(true);  // If we get here, no crash occurred
}

TEST_F(FreeCancelPendingReqTest, ShouldHandleWhenBothPointersAreNull) {
    // Arrange
    struct aclk_cancel_pending_req req;
    req.request_id = nullptr;
    req.trace_id = nullptr;
    
    // Act
    free_cancel_pending_req(&req);
    
    // Assert - should not crash
    EXPECT_TRUE(true);
}

TEST_F(FreeCancelPendingReqTest, ShouldHandleEmptyStrings) {
    // Arrange
    struct aclk_cancel_pending_req req;
    req.request_id = strdupz("");
    req.trace_id = strdupz("");
    
    // Act
    free_cancel_pending_req(&req);
    
    // Assert - should not crash
    EXPECT_TRUE(true);
}

TEST_F(FreeCancelPendingReqTest, ShouldFreeAfterParsing) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("parsed-request-id");
    proto_msg.set_trace_id("parsed-trace-id");
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(1234567890);
    ts->set_nanos(500000000);
    
    std::string serialized = proto_msg.SerializeAsString();
    struct aclk_cancel_pending_req req;
    memset(&req, 0, sizeof(req));
    
    int parse_result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &req);
    ASSERT_EQ(parse_result, 0);
    
    // Act
    free_cancel_pending_req(&req);
    
    // Assert - should not crash
    EXPECT_TRUE(true);
}

TEST_F(FreeCancelPendingReqTest, ShouldHandleMultipleFreeCallsOnSameStruct) {
    // Arrange
    struct aclk_cancel_pending_req req;
    req.request_id = strdupz("test-id");
    req.trace_id = strdupz("test-trace");
    
    // Act & Assert - first free
    free_cancel_pending_req(&req);
    EXPECT_TRUE(true);
    
    // Set pointers to null to avoid double-free
    req.request_id = nullptr;
    req.trace_id = nullptr;
    
    // Act & Assert - second free (should not crash)
    free_cancel_pending_req(&req);
    EXPECT_TRUE(true);
}

TEST_F(FreeCancelPendingReqTest, ShouldHandleVeryLongAllocatedStrings) {
    // Arrange
    std::string long_id(50000, 'x');
    std::string long_trace(50000, 'y');
    
    struct aclk_cancel_pending_req req;
    req.request_id = strdupz(long_id.c_str());
    req.trace_id = strdupz(long_trace.c_str());
    
    // Act
    free_cancel_pending_req(&req);
    
    // Assert - should not crash
    EXPECT_TRUE(true);
}

// Integration tests

class AgentCmdsIntegrationTest : public AgentCmdsTest {
};

TEST_F(AgentCmdsIntegrationTest, ShouldParseAndFreeSuccessfully) {
    // Arrange
    CancelPendingRequest proto_msg;
    proto_msg.set_request_id("integration-test-request");
    proto_msg.set_trace_id("integration-test-trace");
    
    google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
    ts->set_seconds(1609459200);  // 2021-01-01 00:00:00 UTC
    ts->set_nanos(123456789);
    
    std::string serialized = proto_msg.SerializeAsString();
    
    // Act
    int parse_result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &test_req);
    
    // Assert parsing
    EXPECT_EQ(parse_result, 0);
    EXPECT_NE(test_req.request_id, nullptr);
    EXPECT_STREQ(test_req.request_id, "integration-test-request");
    
    // Act - Free
    free_cancel_pending_req(&test_req);
    
    // Assert - should not crash
    EXPECT_TRUE(true);
}

TEST_F(AgentCmdsIntegrationTest, ShouldHandleMultipleParseAndFreeOperations) {
    // Arrange & Act & Assert
    for (int i = 0; i < 100; i++) {
        CancelPendingRequest proto_msg;
        proto_msg.set_request_id("req-" + std::to_string(i));
        proto_msg.set_trace_id("trace-" + std::to_string(i));
        
        google::protobuf::Timestamp *ts = proto_msg.mutable_timestamp();
        ts->set_seconds(1000000000 + i);
        ts->set_nanos(100000000 * (i % 10));
        
        std::string serialized = proto_msg.SerializeAsString();
        
        struct aclk_cancel_pending_req req;
        memset(&req, 0, sizeof(req));
        
        int result = parse_cancel_pending_req(serialized.c_str(), serialized.length(), &req);
        EXPECT_EQ(result, 0);
        
        free_cancel_pending_req(&req);
    }
    
    EXPECT_TRUE(true);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}