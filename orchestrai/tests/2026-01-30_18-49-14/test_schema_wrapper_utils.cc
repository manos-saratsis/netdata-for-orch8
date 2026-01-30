// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sys/time.h>
#include <google/protobuf/timestamp.pb.h>
#include <limits.h>
#include <cstring>

// Forward declarations and mocks
extern "C" {
    // Mock for RRDLABEL_SRC enum if needed
    typedef int RRDLABEL_SRC;
    #define RRDLABEL_SRC_CONFIG 0
    #define RRDLABEL_SRC_CUSTOM 1
    #define RRDLABEL_SRC_AUTO 2
}

// Include the header
#include "../../src/aclk/schema-wrappers/schema_wrapper_utils.h"

// Test suite for set_google_timestamp_from_timeval
class SetGoogleTimestampFromTimevalTest : public ::testing::Test {
protected:
    google::protobuf::Timestamp ts;
    struct timeval tv;

    void SetUp() override {
        ts.Clear();
        memset(&tv, 0, sizeof(tv));
    }
};

// Test basic timestamp conversion with normal values
TEST_F(SetGoogleTimestampFromTimevalTest, BasicConversion) {
    tv.tv_sec = 1234567890;
    tv.tv_usec = 500000;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 1234567890);
    EXPECT_EQ(ts.nanos(), 500000000);
}

// Test with zero values
TEST_F(SetGoogleTimestampFromTimevalTest, ZeroValues) {
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 0);
    EXPECT_EQ(ts.nanos(), 0);
}

// Test with maximum microsecond value (999999)
TEST_F(SetGoogleTimestampFromTimevalTest, MaxMicroseconds) {
    tv.tv_sec = 1000000000;
    tv.tv_usec = 999999;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 1000000000);
    EXPECT_EQ(ts.nanos(), 999999000);
}

// Test with minimum microsecond value (1)
TEST_F(SetGoogleTimestampFromTimevalTest, MinMicroseconds) {
    tv.tv_sec = 100;
    tv.tv_usec = 1;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 100);
    EXPECT_EQ(ts.nanos(), 1000);
}

// Test with large seconds value
TEST_F(SetGoogleTimestampFromTimevalTest, LargeSeconds) {
    tv.tv_sec = 2147483647;  // INT_MAX
    tv.tv_usec = 123456;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 2147483647);
    EXPECT_EQ(ts.nanos(), 123456000);
}

// Test microsecond multiplication (1 usec = 1000 nanos)
TEST_F(SetGoogleTimestampFromTimevalTest, MicrosecondConversion) {
    tv.tv_sec = 42;
    tv.tv_usec = 1;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.nanos(), 1000);
}

// Test with typical Unix timestamp
TEST_F(SetGoogleTimestampFromTimevalTest, TypicalUnixTimestamp) {
    tv.tv_sec = 1609459200;  // 2021-01-01 00:00:00 UTC
    tv.tv_usec = 250000;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 1609459200);
    EXPECT_EQ(ts.nanos(), 250000000);
}

// Test with small microsecond values
TEST_F(SetGoogleTimestampFromTimevalTest, SmallMicroseconds) {
    tv.tv_sec = 500;
    tv.tv_usec = 10;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 500);
    EXPECT_EQ(ts.nanos(), 10000);
}

// Test with sequential microsecond values
TEST_F(SetGoogleTimestampFromTimevalTest, SequentialMicroseconds) {
    for (int usec = 0; usec <= 999999; usec += 100000) {
        tv.tv_sec = 1000;
        tv.tv_usec = usec;

        set_google_timestamp_from_timeval(tv, &ts);

        EXPECT_EQ(ts.seconds(), 1000);
        EXPECT_EQ(ts.nanos(), usec * 1000);
    }
}

// Test suite for set_timeval_from_google_timestamp
class SetTimevalFromGoogleTimestampTest : public ::testing::Test {
protected:
    google::protobuf::Timestamp ts;
    struct timeval tv;

    void SetUp() override {
        ts.Clear();
        memset(&tv, 0, sizeof(tv));
    }
};

// Test basic conversion from timestamp to timeval
TEST_F(SetTimevalFromGoogleTimestampTest, BasicConversion) {
    ts.set_seconds(1234567890);
    ts.set_nanos(500000000);

    set_timeval_from_google_timestamp(ts, &tv);

    EXPECT_EQ(tv.tv_sec, 1234567890);
    EXPECT_EQ(tv.tv_usec, 500000);
}

// Test with zero values
TEST_F(SetTimevalFromGoogleTimestampTest, ZeroValues) {
    ts.set_seconds(0);
    ts.set_nanos(0);

    set_timeval_from_google_timestamp(ts, &tv);

    EXPECT_EQ(tv.tv_sec, 0);
    EXPECT_EQ(tv.tv_usec, 0);
}

// Test with maximum nanos value (999999000)
TEST_F(SetTimevalFromGoogleTimestampTest, MaxNanos) {
    ts.set_seconds(1000000000);
    ts.set_nanos(999999000);

    set_timeval_from_google_timestamp(ts, &tv);

    EXPECT_EQ(tv.tv_sec, 1000000000);
    EXPECT_EQ(tv.tv_usec, 999999);
}

// Test with minimum nanos value (1000 = 1 microsecond)
TEST_F(SetTimevalFromGoogleTimestampTest, MinNanos) {
    ts.set_seconds(100);
    ts.set_nanos(1000);

    set_timeval_from_google_timestamp(ts, &tv);

    EXPECT_EQ(tv.tv_sec, 100);
    EXPECT_EQ(tv.tv_usec, 1);
}

// Test with large seconds value
TEST_F(SetTimevalFromGoogleTimestampTest, LargeSeconds) {
    ts.set_seconds(2147483647);  // INT_MAX
    ts.set_nanos(123456000);

    set_timeval_from_google_timestamp(ts, &tv);

    EXPECT_EQ(tv.tv_sec, 2147483647);
    EXPECT_EQ(tv.tv_usec, 123456);
}

// Test nanos to microseconds conversion (1000 nanos = 1 usec)
TEST_F(SetTimevalFromGoogleTimestampTest, NanosConversion) {
    ts.set_seconds(42);
    ts.set_nanos(5000);

    set_timeval_from_google_timestamp(ts, &tv);

    EXPECT_EQ(tv.tv_usec, 5);
}

// Test with typical Unix timestamp
TEST_F(SetTimevalFromGoogleTimestampTest, TypicalUnixTimestamp) {
    ts.set_seconds(1609459200);  // 2021-01-01 00:00:00 UTC
    ts.set_nanos(250000000);

    set_timeval_from_google_timestamp(ts, &tv);

    EXPECT_EQ(tv.tv_sec, 1609459200);
    EXPECT_EQ(tv.tv_usec, 250000);
}

// Test with small nanos values
TEST_F(SetTimevalFromGoogleTimestampTest, SmallNanos) {
    ts.set_seconds(500);
    ts.set_nanos(10000);

    set_timeval_from_google_timestamp(ts, &tv);

    EXPECT_EQ(tv.tv_sec, 500);
    EXPECT_EQ(tv.tv_usec, 10);
}

// Test with sequential nanos values
TEST_F(SetTimevalFromGoogleTimestampTest, SequentialNanos) {
    for (int nanos = 0; nanos <= 999999000; nanos += 100000000) {
        ts.set_seconds(1000);
        ts.set_nanos(nanos);

        set_timeval_from_google_timestamp(ts, &tv);

        EXPECT_EQ(tv.tv_sec, 1000);
        EXPECT_EQ(tv.tv_usec, nanos / 1000);
    }
}

// Test roundtrip conversion: timeval -> timestamp -> timeval
class TimestampRoundtripTest : public ::testing::Test {
protected:
    google::protobuf::Timestamp ts;
    struct timeval tv_original;
    struct timeval tv_result;

    void SetUp() override {
        ts.Clear();
        memset(&tv_original, 0, sizeof(tv_original));
        memset(&tv_result, 0, sizeof(tv_result));
    }
};

TEST_F(TimestampRoundtripTest, RoundtripBasic) {
    tv_original.tv_sec = 1234567890;
    tv_original.tv_usec = 500000;

    set_google_timestamp_from_timeval(tv_original, &ts);
    set_timeval_from_google_timestamp(ts, &tv_result);

    EXPECT_EQ(tv_result.tv_sec, tv_original.tv_sec);
    EXPECT_EQ(tv_result.tv_usec, tv_original.tv_usec);
}

TEST_F(TimestampRoundtripTest, RoundtripZero) {
    tv_original.tv_sec = 0;
    tv_original.tv_usec = 0;

    set_google_timestamp_from_timeval(tv_original, &ts);
    set_timeval_from_google_timestamp(ts, &tv_result);

    EXPECT_EQ(tv_result.tv_sec, 0);
    EXPECT_EQ(tv_result.tv_usec, 0);
}

TEST_F(TimestampRoundtripTest, RoundtripMaxValues) {
    tv_original.tv_sec = 2147483647;
    tv_original.tv_usec = 999999;

    set_google_timestamp_from_timeval(tv_original, &ts);
    set_timeval_from_google_timestamp(ts, &tv_result);

    EXPECT_EQ(tv_result.tv_sec, 2147483647);
    EXPECT_EQ(tv_result.tv_usec, 999999);
}

// Test suite for label_add_to_map_callback
class LabelAddToMapCallbackTest : public ::testing::Test {
protected:
    google::protobuf::Map<std::string, std::string> test_map;

    void SetUp() override {
        test_map.clear();
    }
};

// Test adding a single label
TEST_F(LabelAddToMapCallbackTest, AddSingleLabel) {
    const char *name = "test_name";
    const char *value = "test_value";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map.size(), 1);
    EXPECT_EQ(test_map[name], "test_value");
}

// Test adding multiple labels
TEST_F(LabelAddToMapCallbackTest, AddMultipleLabels) {
    const char *name1 = "label1";
    const char *value1 = "value1";
    const char *name2 = "label2";
    const char *value2 = "value2";

    label_add_to_map_callback(name1, value1, 0, &test_map);
    int result = label_add_to_map_callback(name2, value2, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map.size(), 2);
    EXPECT_EQ(test_map[name1], "value1");
    EXPECT_EQ(test_map[name2], "value2");
}

// Test with empty name
TEST_F(LabelAddToMapCallbackTest, EmptyName) {
    const char *name = "";
    const char *value = "test_value";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map.size(), 1);
    EXPECT_EQ(test_map[""], "test_value");
}

// Test with empty value
TEST_F(LabelAddToMapCallbackTest, EmptyValue) {
    const char *name = "test_name";
    const char *value = "";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map.size(), 1);
    EXPECT_EQ(test_map[name], "");
}

// Test with both empty
TEST_F(LabelAddToMapCallbackTest, BothEmpty) {
    const char *name = "";
    const char *value = "";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map.size(), 1);
    EXPECT_EQ(test_map[""], "");
}

// Test with special characters in name
TEST_F(LabelAddToMapCallbackTest, SpecialCharactersInName) {
    const char *name = "test_name-with.special@chars";
    const char *value = "normal_value";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map.size(), 1);
    EXPECT_EQ(test_map[name], "normal_value");
}

// Test with special characters in value
TEST_F(LabelAddToMapCallbackTest, SpecialCharactersInValue) {
    const char *name = "test_name";
    const char *value = "value with spaces and !@#$%^&*()";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map.size(), 1);
    EXPECT_EQ(test_map[name], "value with spaces and !@#$%^&*()");
}

// Test with numeric strings
TEST_F(LabelAddToMapCallbackTest, NumericStrings) {
    const char *name = "123";
    const char *value = "456";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map["123"], "456");
}

// Test duplicate key overwrites
TEST_F(LabelAddToMapCallbackTest, DuplicateKeyOverwrites) {
    const char *name = "duplicate_key";
    const char *value1 = "first_value";
    const char *value2 = "second_value";

    label_add_to_map_callback(name, value1, 0, &test_map);
    int result = label_add_to_map_callback(name, value2, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map.size(), 1);
    EXPECT_EQ(test_map[name], "second_value");
}

// Test with different RRDLABEL_SRC values (parameter is unused but should be tested)
TEST_F(LabelAddToMapCallbackTest, DifferentRRDLabelSrcValues) {
    const char *name = "test_label";
    const char *value = "test_value";

    // Test with different values (the parameter is marked __maybe_unused)
    int result1 = label_add_to_map_callback(name, value, 0, &test_map);
    test_map.clear();

    int result2 = label_add_to_map_callback(name, value, 1, &test_map);
    test_map.clear();

    int result3 = label_add_to_map_callback(name, value, 2, &test_map);

    EXPECT_EQ(result1, 1);
    EXPECT_EQ(result2, 1);
    EXPECT_EQ(result3, 1);
    EXPECT_EQ(test_map.size(), 1);
}

// Test return value is always 1
TEST_F(LabelAddToMapCallbackTest, ReturnValueAlways1) {
    for (int i = 0; i < 10; ++i) {
        test_map.clear();
        std::string name = "label_" + std::to_string(i);
        std::string value = "value_" + std::to_string(i);

        int result = label_add_to_map_callback(name.c_str(), value.c_str(), 0, &test_map);
        EXPECT_EQ(result, 1);
    }
}

// Test with very long strings
TEST_F(LabelAddToMapCallbackTest, VeryLongStrings) {
    std::string long_name(1000, 'a');
    std::string long_value(5000, 'b');

    int result = label_add_to_map_callback(long_name.c_str(), long_value.c_str(), 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map[long_name], long_value);
}

// Test with Unicode characters
TEST_F(LabelAddToMapCallbackTest, UnicodeCharacters) {
    const char *name = "ñame_with_unicode_テスト";
    const char *value = "valüe_with_üñícödé_日本語";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(test_map[name], value);
}

// Test preserving insertion order with multiple additions
TEST_F(LabelAddToMapCallbackTest, MultipleInsertsPreserved) {
    std::vector<std::pair<std::string, std::string>> pairs = {
        {"key1", "val1"},
        {"key2", "val2"},
        {"key3", "val3"},
        {"key4", "val4"}
    };

    for (const auto& p : pairs) {
        label_add_to_map_callback(p.first.c_str(), p.second.c_str(), 0, &test_map);
    }

    EXPECT_EQ(test_map.size(), 4);
    for (const auto& p : pairs) {
        EXPECT_EQ(test_map[p.first], p.second);
    }
}

// Edge case: Test with C++ string containing null terminator handling
TEST_F(LabelAddToMapCallbackTest, StringsWithNullContent) {
    const char *name = "test";
    const char *value = "value";

    int result = label_add_to_map_callback(name, value, 0, &test_map);

    EXPECT_EQ(result, 1);
    EXPECT_NE(test_map.find(name), test_map.end());
}

// Test map pointer is correctly cast
TEST_F(LabelAddToMapCallbackTest, MapPointerCasting) {
    void *data = &test_map;
    auto *map = (google::protobuf::Map<std::string, std::string> *)data;

    int result = label_add_to_map_callback("test", "value", 0, data);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(map->size(), 1);
    EXPECT_EQ((*map)["test"], "value");
}

// Test boundary condition: maximum usec value before overflow
TEST_F(SetGoogleTimestampFromTimevalTest, BoundaryMaxUsec) {
    tv.tv_sec = 1000;
    tv.tv_usec = 999999;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 1000);
    EXPECT_EQ(ts.nanos(), 999999000);
}

// Test boundary condition: minimum usec after zero
TEST_F(SetGoogleTimestampFromTimevalTest, BoundaryMinUsec) {
    tv.tv_sec = 1000;
    tv.tv_usec = 1;

    set_google_timestamp_from_timeval(tv, &ts);

    EXPECT_EQ(ts.seconds(), 1000);
    EXPECT_EQ(ts.nanos(), 1000);
}