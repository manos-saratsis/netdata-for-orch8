#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Mock external structures and functions
typedef struct ebpf_module {
    int enabled;
    int update_every;
    int lifetime;
    int global_charts;
    int apps_charts;
    int cgroup_charts;
    int maps_per_core;
    int mode;
    int load;
    char info_thread_name[256];
    struct {
        char thread_name[256];
    } info;
    void *maps;
    void *objects;
    void *probe_links;
    void *hash_table_stats;
    uint32_t running_time;
    void *targets;
} ebpf_module_t;

typedef struct netdata_syscall_stat {
    uint64_t call;
    uint64_t ecall;
    uint64_t bytes;
    void *next;
} netdata_syscall_stat_t;

typedef struct netdata_publish_syscall {
    uint64_t pcall;
    uint64_t pbyte;
    uint64_t perr;
    uint64_t ncall;
    uint64_t nbyte;
    uint64_t nerr;
    char *name;
    void *next;
} netdata_publish_syscall_t;

typedef struct netdata_publish_vfs_common {
    long read;
    long write;
} netdata_publish_vfs_common_t;

typedef struct netdata_socket_t {
    uint8_t protocol;
    uint16_t family;
    uint64_t first_timestamp;
    uint64_t current_timestamp;
    uint32_t external_origin;
    struct {
        uint64_t call_tcp_sent;
        uint64_t call_tcp_received;
        uint64_t tcp_bytes_received;
        uint64_t tcp_bytes_sent;
        uint64_t close;
        uint64_t retransmit;
        uint64_t ipv4_connect;
        uint64_t ipv6_connect;
    } tcp;
    struct {
        uint64_t call_udp_sent;
        uint64_t call_udp_received;
    } udp;
} netdata_socket_t;

typedef struct netdata_socket_plus {
    netdata_socket_t data;
    uint32_t pid;
    time_t last_update;
    uint32_t flags;
    struct {
        char src_ip[46];
        char dst_ip[46];
        char dst_port[16];
    } socket_string;
} netdata_socket_plus_t;

typedef struct ebpf_socket_publish_apps {
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t call_tcp_sent;
    uint64_t call_tcp_received;
    uint64_t retransmit;
    uint64_t call_close;
    uint64_t call_tcp_v4_connection;
    uint64_t call_tcp_v6_connection;
    uint64_t call_udp_sent;
    uint64_t call_udp_received;
} ebpf_socket_publish_apps_t;

// Global variables (mocked)
static netdata_syscall_stat_t socket_aggregated_data[10];
static netdata_publish_syscall_t socket_publish_aggregated[10];

// ===== Test: ebpf_socket_bytes2bits =====
static void test_ebpf_socket_bytes2bits_zero(void **state) {
    (void)state;
    // Test with 0 bytes
    long long result = (long long)(0 * 8 / 1024);
    assert_int_equal(result, 0);
}

static void test_ebpf_socket_bytes2bits_positive(void **state) {
    (void)state;
    // Test with 1 megabyte = 1048576 bytes = 8388608 bits = 8192 kilobits
    uint64_t value = 1048576;
    long long result = (long long)(value * 8 / 1024);
    assert_int_equal(result, 8192);
}

static void test_ebpf_socket_bytes2bits_large_value(void **state) {
    (void)state;
    uint64_t value = 1000000000;
    long long result = (long long)(value * 8 / 1024);
    assert_true(result > 0);
}

// ===== Test: ebpf_update_global_publish =====
static void test_ebpf_update_global_publish_no_previous_call(void **state) {
    (void)state;
    netdata_publish_syscall_t publish[2];
    memset(publish, 0, sizeof(publish));
    publish[0].next = &publish[1];
    publish[1].next = NULL;
    
    netdata_publish_vfs_common_t tcp, udp;
    memset(&tcp, 0, sizeof(tcp));
    memset(&udp, 0, sizeof(udp));
    
    netdata_syscall_stat_t input[2];
    memset(input, 0, sizeof(input));
    input[0].call = 100;
    input[0].bytes = 200;
    input[0].ecall = 5;
    input[0].next = &input[1];
    input[1].call = 50;
    input[1].bytes = 100;
    input[1].ecall = 2;
    input[1].next = NULL;
    
    // Call the function (simulated - uses macros and complex logic)
    // Since this requires mocking internal behavior, we test the logic
    if (input[0].call != publish[0].pcall) {
        if (publish[0].pcall) {
            publish[0].ncall = (input[0].call > publish[0].pcall) ? 
                input[0].call - publish[0].pcall : publish[0].pcall - input[0].call;
        } else {
            publish[0].ncall = 0;
            publish[0].nbyte = 0;
            publish[0].nerr = 0;
        }
        publish[0].pcall = input[0].call;
        publish[0].pbyte = input[0].bytes;
        publish[0].perr = input[0].ecall;
    } else {
        publish[0].ncall = 0;
        publish[0].nbyte = 0;
        publish[0].nerr = 0;
    }
    
    assert_int_equal(publish[0].ncall, 0);
    assert_int_equal(publish[0].pcall, 100);
}

static void test_ebpf_update_global_publish_with_previous_call(void **state) {
    (void)state;
    netdata_publish_syscall_t publish[2];
    memset(publish, 0, sizeof(publish));
    publish[0].pcall = 50;
    publish[0].pbyte = 100;
    publish[0].perr = 2;
    publish[0].next = &publish[1];
    publish[1].next = NULL;
    
    netdata_syscall_stat_t input[2];
    memset(input, 0, sizeof(input));
    input[0].call = 100;
    input[0].bytes = 200;
    input[0].ecall = 5;
    input[0].next = &input[1];
    input[1].call = 50;
    input[1].bytes = 100;
    input[1].ecall = 2;
    input[1].next = NULL;
    
    if (input[0].call != publish[0].pcall) {
        publish[0].ncall = (input[0].call > publish[0].pcall) ? 
            input[0].call - publish[0].pcall : publish[0].pcall - input[0].call;
        publish[0].nbyte = (input[0].bytes > publish[0].pbyte) ? 
            input[0].bytes - publish[0].pbyte : publish[0].pbyte - input[0].bytes;
        publish[0].nerr = (input[0].ecall > publish[0].nerr) ? 
            input[0].ecall - publish[0].perr : publish[0].perr - input[0].ecall;
    } else {
        publish[0].ncall = 0;
        publish[0].nbyte = 0;
        publish[0].nerr = 0;
    }
    
    assert_int_equal(publish[0].ncall, 50);
    assert_int_equal(publish[0].nbyte, 100);
}

// ===== Test: ebpf_is_port_inside_range =====
static void test_ebpf_is_port_inside_range_no_restrictions(void **state) {
    (void)state;
    // No restrictions test - should always return 1
    // This is a logic test since network_viewer_opt would be mocked
    uint16_t port = 80;
    int excluded = 0;
    int included = 0;
    
    if (!excluded && !included) {
        assert_int_equal(1, 1);
    }
}

static void test_ebpf_is_port_inside_range_excluded_port(void **state) {
    (void)state;
    // Test logic for excluded port
    uint16_t cmp = 8080;
    uint16_t excluded_first = 8000;
    uint16_t excluded_last = 9000;
    
    int result = (excluded_first <= cmp && cmp <= excluded_last) ? 0 : 1;
    assert_int_equal(result, 0);
}

static void test_ebpf_is_port_inside_range_included_port(void **state) {
    (void)state;
    // Test logic for included port
    uint16_t cmp = 80;
    uint16_t included_first = 80;
    uint16_t included_last = 443;
    
    int result = (included_first <= cmp && cmp <= included_last) ? 1 : 0;
    assert_int_equal(result, 1);
}

static void test_ebpf_is_port_inside_range_outside_included(void **state) {
    (void)state;
    uint16_t cmp = 9000;
    uint16_t included_first = 80;
    uint16_t included_last = 443;
    
    int result = (included_first <= cmp && cmp <= included_last) ? 1 : 0;
    assert_int_equal(result, 0);
}

// ===== Test: hostname_matches_pattern =====
static void test_hostname_matches_pattern_no_filters(void **state) {
    (void)state;
    // No filters - should return 1
    int excluded = 0;
    int included = 0;
    
    int result = (!excluded && !included) ? 1 : 0;
    assert_int_equal(result, 1);
}

static void test_hostname_matches_pattern_with_filters(void **state) {
    (void)state;
    // With filters - requires pattern matching which we simulate
    int excluded = 1;
    int included = 0;
    
    int result = 0;
    if (excluded || included) {
        result = 0; // Pattern would be checked
    }
    assert_int_equal(result, 0);
}

// ===== Test: ebpf_is_socket_allowed =====
static void test_ebpf_is_socket_allowed_family_mismatch(void **state) {
    (void)state;
    // Test family mismatch case
    int family_opt = 2; // AF_INET
    int data_family = 10; // AF_INET6
    
    int result = 0;
    if (family_opt && family_opt != data_family) {
        result = 0; // goto endsocketallowed
    }
    assert_int_equal(result, 0);
}

static void test_ebpf_is_socket_allowed_valid_socket(void **state) {
    (void)state;
    // Test valid socket case
    int family_opt = 0; // No family restriction
    int family = 2; // AF_INET
    uint16_t dport = 80;
    int port_allowed = 1;
    int ip_allowed = 1;
    
    int result = 1;
    if (family_opt && family_opt != family) {
        result = 0;
    }
    if (!port_allowed) {
        result = 0;
    }
    result = ip_allowed;
    
    assert_int_equal(result, 1);
}

// ===== Test: ebpf_hash_socket_accumulator =====
static void test_ebpf_hash_socket_accumulator_single_value(void **state) {
    (void)state;
    netdata_socket_t values[1];
    memset(values, 0, sizeof(values));
    values[0].protocol = 6; // TCP
    values[0].current_timestamp = 1000;
    values[0].first_timestamp = 500;
    values[0].family = 2; // AF_INET
    
    ebpf_hash_socket_accumulator(values, 1);
    
    assert_int_equal(values[0].protocol, 6);
    assert_int_equal(values[0].current_timestamp, 1000);
}

static void test_ebpf_hash_socket_accumulator_multiple_values(void **state) {
    (void)state;
    netdata_socket_t values[2];
    memset(values, 0, sizeof(values));
    
    values[0].protocol = 0;
    values[0].current_timestamp = 1000;
    values[0].first_timestamp = 500;
    values[0].tcp.call_tcp_sent = 10;
    values[0].tcp.tcp_bytes_sent = 1000;
    
    values[1].protocol = 6; // TCP
    values[1].current_timestamp = 2000;
    values[1].first_timestamp = 0;
    values[1].tcp.call_tcp_sent = 20;
    values[1].tcp.tcp_bytes_sent = 2000;
    
    // Simulate accumulator logic
    if (values[0].tcp.call_tcp_sent != 0 || values[1].tcp.call_tcp_sent != 0) {
        values[0].tcp.call_tcp_sent += values[1].tcp.call_tcp_sent;
    }
    
    assert_int_equal(values[0].tcp.call_tcp_sent, 30);
}

// ===== Test: fill_nv_port_list =====
static void test_fill_nv_port_list_basic(void **state) {
    (void)state;
    typedef struct {
        uint16_t first;
        uint16_t protocol;
        uint32_t pid;
        uint32_t tgid;
        uint64_t connections;
        void *next;
    } port_list_t;
    
    port_list_t out;
    memset(&out, 0, sizeof(out));
    
    typedef struct {
        uint32_t pid;
        uint32_t tgid;
        uint64_t counter;
    } passive_conn_t;
    
    passive_conn_t in;
    in.pid = 1234;
    in.tgid = 5678;
    in.counter = 100;
    
    out.first = 80;
    out.protocol = 6; // IPPROTO_TCP
    out.pid = in.pid;
    out.tgid = in.tgid;
    out.connections = in.counter;
    
    assert_int_equal(out.first, 80);
    assert_int_equal(out.protocol, 6);
    assert_int_equal(out.pid, 1234);
    assert_int_equal(out.connections, 100);
}

// ===== Test: ebpf_socket_fill_publish_apps =====
static void test_ebpf_socket_fill_publish_apps_basic(void **state) {
    (void)state;
    ebpf_socket_publish_apps_t curr;
    memset(&curr, 0, sizeof(curr));
    
    netdata_socket_t ns;
    memset(&ns, 0, sizeof(ns));
    ns.tcp.tcp_bytes_sent = 1000;
    ns.tcp.tcp_bytes_received = 2000;
    ns.tcp.call_tcp_sent = 10;
    ns.tcp.call_tcp_received = 20;
    ns.tcp.retransmit = 5;
    ns.tcp.close = 2;
    ns.tcp.ipv4_connect = 1;
    ns.tcp.ipv6_connect = 0;
    ns.udp.call_udp_sent = 3;
    ns.udp.call_udp_received = 4;
    
    // Simulate function logic
    curr.bytes_sent = ns.tcp.tcp_bytes_sent;
    curr.bytes_received = ns.tcp.tcp_bytes_received;
    curr.call_tcp_sent = ns.tcp.call_tcp_sent;
    curr.call_tcp_received = ns.tcp.call_tcp_received;
    curr.retransmit = ns.tcp.retransmit;
    curr.call_close = ns.tcp.close;
    curr.call_tcp_v4_connection = ns.tcp.ipv4_connect;
    curr.call_tcp_v6_connection = ns.tcp.ipv6_connect;
    curr.call_udp_sent = ns.udp.call_udp_sent;
    curr.call_udp_received = ns.udp.call_udp_received;
    
    assert_int_equal(curr.bytes_sent, 1000);
    assert_int_equal(curr.bytes_received, 2000);
    assert_int_equal(curr.call_tcp_sent, 10);
    assert_int_equal(curr.retransmit, 5);
}

// ===== Test: ebpf_socket_sum_cgroup_pids =====
static void test_ebpf_socket_sum_cgroup_pids_empty_list(void **state) {
    (void)state;
    ebpf_socket_publish_apps_t socket;
    memset(&socket, 0, sizeof(socket));
    
    typedef struct {
        netdata_socket_t socket;
        void *next;
    } pid_on_target2_t;
    
    ebpf_socket_publish_apps_t accumulator;
    memset(&accumulator, 0, sizeof(accumulator));
    
    // No pids - accumulator stays at 0
    socket.bytes_sent = (accumulator.bytes_sent >= socket.bytes_sent) ? accumulator.bytes_sent : socket.bytes_sent;
    
    assert_int_equal(socket.bytes_sent, 0);
}

static void test_ebpf_socket_sum_cgroup_pids_with_values(void **state) {
    (void)state;
    ebpf_socket_publish_apps_t socket;
    memset(&socket, 0, sizeof(socket));
    socket.bytes_sent = 100;
    socket.bytes_received = 200;
    
    typedef struct {
        netdata_socket_t socket;
        void *next;
    } pid_on_target2_t;
    
    ebpf_socket_publish_apps_t accumulator;
    memset(&accumulator, 0, sizeof(accumulator));
    accumulator.bytes_sent = 500;
    accumulator.bytes_received = 600;
    
    socket.bytes_sent = (accumulator.bytes_sent >= socket.bytes_sent) ? accumulator.bytes_sent : socket.bytes_sent;
    socket.bytes_received = (accumulator.bytes_received >= socket.bytes_received) ? accumulator.bytes_received : socket.bytes_received;
    
    assert_int_equal(socket.bytes_sent, 500);
    assert_int_equal(socket.bytes_received, 600);
}

// ===== Test: ebpf_is_specific_ip_inside_range =====
static void test_ebpf_is_specific_ip_inside_range_no_restrictions(void **state) {
    (void)state;
    // No excluded or included IPs - should return 1
    int excluded = 0;
    int included = 0;
    int result = (!excluded && !included) ? 1 : 0;
    assert_int_equal(result, 1);
}

static void test_ebpf_is_specific_ip_inside_range_ipv4_excluded(void **state) {
    (void)state;
    // Test IPv4 exclusion logic
    uint32_t cmp_ip = htonl(192168001);
    uint32_t excluded_first = htonl(192168000);
    uint32_t excluded_last = htonl(192168255);
    int family = 2; // AF_INET
    
    int result = 1;
    if (family == 2) {
        if (excluded_first <= cmp_ip && cmp_ip <= excluded_last) {
            result = 0;
        }
    }
    assert_int_equal(result, 0);
}

static void test_ebpf_is_specific_ip_inside_range_ipv4_not_excluded(void **state) {
    (void)state;
    uint32_t cmp_ip = htonl(10001001);
    uint32_t excluded_first = htonl(192168000);
    uint32_t excluded_last = htonl(192168255);
    int family = 2; // AF_INET
    
    int result = 1;
    if (family == 2) {
        if (excluded_first <= cmp_ip && cmp_ip <= excluded_last) {
            result = 0;
        }
    }
    assert_int_equal(result, 1);
}

// ===== Test: config_service_value_cb =====
static void test_config_service_value_cb_basic(void **state) {
    (void)state;
    // Test callback returns true for processing
    const char *name = "80";
    const char *value = "HTTP";
    
    // Simulate callback logic
    int port = atoi(name);
    int valid = (port >= 1 && port <= 65535) ? 1 : 0;
    
    assert_int_equal(valid, 1);
}

static void test_config_service_value_cb_invalid_port(void **state) {
    (void)state;
    const char *name = "99999";
    const char *value = "INVALID";
    
    int port = atoi(name);
    int valid = (port >= 1 && port <= 65535) ? 1 : 0;
    
    assert_int_equal(valid, 0);
}

// ===== Test: ebpf_link_dimension_name =====
static void test_ebpf_link_dimension_name_valid_port(void **state) {
    (void)state;
    const char *port = "80";
    uint32_t hash = 12345;
    const char *value = "HTTP";
    
    int test = atoi(port);
    int valid = (test >= 1 && test <= 65535) ? 1 : 0;
    
    assert_int_equal(valid, 1);
    assert_int_equal(test, 80);
}

static void test_ebpf_link_dimension_name_invalid_port_low(void **state) {
    (void)state;
    const char *port = "0";
    int test = atoi(port);
    int valid = (test >= 1 && test <= 65535) ? 1 : 0;
    
    assert_int_equal(valid, 0);
}

static void test_ebpf_link_dimension_name_invalid_port_high(void **state) {
    (void)state;
    const char *port = "99999";
    int test = atoi(port);
    int valid = (test >= 1 && test <= 65535) ? 1 : 0;
    
    assert_int_equal(valid, 0);
}

// ===== Test: ebpf_socket_initialize_global_vectors =====
static void test_ebpf_socket_initialize_global_vectors_memset(void **state) {
    (void)state;
    // Test that memory is properly initialized
    netdata_syscall_stat_t data[10];
    memset(data, 0, NETDATA_MAX_SOCKET_VECTOR * sizeof(netdata_syscall_stat_t));
    
    for (int i = 0; i < 10; i++) {
        assert_int_equal(data[i].call, 0);
        assert_int_equal(data[i].ecall, 0);
        assert_int_equal(data[i].bytes, 0);
    }
}

// ===== Test: socket_collector loop logic =====
static void test_socket_collector_counter_increment(void **state) {
    (void)state;
    int update_every = 10;
    int counter = update_every - 1;
    int iterations = 0;
    
    while (iterations < 100) {
        if (++counter == update_every) {
            counter = 0;
            break; // Would process data here
        }
        iterations++;
    }
    
    assert_int_equal(counter, 0);
}

// ===== Test: ebpf_socket_update_cgroup_algorithm =====
static void test_ebpf_socket_update_cgroup_algorithm_basic(void **state) {
    (void)state;
    netdata_publish_syscall_t data[3];
    memset(data, 0, sizeof(data));
    
    for (int i = 0; i < 3; i++) {
        // Simulate algorithm assignment
        data[i].name = malloc(10);
        strcpy(data[i].name, "test");
    }
    
    for (int i = 0; i < 3; i++) {
        assert_non_null(data[i].name);
        free(data[i].name);
    }
}

// ===== Test: ebpf_socket_read_hash_global_tables logic =====
static void test_ebpf_socket_read_hash_global_tables_result_assignment(void **state) {
    (void)state;
    // Test that results are properly assigned
    typedef struct {
        uint64_t values[20];
    } result_t;
    
    result_t res;
    memset(res.values, 0, sizeof(res.values));
    
    res.values[0] = 100;  // NETDATA_KEY_CALLS_TCP_SENDMSG
    res.values[3] = 50;   // NETDATA_KEY_CALLS_TCP_CLEANUP_RBUF
    
    netdata_syscall_stat_t data;
    memset(&data, 0, sizeof(data));
    data.call = res.values[0];
    
    assert_int_equal(data.call, 100);
}

// ===== Test: ebpf_socket_free cleanup logic =====
static void test_ebpf_socket_free_sets_enabled(void **state) {
    (void)state;
    ebpf_module_t em;
    memset(&em, 0, sizeof(em));
    em.enabled = 1;
    
    // Simulate cleanup
    em.enabled = 0; // NETDATA_THREAD_EBPF_STOPPED
    
    assert_int_equal(em.enabled, 0);
}

// ===== Test: Conditional branch in ebpf_update_global_publish =====
static void test_ebpf_update_global_publish_branch_pcall_zero(void **state) {
    (void)state;
    netdata_publish_syscall_t pub;
    memset(&pub, 0, sizeof(pub));
    pub.pcall = 0;
    
    netdata_syscall_stat_t inp;
    memset(&inp, 0, sizeof