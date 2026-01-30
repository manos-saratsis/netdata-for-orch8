// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>
#include <cstdint>

extern "C" {
#include "alarm_stream.h"
}

#include "proto/alarm/v1/stream.pb.h"

using namespace alarms::v1;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

// Helper function to serialize a protobuf message
template <typename T>
std::string SerializeMessage(T& msg) {
    std::string result;
    msg.SerializeToString(&result);
    return result;
}

class AlarmStreamTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize any test data if needed
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

// ============================================================================
// Tests for parse_start_alarm_streaming
// ============================================================================

class ParseStartAlarmStreamingTest : public AlarmStreamTest {};

TEST_F(ParseStartAlarmStreamingTest, should_parse_valid_start_alarm_streaming_message) {
    StartAlarmStreaming msg;
    msg.set_node_id("test-node-123");
    msg.set_version(42);
    msg.set_resets(true);

    std::string serialized = SerializeMessage(msg);

    struct start_alarm_streaming result = parse_start_alarm_streaming(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_STREQ(result.node_id, "test-node-123");
    EXPECT_EQ(result.version, 42);
    EXPECT_TRUE(result.resets);

    free(result.node_id);
}

TEST_F(ParseStartAlarmStreamingTest, should_parse_with_empty_node_id) {
    StartAlarmStreaming msg;
    msg.set_node_id("");
    msg.set_version(0);
    msg.set_resets(false);

    std::string serialized = SerializeMessage(msg);

    struct start_alarm_streaming result = parse_start_alarm_streaming(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_STREQ(result.node_id, "");
    EXPECT_EQ(result.version, 0);
    EXPECT_FALSE(result.resets);

    free(result.node_id);
}

TEST_F(ParseStartAlarmStreamingTest, should_parse_with_max_version) {
    StartAlarmStreaming msg;
    msg.set_node_id("node-max");
    msg.set_version(UINT64_MAX);
    msg.set_resets(true);

    std::string serialized = SerializeMessage(msg);

    struct start_alarm_streaming result = parse_start_alarm_streaming(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_STREQ(result.node_id, "node-max");
    EXPECT_EQ(result.version, UINT64_MAX);
    EXPECT_TRUE(result.resets);

    free(result.node_id);
}

TEST_F(ParseStartAlarmStreamingTest, should_return_zeroed_struct_on_invalid_data) {
    const char *invalid_data = "invalid protobuf data";
    
    struct start_alarm_streaming result = parse_start_alarm_streaming(
        invalid_data, 
        strlen(invalid_data)
    );

    EXPECT_EQ(result.node_id, nullptr);
    EXPECT_EQ(result.version, 0);
    EXPECT_FALSE(result.resets);
}

TEST_F(ParseStartAlarmStreamingTest, should_handle_empty_input) {
    struct start_alarm_streaming result = parse_start_alarm_streaming("", 0);

    EXPECT_EQ(result.node_id, nullptr);
    EXPECT_EQ(result.version, 0);
    EXPECT_FALSE(result.resets);
}

TEST_F(ParseStartAlarmStreamingTest, should_parse_with_long_node_id) {
    std::string long_node_id(1000, 'a');
    StartAlarmStreaming msg;
    msg.set_node_id(long_node_id);
    msg.set_version(999);
    msg.set_resets(true);

    std::string serialized = SerializeMessage(msg);

    struct start_alarm_streaming result = parse_start_alarm_streaming(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_STREQ(result.node_id, long_node_id.c_str());
    EXPECT_EQ(result.version, 999);

    free(result.node_id);
}

TEST_F(ParseStartAlarmStreamingTest, should_parse_resets_false) {
    StartAlarmStreaming msg;
    msg.set_node_id("test");
    msg.set_version(1);
    msg.set_resets(false);

    std::string serialized = SerializeMessage(msg);

    struct start_alarm_streaming result = parse_start_alarm_streaming(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_FALSE(result.resets);

    free(result.node_id);
}

// ============================================================================
// Tests for parse_send_alarm_checkpoint
// ============================================================================

class ParseSendAlarmCheckpointTest : public AlarmStreamTest {};

TEST_F(ParseSendAlarmCheckpointTest, should_parse_valid_send_alarm_checkpoint) {
    SendAlarmCheckpoint msg;
    msg.set_node_id("node-xyz");
    msg.set_claim_id("claim-456");
    msg.set_version(100);

    std::string serialized = SerializeMessage(msg);

    struct send_alarm_checkpoint result = parse_send_alarm_checkpoint(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_STREQ(result.node_id, "node-xyz");
    EXPECT_STREQ(result.claim_id, "claim-456");
    EXPECT_EQ(result.version, 100);

    free(result.node_id);
    free(result.claim_id);
}

TEST_F(ParseSendAlarmCheckpointTest, should_parse_with_empty_fields) {
    SendAlarmCheckpoint msg;
    msg.set_node_id("");
    msg.set_claim_id("");
    msg.set_version(0);

    std::string serialized = SerializeMessage(msg);

    struct send_alarm_checkpoint result = parse_send_alarm_checkpoint(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_STREQ(result.node_id, "");
    EXPECT_STREQ(result.claim_id, "");
    EXPECT_EQ(result.version, 0);

    free(result.node_id);
    free(result.claim_id);
}

TEST_F(ParseSendAlarmCheckpointTest, should_parse_with_max_version) {
    SendAlarmCheckpoint msg;
    msg.set_node_id("node");
    msg.set_claim_id("claim");
    msg.set_version(UINT64_MAX);

    std::string serialized = SerializeMessage(msg);

    struct send_alarm_checkpoint result = parse_send_alarm_checkpoint(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_EQ(result.version, UINT64_MAX);

    free(result.node_id);
    free(result.claim_id);
}

TEST_F(ParseSendAlarmCheckpointTest, should_return_zeroed_struct_on_invalid_data) {
    const char *invalid_data = "this is not valid protobuf";
    
    struct send_alarm_checkpoint result = parse_send_alarm_checkpoint(
        invalid_data, 
        strlen(invalid_data)
    );

    EXPECT_EQ(result.node_id, nullptr);
    EXPECT_EQ(result.claim_id, nullptr);
    EXPECT_EQ(result.version, 0);
}

TEST_F(ParseSendAlarmCheckpointTest, should_handle_empty_input) {
    struct send_alarm_checkpoint result = parse_send_alarm_checkpoint("", 0);

    EXPECT_EQ(result.node_id, nullptr);
    EXPECT_EQ(result.claim_id, nullptr);
    EXPECT_EQ(result.version, 0);
}

TEST_F(ParseSendAlarmCheckpointTest, should_parse_with_long_ids) {
    std::string long_node_id(2000, 'x');
    std::string long_claim_id(2000, 'y');
    
    SendAlarmCheckpoint msg;
    msg.set_node_id(long_node_id);
    msg.set_claim_id(long_claim_id);
    msg.set_version(777);

    std::string serialized = SerializeMessage(msg);

    struct send_alarm_checkpoint result = parse_send_alarm_checkpoint(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_STREQ(result.node_id, long_node_id.c_str());
    EXPECT_STREQ(result.claim_id, long_claim_id.c_str());
    EXPECT_EQ(result.version, 777);

    free(result.node_id);
    free(result.claim_id);
}

// ============================================================================
// Tests for aclk_alarm_status_to_proto (via generate_alarm_log_entry)
// ============================================================================

class AlarmStatusConversionTest : public AlarmStreamTest {};

TEST_F(AlarmStatusConversionTest, should_convert_all_alarm_statuses) {
    // We test this indirectly through generate_alarm_log_entry
    struct alarm_log_entry entry = {};
    entry.node_id = (char*)"test-node";
    entry.claim_id = (char*)"test-claim";
    entry.chart = (char*)"test-chart";
    entry.name = (char*)"test-name";
    
    size_t len = 0;

    // Test NULL status
    entry.status = ALARM_STATUS_NULL;
    entry.old_status = ALARM_STATUS_NULL;
    char *bin = generate_alarm_log_entry(&len, &entry);
    EXPECT_NE(bin, nullptr);
    EXPECT_GT(len, 0);
    free(bin);

    // Test UNKNOWN status
    entry.status = ALARM_STATUS_UNKNOWN;
    entry.old_status = ALARM_STATUS_UNKNOWN;
    bin = generate_alarm_log_entry(&len, &entry);
    EXPECT_NE(bin, nullptr);
    free(bin);

    // Test REMOVED status
    entry.status = ALARM_STATUS_REMOVED;
    entry.old_status = ALARM_STATUS_REMOVED;
    bin = generate_alarm_log_entry(&len, &entry);
    EXPECT_NE(bin, nullptr);
    free(bin);

    // Test NOT_A_NUMBER status
    entry.status = ALARM_STATUS_NOT_A_NUMBER;
    entry.old_status = ALARM_STATUS_NOT_A_NUMBER;
    bin = generate_alarm_log_entry(&len, &entry);
    EXPECT_NE(bin, nullptr);
    free(bin);

    // Test CLEAR status
    entry.status = ALARM_STATUS_CLEAR;
    entry.old_status = ALARM_STATUS_CLEAR;
    bin = generate_alarm_log_entry(&len, &entry);
    EXPECT_NE(bin, nullptr);
    free(bin);

    // Test WARNING status
    entry.status = ALARM_STATUS_WARNING;
    entry.old_status = ALARM_STATUS_WARNING;
    bin = generate_alarm_log_entry(&len, &entry);
    EXPECT_NE(bin, nullptr);
    free(bin);

    // Test CRITICAL status
    entry.status = ALARM_STATUS_CRITICAL;
    entry.old_status = ALARM_STATUS_CRITICAL;
    bin = generate_alarm_log_entry(&len, &entry);
    EXPECT_NE(bin, nullptr);
    free(bin);
}

// ============================================================================
// Tests for destroy_alarm_log_entry
// ============================================================================

class DestroyAlarmLogEntryTest : public AlarmStreamTest {};

TEST_F(DestroyAlarmLogEntryTest, should_free_all_string_fields) {
    struct alarm_log_entry entry = {};
    entry.chart = strdup("test-chart");
    entry.name = strdup("test-name");
    entry.config_hash = strdup("hash123");
    entry.timezone = strdup("UTC");
    entry.exec_path = strdup("/path/to/exec");
    entry.conf_source = strdup("conf-source");
    entry.command = strdup("test-command");
    entry.value_string = strdup("100");
    entry.old_value_string = strdup("50");
    entry.rendered_info = strdup("rendered");
    entry.chart_context = strdup("context");
    entry.transition_id = strdup("trans-123");
    entry.chart_name = strdup("chart-name");
    entry.summary = strdup("summary");

    // Should not crash
    destroy_alarm_log_entry(&entry);

    // All pointers should be freed (they may not be set to NULL, but memory is freed)
    // This test verifies no crashes occur
    EXPECT_TRUE(true);
}

TEST_F(DestroyAlarmLogEntryTest, should_handle_null_fields) {
    struct alarm_log_entry entry = {};
    memset(&entry, 0, sizeof(entry));
    
    // Should not crash with NULL fields
    destroy_alarm_log_entry(&entry);

    EXPECT_TRUE(true);
}

TEST_F(DestroyAlarmLogEntryTest, should_handle_partial_null_fields) {
    struct alarm_log_entry entry = {};
    entry.chart = strdup("chart");
    entry.name = nullptr;
    entry.config_hash = strdup("hash");
    entry.timezone = nullptr;
    
    destroy_alarm_log_entry(&entry);

    EXPECT_TRUE(true);
}

// ============================================================================
// Tests for generate_alarm_log_entry
// ============================================================================

class GenerateAlarmLogEntryTest : public AlarmStreamTest {};

TEST_F(GenerateAlarmLogEntryTest, should_generate_valid_alarm_log_entry) {
    struct alarm_log_entry entry = {};
    entry.node_id = (char*)"node-1";
    entry.claim_id = (char*)"claim-1";
    entry.chart = (char*)"chart-1";
    entry.name = (char*)"alarm-1";
    entry.family = (char*)"family-1";
    entry.when = 1000;
    entry.config_hash = (char*)"hash-1";
    entry.utc_offset = 3600;
    entry.timezone = (char*)"GMT";
    entry.exec_path = (char*)"exec-1";
    entry.conf_source = (char*)"source-1";
    entry.command = (char*)"cmd-1";
    entry.duration = 100;
    entry.non_clear_duration = 50;
    entry.status = ALARM_STATUS_CRITICAL;
    entry.old_status = ALARM_STATUS_WARNING;
    entry.delay = 10;
    entry.delay_up_to_timestamp = 2000;
    entry.last_repeat = 500;
    entry.silenced = 1;
    entry.value_string = (char*)"99.5";
    entry.old_value_string = (char*)"98.5";
    entry.value = 99.5;
    entry.old_value = 98.5;
    entry.updated = 1;
    entry.rendered_info = (char*)"info";
    entry.chart_context = (char*)"context";
    entry.chart_name = (char*)"name";
    entry.event_id = 12345;
    entry.version = 1;
    entry.transition_id = (char*)"trans";
    entry.summary = (char*)"summary";
    entry.health_log_id = 111;
    entry.alarm_id = 222;
    entry.unique_id = 333;
    entry.sequence_id = 444;

    size_t len = 0;
    char *bin = generate_alarm_log_entry(&len, &entry);

    EXPECT_NE(bin, nullptr);
    EXPECT_GT(len, 0);

    // Verify we can parse it back
    AlarmLogEntry parsed_msg;
    EXPECT_TRUE(parsed_msg.ParseFromArray(bin, len));
    EXPECT_STREQ(parsed_msg.node_id().c_str(), "node-1");
    EXPECT_STREQ(parsed_msg.claim_id().c_str(), "claim-1");
    EXPECT_STREQ(parsed_msg.chart().c_str(), "chart-1");
    EXPECT_STREQ(parsed_msg.name().c_str(), "alarm-1");
    EXPECT_EQ(parsed_msg.when(), 1000);
    EXPECT_EQ(parsed_msg.status(), ALARM_STATUS_CRITICAL);
    EXPECT_EQ(parsed_msg.old_status(), ALARM_STATUS_WARNING);
    EXPECT_EQ(parsed_msg.value(), 99.5);
    EXPECT_EQ(parsed_msg.old_value(), 98.5);

    free(bin);
}

TEST_F(GenerateAlarmLogEntryTest, should_generate_with_minimal_fields) {
    struct alarm_log_entry entry = {};
    entry.node_id = (char*)"node";
    entry.claim_id = (char*)"claim";
    entry.chart = (char*)"chart";
    entry.name = (char*)"name";
    entry.status = ALARM_STATUS_CLEAR;
    entry.old_status = ALARM_STATUS_CLEAR;

    size_t len = 0;
    char *bin = generate_alarm_log_entry(&len, &entry);

    EXPECT_NE(bin, nullptr);
    EXPECT_GT(len, 0);

    free(bin);
}

TEST_F(GenerateAlarmLogEntryTest, should_generate_with_null_optional_strings) {
    struct alarm_log_entry entry = {};
    entry.node_id = (char*)"node";
    entry.claim_id = (char*)"claim";
    entry.chart = (char*)"chart";
    entry.name = (char*)"name";
    entry.value_string = nullptr;
    entry.old_value_string = nullptr;
    entry.status = ALARM_STATUS_CLEAR;
    entry.old_status = ALARM_STATUS_CLEAR;

    size_t len = 0;
    char *bin = generate_alarm_log_entry(&len, &entry);

    EXPECT_NE(bin, nullptr);
    EXPECT_GT(len, 0);

    free(bin);
}

TEST_F(GenerateAlarmLogEntryTest, should_generate_with_max_numeric_values) {
    struct alarm_log_entry entry = {};
    entry.node_id = (char*)"node";
    entry.claim_id = (char*)"claim";
    entry.chart = (char*)"chart";
    entry.name = (char*)"name";
    entry.when = UINT64_MAX;
    entry.utc_offset = INT32_MAX;
    entry.duration = UINT32_MAX;
    entry.non_clear_duration = UINT32_MAX;
    entry.delay = UINT64_MAX;
    entry.delay_up_to_timestamp = UINT64_MAX;
    entry.last_repeat = UINT64_MAX;
    entry.value = 1e308;
    entry.old_value = 1e308;
    entry.event_id = UINT64_MAX;
    entry.version = UINT64_MAX;
    entry.status = ALARM_STATUS_CRITICAL;
    entry.old_status = ALARM_STATUS_CRITICAL;

    size_t len = 0;
    char *bin = generate_alarm_log_entry(&len, &entry);

    EXPECT_NE(bin, nullptr);
    EXPECT_GT(len, 0);

    free(bin);
}

TEST_F(GenerateAlarmLogEntryTest, should_generate_with_empty_strings) {
    struct alarm_log_entry entry = {};
    entry.node_id = (char*)"";
    entry.claim_id = (char*)"";
    entry.chart = (char*)"";
    entry.name = (char*)"";
    entry.config_hash = (char*)"";
    entry.timezone = (char*)"";
    entry.status = ALARM_STATUS_CLEAR;
    entry.old_status = ALARM_STATUS_CLEAR;

    size_t len = 0;
    char *bin = generate_alarm_log_entry(&len, &entry);

    EXPECT_NE(bin, nullptr);

    free(bin);
}

TEST_F(GenerateAlarmLogEntryTest, should_generate_with_special_characters) {
    struct alarm_log_entry entry = {};
    entry.node_id = (char*)"node\n\r\t";
    entry.claim_id = (char*)"claim\"'\\";
    entry.chart = (char*)"chart<>&";
    entry.name = (char*)"name\x00test";
    entry.status = ALARM_STATUS_WARNING;
    entry.old_status = ALARM_STATUS_WARNING;

    size_t len = 0;
    char *bin = generate_alarm_log_entry(&len, &entry);

    EXPECT_NE(bin, nullptr);

    free(bin);
}

// ============================================================================
// Tests for generate_alarm_checkpoint
// ============================================================================

class GenerateAlarmCheckpointTest : public AlarmStreamTest {};

TEST_F(GenerateAlarmCheckpointTest, should_generate_valid_alarm_checkpoint) {
    struct alarm_checkpoint data = {};
    data.node_id = (char*)"test-node";
    data.claim_id = (char*)"test-claim";
    data.checksum = (char*)"abc123def456";

    size_t len = 0;
    char *bin = generate_alarm_checkpoint(&len, &data);

    EXPECT_NE(bin, nullptr);
    EXPECT_GT(len, 0);

    // Verify we can parse it back
    AlarmCheckpoint parsed_msg;
    EXPECT_TRUE(parsed_msg.ParseFromArray(bin, len));
    EXPECT_STREQ(parsed_msg.node_id().c_str(), "test-node");
    EXPECT_STREQ(parsed_msg.claim_id().c_str(), "test-claim");
    EXPECT_STREQ(parsed_msg.checksum().c_str(), "abc123def456");

    free(bin);
}

TEST_F(GenerateAlarmCheckpointTest, should_generate_with_empty_fields) {
    struct alarm_checkpoint data = {};
    data.node_id = (char*)"";
    data.claim_id = (char*)"";
    data.checksum = (char*)"";

    size_t len = 0;
    char *bin = generate_alarm_checkpoint(&len, &data);

    EXPECT_NE(bin, nullptr);
    EXPECT_GT(len, 0);

    free(bin);
}

TEST_F(GenerateAlarmCheckpointTest, should_generate_with_long_checksum) {
    std::string long_checksum(10000, 'x');
    
    struct alarm_checkpoint data = {};
    data.node_id = (char*)"node";
    data.claim_id = (char*)"claim";
    data.checksum = (char*)long_checksum.c_str();

    size_t len = 0;
    char *bin = generate_alarm_checkpoint(&len, &data);

    EXPECT_NE(bin, nullptr);

    AlarmCheckpoint parsed_msg;
    EXPECT_TRUE(parsed_msg.ParseFromArray(bin, len));
    EXPECT_STREQ(parsed_msg.checksum().c_str(), long_checksum.c_str());

    free(bin);
}

TEST_F(GenerateAlarmCheckpointTest, should_generate_with_special_characters_in_checksum) {
    struct alarm_checkpoint data = {};
    data.node_id = (char*)"node";
    data.claim_id = (char*)"claim";
    data.checksum = (char*)"sha256:abcdef0123456789!@#$%^&*()";

    size_t len = 0;
    char *bin = generate_alarm_checkpoint(&len, &data);

    EXPECT_NE(bin, nullptr);

    free(bin);
}

TEST_F(GenerateAlarmCheckpointTest, should_generate_with_null_pointers) {
    struct alarm_checkpoint data = {};
    data.node_id = nullptr;
    data.claim_id = nullptr;
    data.checksum = nullptr;

    size_t len = 0;
    char *bin = generate_alarm_checkpoint(&len, &data);

    EXPECT_NE(bin, nullptr);

    free(bin);
}

// ============================================================================
// Tests for parse_send_alarm_snapshot
// ============================================================================

class ParseSendAlarmSnapshotTest : public AlarmStreamTest {};

TEST_F(ParseSendAlarmSnapshotTest, should_parse_valid_send_alarm_snapshot) {
    SendAlarmSnapshot msg;
    msg.set_node_id("snapshot-node");
    msg.set_claim_id("snapshot-claim");
    msg.set_snapshot_uuid("uuid-1234-5678");

    std::string serialized = SerializeMessage(msg);

    struct send_alarm_snapshot *result = parse_send_alarm_snapshot(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result->node_id, "snapshot-node");
    EXPECT_STREQ(result->claim_id, "snapshot-claim");
    EXPECT_STREQ(result->snapshot_uuid, "uuid-1234-5678");

    destroy_send_alarm_snapshot(result);
}

TEST_F(ParseSendAlarmSnapshotTest, should_parse_with_empty_fields) {
    SendAlarmSnapshot msg;
    msg.set_node_id("");
    msg.set_claim_id("");
    msg.set_snapshot_uuid("");

    std::string serialized = SerializeMessage(msg);

    struct send_alarm_snapshot *result = parse_send_alarm_snapshot(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result->node_id, "");
    EXPECT_STREQ(result->claim_id, "");
    EXPECT_STREQ(result->snapshot_uuid, "");

    destroy_send_alarm_snapshot(result);
}

TEST_F(ParseSendAlarmSnapshotTest, should_parse_with_long_uuid) {
    std::string long_uuid(5000, 'f');
    
    SendAlarmSnapshot msg;
    msg.set_node_id("node");
    msg.set_claim_id("claim");
    msg.set_snapshot_uuid(long_uuid);

    std::string serialized = SerializeMessage(msg);

    struct send_alarm_snapshot *result = parse_send_alarm_snapshot(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result->snapshot_uuid, long_uuid.c_str());

    destroy_send_alarm_snapshot(result);
}

TEST_F(ParseSendAlarmSnapshotTest, should_return_null_on_invalid_data) {
    const char *invalid_data = "not protobuf";
    
    struct send_alarm_snapshot *result = parse_send_alarm_snapshot(
        invalid_data, 
        strlen(invalid_data)
    );

    EXPECT_EQ(result, nullptr);
}

TEST_F(ParseSendAlarmSnapshotTest, should_handle_empty_input) {
    struct send_alarm_snapshot *result = parse_send_alarm_snapshot("", 0);

    EXPECT_EQ(result, nullptr);
}

TEST_F(ParseSendAlarmSnapshotTest, should_parse_with_special_characters) {
    SendAlarmSnapshot msg;
    msg.set_node_id("node\n\r");
    msg.set_claim_id("claim<>&");
    msg.set_snapshot_uuid("uuid-!@#$%");

    std::string serialized = SerializeMessage(msg);

    struct send_alarm_snapshot *result = parse_send_alarm_snapshot(
        serialized.c_str(), 
        serialized.size()
    );

    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result->node_id, "node\n\r");
    EXPECT_STREQ(result->claim_id, "claim<>&");
    EXPECT_STREQ(result->snapshot_uuid, "uuid-!@#$%");

    destroy_send_alarm_snapshot(result);
}

// ============================================================================
// Tests for destroy_send_alarm_snapshot
// ============================================================================

class DestroySendAlarmSnapshotTest : public AlarmStreamTest {};

TEST_F(DestroySendAlarmSnapshotTest, should_free_all_fields) {
    struct send_alarm_snapshot *snapshot = 
        (struct send_alarm_snapshot*)calloc(1, sizeof(struct send_alarm_snapshot));
    
    snapshot->node_id = strdup("node");
    snapshot->claim_id = strdup("claim");
    snapshot->snapshot_uuid = strdup("uuid");

    // Should not crash
    destroy_send_alarm_snapshot(snapshot);

    EXPECT_TRUE(true);
}

TEST_F(DestroySendAlarmSnapshotTest, should_handle_null_fields) {
    struct send_alarm_snapshot *snapshot = 
        (struct send_alarm_snapshot*)calloc(1, sizeof(struct send_alarm_snapshot));
    
    snapshot->node_id = nullptr;
    snapshot->claim_id = nullptr;
    snapshot->snapshot_uuid = nullptr;

    destroy_send_alarm_snapshot(snapshot);

    EXPECT_TRUE(true);
}

TEST_F(DestroySendAlarmSnapshotTest, should_handle_partial_null_fields) {
    struct send_alarm_snapshot *snapshot = 
        (struct send_alarm_snapshot*)calloc(1, sizeof(struct send_alarm_snapshot));
    
    snapshot->node_id = strdup("node");
    snapshot->claim_id = nullptr;
    snapshot->snapshot_uuid = strdup("uuid");

    destroy_send_alarm