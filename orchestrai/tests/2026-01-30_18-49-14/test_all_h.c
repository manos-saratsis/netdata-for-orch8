/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Test suite for src/collectors/all.h
 * This file contains comprehensive unit tests for all.h header
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Test framework setup */
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "ASSERTION FAILED: %s (line %d)\n", message, __LINE__); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(actual, expected, message) \
    do { \
        if ((actual) != (expected)) { \
            fprintf(stderr, "ASSERTION FAILED: %s (expected: %d, got: %d, line %d)\n", \
                    message, (int)expected, (int)actual, __LINE__); \
            return 0; \
        } \
    } while(0)

/* Include the header file being tested */
#include "src/collectors/all.h"

/* Test counter */
static int tests_passed = 0;
static int tests_failed = 0;

/* Test: Verify header guard exists */
int test_header_guard_exists(void) {
    /* If this compiles, NETDATA_ALL_H is defined */
#ifdef NETDATA_ALL_H
    tests_passed++;
    return 1;
#else
    tests_failed++;
    return 0;
#endif
}

/* Test: Verify chart priority constants are defined and non-zero */
int test_system_cpu_priority_defined(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_CPU == 100, "CPU priority mismatch");
    tests_passed++;
    return 1;
}

int test_system_load_priority_defined(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_LOAD == 100, "Load priority mismatch");
    tests_passed++;
    return 1;
}

int test_system_io_priority_defined(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_IO == 150, "IO priority mismatch");
    tests_passed++;
    return 1;
}

int test_system_ram_priority_defined(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_RAM == 200, "RAM priority mismatch");
    tests_passed++;
    return 1;
}

int test_system_net_priority_defined(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_NET == 500, "NET priority mismatch");
    tests_passed++;
    return 1;
}

int test_system_processes_priority_defined(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_PROCESSES == 600, "Processes priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Memory section priorities */
int test_mem_system_available_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_MEM_SYSTEM_AVAILABLE == 1010, "Memory available priority mismatch");
    tests_passed++;
    return 1;
}

int test_mem_kernel_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_MEM_KERNEL == 1100, "Memory kernel priority mismatch");
    tests_passed++;
    return 1;
}

int test_mem_zram_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_MEM_ZRAM == 1600, "Memory ZRAM priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Disk section priorities */
int test_disk_io_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_DISK_IO == 2000, "Disk IO priority mismatch");
    tests_passed++;
    return 1;
}

int test_disk_ops_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_DISK_OPS == 2010, "Disk OPS priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Network interface priorities */
int test_first_net_iface_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_FIRST_NET_IFACE == 7000, "First NET iface priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: IPv4 section priorities */
int test_ipv4_packets_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_IPV4_PACKETS == 5000, "IPv4 packets priority mismatch");
    tests_passed++;
    return 1;
}

int test_ipv4_tcp_packets_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_IPV4_TCP_PACKETS == 5170, "IPv4 TCP packets priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: IPv6 section priorities */
int test_ipv6_packets_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_IPV6_PACKETS == 6000, "IPv6 packets priority mismatch");
    tests_passed++;
    return 1;
}

int test_ipv6_tcp_packets_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_IPV6_TCP_PACKETS == 6130, "IPv6 TCP packets priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Hyper-V priorities */
int test_hyperv_vm_cpu_usage_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_WINDOWS_HYPERV_VM_CPU_USAGE == 20000, "Hyper-V CPU usage priority mismatch");
    tests_passed++;
    return 1;
}

int test_hyperv_vswitch_traffic_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_WINDOWS_HYPERV_VSWITCH_TRAFFIC == 20400, "Hyper-V vswitch traffic priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: CGROUP priorities */
int test_cgroups_systemd_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_CGROUPS_SYSTEMD == 19000, "CGROUPS systemd priority mismatch");
    tests_passed++;
    return 1;
}

int test_cgroups_containers_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_CGROUPS_CONTAINERS == 40000, "CGROUPS containers priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Sensor priorities */
int test_sensors_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SENSORS == 70000, "Sensors priority mismatch");
    tests_passed++;
    return 1;
}

int test_sensor_temperature_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SENSOR_TEMPERATURE == 70011, "Sensor temperature priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: ML chart priorities */
int test_ml_dimensions_priority(void) {
    TEST_ASSERT(ML_CHART_PRIO_DIMENSIONS == 39181, "ML dimensions priority mismatch");
    tests_passed++;
    return 1;
}

int test_ml_anomaly_rate_priority(void) {
    TEST_ASSERT(ML_CHART_PRIO_ANOMALY_RATE == 39182, "ML anomaly rate priority mismatch");
    tests_passed++;
    return 1;
}

int test_netdata_ml_chart_running(void) {
    TEST_ASSERT(NETDATA_ML_CHART_RUNNING == 890001, "NetData ML chart running priority mismatch");
    tests_passed++;
    return 1;
}

int test_netdata_ml_chart_training_status(void) {
    TEST_ASSERT(NETDATA_ML_CHART_PRIO_TRAINING_STATUS == 890004, "NetData ML training status priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: ML plugin constants */
int test_netdata_ml_plugin_defined(void) {
#ifdef NETDATA_ML_PLUGIN
    const char *plugin = NETDATA_ML_PLUGIN;
    TEST_ASSERT(plugin != NULL, "ML plugin is null");
    TEST_ASSERT(strcmp(plugin, "ml.plugin") == 0, "ML plugin name mismatch");
    tests_passed++;
    return 1;
#else
    tests_failed++;
    return 0;
#endif
}

int test_netdata_ml_module_training_defined(void) {
#ifdef NETDATA_ML_MODULE_TRAINING
    const char *module = NETDATA_ML_MODULE_TRAINING;
    TEST_ASSERT(module != NULL, "ML training module is null");
    TEST_ASSERT(strcmp(module, "training") == 0, "ML training module name mismatch");
    tests_passed++;
    return 1;
#else
    tests_failed++;
    return 0;
#endif
}

int test_netdata_ml_module_detection_defined(void) {
#ifdef NETDATA_ML_MODULE_DETECTION
    const char *module = NETDATA_ML_MODULE_DETECTION;
    TEST_ASSERT(module != NULL, "ML detection module is null");
    TEST_ASSERT(strcmp(module, "detection") == 0, "ML detection module name mismatch");
    tests_passed++;
    return 1;
#else
    tests_failed++;
    return 0;
#endif
}

int test_netdata_ml_module_prediction_defined(void) {
#ifdef NETDATA_ML_MODULE_PREDICTION
    const char *module = NETDATA_ML_MODULE_PREDICTION;
    TEST_ASSERT(module != NULL, "ML prediction module is null");
    TEST_ASSERT(strcmp(module, "prediction") == 0, "ML prediction module name mismatch");
    tests_passed++;
    return 1;
#else
    tests_failed++;
    return 0;
#endif
}

int test_netdata_ml_family_defined(void) {
#ifdef NETDATA_ML_CHART_FAMILY
    const char *family = NETDATA_ML_CHART_FAMILY;
    TEST_ASSERT(family != NULL, "ML chart family is null");
    TEST_ASSERT(strcmp(family, "machine learning") == 0, "ML chart family name mismatch");
    tests_passed++;
    return 1;
#else
    tests_failed++;
    return 0;
#endif
}

/* Test: Priority ordering - ensure monotonic increase in sections */
int test_priority_monotonic_increase_system(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_CPU < NETDATA_CHART_PRIO_SYSTEM_RAM, 
                "System priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_RAM < NETDATA_CHART_PRIO_SYSTEM_NET,
                "System priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

int test_priority_monotonic_increase_memory(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_MEM_SYSTEM_AVAILABLE < NETDATA_CHART_PRIO_MEM_KERNEL,
                "Memory priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_MEM_KERNEL < NETDATA_CHART_PRIO_MEM_ZRAM,
                "Memory priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

int test_priority_monotonic_increase_disk(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_DISK_IO < NETDATA_CHART_PRIO_DISK_OPS,
                "Disk priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_DISK_OPS < NETDATA_CHART_PRIO_DISK_BUSY,
                "Disk priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

int test_priority_monotonic_increase_network(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_NET < NETDATA_CHART_PRIO_IPV4_PACKETS,
                "Network priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_IPV4_PACKETS < NETDATA_CHART_PRIO_IPV6_PACKETS,
                "Network priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: Section boundaries */
int test_cgroup_boundary(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_CGROUPS_SYSTEMD < NETDATA_CHART_PRIO_CGROUPS_CONTAINERS,
                "CGROUP priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_CGROUPS_CONTAINERS < NETDATA_CHART_PRIO_SENSORS,
                "CGROUP to Sensor priority ordering broken");
    tests_passed++;
    return 1;
}

int test_sensor_boundary(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SENSORS < NETDATA_CHART_PRIO_STATSD_PRIVATE,
                "Sensor to STATSD priority ordering broken");
    tests_passed++;
    return 1;
}

int test_statsd_to_logs_boundary(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_STATSD_PRIVATE < NETDATA_CHART_PRIO_LOGS_BASE,
                "STATSD to Logs priority ordering broken");
    tests_passed++;
    return 1;
}

/* Test: High priority items for special cases */
int test_pci_aer_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_PCI_AER == 100000, "PCI AER priority mismatch");
    tests_passed++;
    return 1;
}

int test_drm_amdgpu_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_DRM_AMDGPU == 39000, "DRM AMDGPU priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: All TCP related priorities */
int test_tcp_priorities_order(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_IP_TCP_PACKETS < NETDATA_CHART_PRIO_IP_TCP_ERRORS,
                "TCP priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_IP_TCP_ERRORS < NETDATA_CHART_PRIO_IP_TCP_ESTABLISHED_CONNS,
                "TCP priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: Clock sync priorities */
int test_clock_sync_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_CLOCK_SYNC_STATE < NETDATA_CHART_PRIO_CLOCK_STATUS,
                "Clock sync priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_CLOCK_STATUS < NETDATA_CHART_PRIO_CLOCK_SYNC_OFFSET,
                "Clock sync priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: IPC priorities */
int test_ipc_msq_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_IPC_MSQ_QUEUES < NETDATA_CHART_PRIO_SYSTEM_IPC_MSQ_MESSAGES,
                "IPC MSQ priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_IPC_MSQ_MESSAGES < NETDATA_CHART_PRIO_SYSTEM_IPC_MSQ_SIZE,
                "IPC MSQ priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

int test_ipc_semaphore_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SYSTEM_IPC_SEMAPHORES < NETDATA_CHART_PRIO_SYSTEM_IPC_SEM_ARRAYS,
                "IPC semaphore priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: NFS priorities */
int test_nfsd_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_NFSD_READCACHE < NETDATA_CHART_PRIO_NFSD_FILEHANDLES,
                "NFSD priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_NFSD_FILEHANDLES < NETDATA_CHART_PRIO_NFSD_IO,
                "NFSD priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: BTRFS priorities */
int test_btrfs_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_BTRFS_DISK < NETDATA_CHART_PRIO_BTRFS_DATA,
                "BTRFS priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_BTRFS_DATA < NETDATA_CHART_PRIO_BTRFS_METADATA,
                "BTRFS priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: ZFS priorities */
int test_zfs_arc_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_ZFS_ARC_SIZE == 2500, "ZFS ARC size priority mismatch");
    TEST_ASSERT(NETDATA_CHART_PRIO_ZFS_READS < NETDATA_CHART_PRIO_ZFS_ACTUAL_HITS,
                "ZFS priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

int test_zfs_io_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_ZFS_IO == 2700, "ZFS IO priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: QoS priorities */
int test_qos_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_TC_QOS < NETDATA_CHART_PRIO_TC_QOS_PACKETS,
                "QoS priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_TC_QOS_PACKETS < NETDATA_CHART_PRIO_TC_QOS_DROPPED,
                "QoS priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: Infiniband priority */
int test_infiniband_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_INFINIBAND == 7100, "Infiniband priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Netfilter priorities */
int test_netfilter_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_NETFILTER_SOCKETS < NETDATA_CHART_PRIO_NETFILTER_NEW,
                "Netfilter priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: Power supply priorities */
int test_power_supply_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_POWER_SUPPLY_CAPACITY < NETDATA_CHART_PRIO_POWER_SUPPLY_POWER,
                "Power supply priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: Hugepages priorities */
int test_hugepages_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_MEM_HUGEPAGES < NETDATA_CHART_PRIO_MEM_HUGEPAGES_DETAILS,
                "Hugepages priorities not monotonically increasing");
    TEST_ASSERT(NETDATA_CHART_PRIO_MEM_HUGEPAGES_DETAILS < NETDATA_CHART_PRIO_MEM_HUGEPAGES_FAULTS,
                "Hugepages priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: NUMA priorities */
int test_numa_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_MEM_NUMA < NETDATA_CHART_PRIO_MEM_NUMA_NODES_NUMASTAT,
                "NUMA priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: Bcache priorities */
int test_bcache_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_BCACHE_CACHE_ALLOC <= NETDATA_CHART_PRIO_BCACHE_HIT_RATIO,
                "Bcache allocation and hit ratio have priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: MDSTAT priorities */
int test_mdstat_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_MDSTAT_HEALTH < NETDATA_CHART_PRIO_MDSTAT_FLUSH,
                "MDSTAT priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: Hardirq latency priority */
int test_hardirq_latency_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_HARDIRQ_LATENCY == 2900, "Hardirq latency priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Softirq priority */
int test_softirq_per_core_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SOFTIRQS_PER_CORE == 3000, "Softirq per core priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: IPVS priorities */
int test_ipvs_priorities(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_IPVS_NET < NETDATA_CHART_PRIO_IPVS_SOCKETS,
                "IPVS priorities not monotonically increasing");
    tests_passed++;
    return 1;
}

/* Test: Softnet per core priority */
int test_softnet_per_core_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SOFTNET_PER_CORE == 4101, "Softnet per core priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: SCTP priority */
int test_sctp_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_SCTP == 7000, "SCTP priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Wireless iface priority */
int test_wireless_iface_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_WIRELESS_IFACE == 7110, "Wireless iface priority mismatch");
    tests_passed++;
    return 1;
}

/* Test: Cgroup net iface priority */
int test_cgroup_net_iface_priority(void) {
    TEST_ASSERT(NETDATA_CHART_PRIO_CGROUP_NET_IFACE == 43000, "Cgroup net iface priority mismatch");
    tests_passed++;
    return 1;
}

/* Run all tests */
int main(void) {
    printf("Running comprehensive tests for src/collectors/all.h\n");
    printf("======================================================\n\n");

    /* Header guard tests */
    test_header_guard_exists();
    
    /* System priorities */
    test_system_cpu_priority_defined();
    test_system_load_priority_defined();
    test_system_io_priority_defined();
    test_system_ram_priority_defined();
    test_system_net_priority_defined();
    test_system_processes_priority_defined();
    
    /* Memory section */
    test_mem_system_available_priority();
    test_mem_kernel_priority();
    test_mem_zram_priority();
    
    /* Disk section */
    test_disk_io_priority();
    test_disk_ops_priority();
    
    /* Network section */
    test_first_net_iface_priority();
    test_ipv4_packets_priority();
    test_ipv4_tcp_packets_priority();
    test_ipv6_packets_priority();
    test_ipv6_tcp_packets_priority();
    
    /* Hyper-V */
    test_hyperv_vm_cpu_usage_priority();
    test_hyperv_vswitch_traffic_priority();
    
    /* CGROUP */
    test_cgroups_systemd_priority();
    test_cgroups_containers_priority();
    
    /* Sensors */
    test_sensors_priority();
    test_sensor_temperature_priority();
    
    /* ML charts */
    test_ml_dimensions_priority();
    test_ml_anomaly_rate_priority();
    test_netdata_ml_chart_running();
    test_netdata_ml_chart_training_status();
    
    /* ML plugin constants */
    test_netdata_ml_plugin_defined();
    test_netdata_ml_module_training_defined();
    test_netdata_ml_module_detection_defined();
    test_netdata_ml_module_prediction_defined();
    test_netdata_ml_family_defined();
    
    /* Priority monotonic increase tests */
    test_priority_monotonic_increase_system();
    test_priority_monotonic_increase_memory();
    test_priority_monotonic_increase_disk();
    test_priority_monotonic_increase_network();
    
    /* Section boundaries */
    test_cgroup_boundary();
    test_sensor_boundary();
    test_statsd_to_logs_boundary();
    
    /* Special cases */
    test_pci_aer_priority();
    test_drm_amdgpu_priority();
    
    /* TCP priorities */
    test_tcp_priorities_order();
    
    /* Clock sync */
    test_clock_sync_priorities();
    
    /* IPC */
    test_ipc_msq_priorities();
    test_ipc_semaphore_priorities();
    
    /* NFS */
    test_nfsd_priorities();
    
    /* BTRFS */
    test_btrfs_priorities();
    
    /* ZFS */
    test_zfs_arc_priorities();
    test_zfs_io_priority();
    
    /* QoS */
    test_qos_priorities();
    
    /* Infiniband */
    test_infiniband_priority();
    
    /* Netfilter */
    test_netfilter_priorities();
    
    /* Power supply */
    test_power_supply_priorities();
    
    /* Hugepages */
    test_hugepages_priorities();
    
    /* NUMA */
    test_numa_priorities();
    
    /* Bcache */
    test_bcache_priorities();
    
    /* MDSTAT */
    test_mdstat_priorities();
    
    /* Hardirq */
    test_hardirq_latency_priority();
    
    /* Softirq */
    test_softirq_per_core_priority();
    
    /* IPVS */
    test_ipvs_priorities();
    
    /* Softnet */
    test_softnet_per_core_priority();
    
    /* SCTP */
    test_sctp_priority();
    
    /* Wireless */
    test_wireless_iface_priority();
    
    /* Cgroup net */
    test_cgroup_net_iface_priority();

    printf("\n======================================================\n");
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);
    printf("Total Tests: %d\n", tests_passed + tests_failed);
    printf("======================================================\n");

    return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}