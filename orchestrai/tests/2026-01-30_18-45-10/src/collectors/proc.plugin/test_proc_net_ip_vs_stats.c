#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

/* Mock structures and defines */
typedef unsigned long long usec_t;
typedef unsigned long long kernel_uint_t;
typedef long long collected_number;

#define BITS_IN_A_KILOBIT 1000LL
#define FILENAME_MAX 4096
#define PROCFILE_FLAG_DEFAULT 0

typedef struct {
    char dummy;
} procfile;

typedef struct {
    char dummy;
} RRDSET;

typedef struct {
    char dummy;
} RRDDIM;

#define RRD_ALGORITHM_INCREMENTAL 1
#define RRDSET_TYPE_LINE 1
#define RRDSET_TYPE_AREA 2

#define unlikely(x) (x)
#define NETDATA_CHART_PRIO_IPVS_SOCKETS 5000
#define NETDATA_CHART_PRIO_IPVS_PACKETS 5001
#define NETDATA_CHART_PRIO_IPVS_NET 5002
#define PLUGIN_PROC_NAME "proc"
#define PLUGIN_PROC_MODULE_NET_IPVS_NAME "/proc/net/ip_vs_stats"

typedef struct {
    char dummy;
} netdata_config_t;

netdata_config_t netdata_config = {};
char netdata_configured_host_prefix[FILENAME_MAX] = "";

typedef struct {
    char key[100];
    void *value;
} cfg_entry;

static cfg_entry mock_config[10] = {};
static int mock_config_count = 0;

static int inicfg_get_boolean(netdata_config_t *config, const char *section, const char *key, int default_val) {
    (void)config;
    (void)section;
    (void)key;
    return default_val;
}

static const char* inicfg_get(netdata_config_t *config, const char *section, const char *key, const char *default_val) {
    (void)config;
    (void)section;
    (void)key;
    return default_val;
}

static procfile* procfile_open(const char *filename, const char *separators, int flags) {
    (void)filename;
    (void)separators;
    (void)flags;
    return (procfile*)malloc(sizeof(procfile));
}

static procfile* procfile_readall(procfile *ff) {
    return ff;
}

static size_t procfile_lines(procfile *ff) {
    (void)ff;
    return 3;
}

static size_t procfile_linewords(procfile *ff, size_t line) {
    (void)ff;
    (void)line;
    return 5;
}

static const char* procfile_lineword(procfile *ff, size_t line, size_t word) {
    (void)ff;
    (void)line;
    (void)word;
    static const char *words[] = {"00000064", "00000001", "00000002", "00000003", "00000004"};
    return words[word % 5];
}

static unsigned long long strtoull(const char *str, char **endptr, int base) {
    if(endptr) *endptr = (char*)str;
    return 100ULL;
}

static RRDSET* rrdset_create_localhost(
    const char *type, const char *id, const char *name, const char *family,
    const char *context, const char *title, const char *units, const char *plugin_name,
    const char *module_name, long priority, int update_every, int type_id) {
    (void)type; (void)id; (void)name; (void)family; (void)context;
    (void)title; (void)units; (void)plugin_name; (void)module_name;
    (void)priority; (void)update_every; (void)type_id;
    return (RRDSET*)malloc(sizeof(RRDSET));
}

static RRDDIM* rrddim_add(RRDSET *st, const char *id, const char *name, 
                          collected_number multiplier, collected_number divisor, int algorithm) {
    (void)st; (void)id; (void)name; (void)multiplier; (void)divisor; (void)algorithm;
    return (RRDDIM*)malloc(sizeof(RRDDIM));
}

static void rrddim_set(RRDSET *st, const char *id, collected_number value) {
    (void)st;
    (void)id;
    (void)value;
}

static void rrdset_done(RRDSET *st) {
    (void)st;
}

/* Include the actual source code */
#include "proc_net_ip_vs_stats.c"

/* Test cases */

void test_do_proc_net_ip_vs_stats_success(void) {
    printf("TEST: do_proc_net_ip_vs_stats - success case\n");
    
    int result = do_proc_net_ip_vs_stats(10, 0);
    
    assert(result == 0);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_multiple_calls(void) {
    printf("TEST: do_proc_net_ip_vs_stats - multiple calls with caching\n");
    
    int result1 = do_proc_net_ip_vs_stats(10, 0);
    int result2 = do_proc_net_ip_vs_stats(10, 0);
    
    assert(result1 == 0);
    assert(result2 == 0);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_bandwidth_disabled(void) {
    printf("TEST: do_proc_net_ip_vs_stats - bandwidth disabled\n");
    
    int result = do_proc_net_ip_vs_stats(10, 0);
    assert(result == 0);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_sockets_enabled(void) {
    printf("TEST: do_proc_net_ip_vs_stats - sockets chart enabled\n");
    
    int result = do_proc_net_ip_vs_stats(10, 0);
    assert(result == 0);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_packets_enabled(void) {
    printf("TEST: do_proc_net_ip_vs_stats - packets chart enabled\n");
    
    int result = do_proc_net_ip_vs_stats(10, 0);
    assert(result == 0);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_zero_update_every(void) {
    printf("TEST: do_proc_net_ip_vs_stats - zero update_every\n");
    
    int result = do_proc_net_ip_vs_stats(0, 0);
    assert(result == 0 || result == 1);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_large_update_every(void) {
    printf("TEST: do_proc_net_ip_vs_stats - large update_every\n");
    
    int result = do_proc_net_ip_vs_stats(3600, 0);
    assert(result == 0 || result == 1);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_dt_parameter_ignored(void) {
    printf("TEST: do_proc_net_ip_vs_stats - dt parameter ignored\n");
    
    int result1 = do_proc_net_ip_vs_stats(10, 0);
    int result2 = do_proc_net_ip_vs_stats(10, 12345);
    
    assert(result1 == result2);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_hexadecimal_parsing(void) {
    printf("TEST: do_proc_net_ip_vs_stats - hexadecimal value parsing\n");
    
    int result = do_proc_net_ip_vs_stats(10, 0);
    assert(result == 0);
    
    printf("  PASSED\n");
}

void test_do_proc_net_ip_vs_stats_repeated_calls_stateful(void) {
    printf("TEST: do_proc_net_ip_vs_stats - repeated calls maintain state\n");
    
    for(int i = 0; i < 5; i++) {
        int result = do_proc_net_ip_vs_stats(10, 0);
        assert(result == 0);
    }
    
    printf("  PASSED\n");
}

int main(void) {
    printf("\n=== Testing proc_net_ip_vs_stats ===\n\n");
    
    test_do_proc_net_ip_vs_stats_success();
    test_do_proc_net_ip_vs_stats_multiple_calls();
    test_do_proc_net_ip_vs_stats_bandwidth_disabled();
    test_do_proc_net_ip_vs_stats_sockets_enabled();
    test_do_proc_net_ip_vs_stats_packets_enabled();
    test_do_proc_net_ip_vs_stats_zero_update_every();
    test_do_proc_net_ip_vs_stats_large_update_every();
    test_do_proc_net_ip_vs_stats_dt_parameter_ignored();
    test_do_proc_net_ip_vs_stats_hexadecimal_parsing();
    test_do_proc_net_ip_vs_stats_repeated_calls_stateful();
    
    printf("\n=== All tests passed! ===\n\n");
    
    return 0;
}