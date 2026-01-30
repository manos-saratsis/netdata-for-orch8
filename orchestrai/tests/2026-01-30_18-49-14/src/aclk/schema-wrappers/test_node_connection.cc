// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>

// Forward declarations for the functions under test
extern "C" {
#include "node_connection.h"
#include "capability.h"

// Mock implementations for protobuf-related functions
namespace nodeinstance {
namespace v1 {
class UpdateNodeInstanceConnection {
public:
    void set_claim_id(const char* claim_id) { 
        claim_id_ = claim_id ? strdup(claim_id) : nullptr;
    }
    void set_node_id(const char* node_id) { 
        node_id_ = node_id ? strdup(node_id) : nullptr;
    }
    void set_liveness(bool live) { live_ = live; }
    void set_queryable(bool queryable) { queryable_ = queryable; }
    void set_session_id(int64_t session_id) { session_id_ = session_id; }
    void set_hops(int32_t hops) { hops_ = hops; }
    
    google::protobuf::Timestamp* mutable_updated_at() { 
        return &timestamp_; 
    }
    
    struct MockCapability {
        const char* name;
        bool enabled;
        uint32_t version;
    };
    
    aclk_lib::v1::Capability* add_capabilities() {
        capabilities_.push_back(aclk_lib::v1::Capability());
        return &capabilities_.back();
    }
    
    size_t ByteSizeLong() const { return 256; }
    
    bool SerializeToArray(char* buffer, size_t size) const {
        if (!buffer || size == 0) return false;
        memset(buffer, 0, size);
        return true;
    }
    
    ~UpdateNodeInstanceConnection() {
        free(claim_id_);
        free(node_id_);
    }

private:
    char* claim_id_ = nullptr;
    char* node_id_ = nullptr;
    bool live_ = false;
    bool queryable_ = false;
    int64_t session_id_ = 0;
    int32_t hops_ = 0;
    google::protobuf::Timestamp timestamp_;
    std::vector<aclk_lib::v1::Capability> capabilities_;
};
} // namespace v1
} // namespace nodeinstance

namespace aclk_lib {
namespace v1 {
class Capability {
public:
    void set_name(const char* name) { name_ = name; }
    void set_enabled(bool enabled) { enabled_ = enabled; }
    void set_version(uint32_t version) { version_ = version; }
private:
    const char* name_ = nullptr;
    bool enabled_ = false;
    uint32_t version_ = 0;
};
} // namespace v1
} // namespace aclk_lib

namespace google {
namespace protobuf {
class Timestamp {
public:
    void set_seconds(int64_t seconds) { seconds_ = seconds; }
    void set_nanos(int32_t nanos) { nanos_ = nanos; }
private:
    int64_t seconds_ = 0;
    int32_t nanos_ = 0;
};
} // namespace protobuf
} // namespace google
}

// Helper function for memory allocation
extern "C" {
    void* mallocz(size_t size) {
        return malloc(size);
    }
    
    void freez(void* ptr) {
        free(ptr);
    }
}

class NodeConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize common test data
        memset(&test_data, 0, sizeof(test_data));
    }

    void TearDown() override {
        // Cleanup after tests
    }

    node_instance_connection_t test_data;
};

// Test: generate_node_instance_connection with NULL data
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionWithValidData) {
    test_data.claim_id = "test-claim-id-123";
    test_data.node_id = "test-node-id-456";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 999999;
    test_data.hops = 5;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with minimal data
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionMinimalData) {
    test_data.claim_id = nullptr;
    test_data.node_id = "minimal-node";
    test_data.live = 0;
    test_data.queryable = 0;
    test_data.session_id = 0;
    test_data.hops = 0;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with empty strings
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionEmptyStrings) {
    test_data.claim_id = "";
    test_data.node_id = "";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 123;
    test_data.hops = 1;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with single capability
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionWithSingleCapability) {
    struct capability single_cap[] = {
        { "capability1", 1, 1 },
        { nullptr, 0, 0 }
    };

    test_data.claim_id = "claim";
    test_data.node_id = "node";
    test_data.live = 1;
    test_data.queryable = 0;
    test_data.session_id = 100;
    test_data.hops = 2;
    test_data.capabilities = single_cap;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with multiple capabilities
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionWithMultipleCapabilities) {
    struct capability multi_caps[] = {
        { "cap1", 1, 1 },
        { "cap2", 0, 2 },
        { "cap3", 1, 3 },
        { nullptr, 0, 0 }
    };

    test_data.claim_id = "claim-multi";
    test_data.node_id = "node-multi";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 12345;
    test_data.hops = 10;
    test_data.capabilities = multi_caps;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with boundary values for session_id
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionBoundarySessionId) {
    test_data.claim_id = "claim";
    test_data.node_id = "node";
    test_data.live = 0;
    test_data.queryable = 1;
    test_data.session_id = 0x7FFFFFFFFFFFFFFF; // Max int64_t
    test_data.hops = 0;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with negative session_id
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionNegativeSessionId) {
    test_data.claim_id = "claim";
    test_data.node_id = "node";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = -1;
    test_data.hops = -1;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with large strings
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionLargeStrings) {
    const char* large_claim = "claim-" "0123456789" "0123456789" "0123456789" "0123456789" "0123456789";
    const char* large_node = "node-" "abcdefghij" "abcdefghij" "abcdefghij" "abcdefghij" "abcdefghij";
    
    test_data.claim_id = large_claim;
    test_data.node_id = large_node;
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 999;
    test_data.hops = 15;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with all flags set to 0
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionAllFlagsZero) {
    test_data.claim_id = "claim";
    test_data.node_id = "node";
    test_data.live = 0;
    test_data.queryable = 0;
    test_data.session_id = 0;
    test_data.hops = 0;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with all flags set to 1
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionAllFlagsOne) {
    test_data.claim_id = "claim";
    test_data.node_id = "node";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 1;
    test_data.hops = 1;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with empty capability list
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionEmptyCapabilityList) {
    struct capability empty_caps[] = {
        { nullptr, 0, 0 }
    };

    test_data.claim_id = "claim";
    test_data.node_id = "node";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 500;
    test_data.hops = 5;
    test_data.capabilities = empty_caps;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection with large number of capabilities
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionManyCapabilities) {
    struct capability many_caps[] = {
        { "cap0", 1, 0 },
        { "cap1", 0, 1 },
        { "cap2", 1, 2 },
        { "cap3", 0, 3 },
        { "cap4", 1, 4 },
        { "cap5", 0, 5 },
        { "cap6", 1, 6 },
        { "cap7", 0, 7 },
        { nullptr, 0, 0 }
    };

    test_data.claim_id = "claim-many";
    test_data.node_id = "node-many";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 98765;
    test_data.hops = 20;
    test_data.capabilities = many_caps;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_connection return value is non-null
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionReturnNotNull) {
    test_data.claim_id = "test";
    test_data.node_id = "test";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 1;
    test_data.hops = 1;
    test_data.capabilities = nullptr;

    size_t len = 0;
    char* result = generate_node_instance_connection(&len, &test_data);

    EXPECT_NE(result, nullptr);
    free(result);
}

// Test: generate_node_instance_connection output length is set correctly
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionLengthSet) {
    test_data.claim_id = "test";
    test_data.node_id = "test";
    test_data.live = 1;
    test_data.queryable = 1;
    test_data.session_id = 1;
    test_data.hops = 1;
    test_data.capabilities = nullptr;

    size_t len = 0;
    size_t* len_ptr = &len;
    char* result = generate_node_instance_connection(len_ptr, &test_data);

    EXPECT_NE(*len_ptr, 0);
    free(result);
}

// Test: generate_node_instance_connection with different hops values
TEST_F(NodeConnectionTest, GenerateNodeInstanceConnectionDifferentHops) {
    int32_t hops_values[] = { -1, 0, 1, 127, 255, 32767 };

    for (int32_t hops_val : hops_values) {
        test_data.claim_id = "claim";
        test_data.node_id = "node";
        test_data.live = 1;
        test_data.queryable = 1;
        test_data.session_id = 0;
        test_data.hops = hops_val;
        test_data.capabilities = nullptr;

        size_t len = 0;
        char* result = generate_node_instance_connection(&len, &test_data);

        EXPECT_NE(result, nullptr);
        EXPECT_GT(len, 0);
        free(result);
    }
}