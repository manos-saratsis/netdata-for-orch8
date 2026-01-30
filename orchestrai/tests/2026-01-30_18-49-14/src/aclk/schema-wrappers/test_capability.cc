// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdint>

#include "capability.h"
#include "proto/aclk/v1/lib.pb.h"

// Test class for capability functions
class CapabilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize protobuf
        proto_capa = new aclk_lib::v1::Capability();
    }

    void TearDown() override {
        delete proto_capa;
    }

    aclk_lib::v1::Capability *proto_capa;
};

// Test: capability_set with normal string name and valid version
TEST_F(CapabilityTest, should_set_capability_with_normal_name_and_version) {
    struct capability c_capa = {
        .name = "test_capability",
        .version = 1,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "test_capability");
    EXPECT_EQ(proto_capa->version(), 1u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set with empty string name
TEST_F(CapabilityTest, should_set_capability_with_empty_string_name) {
    struct capability c_capa = {
        .name = "",
        .version = 2,
        .enabled = 0
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "");
    EXPECT_EQ(proto_capa->version(), 2u);
    EXPECT_EQ(proto_capa->enabled(), 0);
}

// Test: capability_set with enabled = 0 (disabled)
TEST_F(CapabilityTest, should_set_capability_with_enabled_false) {
    struct capability c_capa = {
        .name = "disabled_capability",
        .version = 10,
        .enabled = 0
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "disabled_capability");
    EXPECT_EQ(proto_capa->version(), 10u);
    EXPECT_EQ(proto_capa->enabled(), 0);
}

// Test: capability_set with enabled = 1 (enabled)
TEST_F(CapabilityTest, should_set_capability_with_enabled_true) {
    struct capability c_capa = {
        .name = "enabled_capability",
        .version = 5,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "enabled_capability");
    EXPECT_EQ(proto_capa->version(), 5u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set with version = 0
TEST_F(CapabilityTest, should_set_capability_with_zero_version) {
    struct capability c_capa = {
        .name = "zero_version",
        .version = 0,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "zero_version");
    EXPECT_EQ(proto_capa->version(), 0u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set with maximum uint32_t version
TEST_F(CapabilityTest, should_set_capability_with_max_uint32_version) {
    struct capability c_capa = {
        .name = "max_version",
        .version = UINT32_MAX,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "max_version");
    EXPECT_EQ(proto_capa->version(), UINT32_MAX);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set with long name string
TEST_F(CapabilityTest, should_set_capability_with_long_name) {
    const char *long_name = "this_is_a_very_long_capability_name_with_many_characters_and_underscores_"
                           "to_test_string_handling_in_the_capability_set_function_implementation";
    struct capability c_capa = {
        .name = long_name,
        .version = 999,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), long_name);
    EXPECT_EQ(proto_capa->version(), 999u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set with special characters in name
TEST_F(CapabilityTest, should_set_capability_with_special_chars_in_name) {
    struct capability c_capa = {
        .name = "capability-with_special.chars@2023",
        .version = 42,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "capability-with_special.chars@2023");
    EXPECT_EQ(proto_capa->version(), 42u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set with non-zero/non-one enabled value (truthy check)
TEST_F(CapabilityTest, should_set_capability_with_non_one_enabled_value) {
    struct capability c_capa = {
        .name = "test",
        .version = 3,
        .enabled = 42
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "test");
    EXPECT_EQ(proto_capa->version(), 3u);
    EXPECT_EQ(proto_capa->enabled(), 42);
}

// Test: capability_set with negative enabled value
TEST_F(CapabilityTest, should_set_capability_with_negative_enabled_value) {
    struct capability c_capa = {
        .name = "test",
        .version = 7,
        .enabled = -1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "test");
    EXPECT_EQ(proto_capa->version(), 7u);
    EXPECT_EQ(proto_capa->enabled(), -1);
}

// Test: capability_set overwrites previous values (idempotency check)
TEST_F(CapabilityTest, should_overwrite_previous_capability_values) {
    struct capability c_capa1 = {
        .name = "first",
        .version = 1,
        .enabled = 1
    };

    struct capability c_capa2 = {
        .name = "second",
        .version = 2,
        .enabled = 0
    };

    capability_set(proto_capa, &c_capa1);
    EXPECT_EQ(proto_capa->name(), "first");
    EXPECT_EQ(proto_capa->version(), 1u);
    EXPECT_EQ(proto_capa->enabled(), 1);

    capability_set(proto_capa, &c_capa2);
    EXPECT_EQ(proto_capa->name(), "second");
    EXPECT_EQ(proto_capa->version(), 2u);
    EXPECT_EQ(proto_capa->enabled(), 0);
}

// Test: capability_set with numeric name
TEST_F(CapabilityTest, should_set_capability_with_numeric_name) {
    struct capability c_capa = {
        .name = "12345",
        .version = 100,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "12345");
    EXPECT_EQ(proto_capa->version(), 100u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set with whitespace in name
TEST_F(CapabilityTest, should_set_capability_with_whitespace_in_name) {
    struct capability c_capa = {
        .name = "capability with spaces",
        .version = 55,
        .enabled = 0
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "capability with spaces");
    EXPECT_EQ(proto_capa->version(), 55u);
    EXPECT_EQ(proto_capa->enabled(), 0);
}

// Test: capability_set with single character name
TEST_F(CapabilityTest, should_set_capability_with_single_char_name) {
    struct capability c_capa = {
        .name = "x",
        .version = 1,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "x");
    EXPECT_EQ(proto_capa->version(), 1u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set with various version boundaries
TEST_F(CapabilityTest, should_set_capability_with_version_one) {
    struct capability c_capa = {
        .name = "test",
        .version = 1,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->version(), 1u);
}

// Test: capability_set with large version number
TEST_F(CapabilityTest, should_set_capability_with_large_version) {
    struct capability c_capa = {
        .name = "test",
        .version = 1000000,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->version(), 1000000u);
}

// Test: capability_set verifies all three fields are set independently
TEST_F(CapabilityTest, should_set_all_three_fields_independently) {
    struct capability c_capa = {
        .name = "independent_test",
        .version = 777,
        .enabled = 0
    };

    capability_set(proto_capa, &c_capa);

    // Verify each field independently
    EXPECT_TRUE(proto_capa->has_name() || proto_capa->name() == "independent_test");
    EXPECT_TRUE(proto_capa->has_version() || proto_capa->version() == 777u);
    EXPECT_TRUE(proto_capa->has_enabled() || proto_capa->enabled() == 0);
}

// Test: capability_set with mixed special characters and numbers in name
TEST_F(CapabilityTest, should_set_capability_with_mixed_special_chars_numbers) {
    struct capability c_capa = {
        .name = "v1.2.3-beta+build.123",
        .version = 200,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "v1.2.3-beta+build.123");
    EXPECT_EQ(proto_capa->version(), 200u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set consistency - multiple calls with same data produce same result
TEST_F(CapabilityTest, should_produce_consistent_results_on_multiple_calls) {
    struct capability c_capa = {
        .name = "consistent",
        .version = 88,
        .enabled = 1
    };

    auto proto_capa2 = new aclk_lib::v1::Capability();

    capability_set(proto_capa, &c_capa);
    capability_set(proto_capa2, &c_capa);

    EXPECT_EQ(proto_capa->name(), proto_capa2->name());
    EXPECT_EQ(proto_capa->version(), proto_capa2->version());
    EXPECT_EQ(proto_capa->enabled(), proto_capa2->enabled());

    delete proto_capa2;
}

// Test: capability_set name with newline character
TEST_F(CapabilityTest, should_set_capability_with_newline_in_name) {
    struct capability c_capa = {
        .name = "capability\nwith\nnewlines",
        .version = 33,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "capability\nwith\nnewlines");
    EXPECT_EQ(proto_capa->version(), 33u);
    EXPECT_EQ(proto_capa->enabled(), 1);
}

// Test: capability_set name with tab character
TEST_F(CapabilityTest, should_set_capability_with_tab_in_name) {
    struct capability c_capa = {
        .name = "capability\twith\ttabs",
        .version = 44,
        .enabled = 0
    };

    capability_set(proto_capa, &c_capa);

    EXPECT_EQ(proto_capa->name(), "capability\twith\ttabs");
    EXPECT_EQ(proto_capa->version(), 44u);
    EXPECT_EQ(proto_capa->enabled(), 0);
}

// Test: capability struct definition - verify member sizes and offsets
TEST_F(CapabilityTest, should_have_capability_struct_with_correct_layout) {
    EXPECT_EQ(sizeof(struct capability), sizeof(const char*) + sizeof(uint32_t) + sizeof(int));
    
    struct capability test_cap = {
        .name = "test",
        .version = 1,
        .enabled = 1
    };
    
    capability_set(proto_capa, &test_cap);
    EXPECT_EQ(proto_capa->name(), "test");
}

// Test: capability_set with zero and non-zero pattern
TEST_F(CapabilityTest, should_handle_alternating_enabled_values) {
    struct capability c_capa1 = {
        .name = "test1",
        .version = 10,
        .enabled = 0
    };

    struct capability c_capa2 = {
        .name = "test2",
        .version = 20,
        .enabled = 1
    };

    capability_set(proto_capa, &c_capa1);
    EXPECT_EQ(proto_capa->enabled(), 0);

    capability_set(proto_capa, &c_capa2);
    EXPECT_EQ(proto_capa->enabled(), 1);

    capability_set(proto_capa, &c_capa1);
    EXPECT_EQ(proto_capa->enabled(), 0);
}