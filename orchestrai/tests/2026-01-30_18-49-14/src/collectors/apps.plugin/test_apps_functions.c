// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock structures and defines
#define PLUGINSD_MAX_WORDS 256
#define NSEC_PER_SEC 1000000000
#define RATES_DETAIL 10000ULL
#define MAX_CMDLINE 65536
#define HTTP_RESP_OK 200
#define HTTP_RESP_BAD_REQUEST 400
#define CT_TEXT_PLAIN 1
#define CT_APPLICATION_JSON 2
#define RRDF_FIELD_TYPE_INTEGER 1
#define RRDF_FIELD_TYPE_STRING 2
#define RRDF_FIELD_TYPE_BAR_WITH_INTEGER 3
#define RRDF_FIELD_TYPE_DURATION 4
#define RRDF_FIELD_VISUAL_VALUE 1
#define RRDF_FIELD_VISUAL_BAR 2
#define RRDF_FIELD_TRANSFORM_NUMBER 1
#define RRDF_FIELD_TRANSFORM_NONE 2
#define RRDF_FIELD_TRANSFORM_DURATION_S 3
#define RRDF_FIELD_SORT_ASCENDING 1
#define RRDF_FIELD_SORT_DESCENDING 2
#define RRDF_FIELD_SUMMARY_COUNT 1
#define RRDF_FIELD_SUMMARY_SUM 2
#define RRDF_FIELD_SUMMARY_MAX 3
#define RRDF_FIELD_SUMMARY_MEAN 4
#define RRDF_FIELD_FILTER_MULTISELECT 1
#define RRDF_FIELD_FILTER_RANGE 2
#define RRDF_FIELD_OPTS_NONE 0x00
#define RRDF_FIELD_OPTS_VISIBLE 0x01
#define RRDF_FIELD_OPTS_STICKY 0x02
#define RRDF_FIELD_OPTS_UNIQUE_KEY 0x04
#define NAN 0.0 / 0.0
#define MAX(a,b) ((a) > (b) ? (a) : (b))

typedef double NETDATA_DOUBLE;
typedef unsigned long long uint64_t;
typedef unsigned long long kernel_uint_t;
typedef int pid_t;
typedef int uid_t;
typedef int gid_t;
typedef unsigned int HTTP_ACCESS;
typedef unsigned int RRDF_FIELD_OPTIONS;
typedef struct { char dummy; } BUFFER;
typedef struct { char dummy; } STRING;
typedef struct { char dummy; } SIMPLE_PATTERN;

enum {
    HTTP_ACCESS_SIGNED_ID = 1,
    HTTP_ACCESS_SAME_SPACE = 2,
    HTTP_ACCESS_SENSITIVE_DATA = 4,
    HTTP_ACCESS_VIEW_AGENT_CONFIG = 8,
    PDF_UTIME = 0,
    PDF_STIME = 1,
    PDF_GTIME = 2,
    PDF_CUTIME = 3,
    PDF_CSTIME = 4,
    PDF_CGTIME = 5,
    PDF_MINFLT = 6,
    PDF_MAJFLT = 7,
    PDF_CMINFLT = 8,
    PDF_CMAJFLT = 9,
    PDF_VMSIZE = 10,
    PDF_VMRSS = 11,
    PDF_VMSHARED = 12,
    PDF_MEM_ESTIMATED = 13,
    PDF_PSS = 14,
    PDF_VMSWAP = 15,
    PDF_VOLCTX = 16,
    PDF_NVOLCTX = 17,
    PDF_LREAD = 18,
    PDF_LWRITE = 19,
    PDF_PREAD = 20,
    PDF_PWRITE = 21,
    PDF_OREAD = 22,
    PDF_OWRITE = 23,
    PDF_UPTIME = 24,
    PDF_THREADS = 25,
    PDF_PROCESSES = 26,
    PDF_HANDLES = 27,
    PDF_MAX = 28,
};

typedef enum {
    TARGET_TYPE_APP_GROUP = 1,
    TARGET_TYPE_UID = 2,
    TARGET_TYPE_GID = 3,
    TARGET_TYPE_SID = 4,
} TARGET_TYPE;

struct openfds {
    kernel_uint_t files;
    kernel_uint_t pipes;
    kernel_uint_t sockets;
    kernel_uint_t inotifies;
    kernel_uint_t eventfds;
    kernel_uint_t timerfds;
    kernel_uint_t signalfds;
    kernel_uint_t eventpolls;
    kernel_uint_t other;
};

struct target {
    STRING *id;
    STRING *name;
    STRING *clean_name;
    TARGET_TYPE type;
    kernel_uint_t values[PDF_MAX];
    struct openfds openfds;
    NETDATA_DOUBLE max_open_files_percent;
    NETDATA_DOUBLE pss_total_ratio;
    bool exposed;
    struct target *target;
    struct target *next;
};

struct pid_stat {
    int32_t pid;
    int32_t ppid;
    struct pid_stat *parent;
    struct pid_stat *next;
    struct pid_stat *prev;
    struct target *target;
    struct target *uid_target;
    struct target *gid_target;
    struct target *sid_target;
    STRING *comm;
    STRING *cmdline;
    kernel_uint_t raw[PDF_MAX];
    kernel_uint_t values[PDF_MAX];
    uid_t uid;
    gid_t gid;
    struct openfds openfds;
    NETDATA_DOUBLE openfds_limits_percent;
    bool read;
    bool updated;
    bool merged;
    usec_t stat_collected_usec;
    usec_t last_stat_collected_usec;
    size_t last_pss_iteration;
    NETDATA_DOUBLE pss_total_ratio;
};

typedef long long usec_t;
typedef long long time_t;

// Global mocks
bool enable_function_cmdline = false;
struct target *apps_groups_root_target = NULL;
struct target *users_root_target = NULL;
struct target *groups_root_target = NULL;
struct target *sids_root_target = NULL;
int update_every = 10;
size_t global_iterations_counter = 100;
int pss_refresh_period = 3600;

netdata_mutex_t apps_and_stdout_mutex;

// Function signatures to mock
typedef struct pid_stat *(*root_of_pids_fn)(void);
typedef time_t (*now_realtime_sec_fn)(void);
typedef uint64_t (*apps_os_get_total_memory_fn)(void);
typedef bool (*http_access_user_has_enough_access_level_for_endpoint_fn)(HTTP_ACCESS, HTTP_ACCESS);

// Mock implementations
static struct pid_stat *mock_root_pid = NULL;

struct pid_stat *mock_root_of_pids(void) {
    return mock_root_pid;
}

time_t mock_now_realtime_sec(void) {
    return 1000000;
}

uint64_t mock_apps_os_get_total_memory(void) {
    return 8ULL * 1024 * 1024 * 1024; // 8GB
}

bool mock_http_access_user_has_enough_access(HTTP_ACCESS access, HTTP_ACCESS required) {
    return (access & required) == required;
}

// Minimal mock implementations for buffer and string functions
BUFFER *test_buffer = NULL;

BUFFER *buffer_create(size_t size, void *ptr) {
    BUFFER *buf = malloc(sizeof(BUFFER));
    return buf;
}

void buffer_free(BUFFER *buf) {
    if(buf) free(buf);
}

void buffer_json_initialize(BUFFER *wb, const char *q, const char *qq, int flags, bool minify, int options) {
}

void buffer_json_member_add_uint64(BUFFER *wb, const char *key, uint64_t value) {
}

void buffer_json_member_add_string(BUFFER *wb, const char *key, const char *value) {
}

void buffer_json_member_add_time_t(BUFFER *wb, const char *key, time_t value) {
}

void buffer_json_member_add_boolean(BUFFER *wb, const char *key, bool value) {
}

void buffer_json_member_add_array(BUFFER *wb, const char *key) {
}

void buffer_json_member_add_object(BUFFER *wb, const char *key) {
}

void buffer_json_add_array_item_uint64(BUFFER *wb, uint64_t value) {
}

void buffer_json_add_array_item_double(BUFFER *wb, NETDATA_DOUBLE value) {
}

void buffer_json_add_array_item_string(BUFFER *wb, const char *value) {
}

void buffer_json_add_array_item_array(BUFFER *wb) {
}

void buffer_json_array_close(BUFFER *wb) {
}

void buffer_json_object_close(BUFFER *wb) {
}

void buffer_json_finalize(BUFFER *wb) {
}

const char *string2str(STRING *s) {
    return s ? "test" : NULL;
}

size_t string_strlen(STRING *s) {
    return s ? 4 : 0;
}

void string_freez(STRING *s) {
}

void buffer_sprintf(BUFFER *wb, const char *fmt, ...) {
}

void pluginsd_function_result_to_stdout(const char *transaction, BUFFER *wb) {
}

void pluginsd_function_json_error_to_stdout(const char *transaction, int code, const char *message) {
}

void buffer_rrdf_table_add_field(BUFFER *wb, int id, const char *key, const char *name, 
                                  int type, int visual, int transform, int decimal_points, 
                                  const char *units, NETDATA_DOUBLE max, int sort, const char *sort_column, 
                                  int summary, int filter, RRDF_FIELD_OPTIONS opts, void *ptr) {
}

void netdata_mutex_lock(void *mutex) {
}

void netdata_mutex_unlock(void *mutex) {
}

size_t quoted_strings_splitter_whitespace(char *str, char **out, size_t max) {
    if(!str) return 0;
    size_t count = 0;
    char *copy = strdup(str);
    char *ptr = copy;
    char *word = strtok_r(ptr, " \t\n", &ptr);
    
    while(word && count < max) {
        out[count++] = strdup(word);
        word = strtok_r(NULL, " \t\n", &ptr);
    }
    free(copy);
    return count;
}

const char *get_word(char **words, size_t num_words, size_t index) {
    if(index >= num_words) return NULL;
    return words[index];
}

struct target *find_target_by_name(struct target *base, const char *name) {
    if(!name) return NULL;
    struct target *t = base;
    while(t) {
        if(string2str(t->name) && strcmp(string2str(t->name), name) == 0)
            return t;
        t = t->next;
    }
    return NULL;
}

int str2i(const char *value) {
    return atoi(value);
}

// Test setup and teardown
int setup_function_tests(void **state) {
    mock_root_pid = NULL;
    enable_function_cmdline = false;
    return 0;
}

int teardown_function_tests(void **state) {
    // Cleanup
    return 0;
}

// ============================================================================
// TEST: function_processes with help filter
// ============================================================================
static void test_function_processes_help(void **state) {
    const char *transaction = "test-tx-1";
    char function[] = "processes help";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    // This should return early with help text
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with invalid category
// ============================================================================
static void test_function_processes_invalid_category(void **state) {
    const char *transaction = "test-tx-2";
    char function[] = "processes category:nonexistent";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with info flag
// ============================================================================
static void test_function_processes_with_info_flag(void **state) {
    const char *transaction = "test-tx-3";
    char function[] = "processes info";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with no filters and no processes
// ============================================================================
static void test_function_processes_no_processes(void **state) {
    const char *transaction = "test-tx-4";
    char function[] = "processes";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    mock_root_pid = NULL;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with single process
// ============================================================================
static void test_function_processes_single_process(void **state) {
    struct pid_stat p;
    struct target t;
    struct openfds fd_stats = {10, 5, 3, 0, 0, 0, 0, 0, 2};
    
    memset(&p, 0, sizeof(p));
    memset(&t, 0, sizeof(t));
    
    p.pid = 1234;
    p.ppid = 1;
    p.updated = true;
    p.read = false;
    p.target = &t;
    p.comm = (STRING *)malloc(1);
    p.cmdline = (STRING *)malloc(1);
    p.uid_target = NULL;
    p.gid_target = NULL;
    p.sid_target = NULL;
    p.openfds = fd_stats;
    p.next = NULL;
    
    // Set up values
    for(int i = 0; i < PDF_MAX; i++) {
        p.values[i] = i * 100;
    }
    p.values[PDF_UTIME] = 500;
    p.values[PDF_STIME] = 300;
    p.values[PDF_VMRSS] = 1024 * 1024; // 1MB
    p.values[PDF_VMSIZE] = 10 * 1024 * 1024; // 10MB
    
    t.name = (STRING *)malloc(1);
    t.openfds = fd_stats;
    
    mock_root_pid = &p;
    
    const char *transaction = "test-tx-5";
    char function[] = "processes";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p.comm);
    free(p.cmdline);
    free(t.name);
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with process name filter matching parent
// ============================================================================
static void test_function_processes_process_name_filter_parent(void **state) {
    struct pid_stat parent, child;
    struct target t;
    
    memset(&parent, 0, sizeof(parent));
    memset(&child, 0, sizeof(child));
    memset(&t, 0, sizeof(t));
    
    parent.pid = 100;
    parent.ppid = 1;
    parent.updated = true;
    parent.read = false;
    parent.target = &t;
    parent.comm = (STRING *)malloc(1);
    parent.cmdline = NULL;
    parent.parent = NULL;
    parent.next = &child;
    
    child.pid = 101;
    child.ppid = 100;
    child.updated = true;
    child.read = false;
    child.target = &t;
    child.comm = (STRING *)malloc(1);
    child.cmdline = NULL;
    child.parent = &parent;
    child.next = NULL;
    
    t.name = (STRING *)malloc(1);
    
    mock_root_pid = &parent;
    
    const char *transaction = "test-tx-6";
    char function[] = "processes process:bash";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(parent.comm);
    free(child.comm);
    free(t.name);
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with PID filter matching parent
// ============================================================================
static void test_function_processes_pid_filter_parent(void **state) {
    struct pid_stat p1, p2;
    struct target t;
    
    memset(&p1, 0, sizeof(p1));
    memset(&p2, 0, sizeof(p2));
    memset(&t, 0, sizeof(t));
    
    p1.pid = 1000;
    p1.ppid = 1;
    p1.updated = true;
    p1.read = false;
    p1.target = &t;
    p1.comm = (STRING *)malloc(1);
    p1.cmdline = NULL;
    p1.parent = NULL;
    p1.next = &p2;
    
    p2.pid = 1001;
    p2.ppid = 1000;
    p2.updated = true;
    p2.read = false;
    p2.target = &t;
    p2.comm = (STRING *)malloc(1);
    p2.cmdline = NULL;
    p2.parent = &p1;
    p2.next = NULL;
    
    t.name = (STRING *)malloc(1);
    
    mock_root_pid = &p1;
    
    const char *transaction = "test-tx-7";
    char function[] = "processes pid:1000";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p1.comm);
    free(p2.comm);
    free(t.name);
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with UID filter
// ============================================================================
static void test_function_processes_uid_filter(void **state) {
    struct pid_stat p;
    struct target t_uid, t_group;
    
    memset(&p, 0, sizeof(p));
    memset(&t_uid, 0, sizeof(t_uid));
    memset(&t_group, 0, sizeof(t_group));
    
    p.pid = 2000;
    p.ppid = 1;
    p.uid = 1000;
    p.gid = 1000;
    p.updated = true;
    p.read = false;
    p.target = &t_group;
    p.uid_target = &t_uid;
    p.comm = (STRING *)malloc(1);
    p.cmdline = NULL;
    p.parent = NULL;
    p.next = NULL;
    
    t_uid.name = (STRING *)malloc(1);
    t_group.name = (STRING *)malloc(1);
    
    mock_root_pid = &p;
    
    const char *transaction = "test-tx-8";
    char function[] = "processes uid:1000";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p.comm);
    free(t_uid.name);
    free(t_group.name);
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with GID filter
// ============================================================================
static void test_function_processes_gid_filter(void **state) {
    struct pid_stat p;
    struct target t;
    
    memset(&p, 0, sizeof(p));
    memset(&t, 0, sizeof(t));
    
    p.pid = 3000;
    p.ppid = 1;
    p.gid = 2000;
    p.updated = true;
    p.read = false;
    p.target = &t;
    p.gid_target = &t;
    p.comm = (STRING *)malloc(1);
    p.cmdline = NULL;
    p.parent = NULL;
    p.next = NULL;
    
    t.name = (STRING *)malloc(1);
    
    mock_root_pid = &p;
    
    const char *transaction = "test-tx-9";
    char function[] = "processes gid:2000";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p.comm);
    free(t.name);
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with category filter
// ============================================================================
static void test_function_processes_with_valid_category(void **state) {
    struct pid_stat p;
    struct target category, group;
    
    memset(&p, 0, sizeof(p));
    memset(&category, 0, sizeof(category));
    memset(&group, 0, sizeof(group));
    
    p.pid = 4000;
    p.ppid = 1;
    p.updated = true;
    p.read = false;
    p.target = &category;
    p.gid_target = &group;
    p.comm = (STRING *)malloc(1);
    p.cmdline = NULL;
    p.parent = NULL;
    p.next = NULL;
    
    category.name = (STRING *)malloc(1);
    group.name = (STRING *)malloc(1);
    
    apps_groups_root_target = &category;
    mock_root_pid = &p;
    
    const char *transaction = "test-tx-10";
    char function[] = "processes category:test";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p.comm);
    free(category.name);
    free(group.name);
    buffer_free(payload);
    apps_groups_root_target = NULL;
}

// ============================================================================
// TEST: function_processes access control - cmdline visibility
// ============================================================================
static void test_function_processes_cmdline_access_control(void **state) {
    struct pid_stat p;
    struct target t;
    
    memset(&p, 0, sizeof(p));
    memset(&t, 0, sizeof(t));
    
    p.pid = 5000;
    p.ppid = 1;
    p.updated = true;
    p.read = false;
    p.target = &t;
    p.comm = (STRING *)malloc(1);
    p.cmdline = (STRING *)malloc(1);
    p.parent = NULL;
    p.next = NULL;
    
    t.name = (STRING *)malloc(1);
    
    mock_root_pid = &p;
    
    // Test with low access level
    const char *transaction = "test-tx-11";
    char function[] = "processes";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = 0;  // No access
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p.comm);
    free(p.cmdline);
    free(t.name);
    buffer_free(payload);
}

// ============================================================================
// TEST: function_processes with multiple filters
// ============================================================================
static void test_function_processes_multiple_filters(void **state) {
    struct pid_stat p;
    struct target cat, uid_t;
    
    memset(&p, 0, sizeof(p));
    memset(&cat, 0, sizeof(cat));
    memset(&uid_t, 0, sizeof(uid_t));
    
    p.pid = 6000;
    p.ppid = 1;
    p.uid = 1000;
    p.updated = true;
    p.read = false;
    p.target = &cat;
    p.uid_target = &uid_t;
    p.comm = (STRING *)malloc(1);
    p.cmdline = NULL;
    p.parent = NULL;
    p.next = NULL;
    
    cat.name = (STRING *)malloc(1);
    uid_t.name = (STRING *)malloc(1);
    
    apps_groups_root_target = &cat;
    mock_root_pid = &p;
    
    const char *transaction = "test-tx-12";
    char function[] = "processes category:web uid:1000";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p.comm);
    free(cat.name);
    free(uid_t.name);
    buffer_free(payload);
    apps_groups_root_target = NULL;
}

// ============================================================================
// TEST: function_processes with pss data
// ============================================================================
static void test_function_processes_with_pss_data(void **state) {
    struct pid_stat p;
    struct target t;
    
    memset(&p, 0, sizeof(p));
    memset(&t, 0, sizeof(t));
    
    p.pid = 7000;
    p.ppid = 1;
    p.updated = true;
    p.read = false;
    p.target = &t;
    p.comm = (STRING *)malloc(1);
    p.cmdline = NULL;
    p.parent = NULL;
    p.next = NULL;
    p.last_pss_iteration = 99;
    p.pss_total_ratio = 0.5;
    
    // Set PSS-related values
    p.values[PDF_MEM_ESTIMATED] = 50 * 1024 * 1024;
    p.values[PDF_PSS] = 30 * 1024 * 1024;
    
    t.name = (STRING *)malloc(1);
    
    mock_root_pid = &p;
    pss_refresh_period = 3600;
    
    const char *transaction = "test-tx-13";
    char function[] = "processes";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p.comm);
    free(t.name);
    buffer_free(payload);
    pss_refresh_period = 0;
}

// ============================================================================
// TEST: function_processes with pss age calculation (wrapping)
// ============================================================================
static void test_function_processes_pss_age_wrapping(void **state) {
    struct pid_stat p;
    struct target t;
    
    memset(&p, 0, sizeof(p));
    memset(&t, 0, sizeof(t));
    
    p.pid = 8000;
    p.ppid = 1;
    p.updated = true;
    p.read = false;
    p.target = &t;
    p.comm = (STRING *)malloc(1);
    p.cmdline = NULL;
    p.parent = NULL;
    p.next = NULL;
    p.last_pss_iteration = 105;  // Greater than global_iterations_counter
    p.pss_total_ratio = 0.5;
    
    t.name = (STRING *)malloc(1);
    
    mock_root_pid = &p;
    pss_refresh_period = 3600;
    
    const char *transaction = "test-tx-14";
    char function[] = "processes";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = HTTP_ACCESS_SIGNED_ID;
    
    function_processes(transaction, function, NULL, &cancelled, payload, access, NULL, NULL);
    
    free(p.comm);
    free(t.name);
    buffer_free(payload);
    pss_refresh_period = 0;
}

// ============================================================================
// TEST: function_processes without cmdline enabled
// ============================================================================
static void test_function_processes_cmdline_disabled(void **state) {
    struct pid_stat p;
    struct target t;
    
    memset(&p, 0, sizeof(p));
    memset(&t, 0, sizeof(t));
    
    p.pid = 9000;
    p.ppid = 1;
    p.updated = true;
    p.read = false;
    p.target = &t;
    p.comm = (STRING *)malloc(1);
    p.cmdline = (STRING *)malloc(1);
    p.parent = NULL;
    p.next = NULL;
    
    t.name = (STRING *)malloc(1);
    
    mock_root_pid = &p;
    enable_function_cmdline = false;
    
    const char *transaction = "test-tx-15";
    char function[] = "processes";
    bool cancelled = false;
    BUFFER *payload = buffer_create(0, NULL);
    HTTP_ACCESS access = 0;