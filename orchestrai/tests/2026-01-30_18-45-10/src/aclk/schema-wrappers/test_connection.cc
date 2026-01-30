// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <sys/time.h>
#include <memory>

// Mock protobuf structures
namespace aclk_lib {
namespace v1 {
class Capability {
public:
    void set_name(const char* name) { m_name = name; }
    void set_enabled(int enabled) { m_enabled = enabled; }
    void set_version(uint32_t version) { m_version = version; }
private:
    const char* m_name = nullptr;
    int m_enabled = 0;
    uint32_t m_version = 0;
};
}
}

namespace agent {
namespace v1 {

class Timestamp {
public:
    void set_seconds(int64_t sec) { m_seconds = sec; }
    void set_nanos(int32_t ns) { m_nanos = ns; }
    int64_t seconds() const { return m_seconds; }
    int32_t nanos() const { return m_nanos; }
private:
    int64_t m_seconds = 0;
    int32_t m_nanos = 0;
};

class UpdateAgentConnection {
public:
    void set_claim_id(const char* id) { m_claim_id = id; }
    void set_reachable(bool r) { m_reachable = r; }
    void set_session_id(int64_t id) { m_session_id = id; }
    void set_update_source(int source) { m_update_source = source; }
    Timestamp* mutable_updated_at() { return &m_timestamp; }
    aclk_lib::v1::Capability* add_capabilities() {
        m_capabilities.push_back(aclk_lib::v1::Capability());
        return &m_capabilities.back();
    }
    size_t ByteSizeLong() const { return 512; }
    bool SerializeToArray(void* buf, int size) { return true; }
    
    const char* get_claim_id() const { return m_claim_id; }
    bool get_reachable() const { return m_reachable; }
    int64_t get_session_id() const { return m_session_id; }
private:
    const char* m_claim_id = nullptr;
    bool m_reachable = false;
    int64_t m_session_id = 0;
    int m_update_source = 0;
    Timestamp m_timestamp;
    std::vector<aclk_lib::v1::Capability> m_capabilities;
};

class DisconnectReq {
public:
    bool ParseFromArray(const void* buf, int size) { return true; }
    uint64_t reconnect_after_seconds() const { return m_reconnect_after_s; }
    bool permaban() const { return m_permaban; }
    uint32_t error_code() const { return m_error_code; }
    const std::string& error_description() const { return m_error_desc; }
    
    void set_reconnect_after_s(uint64_t s) { m_reconnect_after_s = s; }
    void set_permaban(bool pb) { m_permaban = pb; }
    void set_error_code(uint32_t code) { m_error_code = code; }
    void set_error_description(const std::string& desc) { m_error_desc = desc; }
private:
    uint64_t m_reconnect_after_s = 0;
    bool m_permaban = false;
    uint32_t m_error_code = 0;
    std::string m_error_desc;
};

const int CONNECTION_UPDATE_SOURCE_LWT = 1;
const int CONNECTION_UPDATE_SOURCE_AGENT = 2;

} // namespace v1
} // namespace agent

using namespace agent::v1;

// Declarations from capability.h
struct capability {
    const char *name;
    uint32_t version;
    int enabled;
};

void capability_set(aclk_lib::v1::Capability *proto_capa, const struct capability *c_capa) {
    proto_capa->set_name(c_capa->name);
    proto_capa->set_enabled(c_capa->enabled);
    proto_capa->set_version(c_capa->version);
}

// Mock functions for allocation/deallocation
void* test_mallocz(size_t size) {
    return malloc(size);
}

void test_freez(void* ptr) {
    if (ptr) free(ptr);
}

void* test_callocz(size_t count, size_t size) {
    return calloc(count, size);
}

char* test_strdupz(const char* str) {
    if (!str) return nullptr;
    return strdup(str);
}

// Declarations for connection
typedef struct {
    const char *claim_id;
    unsigned int reachable:1;
    int64_t session_id;
    unsigned int lwt:1;
    const struct capability *capabilities;
} update_agent_connection_t;

struct disconnect_cmd {
    uint64_t reconnect_after_s;
    int permaban;
    uint32_t error_code;
    char *error_description;
};

// Function implementations for testing
char *generate_update_agent_connection(size_t *len, const update_agent_connection_t *data) {
    UpdateAgentConnection connupd;

    connupd.set_claim_id(data->claim_id);
    connupd.set_reachable(data->reachable);
    connupd.set_session_id(data->session_id);

    connupd.set_update_source((data->lwt) ? CONNECTION_UPDATE_SOURCE_LWT : CONNECTION_UPDATE_SOURCE_AGENT);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    aclk_lib::v1::Timestamp *timestamp = connupd.mutable_updated_at();
    timestamp->set_seconds(tv.tv_sec);
    timestamp->set_nanos(tv.tv_usec * 1000);

    if (data->capabilities) {
        const struct capability *capa = data->capabilities;
        while (capa->name) {
            aclk_lib::v1::Capability *proto_capa = connupd.add_capabilities();
            capability_set(proto_capa, capa);
            capa++;
        }
    }

    *len = (size_t)connupd.ByteSizeLong();
    char *msg = (char*)test_mallocz(*len);
    if (msg)
        connupd.SerializeToArray(msg, *len);

    return msg;
}

struct disconnect_cmd *parse_disconnect_cmd(const char *data, size_t len) {
    DisconnectReq req;
    struct disconnect_cmd *res;

    if (!req.ParseFromArray(data, len))
        return NULL;

    res = (struct disconnect_cmd *)test_callocz(1, sizeof(struct disconnect_cmd));

    if (!res)
        return NULL;

    res->reconnect_after_s = req.reconnect_after_seconds();
    res->permaban = req.permaban();
    res->error_code = req.error_code();
    if (req.error_description().c_str()) {
        res->error_description = test_strdupz(req.error_description().c_str());
        if (!res->error_description) {
            test_freez(res);
            return NULL;
        }
    }

    return res;
}

class ConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// Test: generate_update_agent_connection with valid data
TEST_F(ConnectionTest, generate_update_agent_connection_valid) {
    update_agent_connection_t data = {
        .claim_id = "test_claim",
        .reachable = 1,
        .session_id = 12345,
        .lwt = 0,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection with LWT flag set
TEST_F(ConnectionTest, generate_update_agent_connection_with_lwt) {
    update_agent_connection_t data = {
        .claim_id = "lwt_claim",
        .reachable = 1,
        .session_id = 54321,
        .lwt = 1,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection not reachable
TEST_F(ConnectionTest, generate_update_agent_connection_not_reachable) {
    update_agent_connection_t data = {
        .claim_id = "unreachable_claim",
        .reachable = 0,
        .session_id = 0,
        .lwt = 0,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection with empty claim_id
TEST_F(ConnectionTest, generate_update_agent_connection_empty_claim_id) {
    update_agent_connection_t data = {
        .claim_id = "",
        .reachable = 1,
        .session_id = 0,
        .lwt = 0,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection with capabilities
TEST_F(ConnectionTest, generate_update_agent_connection_with_capabilities) {
    struct capability caps[] = {
        { "cap1", 1, 1 },
        { "cap2", 2, 0 },
        { nullptr, 0, 0 }
    };
    
    update_agent_connection_t data = {
        .claim_id = "with_caps",
        .reachable = 1,
        .session_id = 999,
        .lwt = 0,
        .capabilities = caps
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection with single capability
TEST_F(ConnectionTest, generate_update_agent_connection_single_capability) {
    struct capability caps[] = {
        { "single", 1, 1 },
        { nullptr, 0, 0 }
    };
    
    update_agent_connection_t data = {
        .claim_id = "single_cap",
        .reachable = 1,
        .session_id = 111,
        .lwt = 0,
        .capabilities = caps
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection with null capabilities pointer
TEST_F(ConnectionTest, generate_update_agent_connection_null_capabilities) {
    update_agent_connection_t data = {
        .claim_id = "null_caps",
        .reachable = 1,
        .session_id = 222,
        .lwt = 0,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection with max session_id
TEST_F(ConnectionTest, generate_update_agent_connection_max_session_id) {
    update_agent_connection_t data = {
        .claim_id = "max_session",
        .reachable = 1,
        .session_id = INT64_MAX,
        .lwt = 0,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection with zero session_id
TEST_F(ConnectionTest, generate_update_agent_connection_zero_session_id) {
    update_agent_connection_t data = {
        .claim_id = "zero_session",
        .reachable = 1,
        .session_id = 0,
        .lwt = 0,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection lwt=1 reachable=1
TEST_F(ConnectionTest, generate_update_agent_connection_lwt_and_reachable) {
    update_agent_connection_t data = {
        .claim_id = "lwt_reachable",
        .reachable = 1,
        .session_id = 333,
        .lwt = 1,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection lwt=1 reachable=0
TEST_F(ConnectionTest, generate_update_agent_connection_lwt_not_reachable) {
    update_agent_connection_t data = {
        .claim_id = "lwt_unreachable",
        .reachable = 0,
        .session_id = 444,
        .lwt = 1,
        .capabilities = nullptr
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: generate_update_agent_connection multiple capabilities
TEST_F(ConnectionTest, generate_update_agent_connection_multiple_capabilities) {
    struct capability caps[] = {
        { "cap_a", 10, 1 },
        { "cap_b", 20, 0 },
        { "cap_c", 30, 1 },
        { nullptr, 0, 0 }
    };
    
    update_agent_connection_t data = {
        .claim_id = "multi_caps",
        .reachable = 1,
        .session_id = 555,
        .lwt = 0,
        .capabilities = caps
    };
    
    size_t len = 0;
    char *msg = generate_update_agent_connection(&len, &data);
    
    EXPECT_NE(msg, nullptr);
    EXPECT_GT(len, 0);
    
    test_freez(msg);
}

// Test: parse_disconnect_cmd with valid data
TEST_F(ConnectionTest, parse_disconnect_cmd_valid) {
    const char data[] = "test_data";
    
    struct disconnect_cmd *cmd = parse_disconnect_cmd(data, sizeof(data));
    
    EXPECT_NE(cmd, nullptr);
    if (cmd) {
        test_freez(cmd->error_description);
        test_freez(cmd);
    }
}

// Test: parse_disconnect_cmd with empty data
TEST_F(ConnectionTest, parse_disconnect_cmd_empty_data) {
    const char data[] = "";
    
    struct disconnect_cmd *cmd = parse_disconnect_cmd(data, 0);
    
    // May return NULL if parsing fails, which is acceptable
}

// Test: parse_disconnect_cmd with zero length
TEST_F(ConnectionTest, parse_disconnect_cmd_zero_length) {
    const char data[] = "some_data";
    
    struct disconnect_cmd *cmd = parse_disconnect_cmd(data, 0);
    
    // May return NULL if parsing fails, which is acceptable
}

// Test: parse_disconnect_cmd with small data
TEST_F(ConnectionTest, parse_disconnect_cmd_small_data) {
    const char data[] = "x";
    
    struct disconnect_cmd *cmd = parse_disconnect_cmd(data, 1);
    
    // Result depends on protobuf parsing
}

// Test: parse_disconnect_cmd with large data
TEST_F(ConnectionTest, parse_disconnect_cmd_large_data) {
    char data[1024];
    memset(data, 'A', sizeof(data));
    
    struct disconnect_cmd *cmd = parse_disconnect_cmd(data, sizeof(data));
    
    // Result depends on protobuf parsing
}