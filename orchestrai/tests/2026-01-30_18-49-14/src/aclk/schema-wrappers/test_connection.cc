// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>

// Forward declarations for mocking
extern "C" {
    void *mallocz(size_t size);
    void *callocz(size_t count, size_t size);
    void freez(void *ptr);
    char *strdupz(const char *str);
}

#include "src/aclk/schema-wrappers/connection.h"
#include "src/aclk/schema-wrappers/capability.h"
#include "src/aclk/aclk-schemas/proto/agent/v1/connection.pb.h"
#include "src/aclk/aclk-schemas/proto/agent/v1/disconnect.pb.h"

using namespace agent::v1;
using namespace ::testing;

// ============================================================================
// MOCK DECLARATIONS FOR MEMORY FUNCTIONS
// ============================================================================

class MemoryMock {
public:
    MOCK_METHOD(void*, malloc_impl, (size_t size), ());
    MOCK_METHOD(void*, calloc_impl, (size_t count, size_t size), ());
    MOCK_METHOD(void, free_impl, (void* ptr), ());
    MOCK_METHOD(char*, strdup_impl, (const char* str), ());
};

static MemoryMock* g_memory_mock = nullptr;

// Real implementations that delegate to mock
void* mallocz(size_t size) {
    if (g_memory_mock) {
        return g_memory_mock->malloc_impl(size);
    }
    return malloc(size);
}

void* callocz(size_t count, size_t size) {
    if (g_memory_mock) {
        return g_memory_mock->calloc_impl(count, size);
    }
    return calloc(count, size);
}

void freez(void* ptr) {
    if (g_memory_mock) {
        g_memory_mock->free_impl(ptr);
    } else {
        free(ptr);
    }
}

char* strdupz(const char* str) {
    if (g_memory_mock) {
        return g_memory_mock->strdup_impl(str);
    }
    return strdup(str);
}

// ============================================================================
// TEST FIXTURES
// ============================================================================

class ConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_memory_mock = nullptr;
    }

    void TearDown() override {
        g_memory_mock = nullptr;
    }
};

class DisconnectParseTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_memory_mock = nullptr;
    }

    void TearDown() override {
        g_memory_mock = nullptr;
    }

    // Helper to create a valid DisconnectReq protobuf
    std::string CreateDisconnectReqData(uint64_t reconnect_after = 0,
                                       int permaban = 0,
                                       uint32_t error_code = 0,
                                       const char* error_desc = nullptr) {
        DisconnectReq req;
        req.set_reconnect_after_seconds(reconnect_after);
        req.set_permaban(permaban);
        req.set_error_code(error_code);
        if (error_desc) {
            req.set_error_description(error_desc);
        }
        return req.SerializeAsString();
    }
};

// ============================================================================
// TESTS FOR generate_update_agent_connection()
// ============================================================================

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionBasic) {
    // ARRANGE
    update_agent_connection_t data = {};
    data.claim_id = "test-claim-id";
    data.reachable = 1;
    data.session_id = 12345;
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);

    // Verify the protobuf can be parsed back
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.claim_id(), "test-claim-id");
    EXPECT_EQ(parsed.reachable(), true);
    EXPECT_EQ(parsed.session_id(), 12345);
    EXPECT_EQ(parsed.update_source(), CONNECTION_UPDATE_SOURCE_AGENT);
    EXPECT_EQ(parsed.capabilities_size(), 0);

    // Cleanup
    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionWithLWTTrue) {
    // ARRANGE
    update_agent_connection_t data = {};
    data.claim_id = "lwt-test";
    data.reachable = 1;
    data.session_id = 67890;
    data.lwt = 1;  // Last Will and Testament enabled
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);

    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.update_source(), CONNECTION_UPDATE_SOURCE_LWT);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionNotReachable) {
    // ARRANGE
    update_agent_connection_t data = {};
    data.claim_id = "unreachable-test";
    data.reachable = 0;  // Not reachable
    data.session_id = 111;
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);

    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.reachable(), false);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionWithNullCapabilities) {
    // ARRANGE
    update_agent_connection_t data = {};
    data.claim_id = "null-capabilities";
    data.reachable = 1;
    data.session_id = 222;
    data.lwt = 0;
    data.capabilities = nullptr;  // Explicitly null

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.capabilities_size(), 0);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionWithSingleCapability) {
    // ARRANGE
    struct capability capa[] = {
        {.name = "test-capability", .version = 1, .enabled = 1},
        {.name = nullptr, .version = 0, .enabled = 0}  // Null terminator
    };

    update_agent_connection_t data = {};
    data.claim_id = "single-cap";
    data.reachable = 1;
    data.session_id = 333;
    data.lwt = 0;
    data.capabilities = capa;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.capabilities_size(), 1);
    EXPECT_EQ(parsed.capabilities(0).name(), "test-capability");
    EXPECT_EQ(parsed.capabilities(0).version(), 1);
    EXPECT_EQ(parsed.capabilities(0).enabled(), true);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionWithMultipleCapabilities) {
    // ARRANGE
    struct capability capa[] = {
        {.name = "cap1", .version = 1, .enabled = 1},
        {.name = "cap2", .version = 2, .enabled = 0},
        {.name = "cap3", .version = 3, .enabled = 1},
        {.name = nullptr, .version = 0, .enabled = 0}  // Null terminator
    };

    update_agent_connection_t data = {};
    data.claim_id = "multi-cap";
    data.reachable = 1;
    data.session_id = 444;
    data.lwt = 0;
    data.capabilities = capa;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.capabilities_size(), 3);

    EXPECT_EQ(parsed.capabilities(0).name(), "cap1");
    EXPECT_EQ(parsed.capabilities(0).version(), 1);
    EXPECT_EQ(parsed.capabilities(0).enabled(), true);

    EXPECT_EQ(parsed.capabilities(1).name(), "cap2");
    EXPECT_EQ(parsed.capabilities(1).version(), 2);
    EXPECT_EQ(parsed.capabilities(1).enabled(), false);

    EXPECT_EQ(parsed.capabilities(2).name(), "cap3");
    EXPECT_EQ(parsed.capabilities(2).version(), 3);
    EXPECT_EQ(parsed.capabilities(2).enabled(), true);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionWithEmptyCapabilities) {
    // ARRANGE
    struct capability capa[] = {
        {.name = nullptr, .version = 0, .enabled = 0}  // Empty array (just terminator)
    };

    update_agent_connection_t data = {};
    data.claim_id = "empty-cap";
    data.reachable = 1;
    data.session_id = 555;
    data.lwt = 0;
    data.capabilities = capa;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.capabilities_size(), 0);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionTimestampPresent) {
    // ARRANGE
    update_agent_connection_t data = {};
    data.claim_id = "timestamp-test";
    data.reachable = 1;
    data.session_id = 666;
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));

    // Verify timestamp is set
    EXPECT_TRUE(parsed.has_updated_at());
    EXPECT_GT(parsed.updated_at().seconds(), 0);
    // Nanos should be between 0 and 999,999,999
    EXPECT_GE(parsed.updated_at().nanos(), 0);
    EXPECT_LE(parsed.updated_at().nanos(), 999999999);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionMemoryAllocationFailure) {
    // ARRANGE
    update_agent_connection_t data = {};
    data.claim_id = "alloc-fail";
    data.reachable = 1;
    data.session_id = 777;
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // Setup mock to fail malloc
    MemoryMock mock;
    g_memory_mock = &mock;
    EXPECT_CALL(mock, malloc_impl(_)).WillOnce(Return(nullptr));

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT - should return nullptr on malloc failure
    EXPECT_EQ(result, nullptr);

    g_memory_mock = nullptr;
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionSessionIdEdgeCases) {
    // ARRANGE - Test with extreme session ID values
    update_agent_connection_t data = {};
    data.claim_id = "edge-session";
    data.reachable = 1;
    data.session_id = INT64_MAX;  // Maximum int64
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.session_id(), INT64_MAX);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionSessionIdNegative) {
    // ARRANGE - Test with negative session ID
    update_agent_connection_t data = {};
    data.claim_id = "negative-session";
    data.reachable = 1;
    data.session_id = -1;
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.session_id(), -1);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionZeroSessionId) {
    // ARRANGE
    update_agent_connection_t data = {};
    data.claim_id = "zero-session";
    data.reachable = 1;
    data.session_id = 0;
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.session_id(), 0);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionEmptyClaimId) {
    // ARRANGE
    update_agent_connection_t data = {};
    data.claim_id = "";  // Empty string
    data.reachable = 1;
    data.session_id = 888;
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.claim_id(), "");

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionLongClaimId) {
    // ARRANGE
    std::string long_id(1000, 'a');
    update_agent_connection_t data = {};
    data.claim_id = long_id.c_str();
    data.reachable = 1;
    data.session_id = 999;
    data.lwt = 0;
    data.capabilities = nullptr;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.claim_id(), long_id);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionCapabilityVersionZero) {
    // ARRANGE
    struct capability capa[] = {
        {.name = "zero-version", .version = 0, .enabled = 1},
        {.name = nullptr, .version = 0, .enabled = 0}
    };

    update_agent_connection_t data = {};
    data.claim_id = "cap-version";
    data.reachable = 1;
    data.session_id = 100;
    data.lwt = 0;
    data.capabilities = capa;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.capabilities(0).version(), 0);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionCapabilityVersionMax) {
    // ARRANGE
    struct capability capa[] = {
        {.name = "max-version", .version = UINT32_MAX, .enabled = 1},
        {.name = nullptr, .version = 0, .enabled = 0}
    };

    update_agent_connection_t data = {};
    data.claim_id = "cap-max";
    data.reachable = 1;
    data.session_id = 101;
    data.lwt = 0;
    data.capabilities = capa;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.capabilities(0).version(), UINT32_MAX);

    free(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnectionCapabilityDisabled) {
    // ARRANGE
    struct capability capa[] = {
        {.name = "disabled-cap", .version = 1, .enabled = 0},
        {.name = nullptr, .version = 0, .enabled = 0}
    };

    update_agent_connection_t data = {};
    data.claim_id = "disabled";
    data.reachable = 1;
    data.session_id = 102;
    data.lwt = 0;
    data.capabilities = capa;

    size_t len = 0;

    // ACT
    char* result = generate_update_agent_connection(&len, &data);

    // ASSERT
    EXPECT_NE(result, nullptr);
    UpdateAgentConnection parsed;
    EXPECT_TRUE(parsed.ParseFromArray(result, len));
    EXPECT_EQ(parsed.capabilities(0).enabled(), false);

    free(result);
}

// ============================================================================
// TESTS FOR parse_disconnect_cmd()
// ============================================================================

TEST_F(DisconnectParseTest, ParseDisconnectCmdBasic) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(10, 0, 0, nullptr);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->reconnect_after_s, 10);
    EXPECT_EQ(result->permaban, 0);
    EXPECT_EQ(result->error_code, 0);
    EXPECT_EQ(result->error_description, nullptr);

    // Cleanup
    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdWithErrorDescription) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(5, 0, 0, "Test error message");

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->reconnect_after_s, 5);
    EXPECT_EQ(result->permaban, 0);
    EXPECT_EQ(result->error_code, 0);
    EXPECT_NE(result->error_description, nullptr);
    EXPECT_STREQ(result->error_description, "Test error message");

    // Cleanup
    freez(result->error_description);
    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdPermaban) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(0, 1, 0, nullptr);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->permaban, 1);

    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdErrorCode) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(0, 0, 42, nullptr);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->error_code, 42);

    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdAllFieldsSet) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(100, 1, 99, "Critical error");

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->reconnect_after_s, 100);
    EXPECT_EQ(result->permaban, 1);
    EXPECT_EQ(result->error_code, 99);
    EXPECT_STREQ(result->error_description, "Critical error");

    freez(result->error_description);
    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdInvalidData) {
    // ARRANGE
    const char* invalid_data = "this is not valid protobuf data";

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(invalid_data, strlen(invalid_data));

    // ASSERT - Should return nullptr for invalid protobuf
    EXPECT_EQ(result, nullptr);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdEmptyData) {
    // ARRANGE
    const char* empty_data = "";

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(empty_data, 0);

    // ASSERT
    EXPECT_EQ(result, nullptr);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdCallocFailure) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(5, 0, 0, nullptr);

    MemoryMock mock;
    g_memory_mock = &mock;
    EXPECT_CALL(mock, calloc_impl(1, sizeof(struct disconnect_cmd))).WillOnce(Return(nullptr));

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_EQ(result, nullptr);

    g_memory_mock = nullptr;
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdStrdupFailureWithDescription) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(5, 0, 0, "Error message");

    MemoryMock mock;
    g_memory_mock = &mock;

    // First call succeeds (callocz), second call fails (strdupz)
    EXPECT_CALL(mock, calloc_impl(1, sizeof(struct disconnect_cmd))).WillOnce([](size_t, size_t size) {
        return calloc(1, size);
    });
    EXPECT_CALL(mock, strdup_impl(_)).WillOnce(Return(nullptr));

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_EQ(result, nullptr);

    g_memory_mock = nullptr;
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdZeroReconnectTime) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(0, 0, 0, nullptr);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->reconnect_after_s, 0);

    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdMaxReconnectTime) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(UINT64_MAX, 0, 0, nullptr);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->reconnect_after_s, UINT64_MAX);

    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdMaxErrorCode) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(0, 0, UINT32_MAX, nullptr);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->error_code, UINT32_MAX);

    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdPermaBanZero) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(0, 0, 0, nullptr);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->permaban, 0);

    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdPermaBanOne) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(0, 1, 0, nullptr);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->permaban, 1);

    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdEmptyErrorDescription) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(0, 0, 0, "");

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_NE(result->error_description, nullptr);
    EXPECT_STREQ(result->error_description, "");

    freez(result->error_description);
    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdLongErrorDescription) {
    // ARRANGE
    std::string long_error(5000, 'e');
    std::string data = CreateDisconnectReqData(0, 0, 0, long_error.c_str());

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result->error_description, long_error.c_str());

    freez(result->error_description);
    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdTruncatedData) {
    // ARRANGE
    std::string data = CreateDisconnectReqData(10, 0, 0, "Error");
    // Truncate the data to make it invalid
    std::string truncated = data.substr(0, data.length() / 2);

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(truncated.c_str(), truncated.length());

    // ASSERT - Should fail to parse
    EXPECT_EQ(result, nullptr);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdNullData) {
    // ARRANGE
    const char* null_data = nullptr;

    // ACT & ASSERT - This should ideally be handled gracefully
    // The actual implementation may crash, so we test what we can
    // In a real scenario, the function should check for nullptr
    struct disconnect_cmd* result = parse_disconnect_cmd(null_data, 0);
    EXPECT_EQ(result, nullptr);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdWithSpecialCharactersInDescription) {
    // ARRANGE
    std::string special_desc = "Error: \"special\" <chars> & symbols!";
    std::string data = CreateDisconnectReqData(0, 0, 0, special_desc.c_str());

    // ACT
    struct disconnect_cmd* result = parse_disconnect_cmd(data.c_str(), data.length());

    // ASSERT
    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result->error_description, special_desc.c_str());

    freez(result->error_description);
    freez(result);
}

TEST_F(DisconnectParseTest, ParseDisconnectCmdMultipleC