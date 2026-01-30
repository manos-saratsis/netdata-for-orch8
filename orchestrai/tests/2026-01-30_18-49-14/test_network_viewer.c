#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <setjmp.h>
#include <cmocka.h>

/* Mock declarations and structures */
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} BUFFER;

typedef struct {
    int id;
    const char *name;
} ENUM_STR_MAP;

typedef struct {
    uint32_t ipv4;
} ipv4_addr;

typedef struct {
    unsigned char ipv6[16];
} ipv6_addr;

typedef struct {
    int family;
    int protocol;
    uint16_t port;
    union {
        ipv4_addr ipv4;
        ipv6_addr ipv6;
    } ip;
} socket_endpoint;

typedef struct {
    uint32_t tcpi_rtt;
    uint32_t tcpi_rcv_rtt;
    uint32_t tcpi_total_retrans;
    uint32_t tcpi_retransmits;
    uint32_t tcpi_retrans;
    uint32_t tcpi_probes;
    uint32_t tcpi_backoff;
    uint32_t tcpi_options;
    uint8_t tcpi_snd_wscale;
    uint8_t tcpi_rcv_wscale;
    uint32_t tcpi_rto;
    uint32_t tcpi_ato;
    uint32_t tcpi_snd_mss;
    uint32_t tcpi_rcv_mss;
    uint32_t tcpi_unacked;
    uint32_t tcpi_sacked;
    uint32_t tcpi_lost;
    uint32_t tcpi_fackets;
    uint32_t tcpi_last_data_sent;
    uint32_t tcpi_last_ack_sent;
    uint32_t tcpi_last_data_recv;
    uint32_t tcpi_last_ack_recv;
    uint32_t tcpi_pmtu;
    uint32_t tcpi_rcv_ssthresh;
    uint32_t tcpi_snd_ssthresh;
    uint32_t tcpi_rttvar;
    uint32_t tcpi_snd_cwnd;
    uint32_t tcpi_advmss;
    uint32_t tcpi_reordering;
    uint32_t tcpi_rcv_space;
} tcp_info;

typedef enum {
    SOCKET_DIRECTION_NONE = 0,
    SOCKET_DIRECTION_LISTEN = 1,
    SOCKET_DIRECTION_INBOUND = 2,
    SOCKET_DIRECTION_OUTBOUND = 3,
    SOCKET_DIRECTION_LOCAL_INBOUND = 4,
    SOCKET_DIRECTION_LOCAL_OUTBOUND = 5,
} SOCKET_DIRECTION;

typedef struct {
    pid_t pid;
    uid_t uid;
    SOCKET_DIRECTION direction;
    int state;
    uint64_t net_ns_inode;
    socket_endpoint local;
    socket_endpoint remote;
    char comm[16];
    uint32_t timer;
    uint32_t retransmits;
    uint32_t expires;
    uint32_t rqueue;
    uint32_t wqueue;
    tcp_info info;
    struct {
        size_t count;
        struct {
            pid_t pid;
            uid_t uid;
            SOCKET_DIRECTION direction;
            int state;
            uint64_t net_ns_inode;
            socket_endpoint server;
            const char *local_address_space;
            const char *remote_address_space;
        } aggregated_key;
    } network_viewer;
} LOCAL_SOCKET;

typedef struct {
    BUFFER *wb;
    struct {
        uint32_t tcpi_rtt;
        uint32_t tcpi_rcv_rtt;
        uint32_t tcpi_total_retrans;
    } max;
} sockets_stats;

typedef struct {
    uint64_t proc_self_net_ns_inode;
} LS_STATE;

/* Mock functions */
BUFFER *buffer_create(size_t size, void *ptr) {
    BUFFER *b = malloc(sizeof(BUFFER));
    b->data = malloc(4096);
    b->size = 0;
    b->capacity = 4096;
    return b;
}

void buffer_flush(BUFFER *b) {
    if (b) b->size = 0;
}

void buffer_free(BUFFER *b) {
    if (b) {
        free(b->data);
        free(b);
    }
}

void buffer_json_initialize(BUFFER *b, const char *a, const char *c, int d, bool e, int f) {}
void buffer_json_member_add_uint64(BUFFER *b, const char *key, uint64_t value) {}
void buffer_json_member_add_string(BUFFER *b, const char *key, const char *value) {}
void buffer_json_member_add_time_t(BUFFER *b, const char *key, time_t value) {}
void buffer_json_member_add_boolean(BUFFER *b, const char *key, bool value) {}
void buffer_json_member_add_array(BUFFER *b, const char *key) {}
void buffer_json_member_add_object(BUFFER *b, const char *key) {}
void buffer_json_array_close(BUFFER *b) {}
void buffer_json_object_close(BUFFER *b) {}
void buffer_json_add_array_item_string(BUFFER *b, const char *value) {}
void buffer_json_add_array_item_object(BUFFER *b) {}
void buffer_json_add_array_item_uint64(BUFFER *b, uint64_t value) {}
void buffer_json_add_array_item_double(BUFFER *b, double value) {}
void buffer_json_add_array_item_array(BUFFER *b) {}
void buffer_json_finalize(BUFFER *b) {}
void buffer_rrdf_table_add_field(BUFFER *b, size_t id, const char *name, const char *help,
                                  int type, int visual, int transform, int decimals,
                                  const char *units, double max, int sort, void *opts,
                                  int summary, int filter, int field_opts, void *ptr) {}
void pluginsd_function_result_to_stdout(const char *txn, BUFFER *b) {}

time_t now_realtime_sec(void) {
    return time(NULL);
}

const char *SOCKET_DIRECTION_2str(SOCKET_DIRECTION d) {
    switch(d) {
        case SOCKET_DIRECTION_LISTEN: return "listen";
        case SOCKET_DIRECTION_LOCAL_INBOUND: return "local";
        case SOCKET_DIRECTION_LOCAL_OUTBOUND: return "local";
        case SOCKET_DIRECTION_INBOUND: return "inbound";
        case SOCKET_DIRECTION_OUTBOUND: return "outbound";
        default: return "unknown";
    }
}

const char *TCP_STATE_2str(int state) {
    return "established";
}

void ipv4_address_to_txt(uint32_t addr, char *buf) {
    snprintf(buf, INET_ADDRSTRLEN, "%u.%u.%u.%u",
             (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
             (addr >> 8) & 0xFF, addr & 0xFF);
}

void ipv6_address_to_txt(const ipv6_addr *addr, char *buf) {
    strcpy(buf, "::1");
}

bool local_sockets_is_zero_address(const socket_endpoint *ep) {
    return ep->ip.ipv4 == 0;
}

bool is_local_socket_ipv46(const LOCAL_SOCKET *s) {
    return false;
}

const char *local_sockets_address_space(const socket_endpoint *ep) {
    return "public";
}

typedef struct {
    char *username;
} CACHED_USERNAME;

CACHED_USERNAME cached_username_get_by_uid(uid_t uid) {
    CACHED_USERNAME cu;
    cu.username = strdup("root");
    return cu;
}

void cached_username_release(CACHED_USERNAME cu) {
    free(cu.username);
}

typedef void *STRING;

STRING system_servicenames_cache_lookup(void *sc, uint16_t port, int protocol) {
    return (STRING)"http";
}

size_t quoted_strings_splitter_whitespace(char *buf, char **words, size_t max_words) {
    int count = 0;
    char *ptr = buf;
    while (*ptr && count < max_words) {
        if (*ptr != ' ' && *ptr != '\t') {
            words[count++] = ptr;
            while (*ptr && *ptr != ' ' && *ptr != '\t') ptr++;
            if (*ptr) *ptr++ = '\0';
        } else {
            ptr++;
        }
    }
    return count;
}

char *get_word(char **words, size_t num_words, size_t index) {
    if (index < num_words) return words[index];
    return "";
}

void local_sockets_process(LS_STATE *ls) {}

char *string2str(void *s) {
    return s ? (char *)s : "";
}

#define UID_UNSET (uid_t)-1
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define AF_INET 2
#define AF_INET6 10
#define USEC_PER_MS 1000
#define TCP_ESTABLISHED 1
#define TCP_SYN_SENT 2
#define TCP_SYN_RECV 3
#define TCP_FIN_WAIT1 4
#define TCP_FIN_WAIT2 5
#define TCP_TIME_WAIT 6
#define TCP_CLOSE 7
#define TCP_CLOSE_WAIT 8
#define TCP_LAST_ACK 9
#define TCP_LISTEN 10
#define TCP_CLOSING 11

/* Test cases */
static void test_local_socket_to_json_array_ipv4_tcp_listen(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_LISTEN,
        .local.family = AF_INET,
        .local.protocol = IPPROTO_TCP,
        .local.port = 80,
        .remote.family = AF_INET,
        .remote.ip.ipv4 = 0,
        .pid = 1234,
        .uid = 0,
        .comm = "test",
        .state = TCP_LISTEN,
        .net_ns_inode = 1,
        .info.tcp.tcpi_rtt = 5000,
        .info.tcp.tcpi_rcv_rtt = 3000,
        .info.tcp.tcpi_total_retrans = 10,
        .network_viewer.count = 1,
        .network_viewer.aggregated_key.local_address_space = "public",
        .network_viewer.aggregated_key.remote_address_space = "public",
    };
    
    local_socket_to_json_array(&st, &sock, 1, false);
    
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_ipv6_tcp(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_INBOUND,
        .local.family = AF_INET6,
        .local.protocol = IPPROTO_TCP,
        .local.port = 443,
        .remote.family = AF_INET6,
        .pid = 5678,
        .uid = 1000,
        .comm = "sshd",
        .state = TCP_ESTABLISHED,
        .net_ns_inode = 1,
        .info.tcp.tcpi_rtt = 10000,
        .info.tcp.tcpi_rcv_rtt = 8000,
        .info.tcp.tcpi_total_retrans = 5,
        .network_viewer.count = 1,
        .network_viewer.aggregated_key.local_address_space = "public",
        .network_viewer.aggregated_key.remote_address_space = "private",
    };
    
    local_socket_to_json_array(&st, &sock, 1, false);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_udp(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_OUTBOUND,
        .local.family = AF_INET,
        .local.protocol = IPPROTO_UDP,
        .local.port = 53,
        .remote.family = AF_INET,
        .remote.port = 53,
        .pid = 9999,
        .uid = 0,
        .comm = "resolved",
        .state = 0,
        .net_ns_inode = 2,
        .info.tcp.tcpi_rtt = 0,
        .info.tcp.tcpi_rcv_rtt = 0,
        .info.tcp.tcpi_total_retrans = 0,
        .network_viewer.count = 1,
        .network_viewer.aggregated_key.local_address_space = "loopback",
        .network_viewer.aggregated_key.remote_address_space = "public",
    };
    
    local_socket_to_json_array(&st, &sock, 2, false);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_direction_none(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_NONE,
        .pid = 0,
    };
    
    local_socket_to_json_array(&st, &sock, 1, false);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_aggregated(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_LISTEN,
        .local.family = AF_INET,
        .local.protocol = IPPROTO_TCP,
        .local.port = 8080,
        .remote.family = AF_INET,
        .pid = 1234,
        .uid = 1000,
        .comm = "nginx",
        .state = TCP_LISTEN,
        .net_ns_inode = 1,
        .info.tcp.tcpi_rtt = 15000,
        .info.tcp.tcpi_rcv_rtt = 12000,
        .info.tcp.tcpi_total_retrans = 20,
        .network_viewer.count = 5,
        .network_viewer.aggregated_key.local_address_space = "public",
        .network_viewer.aggregated_key.remote_address_space = "public",
        .network_viewer.aggregated_key.server.port = 8080,
    };
    
    local_socket_to_json_array(&st, &sock, 1, true);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_container_namespace(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_INBOUND,
        .local.family = AF_INET,
        .local.protocol = IPPROTO_TCP,
        .local.port = 3306,
        .remote.family = AF_INET,
        .remote.ip.ipv4 = 0xC0A80101,
        .remote.port = 12345,
        .pid = 2000,
        .uid = 999,
        .comm = "mysql",
        .state = TCP_ESTABLISHED,
        .net_ns_inode = 999,
        .info.tcp.tcpi_rtt = 20000,
        .info.tcp.tcpi_rcv_rtt = 18000,
        .info.tcp.tcpi_total_retrans = 15,
        .network_viewer.count = 3,
        .network_viewer.aggregated_key.local_address_space = "private",
        .network_viewer.aggregated_key.remote_address_space = "private",
        .network_viewer.aggregated_key.server.port = 3306,
    };
    
    local_socket_to_json_array(&st, &sock, 1, false);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_unknown_namespace(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_OUTBOUND,
        .local.family = AF_INET,
        .local.protocol = IPPROTO_TCP,
        .local.port = 54321,
        .remote.family = AF_INET,
        .remote.ip.ipv4 = 0x08080808,
        .remote.port = 443,
        .pid = 3000,
        .uid = 100,
        .comm = "curl",
        .state = TCP_ESTABLISHED,
        .net_ns_inode = 0,
        .info.tcp.tcpi_rtt = 25000,
        .info.tcp.tcpi_rcv_rtt = 20000,
        .info.tcp.tcpi_total_retrans = 0,
        .network_viewer.count = 1,
        .network_viewer.aggregated_key.local_address_space = "public",
        .network_viewer.aggregated_key.remote_address_space = "public",
        .network_viewer.aggregated_key.server.port = 443,
    };
    
    local_socket_to_json_array(&st, &sock, 555, false);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_unknown_uid(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_LISTEN,
        .local.family = AF_INET,
        .local.protocol = IPPROTO_TCP,
        .local.port = 9000,
        .remote.family = AF_INET,
        .pid = 4000,
        .uid = UID_UNSET,
        .comm = "",
        .state = TCP_LISTEN,
        .net_ns_inode = 1,
        .info.tcp.tcpi_rtt = 0,
        .info.tcp.tcpi_rcv_rtt = 0,
        .info.tcp.tcpi_total_retrans = 0,
        .network_viewer.count = 1,
        .network_viewer.aggregated_key.local_address_space = "public",
        .network_viewer.aggregated_key.remote_address_space = "public",
        .network_viewer.aggregated_key.server.port = 9000,
    };
    
    local_socket_to_json_array(&st, &sock, 1, false);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_local_outbound(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_LOCAL_OUTBOUND,
        .local.family = AF_INET,
        .local.protocol = IPPROTO_TCP,
        .local.port = 12345,
        .remote.family = AF_INET,
        .remote.port = 5432,
        .pid = 5000,
        .uid = 1000,
        .comm = "python",
        .state = TCP_ESTABLISHED,
        .net_ns_inode = 1,
        .info.tcp.tcpi_rtt = 1000,
        .info.tcp.tcpi_rcv_rtt = 500,
        .info.tcp.tcpi_total_retrans = 0,
        .network_viewer.count = 1,
        .network_viewer.aggregated_key.local_address_space = "loopback",
        .network_viewer.aggregated_key.remote_address_space = "loopback",
        .network_viewer.aggregated_key.server.port = 5432,
    };
    
    local_socket_to_json_array(&st, &sock, 1, false);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_local_socket_to_json_array_local_inbound(void **state) {
    sockets_stats st = { .wb = buffer_create(0, NULL), .max = {0} };
    
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_LOCAL_INBOUND,
        .local.family = AF_INET,
        .local.protocol = IPPROTO_TCP,
        .local.port = 5432,
        .remote.family = AF_INET,
        .remote.port = 12345,
        .pid = 6000,
        .uid = 1000,
        .comm = "postgres",
        .state = TCP_ESTABLISHED,
        .net_ns_inode = 1,
        .info.tcp.tcpi_rtt = 2000,
        .info.tcp.tcpi_rcv_rtt = 1500,
        .info.tcp.tcpi_total_retrans = 1,
        .network_viewer.count = 1,
        .network_viewer.aggregated_key.local_address_space = "loopback",
        .network_viewer.aggregated_key.remote_address_space = "loopback",
        .network_viewer.aggregated_key.server.port = 5432,
    };
    
    local_socket_to_json_array(&st, &sock, 1, false);
    assert_non_null(st.wb);
    buffer_free(st.wb);
}

static void test_populate_aggregated_key_listen(void **state) {
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_LISTEN,
        .pid = 100,
        .uid = 0,
        .state = TCP_LISTEN,
        .net_ns_inode = 1,
        .local.port = 80,
        .local.protocol = IPPROTO_TCP,
        .local.family = AF_INET,
    };
    
    populate_aggregated_key(&sock);
    
    assert_int_equal(sock.network_viewer.count, 1);
    assert_int_equal(sock.network_viewer.aggregated_key.pid, 100);
    assert_int_equal(sock.network_viewer.aggregated_key.uid, 0);
    assert_int_equal(sock.network_viewer.aggregated_key.direction, SOCKET_DIRECTION_LISTEN);
    assert_int_equal(sock.network_viewer.aggregated_key.server.port, 80);
}

static void test_populate_aggregated_key_inbound(void **state) {
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_INBOUND,
        .pid = 200,
        .uid = 1000,
        .state = TCP_ESTABLISHED,
        .net_ns_inode = 2,
        .local.port = 443,
        .local.protocol = IPPROTO_TCP,
        .local.family = AF_INET6,
        .remote.port = 54321,
    };
    
    populate_aggregated_key(&sock);
    
    assert_int_equal(sock.network_viewer.count, 1);
    assert_int_equal(sock.network_viewer.aggregated_key.pid, 200);
    assert_int_equal(sock.network_viewer.aggregated_key.server.port, 443);
}

static void test_populate_aggregated_key_outbound(void **state) {
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_OUTBOUND,
        .pid = 300,
        .uid = 500,
        .state = TCP_ESTABLISHED,
        .net_ns_inode = 3,
        .local.port = 54321,
        .remote.port = 443,
        .remote.protocol = IPPROTO_TCP,
    };
    
    populate_aggregated_key(&sock);
    
    assert_int_equal(sock.network_viewer.count, 1);
    assert_int_equal(sock.network_viewer.aggregated_key.server.port, 443);
}

static void test_populate_aggregated_key_local_inbound(void **state) {
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_LOCAL_INBOUND,
        .pid = 400,
        .uid = 100,
        .local.port = 5432,
        .remote.port = 12345,
    };
    
    populate_aggregated_key(&sock);
    
    assert_int_equal(sock.network_viewer.count, 1);
    assert_int_equal(sock.network_viewer.aggregated_key.server.port, 5432);
}

static void test_populate_aggregated_key_local_outbound(void **state) {
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_LOCAL_OUTBOUND,
        .pid = 500,
        .uid = 200,
        .local.port = 12345,
        .remote.port = 5432,
    };
    
    populate_aggregated_key(&sock);
    
    assert_int_equal(sock.network_viewer.count, 1);
    assert_int_equal(sock.network_viewer.aggregated_key.server.port, 5432);
}

static void test_populate_aggregated_key_none(void **state) {
    LOCAL_SOCKET sock = {
        .direction = SOCKET_DIRECTION_NONE,
        .pid = 600,
        .uid = 300,
    };
    
    populate_aggregated_key(&sock);
    
    assert_int_equal(sock.network_viewer.count, 1);
}

static void test_local_sockets_compar(void **state) {
    LOCAL_SOCKET sock1 = { .comm = "aaa" };
    LOCAL_SOCKET sock2 = { .comm = "bbb" };
    LOCAL_SOCKET sock3 = { .comm = "aaa" };
    
    LOCAL_SOCKET *p1 = &sock1;
    LOCAL_SOCKET *p2 = &sock2;
    LOCAL_SOCKET *p3 = &sock3;
    
    assert_true(local_sockets_compar(&p1, &p2) < 0);
    assert_true(local_sockets_compar(&p2, &p1) > 0);
    assert_int_equal(local_sockets_compar(&p1, &p3), 0);
}

int run_network_viewer_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_local_socket_to_json_array_ipv4_tcp_listen),
        cmocka_unit_test(test_local_socket_to_json_array_ipv6_tcp),
        cmocka_unit_test(test_local_socket_to_json_array_udp),
        cmocka_unit_test(test_local_socket_to_json_array_direction_none),
        cmocka_unit_test(test_local_socket_to_json_array_aggregated),
        cmocka_unit_test(test_local_socket_to_json_array_container_namespace),
        cmocka_unit_test(test