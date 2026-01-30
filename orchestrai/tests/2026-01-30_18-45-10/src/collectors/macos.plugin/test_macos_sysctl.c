#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <cmocka.h>
#include <setjmp.h>

// Mock structures and functions
typedef struct {
    double ldavg[3];
    uint32_t fscale;
} loadavg_t;

typedef struct {
    uint64_t xsu_total;
    uint64_t xsu_avail;
    uint64_t xsu_used;
} xsw_usage_t;

typedef struct {
    uint32_t tcps_rcvtotal;
    uint32_t tcps_sndtotal;
    uint32_t tcps_rcvbadoff;
    uint32_t tcps_rcvshort;
    uint32_t tcps_rcvbadsum;
    uint32_t tcps_sndrexmitpack;
    uint32_t tcps_drops;
    uint32_t tcps_connattempt;
    uint32_t tcps_accepts;
    uint32_t tcps_conndrops;
    uint32_t tcps_rcvpackafterwin;
    uint32_t tcps_rcvafterclose;
    uint32_t tcps_rcvmemdrop;
    uint32_t tcps_persistdrop;
    uint32_t tcps_rcvoopack;
    uint32_t tcps_sc_recvcookie;
    uint32_t tcps_sc_sendcookie;
    uint32_t tcps_sc_zonefail;
    uint32_t tcps_ecn_recv_ce;
    uint32_t tcps_ecn_not_supported;
} tcpstat_t;

typedef struct {
    uint32_t udps_ipackets;
    uint32_t udps_opackets;
    uint32_t udps_hdrops;
    uint32_t udps_badlen;
    uint32_t udps_noport;
    uint32_t udps_fullsock;
    uint32_t udps_badsum;
    uint32_t udps_nosum;
    uint32_t udps_filtermcast;
} udpstat_t;

typedef struct {
    uint32_t icps_inhist[256];
    uint32_t icps_outhist[256];
    uint32_t icps_badcode;
    uint32_t icps_badlen;
    uint32_t icps_checksum;
    uint32_t icps_tooshort;
    uint32_t icps_error;
} icmpstat_t;

typedef struct {
    uint32_t ips_total;
    uint32_t ips_localout;
    uint32_t ips_forward;
    uint32_t ips_delivered;
    uint32_t ips_fragmented;
    uint32_t ips_cantfrag;
    uint32_t ips_ofragments;
    uint32_t ips_fragments;
    uint32_t ips_fragdropped;
    uint32_t ips_reassembled;
    uint32_t ips_badsum;
    uint32_t ips_tooshort;
    uint32_t ips_toosmall;
    uint32_t ips_toolong;
    uint32_t ips_odropped;
    uint32_t ips_badhlen;
    uint32_t ips_badlen;
    uint32_t ips_badoptions;
    uint32_t ips_badvers;
    uint32_t ips_badaddr;
    uint32_t ips_noproto;
    uint32_t ips_noroute;
} ipstat_t;

typedef struct {
    uint64_t icp6s_inhist[256];
    uint64_t icp6s_outhist[256];
    uint64_t icp6s_badcode;
    uint64_t icp6s_badlen;
    uint64_t icp6s_checksum;
    uint64_t icp6s_tooshort;
    uint64_t icp6s_error;
} icmp6stat_t;

// Mock implementation
extern int do_macos_sysctl(int update_every, usec_t dt);

// Mock functions
static int mock_getsysctl_load_success = 1;
static int mock_getsysctl_swap_success = 1;
static int mock_getsysctl_bandwidth_success = 1;

int __wrap_GETSYSCTL_BY_NAME(const char *name, void *ptr) {
    if (strstr(name, "vm.loadavg")) {
        if (!mock_getsysctl_load_success) return 1;
        loadavg_t *sysload = (loadavg_t *)ptr;
        sysload->ldavg[0] = 1000;
        sysload->ldavg[1] = 2000;
        sysload->ldavg[2] = 3000;
        sysload->fscale = 65536;
        return 0;
    } else if (strstr(name, "vm.swapusage")) {
        if (!mock_getsysctl_swap_success) return 1;
        xsw_usage_t *swap = (xsw_usage_t *)ptr;
        swap->xsu_avail = 1000000;
        swap->xsu_used = 500000;
        return 0;
    }
    return 1;
}

void test_do_macos_sysctl_returns_zero(void **state) {
    (void) state;
    int result = do_macos_sysctl(10, 10 * 1000000);
    assert_int_equal(result, 0);
}

void test_do_macos_sysctl_with_zero_update_every(void **state) {
    (void) state;
    int result = do_macos_sysctl(0, 0);
    assert_int_equal(result, 0);
}

void test_do_macos_sysctl_with_large_dt(void **state) {
    (void) state;
    int result = do_macos_sysctl(10, 1000000000);
    assert_int_equal(result, 0);
}

void test_do_macos_sysctl_load_avg_disabled(void **state) {
    (void) state;
    mock_getsysctl_load_success = 0;
    int result = do_macos_sysctl(10, 10 * 1000000);
    assert_int_equal(result, 0);
    mock_getsysctl_load_success = 1;
}

void test_do_macos_sysctl_swap_disabled(void **state) {
    (void) state;
    mock_getsysctl_swap_success = 0;
    int result = do_macos_sysctl(10, 10 * 1000000);
    assert_int_equal(result, 0);
    mock_getsysctl_swap_success = 1;
}

void test_do_macos_sysctl_multiple_calls(void **state) {
    (void) state;
    int result1 = do_macos_sysctl(10, 10 * 1000000);
    int result2 = do_macos_sysctl(10, 10 * 1000000);
    int result3 = do_macos_sysctl(10, 10 * 1000000);
    assert_int_equal(result1, 0);
    assert_int_equal(result2, 0);
    assert_int_equal(result3, 0);
}

void test_do_macos_sysctl_min_loadavg_update(void **state) {
    (void) state;
    // With MIN_LOADAVG_UPDATE_EVERY = 5 seconds = 5000000 usec
    int result = do_macos_sysctl(1, 1000000); // 1 second
    assert_int_equal(result, 0);
}

void test_do_macos_sysctl_with_minimal_dt(void **state) {
    (void) state;
    int result = do_macos_sysctl(10, 1);
    assert_int_equal(result, 0);
}

void test_do_macos_sysctl_alternating_calls(void **state) {
    (void) state;
    for (int i = 0; i < 5; i++) {
        int result = do_macos_sysctl(10, 2 * 1000000);
        assert_int_equal(result, 0);
    }
}

void test_do_macos_sysctl_with_max_int_dt(void **state) {
    (void) state;
    int result = do_macos_sysctl(10, INT_MAX);
    assert_int_equal(result, 0);
}

void test_do_macos_sysctl_with_negative_dt(void **state) {
    (void) state;
    // Note: dt is unsigned, so this would overflow in practice
    int result = do_macos_sysctl(10, 10 * 1000000);
    assert_int_equal(result, 0);
}

void test_do_macos_sysctl_rapid_succession(void **state) {
    (void) state;
    for (int i = 0; i < 100; i++) {
        int result = do_macos_sysctl(10, 100000);
        assert_int_equal(result, 0);
    }
}

void test_do_macos_sysctl_varying_update_every(void **state) {
    (void) state;
    int freqs[] = {1, 5, 10, 30, 60, 300};
    for (size_t i = 0; i < sizeof(freqs) / sizeof(freqs[0]); i++) {
        int result = do_macos_sysctl(freqs[i], freqs[i] * 1000000);
        assert_int_equal(result, 0);
    }
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_macos_sysctl_returns_zero),
        cmocka_unit_test(test_do_macos_sysctl_with_zero_update_every),
        cmocka_unit_test(test_do_macos_sysctl_with_large_dt),
        cmocka_unit_test(test_do_macos_sysctl_load_avg_disabled),
        cmocka_unit_test(test_do_macos_sysctl_swap_disabled),
        cmocka_unit_test(test_do_macos_sysctl_multiple_calls),
        cmocka_unit_test(test_do_macos_sysctl_min_loadavg_update),
        cmocka_unit_test(test_do_macos_sysctl_with_minimal_dt),
        cmocka_unit_test(test_do_macos_sysctl_alternating_calls),
        cmocka_unit_test(test_do_macos_sysctl_with_max_int_dt),
        cmocka_unit_test(test_do_macos_sysctl_with_negative_dt),
        cmocka_unit_test(test_do_macos_sysctl_rapid_succession),
        cmocka_unit_test(test_do_macos_sysctl_varying_update_every),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}