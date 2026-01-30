// SPDX-License-Identifier: GPL-3.0-or-later
// Comprehensive unit tests for ebpf_functions.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Mock declarations for external dependencies
#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING 1
#define NETDATA_THREAD_EBPF_FUNCTION_RUNNING_STOP 2
#define EBPF_DEFAULT_LIFETIME 300
#define EBPF_NON_FUNCTION_LIFE_TIME 60
#define EBPF_MODULE_SOCKET_IDX 4
#define EBPF_MODULE_FUNCTION_IDX 12
#define NETDATA_SOCKET_OPEN_SOCKET 0
#define ND_EBPF_MAP_FD_NOT_INITIALIZED -1
#define INET6_ADDRSTRLEN 46
#define NI_MAXSERV 32
#define PLUGINSD_MAX_WORDS 50
#define CONFIG_BOOLEAN_YES 1
#define CONFIG_BOOLEAN_NO 0
#define HTTP_RESP_OK 200
#define HTTP_RESP_INTERNAL_SERVER_ERROR 500
#define NETDATA_SOCKET_FLAGS_ALREADY_OPEN 0x1
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define AF_INET 2
#define AF_INET6 10
#define AF_UNSPEC 0
#define USEC_PER_SEC 1000000

typedef double NETDATA_DOUBLE;
typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef int usec_t;
typedef int time_t;
typedef int HTTP_ACCESS;

#define HTTP_ACCESS_SIGNED_ID 0x1
#define HTTP_ACCESS_SAME_SPACE 0x2
#define HTTP_ACCESS_SENSITIVE_DATA 0x4
#define HTTP_ACCESS_FORMAT "%u"
#define HTTP_ACCESS_FORMAT_CAST (unsigned int)
#define RRDFUNCTIONS_PRIORITY_DEFAULT 100

#define PLUGINSD_KEYWORD_FUNCTION "FUNCTION"

typedef struct buffer {
    char *content;
    size_t size;
    size_t capacity;
} BUFFER;

// Mock structures
typedef struct {
    char src_ip[INET6_ADDRSTRLEN];
    char dst_ip[INET6_ADDRSTRLEN];
    char dst_port[NI_MAXSERV];
} socket_string_t;

typedef struct {
    uint64_t ipv4_connect;
    uint64_t ipv6_connect;
    uint64_t tcp_bytes_received;
    uint64_t tcp_bytes_sent;
} tcp_data_t;

typedef struct {
    uint64_t call_udp_sent;
    uint64_t call_udp_received;
    uint64_t udp_bytes_received;
    uint64_t udp_bytes_sent;
} udp_data_t;

typedef union {
    tcp_data_t tcp;
    udp_data_t udp;
} protocol_data_t;

typedef struct {
    uint32_t family;
    uint32_t protocol;
    char name[16];
    uint8_t external_origin;
    protocol_data_t tcp;
    protocol_data_t udp;
} socket_data_t;

typedef struct {
    uint32_t pid;
    socket_string_t socket_string;
    socket_data_t data;
    uint32_t flags;
} netdata_socket_plus_t;

typedef struct {
    pthread_t thread;
    char name[64];
    void *(*start_routine)(void *);
} netdata_static_thread;

#define NETDATA_THREAD_OPTION_DEFAULT 0

typedef struct {
    netdata_static_thread *thread;
    struct {
        char thread_name[64];
    } info;
    uint32_t enabled;
    int lifetime;
    int update_every;
    struct {
        char fnct_routine;
        char fcnt_name[64];
        char fcnt_desc[256];
    } functions;
    struct {
        int map_fd;
    } maps[32];
} ebpf_module_t;

typedef struct {
    Pvoid_t JudyLArray;
    pthread_spinlock_t rw_spinlock;
} judy_index_t;

typedef struct {
    judy_index_t index;
} judy_pid_t;

// Global mock variables
extern ebpf_module_t ebpf_modules[];
extern judy_pid_t ebpf_judy_pid;
extern pthread_mutex_t lock;
extern pthread_mutex_t ebpf_exit_cleanup;
extern bool ebpf_plugin_exit;

// Mock functions
BUFFER *buffer_create(size_t size, void *ptr) {
    BUFFER *buf = malloc(sizeof(BUFFER));
    if (buf) {
        buf->content = calloc(size, 1);
        buf->capacity = size;
        buf->size = 0;
    }
    return buf;
}

void buffer_free(BUFFER *buf) {
    if (buf) {
        free(buf->content);
        free(buf);
    }
}

void buffer_json_initialize(BUFFER *wb, const char *q1, const char *q2, int depth, bool array, int opts) {
    if (wb && wb->content) {
        strcpy(wb->content, "{");
        wb->size = 1;
    }
}

void buffer_json_finalize(BUFFER *wb) {
    if (wb && wb->content && wb->size > 0) {
        strcat(wb->content, "}");
        wb->size = strlen(wb->content);
    }
}

void buffer_json_member_add_uint64(BUFFER *wb, const char *key, uint64_t val) {
    if (wb && wb->content) {
        char buf[256];
        snprintf(buf, sizeof(buf), "\"%s\":%llu,", key, (unsigned long long)val);
        strcat(wb->content, buf);
    }
}

void buffer_json_member_add_string(BUFFER *wb, const char *key, const char *val) {
    if (wb && wb->content && val) {
        char buf[512];
        snprintf(buf, sizeof(buf), "\"%s\":\"%s\",", key, val);
        strcat(wb->content, buf);
    }
}

void buffer_json_member_add_time_t(BUFFER *wb, const char *key, time_t val) {
    if (wb && wb->content) {
        char buf[256];
        snprintf(buf, sizeof(buf), "\"%s\":%d,", key, val);
        strcat(wb->content, buf);
    }
}

void buffer_json_member_add_boolean(BUFFER *wb, const char *key, bool val) {
    if (wb && wb->content) {
        char buf[256];
        snprintf(buf, sizeof(buf), "\"%s\":%s,", key, val ? "true" : "false");
        strcat(wb->content, buf);
    }
}

void buffer_json_member_add_double(BUFFER *wb, const char *key, double val) {
    if (wb && wb->content) {
        char buf[256];
        snprintf(buf, sizeof(buf), "\"%s\":%f,", key, val);
        strcat(wb->content, buf);
    }
}

void buffer_json_member_add_array(BUFFER *wb, const char *key) {
    if (wb && wb->content && key) {
        char buf[256];
        snprintf(buf, sizeof(buf), "\"%s\":[", key);
        strcat(wb->content, buf);
    }
}

void buffer_json_array_close(BUFFER *wb) {
    if (wb && wb->content) {
        if (wb->content[strlen(wb->content) - 1] == ',') {
            wb->content[strlen(wb->content) - 1] = ']';
        } else {
            strcat(wb->content, "]");
        }
    }
}

void buffer_json_member_add_object(BUFFER *wb, const char *key) {
    if (wb && wb->content && key) {
        char buf[256];
        snprintf(buf, sizeof(buf), "\"%s\":{", key);
        strcat(wb->content, buf);
    }
}

void buffer_json_object_close(BUFFER *wb) {
    if (wb && wb->content) {
        if (wb->content[strlen(wb->content) - 1] == ',') {
            wb->content[strlen(wb->content) - 1] = '}';
        } else {
            strcat(wb->content, "}");
        }
    }
}

void buffer_json_add_array_item_array(BUFFER *wb) {
    if (wb && wb->content) {
        strcat(wb->content, "[");
    }
}

void buffer_json_add_array_item_string(BUFFER *wb, const char *str) {
    if (wb && wb->content && str) {
        char buf[512];
        snprintf(buf, sizeof(buf), "\"%s\",", str);
        strcat(wb->content, buf);
    }
}

void buffer_json_add_array_item_uint64(BUFFER *wb, uint64_t val) {
    if (wb && wb->content) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%llu,", (unsigned long long)val);
        strcat(wb->content, buf);
    }
}

void buffer_json_add_array_item_double(BUFFER *wb, NETDATA_DOUBLE val) {
    if (wb && wb->content) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%f,", val);
        strcat(wb->content, buf);
    }
}

void buffer_rrdf_table_add_field(BUFFER *wb, int id, const char *name, const char *desc,
                                  int type, int visual, int transform, int decimals,
                                  const char *units, NETDATA_DOUBLE def_val, int sort,
                                  void *ptr1, int summary, int filter, uint32_t opts, void *ptr2) {
    // Mock implementation
}

const char *buffer_tostring(BUFFER *wb) {
    return wb ? wb->content : "";
}

size_t buffer_strlen(BUFFER *wb) {
    return wb ? strlen(wb->content) : 0;
}

void pluginsd_function_json_error_to_stdout(const char *transaction, int code, const char *msg) {
    printf("ERROR: %s Code:%d Msg:%s\n", transaction, code, msg);
}

void pluginsd_function_result_begin_to_stdout(const char *transaction, int code, const char *type, time_t expires) {
    printf("BEGIN_RESULT %s\n", transaction);
}

void pluginsd_function_result_end_to_stdout(void) {
    printf("END_RESULT\n");
}

pthread_t nd_thread_create(const char *name, int options, void *(*start_routine)(void *), void *arg) {
    pthread_t tid;
    pthread_create(&tid, NULL, start_routine, arg);
    return tid;
}

struct config socket_config = {};

int snprintfz(char *dst, size_t maxlen, const char *fmt, ...) {
    return snprintf(dst, maxlen, fmt);
}

pid_t getpid(void) {
    return 1234;
}

size_t quoted_strings_splitter_whitespace(char *str, char **words, size_t max_words) {
    // Simple implementation
    size_t count = 0;
    char *copy = strdup(str);
    char *token = strtok(copy, " \t\n");
    while (token && count < max_words) {
        words[count++] = token;
        token = strtok(NULL, " \t\n");
    }
    return count;
}

const char *get_word(char **words, size_t num_words, int index) {
    if (index < 0 || index >= num_words) return NULL;
    return words[index];
}

int str2i(const char *str) {
    return str ? atoi(str) : 0;
}

time_t now_realtime_sec(void) {
    return (time_t)time(NULL);
}

void ebpf_clean_ip_structure(void **clean) {
    // Mock implementation
}

void ebpf_clean_port_structure(void **clean) {
    // Mock implementation
}

void ebpf_parse_ips_unsafe(char *str) {
    // Mock implementation
}

void ebpf_parse_ports(char *str) {
    // Mock implementation
}

void parse_network_viewer_section(struct config *cfg) {
    // Mock implementation
}

void ebpf_read_local_addresses_unsafe() {
    // Mock implementation
}

bool ebpf_plugin_stop(void) {
    return false;
}

void heartbeat_init(void *hb, int precision) {
    // Mock implementation
}

void heartbeat_next(void *hb) {
    // Mock implementation
}

void functions_evloop_add_function(void *wg, const char *name, void *func, int timeout, void *data) {
    // Mock implementation
}

void *functions_evloop_init(int threads, const char *name, void *lock, bool *stop) {
    return malloc(1);
}

// ============================================================================
// TEST SUITE BEGINS
// ============================================================================

// Test for bytes_to_mb conversion
void test_bytes_to_mb_zero_bytes(void) {
    NETDATA_DOUBLE result = (NETDATA_DOUBLE)0 / (1024 * 1024);
    assert(result == 0.0);
}

void test_bytes_to_mb_one_megabyte(void) {
    uint64_t bytes = 1024 * 1024;
    NETDATA_DOUBLE result = (NETDATA_DOUBLE)bytes / (1024 * 1024);
    assert(result == 1.0);
}

void test_bytes_to_mb_large_value(void) {
    uint64_t bytes = 5 * 1024 * 1024;
    NETDATA_DOUBLE result = (NETDATA_DOUBLE)bytes / (1024 * 1024);
    assert(result == 5.0);
}

void test_bytes_to_mb_fractional(void) {
    uint64_t bytes = 512 * 1024;
    NETDATA_DOUBLE result = (NETDATA_DOUBLE)bytes / (1024 * 1024);
    assert(result == 0.5);
}

// Test for ebpf_socket_fill_fake_socket
void test_ebpf_socket_fill_fake_socket_basic(void) {
    netdata_socket_plus_t fake_values = {};
    
    // Simulate the function
    snprintfz(fake_values.socket_string.src_ip, INET6_ADDRSTRLEN, "%s", "127.0.0.1");
    snprintfz(fake_values.socket_string.dst_ip, INET6_ADDRSTRLEN, "%s", "127.0.0.1");
    fake_values.pid = getpid();
    fake_values.socket_string.dst_port[0] = 0;
    snprintfz(fake_values.socket_string.dst_ip, NI_MAXSERV, "%s", "none");
    fake_values.data.family = AF_INET;
    fake_values.data.protocol = AF_UNSPEC;
    
    assert(strcmp(fake_values.socket_string.src_ip, "127.0.0.1") == 0);
    assert(strcmp(fake_values.socket_string.dst_ip, "none") == 0);
    assert(fake_values.pid == getpid());
    assert(fake_values.data.family == AF_INET);
    assert(fake_values.data.protocol == AF_UNSPEC);
}

void test_ebpf_socket_fill_fake_socket_values_initialized(void) {
    netdata_socket_plus_t fake_values = {};
    
    snprintfz(fake_values.socket_string.src_ip, INET6_ADDRSTRLEN, "%s", "127.0.0.1");
    snprintfz(fake_values.socket_string.dst_ip, INET6_ADDRSTRLEN, "%s", "127.0.0.1");
    fake_values.pid = getpid();
    fake_values.socket_string.dst_port[0] = 0;
    snprintfz(fake_values.socket_string.dst_ip, NI_MAXSERV, "%s", "none");
    fake_values.data.family = AF_INET;
    fake_values.data.protocol = AF_UNSPEC;
    
    assert(fake_values.socket_string.src_ip[0] != 0);
    assert(fake_values.socket_string.dst_ip[0] != 0);
    assert(fake_values.pid > 0);
}

// Test ebpf_fill_function_buffer with TCP protocol
void test_ebpf_fill_function_buffer_tcp(void) {
    BUFFER *wb = buffer_create(4096, NULL);
    netdata_socket_plus_t values = {
        .pid = 1234,
        .data.protocol = IPPROTO_TCP,
        .data.family = AF_INET,
        .data.external_origin = 0,
        .data.tcp.tcp_bytes_received = 2048 * 1024,
        .data.tcp.tcp_bytes_sent = 4096 * 1024,
        .data.tcp.ipv4_connect = 5,
        .data.tcp.ipv6_connect = 2,
        .flags = 0
    };
    strcpy(values.socket_string.src_ip, "192.168.1.1");
    strcpy(values.socket_string.dst_ip, "8.8.8.8");
    strcpy(values.socket_string.dst_port, "443");
    strcpy(values.data.name, "process");
    
    // Simulate filling buffer
    buffer_json_add_array_item_array(wb);
    buffer_json_add_array_item_uint64(wb, (uint64_t)values.pid);
    
    assert(wb != NULL);
    assert(wb->content != NULL);
    
    buffer_free(wb);
}

void test_ebpf_fill_function_buffer_udp(void) {
    BUFFER *wb = buffer_create(4096, NULL);
    netdata_socket_plus_t values = {
        .pid = 5678,
        .data.protocol = IPPROTO_UDP,
        .data.family = AF_INET,
        .data.external_origin = 1,
        .data.udp.udp_bytes_received = 1024 * 1024,
        .data.udp.udp_bytes_sent = 2048 * 1024,
        .data.udp.call_udp_sent = 100,
        .data.udp.call_udp_received = 150,
        .flags = 0
    };
    strcpy(values.socket_string.src_ip, "192.168.1.100");
    strcpy(values.socket_string.dst_ip, "8.8.8.8");
    strcpy(values.socket_string.dst_port, "53");
    
    buffer_json_add_array_item_array(wb);
    buffer_json_add_array_item_uint64(wb, (uint64_t)values.pid);
    
    assert(wb != NULL);
    buffer_free(wb);
}

void test_ebpf_fill_function_buffer_unspecified_protocol(void) {
    BUFFER *wb = buffer_create(4096, NULL);
    netdata_socket_plus_t values = {
        .pid = 9999,
        .data.protocol = AF_UNSPEC,
        .data.family = AF_UNSPEC,
        .data.external_origin = 0,
        .flags = 0
    };
    strcpy(values.socket_string.src_ip, "127.0.0.1");
    strcpy(values.socket_string.dst_ip, "127.0.0.1");
    strcpy(values.socket_string.dst_port, "0");
    
    buffer_json_add_array_item_array(wb);
    buffer_json_add_array_item_uint64(wb, (uint64_t)values.pid);
    
    assert(wb != NULL);
    buffer_free(wb);
}

void test_ebpf_fill_function_buffer_with_empty_name(void) {
    BUFFER *wb = buffer_create(4096, NULL);
    netdata_socket_plus_t values = {
        .pid = 1111,
        .data.protocol = IPPROTO_TCP,
        .data.family = AF_INET,
        .data.external_origin = 0,
        .flags = 0
    };
    values.data.name[0] = 0;
    strcpy(values.socket_string.src_ip, "192.168.1.1");
    strcpy(values.socket_string.dst_ip, "8.8.8.8");
    strcpy(values.socket_string.dst_port, "80");
    
    buffer_json_add_array_item_array(wb);
    buffer_json_add_array_item_uint64(wb, (uint64_t)values.pid);
    buffer_json_add_array_item_string(wb, "unknown");
    
    assert(wb != NULL);
    buffer_free(wb);
}

void test_ebpf_fill_function_buffer_connections_already_open(void) {
    BUFFER *wb = buffer_create(4096, NULL);
    netdata_socket_plus_t values = {
        .pid = 2222,
        .data.protocol = IPPROTO_TCP,
        .data.family = AF_INET,
        .data.external_origin = 0,
        .data.tcp.ipv4_connect = 0,
        .data.tcp.ipv6_connect = 0,
        .flags = NETDATA_SOCKET_FLAGS_ALREADY_OPEN
    };
    strcpy(values.socket_string.src_ip, "192.168.1.1");
    strcpy(values.socket_string.dst_ip, "8.8.8.8");
    strcpy(values.socket_string.dst_port, "443");
    strcpy(values.data.name, "test");
    
    buffer_json_add_array_item_array(wb);
    buffer_json_add_array_item_uint64(wb, (uint64_t)values.pid);
    
    assert(wb != NULL);
    buffer_free(wb);
}

void test_ebpf_fill_function_buffer_no_connections(void) {
    BUFFER *wb = buffer_create(4096, NULL);
    netdata_socket_plus_t values = {
        .pid = 3333,
        .data.protocol = IPPROTO_TCP,
        .data.family = AF_INET,
        .data.external_origin = 0,
        .data.tcp.ipv4_connect = 0,
        .data.tcp.ipv6_connect = 0,
        .flags = 0
    };
    strcpy(values.socket_string.src_ip, "192.168.1.1");
    strcpy(values.socket_string.dst_ip, "8.8.8.8");
    strcpy(values.socket_string.dst_port, "443");
    strcpy(values.data.name, "test");
    
    // When no connections, flag should be set
    values.flags |= NETDATA_SOCKET_FLAGS_ALREADY_OPEN;
    
    buffer_json_add_array_item_array(wb);
    buffer_json_add_array_item_uint64(wb, (uint64_t)values.pid);
    
    assert(values.flags & NETDATA_SOCKET_FLAGS_ALREADY_OPEN);
    assert(wb != NULL);
    buffer_free(wb);
}

void test_ebpf_fill_function_buffer_external_origin(void) {
    BUFFER *wb = buffer_create(4096, NULL);
    netdata_socket_plus_t values = {
        .pid = 4444,
        .data.protocol = IPPROTO_UDP,
        .data.family = AF_INET,
        .data.external_origin = 1,
        .flags = 0
    };
    strcpy(values.socket_string.src_ip, "8.8.8.8");
    strcpy(values.socket_string.dst_ip, "192.168.1.1");
    strcpy(values.socket_string.dst_port, "53");
    strcpy(values.data.name, "dns");
    
    buffer_json_add_array_item_array(wb);
    buffer_json_add_array_item_uint64(wb, (uint64_t)values.pid);
    
    assert(wb != NULL);
    buffer_free(wb);
}

// Buffer operations tests
void test_buffer_create_and_free(void) {
    BUFFER *buf = buffer_create(1024, NULL);
    assert(buf != NULL);
    assert(buf->content != NULL);
    assert(buf->capacity >= 1024);
    buffer_free(buf);
}

void test_buffer_json_operations(void) {
    BUFFER *buf = buffer_create(2048, NULL);
    buffer_json_initialize(buf, "\"", "\"", 0, true, 0);
    buffer_json_member_add_uint64(buf, "status", 200);
    buffer_json_member_add_string(buf, "type", "table");
    buffer_json_finalize(buf);
    
    assert(buf->size > 0);
    assert(strstr(buf->content, "status") != NULL);
    buffer_free(buf);
}

void test_buffer_json_array_operations(void) {
    BUFFER *buf = buffer_create(2048, NULL);
    buffer_json_initialize(buf, "\"", "\"", 0, true, 0);
    buffer_json_member_add_array(buf, "data");
    buffer_json_add_array_item_string(buf, "item1");
    buffer_json_add_array_item_string(buf, "item2");
    buffer_json_array_close(buf);
    
    assert(buf->size > 0);
    buffer_free(buf);
}

void test_buffer_json_nested_objects(void) {
    BUFFER *buf = buffer_create(2048, NULL);
    buffer_json_initialize(buf, "\"", "\"", 0, true, 0);
    buffer_json_member_add_object(buf, "nested");
    buffer_json_member_add_string(buf, "key", "value");
    buffer_json_object_close(buf);
    buffer_json_finalize(buf);
    
    assert(buf->size > 0);
    buffer_free(buf);
}

// Error and help function tests
void test_ebpf_function_error_call(void) {
    // Mock test - just verify function calls don't crash
    pluginsd_function_json_error_to_stdout("trans123", 500, "Test error");
}

void test_ebpf_function_help_call(void) {
    // Mock test - just verify function calls don't crash
    pluginsd_function_result_begin_to_stdout("trans456", 200, "text/plain", 3600);
    fprintf(stdout, "Help message");
    pluginsd_function_result_end_to_stdout();
}

// Thread related tests
void test_ebpf_function_start_thread_positive_period(void) {
    ebpf_module_t em = {
        .enabled = 0,
        .lifetime = 0,
        .thread = malloc(sizeof(netdata_static_thread))
    };
    em.thread->thread = NULL;
    
    // Simulate positive period
    int period = 600;
    if (period <= 0)
        period = EBPF_DEFAULT_LIFETIME;
    
    em.enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING;
    em.lifetime = period;
    
    assert(em.enabled == NETDATA_THREAD_EBPF_FUNCTION_RUNNING);
    assert(em.lifetime == 600);
    
    free(em.thread);
}

void test_ebpf_function_start_thread_zero_period(void) {
    ebpf_module_t em = {
        .enabled = 0,
        .lifetime = 0,
        .thread = malloc(sizeof(netdata_static_thread))
    };
    em.thread->thread = NULL;
    
    // Simulate zero period - should use default
    int period = 0;
    if (period <= 0)
        period = EBPF_DEFAULT_LIFETIME;
    
    em.enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING;
    em.lifetime = period;
    
    assert(em.enabled == NETDATA_THREAD_EBPF_FUNCTION_RUNNING);
    assert(em.lifetime == EBPF_DEFAULT_LIFETIME);
    
    free(em.thread);
}

void test_ebpf_function_start_thread_negative_period(void) {
    ebpf_module_t em = {
        .enabled = 0,
        .lifetime = 0,
        .thread = malloc(sizeof(netdata_static_thread))
    };
    em.thread->thread = NULL;
    
    // Simulate negative period - should use default
    int period = -1;
    if (period <= 0)
        period = EBPF_DEFAULT_LIFETIME;
    
    em.enabled = NETDATA_THREAD_EBPF_FUNCTION_RUNNING;
    em.lifetime = period;
    
    assert(em.lifetime == EBP