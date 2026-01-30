// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <memory>

// Forward declarations and mocks for protobuf classes
namespace aclk_lib {
namespace v1 {
class Capability {
public:
    void set_name(const char* name) { m_name = name; }
    void set_enabled(int enabled) { m_enabled = enabled; }
    void set_version(uint32_t version) { m_version = version; }
    
    const char* get_name() const { return m_name; }
    int get_enabled() const { return m_enabled; }
    uint32_t get_version() const { return m_version; }
private:
    const char* m_name = nullptr;
    int m_enabled = 0;
    uint32_t m_version = 0;
};
}
}

// Mock the capability structure
struct capability {
    const char *name;
    uint32_t version;
    int enabled;
};

// Function under test
void capability_set(aclk_lib::v1::Capability *proto_capa, const struct capability *c_capa) {
    proto_capa->set_name(c_capa->name);
    proto_capa->set_enabled(c_capa->enabled);
    proto_capa->set_version(c_capa->version);
}

class CapabilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test capabilities
    }

    void TearDown() override {
        // Cleanup after test
    }
};

// Test: capability_set with valid inputs
TEST_F(CapabilityTest, capability_set_with_valid_inputs) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "test_capability";
    c_capa.version = 1;
    c_capa.enabled = 1;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_STREQ(proto_capa.get_name(), "test_capability");
    EXPECT_EQ(proto_capa.get_version(), 1);
    EXPECT_EQ(proto_capa.get_enabled(), 1);
}

// Test: capability_set with disabled capability
TEST_F(CapabilityTest, capability_set_with_disabled_capability) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "disabled_cap";
    c_capa.version = 2;
    c_capa.enabled = 0;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_STREQ(proto_capa.get_name(), "disabled_cap");
    EXPECT_EQ(proto_capa.get_version(), 2);
    EXPECT_EQ(proto_capa.get_enabled(), 0);
}

// Test: capability_set with empty string name
TEST_F(CapabilityTest, capability_set_with_empty_name) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "";
    c_capa.version = 0;
    c_capa.enabled = 1;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_STREQ(proto_capa.get_name(), "");
    EXPECT_EQ(proto_capa.get_version(), 0);
    EXPECT_EQ(proto_capa.get_enabled(), 1);
}

// Test: capability_set with version 0
TEST_F(CapabilityTest, capability_set_with_version_zero) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "zero_version";
    c_capa.version = 0;
    c_capa.enabled = 1;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_EQ(proto_capa.get_version(), 0);
}

// Test: capability_set with max uint32_t version
TEST_F(CapabilityTest, capability_set_with_max_version) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "max_version";
    c_capa.version = UINT32_MAX;
    c_capa.enabled = 1;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_EQ(proto_capa.get_version(), UINT32_MAX);
}

// Test: capability_set with long name
TEST_F(CapabilityTest, capability_set_with_long_name) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "this_is_a_very_long_capability_name_that_tests_string_handling";
    c_capa.version = 5;
    c_capa.enabled = 1;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_STREQ(proto_capa.get_name(), "this_is_a_very_long_capability_name_that_tests_string_handling");
}

// Test: capability_set with special characters in name
TEST_F(CapabilityTest, capability_set_with_special_chars_in_name) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "cap-ability_name.v1";
    c_capa.version = 3;
    c_capa.enabled = 1;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_STREQ(proto_capa.get_name(), "cap-ability_name.v1");
}

// Test: capability_set with enabled=1 explicitly
TEST_F(CapabilityTest, capability_set_enabled_explicitly) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "cap1";
    c_capa.version = 1;
    c_capa.enabled = 1;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_EQ(proto_capa.get_enabled(), 1);
}

// Test: capability_set with enabled=0 explicitly
TEST_F(CapabilityTest, capability_set_disabled_explicitly) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "cap0";
    c_capa.version = 1;
    c_capa.enabled = 0;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_EQ(proto_capa.get_enabled(), 0);
}

// Test: capability_set transfers all fields correctly
TEST_F(CapabilityTest, capability_set_all_fields_transferred) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa = {
        "multi_field_test",
        42,
        1
    };
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_STREQ(proto_capa.get_name(), "multi_field_test");
    EXPECT_EQ(proto_capa.get_version(), 42);
    EXPECT_EQ(proto_capa.get_enabled(), 1);
}

// Test: capability_set multiple times overwrites values
TEST_F(CapabilityTest, capability_set_multiple_calls_overwrite) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa1 = { "first", 1, 1 };
    struct capability c_capa2 = { "second", 2, 0 };
    
    capability_set(&proto_capa, &c_capa1);
    capability_set(&proto_capa, &c_capa2);
    
    EXPECT_STREQ(proto_capa.get_name(), "second");
    EXPECT_EQ(proto_capa.get_version(), 2);
    EXPECT_EQ(proto_capa.get_enabled(), 0);
}

// Test: capability_set with numeric string name
TEST_F(CapabilityTest, capability_set_with_numeric_string_name) {
    aclk_lib::v1::Capability proto_capa;
    
    struct capability c_capa;
    c_capa.name = "12345";
    c_capa.version = 100;
    c_capa.enabled = 1;
    
    capability_set(&proto_capa, &c_capa);
    
    EXPECT_STREQ(proto_capa.get_name(), "12345");
}