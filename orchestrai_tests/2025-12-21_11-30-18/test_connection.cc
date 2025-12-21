```cpp
#include <gtest/gtest.h>
extern "C" {
#include "connection.h"
}

class ConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ConnectionTest, GenerateUpdateAgentConnection_ValidInput) {
    update_agent_connection_t data = {
        .claim_id = "test_claim_id",
        .reachable = 1,
        .session_id = 12345,
        .lwt = 0,
        .capabilities = NULL
    };
    size_t len = 0;
    
    char* result = generate_update_agent_connection(&len, &data);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(ConnectionTest, GenerateUpdateAgentConnection_WithCapabilities) {
    struct capability caps[] = {
        {"test_cap1", 1},
        {NULL, 0}
    };
    
    update_agent_connection_t data = {
        .claim_id = "test_claim_id",
        .reachable = 1,
        .session_id = 12345,
        .lwt = 1,
        .capabilities = caps
    };
    size_t len = 0;
    
    char* result = generate_update_agent_connection(&len, &data);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(ConnectionTest, ParseDisconnectCmd_ValidInput) {
    // Simulate a protobuf serialized DisconnectReq
    char data[] = {/* Minimal valid protobuf data */};
    size_t len = sizeof(data);
    
    struct disconnect_cmd* result = parse_disconnect_cmd(data, len);
    
    ASSERT_NE(result, nullptr);
    
    // Cleanup
    if (result->error_description) {
        freez(result->error_description);
    }
    freez(result);
}

TEST_F(ConnectionTest, ParseDisconnectCmd_InvalidInput) {
    // Invalid/empty data
    char data[] = {};
    size_t len = 0;
    
    struct disconnect_cmd* result = parse_disconnect_cmd(data, len);
    
    ASSERT_EQ(result, nullptr);
}
```