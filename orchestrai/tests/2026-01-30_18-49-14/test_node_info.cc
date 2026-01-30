#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>

extern "C" {
#include "src/aclk/schema-wrappers/node_info.h"
#include "src/aclk/schema-wrappers/capability.h"
#include "src/aclk/schema-wrappers/schema_wrapper_utils.h"
}

// Mock for RRDLABELS
typedef void RRDLABELS;

// Mock for DICTIONARY
typedef void DICTIONARY;

// Mock functions
extern "C" {
    void *mallocz(size_t size) {
        return malloc(size);
    }
    
    void freez(void *ptr) {
        free(ptr);
    }
    
    void rrdlabels_walkthrough_read(RRDLABELS *labels, int (*callback)(const char *, const char *, int, void *), void *data) {
        // Mock implementation - calls callback with test label data if labels is non-null
        if (labels != nullptr) {
            // Add a sample label for testing
            callback("test_label", "test_value", 0, data);
        }
    }
    
    int dfe_start_read(DICTIONARY *dict, void *&item) {
        // This is a macro-like function - mock to return 1 for iteration start
        return 1;
    }
    
    void dfe_done(void *item) {
        // Mock cleanup
    }
}

// Test fixture for node_info tests
class NodeInfoTest : public ::testing::Test {
protected:
    struct update_node_info test_info;
    struct update_node_collectors test_collectors;
    
    void SetUp() override {
        // Initialize test_info with default values
        test_info.node_id = const_cast<char*>("test_node_id");
        test_info.claim_id = const_cast<char*>("test_claim_id");
        test_info.machine_guid = const_cast<char*>("test_machine_guid");
        test_info.child = 0;
        
        // Initialize aclk_node_info
        test_info.data.name = "test_node";
        test_info.data.os = "linux";
        test_info.data.os_name = "Ubuntu";
        test_info.data.os_version = "20.04";
        test_info.data.kernel_name = "Linux";
        test_info.data.kernel_version = "5.4.0";
        test_info.data.architecture = "x86_64";
        test_info.data.cpus = 4;
        test_info.data.cpu_frequency = "2.4 GHz";
        test_info.data.memory = "8GB";
        test_info.data.disk_space = "256GB";
        test_info.data.version = "1.0.0";
        test_info.data.release_channel = "stable";
        test_info.data.timezone = "UTC";
        test_info.data.virtualization_type = "kvm";
        test_info.data.container_type = "docker";
        test_info.data.custom_info = "custom data";
        test_info.data.machine_guid = "machine_guid_value";
        test_info.data.host_labels_ptr = nullptr;
        test_info.data.ml_info.ml_capable = true;
        test_info.data.ml_info.ml_enabled = true;
        
        // Initialize ml_info
        test_info.ml_info.ml_capable = true;
        test_info.ml_info.ml_enabled = false;
        
        // Initialize capabilities
        test_info.node_capabilities = nullptr;
        test_info.node_instance_capabilities = nullptr;
        
        // Initialize timeval
        test_info.updated_at.tv_sec = 1000000;
        test_info.updated_at.tv_usec = 500000;
        
        // Initialize collectors
        test_collectors.node_id = const_cast<char*>("test_node_id");
        test_collectors.claim_id = const_cast<char*>("test_claim_id");
        test_collectors.node_collectors = nullptr;
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
};

// Test cases for generate_update_node_info_message
TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithAllFieldsPopulated) {
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithNullFields) {
    // Set string fields to null
    test_info.data.name = nullptr;
    test_info.data.os = nullptr;
    test_info.data.os_name = nullptr;
    test_info.data.os_version = nullptr;
    test_info.data.kernel_name = nullptr;
    test_info.data.kernel_version = nullptr;
    test_info.data.architecture = nullptr;
    test_info.data.cpu_frequency = nullptr;
    test_info.data.memory = nullptr;
    test_info.data.disk_space = nullptr;
    test_info.data.version = nullptr;
    test_info.data.release_channel = nullptr;
    test_info.data.timezone = nullptr;
    test_info.data.virtualization_type = nullptr;
    test_info.data.container_type = nullptr;
    test_info.data.custom_info = nullptr;
    test_info.data.machine_guid = nullptr;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithZeroCpus) {
    test_info.data.cpus = 0;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithMaxCpus) {
    test_info.data.cpus = 0xFFFFFFFF; // Max uint32_t
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithChildTrue) {
    test_info.child = 1;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithChildFalse) {
    test_info.child = 0;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithMLCapableAndEnabledTrue) {
    test_info.ml_info.ml_capable = true;
    test_info.ml_info.ml_enabled = true;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithMLCapableAndEnabledFalse) {
    test_info.ml_info.ml_capable = false;
    test_info.ml_info.ml_enabled = false;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithNodeCapabilities) {
    struct capability caps[2] = {
        {"cap1", 1, 1},
        {nullptr, 0, 0}  // Terminator
    };
    test_info.node_capabilities = caps;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithNodeInstanceCapabilities) {
    struct capability caps[2] = {
        {"cap2", 2, 0},
        {nullptr, 0, 0}  // Terminator
    };
    test_info.node_instance_capabilities = caps;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithBothCapabilities) {
    struct capability node_caps[2] = {
        {"node_cap", 1, 1},
        {nullptr, 0, 0}
    };
    struct capability instance_caps[2] = {
        {"instance_cap", 2, 0},
        {nullptr, 0, 0}
    };
    test_info.node_capabilities = node_caps;
    test_info.node_instance_capabilities = instance_caps;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithZeroTimestamp) {
    test_info.updated_at.tv_sec = 0;
    test_info.updated_at.tv_usec = 0;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithMaxTimestamp) {
    test_info.updated_at.tv_sec = 0x7FFFFFFF; // Max time_t for 32-bit
    test_info.updated_at.tv_usec = 999999;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithEmptyStrings) {
    test_info.data.name = "";
    test_info.data.os = "";
    test_info.data.os_name = "";
    test_info.data.version = "";
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithHostLabels) {
    // Create a mock labels pointer (non-null)
    test_info.data.host_labels_ptr = reinterpret_cast<RRDLABELS*>(0x1);
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageDataMLInfo) {
    test_info.data.ml_info.ml_capable = true;
    test_info.data.ml_info.ml_enabled = false;
    test_info.ml_info.ml_capable = false;
    test_info.ml_info.ml_enabled = true;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageLengthIsPopulated) {
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

// Test cases for generate_update_node_collectors_message
TEST_F(NodeInfoTest, GenerateUpdateNodeCollectorsMessageWithNullDictionary) {
    test_collectors.node_collectors = nullptr;
    
    size_t len = 0;
    char *result = generate_update_node_collectors_message(&len, &test_collectors);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeCollectorsMessageWithValidDictionary) {
    // Create a mock dictionary (non-null)
    test_collectors.node_collectors = reinterpret_cast<DICTIONARY*>(0x1);
    
    size_t len = 0;
    char *result = generate_update_node_collectors_message(&len, &test_collectors);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeCollectorsMessageNodeIdPopulated) {
    size_t len = 0;
    char *result = generate_update_node_collectors_message(&len, &test_collectors);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeCollectorsMessageClaimIdPopulated) {
    test_collectors.claim_id = const_cast<char*>("another_claim_id");
    
    size_t len = 0;
    char *result = generate_update_node_collectors_message(&len, &test_collectors);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeCollectorsMessageLengthIsPopulated) {
    size_t len = 0;
    char *result = generate_update_node_collectors_message(&len, &test_collectors);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeCollectorsMessageMultipleTimes) {
    // Call multiple times to ensure no memory leaks or state issues
    for (int i = 0; i < 5; i++) {
        size_t len = 0;
        char *result = generate_update_node_collectors_message(&len, &test_collectors);
        
        ASSERT_NE(result, nullptr);
        ASSERT_GT(len, 0);
        
        freez(result);
    }
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageMultipleTimes) {
    // Call multiple times to ensure no memory leaks or state issues
    for (int i = 0; i < 5; i++) {
        size_t len = 0;
        char *result = generate_update_node_info_message(&len, &test_info);
        
        ASSERT_NE(result, nullptr);
        ASSERT_GT(len, 0);
        
        freez(result);
    }
}

// Edge cases for structure fields
TEST_F(NodeInfoTest, StructUpdateNodeInfoFieldTypes) {
    // Verify structure has correct field types
    ASSERT_EQ(sizeof(test_info.node_id), sizeof(char*));
    ASSERT_EQ(sizeof(test_info.child), sizeof(int));
    ASSERT_EQ(sizeof(test_info.updated_at), sizeof(struct timeval));
}

TEST_F(NodeInfoTest, StructAclkNodeInfoFieldTypes) {
    // Verify structure has correct field types
    ASSERT_EQ(sizeof(test_info.data.cpus), sizeof(uint32_t));
    ASSERT_EQ(sizeof(test_info.data.ml_info.ml_capable), sizeof(bool));
    ASSERT_EQ(sizeof(test_info.data.ml_info.ml_enabled), sizeof(bool));
}

TEST_F(NodeInfoTest, StructMachineLearningInfoFieldTypes) {
    // Verify ml_info structure
    ASSERT_EQ(sizeof(test_info.ml_info.ml_capable), sizeof(bool));
    ASSERT_EQ(sizeof(test_info.ml_info.ml_enabled), sizeof(bool));
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithLongStrings) {
    // Test with very long strings
    std::string long_string(1000, 'a');
    test_info.data.name = long_string.c_str();
    test_info.data.custom_info = long_string.c_str();
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithSpecialCharacters) {
    // Test with special characters in strings
    test_info.data.name = "test\n\t\r";
    test_info.data.custom_info = "special!@#$%^&*()";
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeCollectorsMessageEmptyNodeId) {
    test_collectors.node_id = const_cast<char*>("");
    
    size_t len = 0;
    char *result = generate_update_node_collectors_message(&len, &test_collectors);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeCollectorsMessageEmptyClaimId) {
    test_collectors.claim_id = const_cast<char*>("");
    
    size_t len = 0;
    char *result = generate_update_node_collectors_message(&len, &test_collectors);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithNullNodeId) {
    test_info.node_id = nullptr;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithNullClaimId) {
    test_info.claim_id = nullptr;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoMessageWithNullMachineGuid) {
    test_info.machine_guid = nullptr;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

// Integration tests
TEST_F(NodeInfoTest, GenerateUpdateNodeInfoAndCollectorsInSequence) {
    // Generate both messages in sequence
    size_t len1 = 0;
    char *result1 = generate_update_node_info_message(&len1, &test_info);
    
    size_t len2 = 0;
    char *result2 = generate_update_node_collectors_message(&len2, &test_collectors);
    
    ASSERT_NE(result1, nullptr);
    ASSERT_NE(result2, nullptr);
    ASSERT_GT(len1, 0);
    ASSERT_GT(len2, 0);
    
    freez(result1);
    freez(result2);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoWithMultipleCapabilitiesTerminated) {
    struct capability caps[5] = {
        {"cap1", 1, 1},
        {"cap2", 2, 0},
        {"cap3", 3, 1},
        {"cap4", 4, 0},
        {nullptr, 0, 0}  // Terminator
    };
    test_info.node_capabilities = caps;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}

TEST_F(NodeInfoTest, GenerateUpdateNodeInfoWithLargeTimestampValues) {
    test_info.updated_at.tv_sec = 1609459200; // 2021-01-01
    test_info.updated_at.tv_usec = 123456;
    
    size_t len = 0;
    char *result = generate_update_node_info_message(&len, &test_info);
    
    ASSERT_NE(result, nullptr);
    ASSERT_GT(len, 0);
    
    freez(result);
}