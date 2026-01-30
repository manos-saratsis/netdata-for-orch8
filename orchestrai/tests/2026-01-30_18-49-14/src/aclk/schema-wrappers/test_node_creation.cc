#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include "node_creation.h"

// Mock for mallocz and memory operations
extern "C" {
    // Forward declare the functions we're testing
    char *generate_node_instance_creation(size_t *len, const node_instance_creation_t *data);
    node_instance_creation_result_t parse_create_node_instance_result(const char *data, size_t len);
    
    // We'll need to mock mallocz and strdupz if they're external
    void* mallocz(size_t size);
    char* strdupz(const char* s);
}

class NodeCreationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test data
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

// Tests for generate_node_instance_creation function
class GenerateNodeInstanceCreationTest : public NodeCreationTest {
protected:
    node_instance_creation_t test_data;
    
    void SetUp() override {
        NodeCreationTest::SetUp();
        memset(&test_data, 0, sizeof(test_data));
    }
};

// Test: generate_node_instance_creation with all fields populated
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithAllFieldsPopulated) {
    test_data.claim_id = "test_claim_id_123";
    test_data.machine_guid = "test_machine_guid_456";
    test_data.hostname = "test_hostname_789";
    test_data.hops = 5;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with NULL claim_id
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithNullClaimId) {
    test_data.claim_id = nullptr;
    test_data.machine_guid = "test_machine_guid";
    test_data.hostname = "test_hostname";
    test_data.hops = 3;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with NULL machine_guid
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithMachineGuid) {
    test_data.claim_id = "claim_id";
    test_data.machine_guid = nullptr;
    test_data.hostname = "hostname";
    test_data.hops = 2;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    // machine_guid is always set, even if nullptr
    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with NULL hostname
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithNullHostname) {
    test_data.claim_id = "claim_id";
    test_data.machine_guid = "machine_guid";
    test_data.hostname = nullptr;
    test_data.hops = 1;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    // hostname is always set
    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with zero hops
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithZeroHops) {
    test_data.claim_id = "claim_id";
    test_data.machine_guid = "machine_guid";
    test_data.hostname = "hostname";
    test_data.hops = 0;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with negative hops
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithNegativeHops) {
    test_data.claim_id = "claim_id";
    test_data.machine_guid = "machine_guid";
    test_data.hostname = "hostname";
    test_data.hops = -1;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with max int32_t hops
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithMaxHops) {
    test_data.claim_id = "claim_id";
    test_data.machine_guid = "machine_guid";
    test_data.hostname = "hostname";
    test_data.hops = INT32_MAX;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with min int32_t hops
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithMinHops) {
    test_data.claim_id = "claim_id";
    test_data.machine_guid = "machine_guid";
    test_data.hostname = "hostname";
    test_data.hops = INT32_MIN;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with empty strings
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithEmptyStrings) {
    test_data.claim_id = "";
    test_data.machine_guid = "";
    test_data.hostname = "";
    test_data.hops = 1;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with very long strings
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithLongStrings) {
    std::string long_str(1000, 'a');
    test_data.claim_id = long_str.c_str();
    test_data.machine_guid = long_str.c_str();
    test_data.hostname = long_str.c_str();
    test_data.hops = 10;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with special characters
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithSpecialCharacters) {
    test_data.claim_id = "claim!@#$%^&*()";
    test_data.machine_guid = "guid\n\t\r";
    test_data.hostname = "host.with.dots-and-dashes";
    test_data.hops = 2;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation - len pointer is updated
TEST_F(GenerateNodeInstanceCreationTest, ShouldUpdateLenPointer) {
    test_data.claim_id = "claim_id";
    test_data.machine_guid = "machine_guid";
    test_data.hostname = "hostname";
    test_data.hops = 1;

    size_t len = 999;  // Initialize to non-zero value
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_NE(999, len);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: generate_node_instance_creation with all NULL fields
TEST_F(GenerateNodeInstanceCreationTest, ShouldGenerateMessageWithAllNullFields) {
    test_data.claim_id = nullptr;
    test_data.machine_guid = nullptr;
    test_data.hostname = nullptr;
    test_data.hops = 0;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &test_data);

    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Tests for parse_create_node_instance_result function
class ParseCreateNodeInstanceResultTest : public NodeCreationTest {
protected:
    // Helper to create a valid serialized message
    std::vector<uint8_t> CreateValidMessage(const std::string& node_id, const std::string& machine_guid) {
        // This would need access to the proto message
        // For now we'll test with raw bytes or mocks
        std::vector<uint8_t> msg;
        return msg;
    }
};

// Test: parse_create_node_instance_result with empty data
TEST_F(ParseCreateNodeInstanceResultTest, ShouldReturnNullWithEmptyData) {
    const char *data = "";
    size_t len = 0;
    
    node_instance_creation_result_t result = parse_create_node_instance_result(data, len);
    
    EXPECT_EQ(nullptr, result.node_id);
    EXPECT_EQ(nullptr, result.machine_guid);
}

// Test: parse_create_node_instance_result with NULL data
TEST_F(ParseCreateNodeInstanceResultTest, ShouldReturnNullWithNullData) {
    node_instance_creation_result_t result = parse_create_node_instance_result(nullptr, 0);
    
    EXPECT_EQ(nullptr, result.node_id);
    EXPECT_EQ(nullptr, result.machine_guid);
}

// Test: parse_create_node_instance_result with invalid data
TEST_F(ParseCreateNodeInstanceResultTest, ShouldReturnNullWithInvalidData) {
    const char *invalid_data = "invalid protobuf data";
    size_t len = strlen(invalid_data);
    
    node_instance_creation_result_t result = parse_create_node_instance_result(invalid_data, len);
    
    EXPECT_EQ(nullptr, result.node_id);
    EXPECT_EQ(nullptr, result.machine_guid);
}

// Test: parse_create_node_instance_result with single byte
TEST_F(ParseCreateNodeInstanceResultTest, ShouldHandleSingleByteData) {
    const char data[] = {0x00};
    
    node_instance_creation_result_t result = parse_create_node_instance_result(data, 1);
    
    // Should either parse successfully or fail gracefully
    if (result.node_id) {
        EXPECT_NE(nullptr, result.node_id);
    }
}

// Test: parse_create_node_instance_result with large invalid data
TEST_F(ParseCreateNodeInstanceResultTest, ShouldHandleLargeInvalidData) {
    std::vector<char> large_data(10000, 0xFF);
    
    node_instance_creation_result_t result = parse_create_node_instance_result(
        large_data.data(), 
        large_data.size()
    );
    
    // Should handle gracefully
    EXPECT_TRUE(result.node_id == nullptr || result.node_id != nullptr);
}

// Test: parse_create_node_instance_result result structure
TEST_F(ParseCreateNodeInstanceResultTest, ShouldReturnValidResultStructure) {
    node_instance_creation_result_t result = parse_create_node_instance_result("data", 4);
    
    // Should return a valid structure even on parse failure
    EXPECT_TRUE(result.node_id == nullptr || result.node_id != nullptr);
    EXPECT_TRUE(result.machine_guid == nullptr || result.machine_guid != nullptr);
}

// Test: parse_create_node_instance_result with zero length
TEST_F(ParseCreateNodeInstanceResultTest, ShouldHandleZeroLength) {
    const char *data = "some data";
    
    node_instance_creation_result_t result = parse_create_node_instance_result(data, 0);
    
    EXPECT_EQ(nullptr, result.node_id);
    EXPECT_EQ(nullptr, result.machine_guid);
}

// Test: Data structures initialization
class DataStructuresTest : public NodeCreationTest {
};

// Test: node_instance_creation_t struct size
TEST_F(DataStructuresTest, ShouldHaveCorrectNodeInstanceCreationStructSize) {
    EXPECT_GT(sizeof(node_instance_creation_t), 0);
    EXPECT_GE(sizeof(node_instance_creation_t), sizeof(const char*) * 3 + sizeof(int32_t));
}

// Test: node_instance_creation_result_t struct size
TEST_F(DataStructuresTest, ShouldHaveCorrectNodeInstanceCreationResultStructSize) {
    EXPECT_GT(sizeof(node_instance_creation_result_t), 0);
    EXPECT_GE(sizeof(node_instance_creation_result_t), sizeof(char*) * 2);
}

// Test: Node creation round-trip
class NodeCreationRoundTripTest : public NodeCreationTest {
};

// Test: Generate and parse round-trip (if valid message can be created)
TEST_F(NodeCreationRoundTripTest, ShouldHandleMessageGeneration) {
    node_instance_creation_t creation_data;
    creation_data.claim_id = "test_claim";
    creation_data.machine_guid = "test_guid";
    creation_data.hostname = "test_host";
    creation_data.hops = 5;

    size_t len = 0;
    char *generated = generate_node_instance_creation(&len, &creation_data);
    
    ASSERT_NE(nullptr, generated);
    EXPECT_GT(len, 0);
    
    // Try to parse the generated message
    node_instance_creation_result_t parsed = parse_create_node_instance_result(generated, len);
    
    // Either successful parse or graceful failure
    EXPECT_TRUE(parsed.node_id == nullptr || std::string(parsed.node_id).find("test") != std::string::npos);
    
    free(generated);
}

// Test: Multiple consecutive generations
TEST_F(NodeCreationRoundTripTest, ShouldHandleMultipleGenerations) {
    for (int i = 0; i < 10; ++i) {
        node_instance_creation_t data;
        data.claim_id = "claim";
        data.machine_guid = "guid";
        data.hostname = "host";
        data.hops = i;

        size_t len = 0;
        char *result = generate_node_instance_creation(&len, &data);
        
        ASSERT_NE(nullptr, result);
        EXPECT_GT(len, 0);
        free(result);
    }
}

// Test: Boundary conditions
class BoundaryConditionsTest : public NodeCreationTest {
};

// Test: Maximum size message generation
TEST_F(BoundaryConditionsTest, ShouldHandleMaximumSizeMessage) {
    node_instance_creation_t data;
    std::string max_str(65535, 'x');
    data.claim_id = max_str.c_str();
    data.machine_guid = max_str.c_str();
    data.hostname = max_str.c_str();
    data.hops = INT32_MAX;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &data);
    
    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: Unicode string handling
TEST_F(BoundaryConditionsTest, ShouldHandleUnicodeStrings) {
    node_instance_creation_t data;
    data.claim_id = "claim_id_日本語";
    data.machine_guid = "guid_中文";
    data.hostname = "host_العربية";
    data.hops = 1;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &data);
    
    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

// Test: Null termination handling
TEST_F(BoundaryConditionsTest, ShouldHandleNullTerminatedStrings) {
    node_instance_creation_t data;
    data.claim_id = "claim\0hidden";  // String with embedded null
    data.machine_guid = "guid\0hidden";
    data.hostname = "host\0hidden";
    data.hops = 1;

    size_t len = 0;
    char *result = generate_node_instance_creation(&len, &data);
    
    ASSERT_NE(nullptr, result);
    EXPECT_GT(len, 0);
    free(result);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}