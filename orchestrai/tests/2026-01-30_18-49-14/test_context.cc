#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>

// Mock for libnetdata functions
extern "C" {
    void* __wrap_malloc(size_t size);
    void* __wrap_calloc(size_t nmemb, size_t size);
    void __wrap_free(void* ptr);
    
    // Forward declarations from context.cc
    typedef void* contexts_snapshot_t;
    typedef void* contexts_updated_t;
    
    struct context_updated {
        const char *id;
        uint64_t version;
        uint64_t first_entry;
        uint64_t last_entry;
        int deleted;
        const char *title;
        uint64_t priority;
        const char *chart_type;
        const char *units;
        const char *family;
    };
    
    // Function declarations
    contexts_snapshot_t contexts_snapshot_new(const char *claim_id, const char *node_id, uint64_t version);
    void contexts_snapshot_delete(contexts_snapshot_t ctxs_snapshot);
    void contexts_snapshot_set_version(contexts_snapshot_t ctxs_snapshot, uint64_t version);
    void contexts_snapshot_add_ctx_update(contexts_snapshot_t ctxs_snapshot, struct context_updated *ctx_update);
    char *contexts_snapshot_2bin(contexts_snapshot_t ctxs_snapshot, size_t *len);
    
    contexts_updated_t contexts_updated_new(const char *claim_id, const char *node_id, uint64_t version_hash, uint64_t created_at);
    void contexts_updated_delete(contexts_updated_t ctxs_updated);
    void contexts_updated_update_version_hash(contexts_updated_t ctxs_updated, uint64_t version_hash);
    void contexts_updated_add_ctx_update(contexts_updated_t ctxs_updated, struct context_updated *ctx_update);
    char *contexts_updated_2bin(contexts_updated_t ctxs_updated, size_t *len);
}

class ContextSnapshotTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup common test data
    }
    
    void TearDown() override {
        // Cleanup
    }
};

class ContextUpdatedTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup common test data
    }
    
    void TearDown() override {
        // Cleanup
    }
};

// ContextsSnapshot Tests
TEST_F(ContextSnapshotTest, contexts_snapshot_new_should_create_snapshot_with_valid_parameters) {
    const char *claim_id = "claim123";
    const char *node_id = "node456";
    uint64_t version = 100;
    
    contexts_snapshot_t snapshot = contexts_snapshot_new(claim_id, node_id, version);
    
    ASSERT_NE(snapshot, nullptr);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_new_should_handle_empty_claim_id) {
    const char *claim_id = "";
    const char *node_id = "node456";
    uint64_t version = 100;
    
    contexts_snapshot_t snapshot = contexts_snapshot_new(claim_id, node_id, version);
    
    ASSERT_NE(snapshot, nullptr);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_new_should_handle_empty_node_id) {
    const char *claim_id = "claim123";
    const char *node_id = "";
    uint64_t version = 100;
    
    contexts_snapshot_t snapshot = contexts_snapshot_new(claim_id, node_id, version);
    
    ASSERT_NE(snapshot, nullptr);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_new_should_handle_zero_version) {
    const char *claim_id = "claim123";
    const char *node_id = "node456";
    uint64_t version = 0;
    
    contexts_snapshot_t snapshot = contexts_snapshot_new(claim_id, node_id, version);
    
    ASSERT_NE(snapshot, nullptr);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_new_should_handle_max_version) {
    const char *claim_id = "claim123";
    const char *node_id = "node456";
    uint64_t version = UINT64_MAX;
    
    contexts_snapshot_t snapshot = contexts_snapshot_new(claim_id, node_id, version);
    
    ASSERT_NE(snapshot, nullptr);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_delete_should_delete_valid_snapshot) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    // Should not crash or throw
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_delete_with_null_pointer) {
    // Edge case: should handle null gracefully
    contexts_snapshot_t snapshot = nullptr;
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_set_version_should_update_version) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    contexts_snapshot_set_version(snapshot, 200);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_set_version_with_zero) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    contexts_snapshot_set_version(snapshot, 0);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_set_version_with_max_value) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    contexts_snapshot_set_version(snapshot, UINT64_MAX);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_add_ctx_update_should_add_single_context) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    struct context_updated ctx_update = {
        .id = "ctx1",
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 0,
        .title = "Test Context",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    contexts_snapshot_add_ctx_update(snapshot, &ctx_update);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_add_ctx_update_with_null_id) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    struct context_updated ctx_update = {
        .id = nullptr,
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 0,
        .title = "Test",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    contexts_snapshot_add_ctx_update(snapshot, &ctx_update);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_add_ctx_update_with_empty_strings) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    struct context_updated ctx_update = {
        .id = "",
        .version = 0,
        .first_entry = 0,
        .last_entry = 0,
        .deleted = 0,
        .title = "",
        .priority = 0,
        .chart_type = "",
        .units = "",
        .family = ""
    };
    
    contexts_snapshot_add_ctx_update(snapshot, &ctx_update);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_add_ctx_update_with_deleted_flag) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    struct context_updated ctx_update = {
        .id = "ctx1",
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 1,
        .title = "Deleted Context",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    contexts_snapshot_add_ctx_update(snapshot, &ctx_update);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_add_multiple_ctx_updates) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    struct context_updated ctx_update1 = {
        .id = "ctx1",
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 0,
        .title = "Context 1",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    struct context_updated ctx_update2 = {
        .id = "ctx2",
        .version = 2,
        .first_entry = 2000,
        .last_entry = 3000,
        .deleted = 0,
        .title = "Context 2",
        .priority = 20,
        .chart_type = "area",
        .units = "packets",
        .family = "network"
    };
    
    contexts_snapshot_add_ctx_update(snapshot, &ctx_update1);
    contexts_snapshot_add_ctx_update(snapshot, &ctx_update2);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_2bin_should_serialize_snapshot) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    struct context_updated ctx_update = {
        .id = "ctx1",
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 0,
        .title = "Test",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    contexts_snapshot_add_ctx_update(snapshot, &ctx_update);
    
    size_t len = 0;
    char *bin = contexts_snapshot_2bin(snapshot, &len);
    
    ASSERT_NE(bin, nullptr);
    ASSERT_GT(len, 0);
    
    free(bin);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_2bin_empty_snapshot) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    size_t len = 0;
    char *bin = contexts_snapshot_2bin(snapshot, &len);
    
    ASSERT_NE(bin, nullptr);
    ASSERT_GT(len, 0);
    
    free(bin);
}

TEST_F(ContextSnapshotTest, contexts_snapshot_2bin_with_large_version) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", UINT64_MAX);
    
    size_t len = 0;
    char *bin = contexts_snapshot_2bin(snapshot, &len);
    
    ASSERT_NE(bin, nullptr);
    ASSERT_GT(len, 0);
    
    free(bin);
}

// ContextsUpdated Tests
TEST_F(ContextUpdatedTest, contexts_updated_new_should_create_updated_with_valid_parameters) {
    const char *claim_id = "claim123";
    const char *node_id = "node456";
    uint64_t version_hash = 789;
    uint64_t created_at = 1234567890;
    
    contexts_updated_t updated = contexts_updated_new(claim_id, node_id, version_hash, created_at);
    
    ASSERT_NE(updated, nullptr);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_new_should_handle_empty_claim_id) {
    contexts_updated_t updated = contexts_updated_new("", "node456", 789, 1234567890);
    
    ASSERT_NE(updated, nullptr);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_new_should_handle_empty_node_id) {
    contexts_updated_t updated = contexts_updated_new("claim123", "", 789, 1234567890);
    
    ASSERT_NE(updated, nullptr);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_new_should_handle_zero_version_hash) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 0, 1234567890);
    
    ASSERT_NE(updated, nullptr);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_new_should_handle_zero_created_at) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 0);
    
    ASSERT_NE(updated, nullptr);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_new_should_handle_max_values) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", UINT64_MAX, UINT64_MAX);
    
    ASSERT_NE(updated, nullptr);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_delete_should_delete_valid_updated) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    // Should not crash or throw
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_delete_with_null_pointer) {
    contexts_updated_t updated = nullptr;
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_update_version_hash_should_update_hash) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    contexts_updated_update_version_hash(updated, 999);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_update_version_hash_to_zero) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    contexts_updated_update_version_hash(updated, 0);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_update_version_hash_to_max) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    contexts_updated_update_version_hash(updated, UINT64_MAX);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_add_ctx_update_should_add_single_context) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    struct context_updated ctx_update = {
        .id = "ctx1",
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 0,
        .title = "Test Context",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    contexts_updated_add_ctx_update(updated, &ctx_update);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_add_ctx_update_with_null_id) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    struct context_updated ctx_update = {
        .id = nullptr,
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 0,
        .title = "Test",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    contexts_updated_add_ctx_update(updated, &ctx_update);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_add_ctx_update_with_empty_strings) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    struct context_updated ctx_update = {
        .id = "",
        .version = 0,
        .first_entry = 0,
        .last_entry = 0,
        .deleted = 0,
        .title = "",
        .priority = 0,
        .chart_type = "",
        .units = "",
        .family = ""
    };
    
    contexts_updated_add_ctx_update(updated, &ctx_update);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_add_ctx_update_with_deleted_flag) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    struct context_updated ctx_update = {
        .id = "ctx1",
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 1,
        .title = "Deleted",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    contexts_updated_add_ctx_update(updated, &ctx_update);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_add_multiple_ctx_updates) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    struct context_updated ctx_update1 = {
        .id = "ctx1",
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 0,
        .title = "Context 1",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    struct context_updated ctx_update2 = {
        .id = "ctx2",
        .version = 2,
        .first_entry = 2000,
        .last_entry = 3000,
        .deleted = 0,
        .title = "Context 2",
        .priority = 20,
        .chart_type = "area",
        .units = "packets",
        .family = "network"
    };
    
    contexts_updated_add_ctx_update(updated, &ctx_update1);
    contexts_updated_add_ctx_update(updated, &ctx_update2);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_add_ctx_update_with_long_strings) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    struct context_updated ctx_update = {
        .id = "very_long_context_id_with_many_characters_and_numbers_0123456789",
        .version = 999,
        .first_entry = 9999999,
        .last_entry = 9999999999,
        .deleted = 0,
        .title = "This is a very long context title with lots of description information",
        .priority = 9999,
        .chart_type = "complicated_chart_type_with_underscores",
        .units = "very_long_unit_name_with_many_characters",
        .family = "a_very_long_family_name_with_lots_of_detail"
    };
    
    contexts_updated_add_ctx_update(updated, &ctx_update);
    
    contexts_updated_delete(updated);
}

TEST_F(ContextUpdatedTest, contexts_updated_2bin_should_serialize_updated) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    struct context_updated ctx_update = {
        .id = "ctx1",
        .version = 1,
        .first_entry = 1000,
        .last_entry = 2000,
        .deleted = 0,
        .title = "Test",
        .priority = 10,
        .chart_type = "line",
        .units = "bytes",
        .family = "memory"
    };
    
    contexts_updated_add_ctx_update(updated, &ctx_update);
    
    size_t len = 0;
    char *bin = contexts_updated_2bin(updated, &len);
    
    ASSERT_NE(bin, nullptr);
    ASSERT_GT(len, 0);
    
    free(bin);
}

TEST_F(ContextUpdatedTest, contexts_updated_2bin_empty_updated) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    size_t len = 0;
    char *bin = contexts_updated_2bin(updated, &len);
    
    ASSERT_NE(bin, nullptr);
    ASSERT_GT(len, 0);
    
    free(bin);
}

TEST_F(ContextUpdatedTest, contexts_updated_2bin_with_max_values) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", UINT64_MAX, UINT64_MAX);
    
    size_t len = 0;
    char *bin = contexts_updated_2bin(updated, &len);
    
    ASSERT_NE(bin, nullptr);
    ASSERT_GT(len, 0);
    
    free(bin);
}

TEST_F(ContextUpdatedTest, contexts_updated_2bin_with_multiple_contexts) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    for (int i = 0; i < 10; i++) {
        struct context_updated ctx_update = {
            .id = "ctx_id",
            .version = (uint64_t)i,
            .first_entry = 1000 + i * 100,
            .last_entry = 2000 + i * 100,
            .deleted = i % 2,
            .title = "Context",
            .priority = (uint64_t)i * 10,
            .chart_type = "line",
            .units = "bytes",
            .family = "memory"
        };
        
        contexts_updated_add_ctx_update(updated, &ctx_update);
    }
    
    size_t len = 0;
    char *bin = contexts_updated_2bin(updated, &len);
    
    ASSERT_NE(bin, nullptr);
    ASSERT_GT(len, 0);
    
    free(bin);
}

TEST_F(ContextSnapshotTest, fill_ctx_updated_all_fields) {
    contexts_snapshot_t snapshot = contexts_snapshot_new("claim123", "node456", 100);
    
    struct context_updated ctx_update = {
        .id = "test_id_123",
        .version = 42,
        .first_entry = 5000,
        .last_entry = 10000,
        .deleted = 1,
        .title = "Test Title",
        .priority = 100,
        .chart_type = "stacked",
        .units = "megabytes",
        .family = "system"
    };
    
    contexts_snapshot_add_ctx_update(snapshot, &ctx_update);
    
    contexts_snapshot_delete(snapshot);
}

TEST_F(ContextUpdatedTest, fill_ctx_updated_all_fields_in_updated) {
    contexts_updated_t updated = contexts_updated_new("claim123", "node456", 789, 1234567890);
    
    struct context_updated ctx_update = {
        .id = "test_id_123",
        .version = 42,
        .first_entry = 5000,
        .last_entry = 10000,
        .deleted = 1,
        .title = "Test Title",
        .priority = 100,
        .chart_type = "stacked",
        .units = "megabytes",
        .family = "system"
    };
    
    contexts_updated_add_ctx_update(updated, &ctx_update);
    
    contexts_updated_delete(updated);
}