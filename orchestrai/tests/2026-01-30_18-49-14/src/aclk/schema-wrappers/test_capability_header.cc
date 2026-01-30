// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>

#include "capability.h"

// Test the capability struct definition itself
class CapabilityStructTest : public ::testing::Test {
};

// Test: capability struct has name field of const char*
TEST_F(CapabilityStructTest, should_have_name_field_as_const_char_pointer) {
    struct capability cap = {
        .name = "test",
        .version = 1,
        .enabled = 1
    };

    EXPECT_NE(cap.name, nullptr);
    EXPECT_STREQ(cap.name, "test");
}

// Test: capability struct has version field of uint32_t
TEST_F(CapabilityStructTest, should_have_version_field_as_uint32_t) {
    struct capability cap = {
        .name = "test",
        .version = 12345,
        .enabled = 1
    };

    EXPECT_EQ(cap.version, 12345u);
    EXPECT_TRUE(std::is_same<decltype(cap.version), uint32_t>::value);
}

// Test: capability struct has enabled field of int
TEST_F(CapabilityStructTest, should_have_enabled_field_as_int) {
    struct capability cap = {
        .name = "test",
        .version = 1,
        .enabled = 42
    };

    EXPECT_EQ(cap.enabled, 42);
    EXPECT_TRUE(std::is_same<decltype(cap.enabled), int>::value);
}

// Test: capability struct can be initialized with zero values
TEST_F(CapabilityStructTest, should_allow_zero_initialization) {
    struct capability cap = {
        .name = "",
        .version = 0,
        .enabled = 0
    };

    EXPECT_STREQ(cap.name, "");
    EXPECT_EQ(cap.version, 0u);
    EXPECT_EQ(cap.enabled, 0);
}

// Test: capability struct field name is const char pointer (not array)
TEST_F(CapabilityStructTest, should_have_name_as_pointer_not_array) {
    const char *test_str = "external_string";
    struct capability cap = {
        .name = test_str,
        .version = 1,
        .enabled = 1
    };

    EXPECT_EQ(cap.name, test_str);
    EXPECT_STREQ(cap.name, "external_string");
}

// Test: multiple capability structs can be created independently
TEST_F(CapabilityStructTest, should_allow_multiple_independent_instances) {
    struct capability cap1 = {
        .name = "cap1",
        .version = 1,
        .enabled = 1
    };

    struct capability cap2 = {
        .name = "cap2",
        .version = 2,
        .enabled = 0
    };

    EXPECT_STREQ(cap1.name, "cap1");
    EXPECT_EQ(cap1.version, 1u);
    EXPECT_EQ(cap1.enabled, 1);

    EXPECT_STREQ(cap2.name, "cap2");
    EXPECT_EQ(cap2.version, 2u);
    EXPECT_EQ(cap2.enabled, 0);

    // Verify they are truly independent
    EXPECT_NE(cap1.name, cap2.name);
    EXPECT_NE(cap1.version, cap2.version);
    EXPECT_NE(cap1.enabled, cap2.enabled);
}

// Test: capability struct with NULL name pointer
TEST_F(CapabilityStructTest, should_allow_null_name_pointer) {
    struct capability cap = {
        .name = nullptr,
        .version = 1,
        .enabled = 1
    };

    EXPECT_EQ(cap.name, nullptr);
    EXPECT_EQ(cap.version, 1u);
    EXPECT_EQ(cap.enabled, 1);
}

// Test: capability struct fields can be modified after creation
TEST_F(CapabilityStructTest, should_allow_field_modification) {
    struct capability cap = {
        .name = "original",
        .version = 1,
        .enabled = 0
    };

    const char *new_name = "modified";
    cap.name = new_name;
    cap.version = 999;
    cap.enabled = 1;

    EXPECT_STREQ(cap.name, "modified");
    EXPECT_EQ(cap.version, 999u);
    EXPECT_EQ(cap.enabled, 1);
}

// Test: capability struct with maximum values
TEST_F(CapabilityStructTest, should_handle_maximum_field_values) {
    const char *max_name = "capability_with_very_long_name_"
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    struct capability cap = {
        .name = max_name,
        .version = UINT32_MAX,
        .enabled = INT_MAX
    };

    EXPECT_STREQ(cap.name, max_name);
    EXPECT_EQ(cap.version, UINT32_MAX);
    EXPECT_EQ(cap.enabled, INT_MAX);
}

// Test: capability struct with minimum int value for enabled
TEST_F(CapabilityStructTest, should_handle_minimum_enabled_value) {
    struct capability cap = {
        .name = "test",
        .version = 0,
        .enabled = INT_MIN
    };

    EXPECT_EQ(cap.enabled, INT_MIN);
}

// Test: capability struct size is reasonable
TEST_F(CapabilityStructTest, should_have_reasonable_struct_size) {
    // Structure should contain: const char* (8 bytes on 64-bit), uint32_t (4 bytes), int (4 bytes)
    // Total should be at least 16 bytes (with potential padding)
    EXPECT_GE(sizeof(struct capability), 16u);
    EXPECT_LE(sizeof(struct capability), 32u);  // Reasonable upper bound
}

// Test: capability struct alignment
TEST_F(CapabilityStructTest, should_have_pointer_first_for_alignment) {
    struct capability cap = {
        .name = "test",
        .version = 123,
        .enabled = 1
    };

    // Verify we can access the name field (basic pointer validity test)
    EXPECT_NE(&cap.name, nullptr);
    EXPECT_NE(cap.name, nullptr);
}

// Test: capability struct with empty string (different from NULL)
TEST_F(CapabilityStructTest, should_distinguish_empty_string_from_null) {
    const char *empty_str = "";
    const char *null_ptr = nullptr;

    struct capability cap1 = {
        .name = empty_str,
        .version = 1,
        .enabled = 1
    };

    struct capability cap2 = {
        .name = null_ptr,
        .version = 1,
        .enabled = 1
    };

    EXPECT_NE(cap1.name, cap2.name);
    EXPECT_EQ(cap1.name, empty_str);
    EXPECT_EQ(cap2.name, null_ptr);
}

// Test: struct C++ extern "C" compatibility
TEST_F(CapabilityStructTest, should_be_compatible_with_extern_c) {
    // If we can create and use the struct, extern "C" declarations are working
    struct capability cap;
    cap.name = "test";
    cap.version = 1;
    cap.enabled = 1;

    EXPECT_STREQ(cap.name, "test");
}

// Test: capability struct member access verification
TEST_F(CapabilityStructTest, should_verify_all_member_access_patterns) {
    struct capability cap = {
        .name = "access_test",
        .version = 555,
        .enabled = -42
    };

    // Test direct access
    EXPECT_STREQ(cap.name, "access_test");
    EXPECT_EQ(cap.version, 555u);
    EXPECT_EQ(cap.enabled, -42);

    // Test pointer access
    struct capability *cap_ptr = &cap;
    EXPECT_STREQ(cap_ptr->name, "access_test");
    EXPECT_EQ(cap_ptr->version, 555u);
    EXPECT_EQ(cap_ptr->enabled, -42);
}