#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Mock definitions for the header file structure tests
#define TEST_MODULE_NAME NETDATA_EBPF_MODULE_NAME_VFS
#define TEST_VFS_FILE_CLEAN_COUNT NETDATA_VFS_FILE_CLEAN_COUNT
#define TEST_VFS_FILE_IO_COUNT NETDATA_VFS_FILE_IO_COUNT
#define TEST_VFS_FILE_ERR_COUNT NETDATA_VFS_FILE_ERR_COUNT

// Test enum values
static void test_netdata_publish_vfs_list_enum() {
    // Verify enum values exist and are sequential
    assert(NETDATA_KEY_PUBLISH_VFS_UNLINK == 0);
    assert(NETDATA_KEY_PUBLISH_VFS_READ == 1);
    assert(NETDATA_KEY_PUBLISH_VFS_WRITE == 2);
    assert(NETDATA_KEY_PUBLISH_VFS_FSYNC == 3);
    assert(NETDATA_KEY_PUBLISH_VFS_OPEN == 4);
    assert(NETDATA_KEY_PUBLISH_VFS_CREATE == 5);
    assert(NETDATA_KEY_PUBLISH_VFS_END == 6);
}

static void test_vfs_counters_enum() {
    // Verify vfs_counters enum has correct count
    assert(NETDATA_VFS_COUNTER == 24);
    assert(NETDATA_KEY_CALLS_VFS_WRITE == 0);
    assert(NETDATA_KEY_ERROR_VFS_WRITE == 1);
    assert(NETDATA_KEY_BYTES_VFS_WRITE == 2);
    assert(NETDATA_KEY_CALLS_VFS_CREATE == 18);
    assert(NETDATA_KEY_ERROR_VFS_CREATE == 19);
}

static void test_vfs_tables_enum() {
    // Test vfs_tables enum values
    assert(NETDATA_VFS_PID == 0);
    assert(NETDATA_VFS_ALL == 1);
    assert(NETDATA_VFS_CTRL == 2);
}

static void test_vfs_calls_name_enum() {
    // Test vfs_calls_name enum
    assert(NETDATA_EBPF_VFS_WRITE == 0);
    assert(NETDATA_EBPF_VFS_WRITEV == 1);
    assert(NETDATA_EBPF_VFS_READ == 2);
    assert(NETDATA_EBPF_VFS_READV == 3);
    assert(NETDATA_EBPF_VFS_UNLINK == 4);
    assert(NETDATA_EBPF_VFS_FSYNC == 5);
    assert(NETDATA_EBPF_VFS_OPEN == 6);
    assert(NETDATA_EBPF_VFS_CREATE == 7);
    assert(NETDATA_VFS_END_LIST == 8);
}

static void test_chart_name_defines() {
    // Test chart name constants
    const char *clean_count = NETDATA_VFS_FILE_CLEAN_COUNT;
    const char *io_count = NETDATA_VFS_FILE_IO_COUNT;
    const char *err_count = NETDATA_VFS_FILE_ERR_COUNT;
    const char *io_bytes = NETDATA_VFS_IO_FILE_BYTES;
    
    assert(clean_count != NULL);
    assert(io_count != NULL);
    assert(err_count != NULL);
    assert(io_bytes != NULL);
    assert(strcmp(clean_count, "vfs_deleted_objects") == 0);
    assert(strcmp(io_count, "vfs_io") == 0);
    assert(strcmp(err_count, "vfs_io_error") == 0);
    assert(strcmp(io_bytes, "vfs_io_bytes") == 0);
}

static void test_apps_chart_defines() {
    // Test app chart constants
    const char *file_deleted = NETDATA_SYSCALL_APPS_FILE_DELETED;
    const char *vfs_write = NETDATA_SYSCALL_APPS_VFS_WRITE_CALLS;
    
    assert(strcmp(file_deleted, "file_deleted") == 0);
    assert(strcmp(vfs_write, "vfs_write_call") == 0);
}

static void test_context_defines() {
    // Test context constants
    const char *unlink_ctx = NETDATA_CGROUP_VFS_UNLINK_CONTEXT;
    const char *write_ctx = NETDATA_CGROUP_VFS_WRITE_CONTEXT;
    
    assert(strcmp(unlink_ctx, "cgroup.vfs_unlink") == 0);
    assert(strcmp(write_ctx, "cgroup.vfs_write") == 0);
}

static void test_systemd_context_defines() {
    // Test systemd context constants
    const char *systemd_unlink = NETDATA_SYSTEMD_VFS_UNLINK_CONTEXT;
    const char *systemd_write = NETDATA_SYSTEMD_VFS_WRITE_CONTEXT;
    
    assert(strcmp(systemd_unlink, "systemd.service.vfs_unlink") == 0);
    assert(strcmp(systemd_write, "systemd.service.vfs_write") == 0);
}

static void test_group_define() {
    // Test VFS group
    const char *group = NETDATA_VFS_GROUP;
    assert(strcmp(group, "vfs") == 0);
}

static void test_aral_name_define() {
    // Test ARAL name
    const char *aral_name = NETDATA_EBPF_VFS_ARAL_NAME;
    assert(strcmp(aral_name, "ebpf_vfs") == 0);
}

static void test_module_name_define() {
    // Test module name
    const char *module_name = NETDATA_EBPF_MODULE_NAME_VFS;
    assert(strcmp(module_name, "vfs") == 0);
}

static void test_module_desc_define() {
    // Test module description
    const char *module_desc = NETDATA_EBPF_VFS_MODULE_DESC;
    assert(strlen(module_desc) > 0);
    assert(strstr(module_desc, "Virtual File System") != NULL);
}

static void test_config_file_define() {
    // Test config file name
    const char *config_file = NETDATA_DIRECTORY_VFS_CONFIG_FILE;
    assert(strcmp(config_file, "vfs.conf") == 0);
}

static void test_units_bytes_define() {
    // Test units constant
    const char *units = EBPF_COMMON_UNITS_BYTES;
    assert(strcmp(units, "bytes/s") == 0);
}

int main() {
    test_netdata_publish_vfs_list_enum();
    test_vfs_counters_enum();
    test_vfs_tables_enum();
    test_vfs_calls_name_enum();
    test_chart_name_defines();
    test_apps_chart_defines();
    test_context_defines();
    test_systemd_context_defines();
    test_group_define();
    test_aral_name_define();
    test_module_name_define();
    test_module_desc_define();
    test_config_file_define();
    test_units_bytes_define();
    
    return 0;
}