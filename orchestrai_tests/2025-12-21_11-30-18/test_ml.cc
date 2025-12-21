#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ml.cc"  // Include the source file to test internals

using ::testing::_;
using ::testing::Return;
using ::testing::Test;

class MLTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Any setup required before each test
    }

    void TearDown() override {
        // Any cleanup required after each test
    }
};

// Test ml_dimension_calculated_numbers
TEST_F(MLTest, ml_dimension_calculated_numbers_success) {
    ml_worker_t worker = {};
    ml_dimension_t dim = {};
    
    // Mock necessary dependencies
    // Setup dim with valid data for test
    auto result = ml_dimension_calculated_numbers(&worker, &dim);
    
    EXPECT_EQ(result.first, ML_WORKER_RESULT_OK);
    // More specific assertions based on expected behavior
}

TEST_F(MLTest, ml_dimension_calculated_numbers_insufficient_data) {
    ml_worker_t worker = {};
    ml_dimension_t dim = {};
    
    // Setup dim with insufficient data
    auto result = ml_dimension_calculated_numbers(&worker, &dim);
    
    EXPECT_EQ(result.first, ML_WORKER_RESULT_NOT_ENOUGH_COLLECTED_VALUES);
}

// Test ml_dimension_add_model
TEST_F(MLTest, ml_dimension_add_model_success) {
    nd_uuid_t metric_uuid = {};
    ml_kmeans_inlined_t inlined_km = {};
    
    // Mock ML database initialization
    ml_db = sqlite3_open_v2(":memory:", NULL, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    
    int result = ml_dimension_add_model(&metric_uuid, &inlined_km);
    
    EXPECT_EQ(result, 0);  // Expecting successful model addition
    
    sqlite3_close(ml_db);
    ml_db = nullptr;
}

TEST_F(MLTest, ml_dimension_add_model_database_not_initialized) {
    nd_uuid_t metric_uuid = {};
    ml_kmeans_inlined_t inlined_km = {};
    
    ml_db = nullptr;  // Simulate uninitialized database
    
    int result = ml_dimension_add_model(&metric_uuid, &inlined_km);
    
    EXPECT_NE(result, 0);  // Expecting error
}

// Test ml_dimension_delete_models
TEST_F(MLTest, ml_dimension_delete_models_success) {
    nd_uuid_t metric_uuid = {};
    time_t before_time = time(nullptr);
    
    ml_db = sqlite3_open_v2(":memory:", NULL, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    
    int result = ml_dimension_delete_models(&metric_uuid, before_time);
    
    EXPECT_EQ(result, 0);  // Expecting successful deletion
    
    sqlite3_close(ml_db);
    ml_db = nullptr;
}

// Test ml_dimension_deserialize_kmeans
TEST_F(MLTest, ml_dimension_deserialize_kmeans_valid_json) {
    const char* valid_json = R"(
    {
        "version": "1",
        "machine-guid": "test-guid",
        "chart": "test-chart",
        "dimension": "test-dimension",
        "model": {
            "after": 1234,
            "before": 5678,
            "min_dist": 0.1,
            "max_dist": 0.9,
            "clusters": [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
        }
    })";
    
    bool result = ml_dimension_deserialize_kmeans(valid_json);
    
    EXPECT_TRUE(result);
}

TEST_F(MLTest, ml_dimension_deserialize_kmeans_invalid_json) {
    const char* invalid_json = R"(
    {
        "version": "2",  // Wrong version
        "machine-guid": "test-guid"
    })";
    
    bool result = ml_dimension_deserialize_kmeans(invalid_json);
    
    EXPECT_FALSE(result);
}

// Test ml_dimension_predict
TEST_F(MLTest, ml_dimension_predict_normal_value) {
    ml_dimension_t dim = {};
    dim.mls = MACHINE_LEARNING_STATUS_ENABLED;
    dim.cns.resize(10);  // Populate with some values
    
    calculated_number_t value = 42.0;
    bool result = ml_dimension_predict(&dim, value, true);
    
    EXPECT_FALSE(result);  // Expecting no anomaly
}

TEST_F(MLTest, ml_dimension_predict_anomalous_value) {
    ml_dimension_t dim = {};
    dim.mls = MACHINE_LEARNING_STATUS_ENABLED;
    dim.cns.resize(10);  // Populate with some values
    dim.km_contexts.resize(1);  // Add mock model context
    
    calculated_number_t value = 9999.0;  // Extreme value to trigger anomaly
    bool result = ml_dimension_predict(&dim, value, true);
    
    EXPECT_TRUE(result);  // Expecting anomaly detected
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}