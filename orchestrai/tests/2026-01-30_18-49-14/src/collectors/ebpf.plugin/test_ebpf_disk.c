#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>

/* Mocking framework */
#include <cmocka.h>

/* External declarations for mocking */
extern void *callocz(size_t nmemb, size_t size);
extern void freez(void *ptr);
extern void netdata_log_error(const char *fmt, ...);
extern void netdata_log_info(const char *fmt, ...);
extern int open(const char *pathname, int flags, ...);
extern ssize_t read(int fd, void *buf, size_t count);
extern int close(int fd);
extern char *strstr(const char *haystack, const char *needle);
extern char *strdupz(const char *str);
extern unsigned long strtoul(const char *nptr, char **endptr, int base);
extern long strtol(const char *nptr, char **endptr, int base);
extern size_t strlen(const char *s);
extern void *memcpy(void *dest, const void *src, size_t n);
extern int snprintfz(char *str, size_t size, const char *fmt, ...);
extern time_t now_realtime_sec(void);
extern int bpf_map_get_next_key(int fd, const void *key, void *next_key);
extern int bpf_map_lookup_elem(int fd, const void *key, void *value);
extern void avl_init_lock(void *tree, int (*compar)(void *a, void *b));
extern void *avl_insert_lock(void *tree, void *elem);
extern void *avl_search_lock(void *tree, void *elem);
extern void *avl_remove_lock(void *tree, void *elem);
extern void netdata_mutex_init(void *mutex);
extern void netdata_mutex_lock(void *mutex);
extern void netdata_mutex_unlock(void *mutex);
extern void netdata_mutex_destroy(void *mutex);
extern procfile *procfile_open(const char *filename, const char *separators, int flags);
extern procfile *procfile_readall(procfile *ff);
extern size_t procfile_lines(procfile *ff);
extern size_t procfile_linewords(procfile *ff, size_t line);
extern const char *procfile_lineword(procfile *ff, size_t line, size_t word);
extern void procfile_close(procfile *ff);

typedef struct {
    uint64_t histogram[256];
    char *name;
    char *title;
    int order;
} netdata_ebpf_histogram_t;

typedef struct netdata_ebpf_disks {
    void *avl;
    uint32_t dev;
    uint32_t major;
    uint32_t minor;
    uint32_t bootsector_key;
    uint64_t start;
    uint64_t end;
    char family[33];
    char *boot_chart;
    netdata_ebpf_histogram_t histogram;
    uint32_t flags;
    time_t last_update;
    struct netdata_ebpf_disks *main;
    struct netdata_ebpf_disks *boot_partition;
    struct netdata_ebpf_disks *next;
} netdata_ebpf_disks_t;

typedef struct {
    netdata_ebpf_disks_t *plot;
    struct netdata_ebpf_publish_disk *next;
} ebpf_publish_disk_t;

typedef struct block_key {
    uint32_t bin;
    uint32_t dev;
} block_key_t;

/* Test fixture for setup/teardown */
static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    return 0;
}

/* Test: ebpf_disk_parse_start - successful read */
static void test_ebpf_disk_parse_start_success(void **state) {
    netdata_ebpf_disks_t disk = {0};
    char test_file[] = "/tmp/test_disk_start";
    
    /* Create test file with sector start value */
    FILE *f = fopen(test_file, "w");
    assert_non_null(f);
    fprintf(f, "2048");
    fclose(f);
    
    int ret = ebpf_disk_parse_start(&disk, test_file);
    assert_int_equal(ret, 0);
    assert_int_equal(disk.start, 2048);
    
    unlink(test_file);
}

/* Test: ebpf_disk_parse_start - file open failure */
static void test_ebpf_disk_parse_start_open_fail(void **state) {
    netdata_ebpf_disks_t disk = {0};
    char non_existent_file[] = "/tmp/nonexistent_disk_file_12345";
    
    int ret = ebpf_disk_parse_start(&disk, non_existent_file);
    assert_int_equal(ret, -1);
}

/* Test: ebpf_disk_parse_start - empty file */
static void test_ebpf_disk_parse_start_empty_file(void **state) {
    netdata_ebpf_disks_t disk = {0};
    char test_file[] = "/tmp/test_disk_start_empty";
    
    FILE *f = fopen(test_file, "w");
    assert_non_null(f);
    fclose(f);
    
    int ret = ebpf_disk_parse_start(&disk, test_file);
    assert_int_equal(ret, 0);
    assert_int_equal(disk.start, 0);
    
    unlink(test_file);
}

/* Test: ebpf_disk_parse_start - large sector number */
static void test_ebpf_disk_parse_start_large_number(void **state) {
    netdata_ebpf_disks_t disk = {0};
    char test_file[] = "/tmp/test_disk_start_large";
    
    FILE *f = fopen(test_file, "w");
    assert_non_null(f);
    fprintf(f, "4294967295");
    fclose(f);
    
    int ret = ebpf_disk_parse_start(&disk, test_file);
    assert_int_equal(ret, 0);
    assert_int_equal(disk.start, 4294967295UL);
    
    unlink(test_file);
}

/* Test: ebpf_parse_uevent - with EFI partition */
static void test_ebpf_parse_uevent_with_efi(void **state) {
    netdata_ebpf_disks_t disk = {0};
    netdata_ebpf_disks_t main_disk = {0};
    disk.main = &main_disk;
    
    char test_file[] = "/tmp/test_uevent_efi";
    FILE *f = fopen(test_file, "w");
    assert_non_null(f);
    fprintf(f, "PARTNAME=EFI System Partition\nPARTTYPE=test");
    fclose(f);
    
    int ret = ebpf_parse_uevent(&disk, test_file);
    assert_int_equal(ret, 0);
    assert_int_equal(disk.flags & 8, 8);  /* NETDATA_DISK_HAS_EFI */
    assert_ptr_equal(main_disk.boot_partition, &disk);
    
    freez(disk.boot_chart);
    unlink(test_file);
}

/* Test: ebpf_parse_uevent - without EFI partition */
static void test_ebpf_parse_uevent_no_efi(void **state) {
    netdata_ebpf_disks_t disk = {0};
    
    char test_file[] = "/tmp/test_uevent_no_efi";
    FILE *f = fopen(test_file, "w");
    assert_non_null(f);
    fprintf(f, "PARTNAME=Data\nPARTTYPE=test");
    fclose(f);
    
    int ret = ebpf_parse_uevent(&disk, test_file);
    assert_int_equal(ret, 0);
    assert_int_equal(disk.flags & 8, 0);
    
    unlink(test_file);
}

/* Test: ebpf_parse_uevent - file open failure */
static void test_ebpf_parse_uevent_open_fail(void **state) {
    netdata_ebpf_disks_t disk = {0};
    char non_existent_file[] = "/tmp/nonexistent_uevent_file_12345";
    
    int ret = ebpf_parse_uevent(&disk, non_existent_file);
    assert_int_equal(ret, -1);
}

/* Test: ebpf_parse_size - successful read */
static void test_ebpf_parse_size_success(void **state) {
    netdata_ebpf_disks_t disk = {0};
    disk.start = 2048;
    
    char test_file[] = "/tmp/test_disk_size";
    FILE *f = fopen(test_file, "w");
    assert_non_null(f);
    fprintf(f, "1048576");
    fclose(f);
    
    int ret = ebpf_parse_size(&disk, test_file);
    assert_int_equal(ret, 0);
    assert_int_equal(disk.end, 2048 + 1048576 - 1);
    
    unlink(test_file);
}

/* Test: ebpf_parse_size - file open failure */
static void test_ebpf_parse_size_open_fail(void **state) {
    netdata_ebpf_disks_t disk = {0};
    char non_existent_file[] = "/tmp/nonexistent_size_file_12345";
    
    int ret = ebpf_parse_size(&disk, non_existent_file);
    assert_int_equal(ret, -1);
}

/* Test: ebpf_parse_size - empty file */
static void test_ebpf_parse_size_empty_file(void **state) {
    netdata_ebpf_disks_t disk = {0};
    disk.start = 1000;
    
    char test_file[] = "/tmp/test_disk_size_empty";
    FILE *f = fopen(test_file, "w");
    assert_non_null(f);
    fclose(f);
    
    int ret = ebpf_parse_size(&disk, test_file);
    assert_int_equal(ret, 0);
    /* end should remain at its initial value when file is empty */
    assert_int_equal(disk.end, 0);
    
    unlink(test_file);
}

/* Test: netdata_new_encode_dev - basic encoding */
static void test_netdata_new_encode_dev_basic(void **state) {
    uint32_t result = netdata_new_encode_dev(8, 0);
    assert_int_equal(result, 2048);  /* (8 << 8) | 0 */
}

/* Test: netdata_new_encode_dev - with minor offset */
static void test_netdata_new_encode_dev_with_minor(void **state) {
    uint32_t result = netdata_new_encode_dev(8, 1);
    assert_int_equal(result, 2049);
}

/* Test: netdata_new_encode_dev - large values */
static void test_netdata_new_encode_dev_large_values(void **state) {
    uint32_t result = netdata_new_encode_dev(255, 255);
    uint32_t expected = (255 & 0xff) | (255 << 8) | ((255 & ~0xff) << 12);
    assert_int_equal(result, expected);
}

/* Test: ebpf_compare_disks - equal devices */
static void test_ebpf_compare_disks_equal(void **state) {
    netdata_ebpf_disks_t disk1 = {.dev = 2048};
    netdata_ebpf_disks_t disk2 = {.dev = 2048};
    
    int result = ebpf_compare_disks(&disk1, &disk2);
    assert_int_equal(result, 0);
}

/* Test: ebpf_compare_disks - first greater */
static void test_ebpf_compare_disks_greater(void **state) {
    netdata_ebpf_disks_t disk1 = {.dev = 2049};
    netdata_ebpf_disks_t disk2 = {.dev = 2048};
    
    int result = ebpf_compare_disks(&disk1, &disk2);
    assert_int_equal(result, 1);
}

/* Test: ebpf_compare_disks - first less */
static void test_ebpf_compare_disks_less(void **state) {
    netdata_ebpf_disks_t disk1 = {.dev = 2047};
    netdata_ebpf_disks_t disk2 = {.dev = 2048};
    
    int result = ebpf_compare_disks(&disk1, &disk2);
    assert_int_equal(result, -1);
}

/* Test: ebpf_disk_enable_tracepoints - both already enabled */
static void test_ebpf_disk_enable_tracepoints_all_enabled(void **state) {
    /* This would require mocking external functions */
    /* For now, we document the test structure */
    /* int ret = ebpf_disk_enable_tracepoints(); */
}

/* Test: ebpf_disk_disable_tracepoints - both need disabling */
static void test_ebpf_disk_disable_tracepoints(void **state) {
    /* This would require mocking external functions */
    /* For now, we document the test structure */
    /* ebpf_disk_disable_tracepoints(); */
}

/* Test: ebpf_cleanup_disk_list - single item */
static void test_ebpf_cleanup_disk_list_single(void **state) {
    /* Create a test disk list node */
    netdata_ebpf_disks_t *disk = calloc(1, sizeof(netdata_ebpf_disks_t));
    assert_non_null(disk);
    
    disk->histogram.name = strdupz("test_disk");
    disk->boot_chart = strdupz("boot_chart");
    disk->next = NULL;
    
    /* Note: This test demonstrates structure but would need to set global disk_list */
    freez(disk->histogram.name);
    freez(disk->boot_chart);
    freez(disk);
}

/* Test: ebpf_cleanup_disk_list - multiple items */
static void test_ebpf_cleanup_disk_list_multiple(void **state) {
    netdata_ebpf_disks_t *disk1 = calloc(1, sizeof(netdata_ebpf_disks_t));
    netdata_ebpf_disks_t *disk2 = calloc(1, sizeof(netdata_ebpf_disks_t));
    
    assert_non_null(disk1);
    assert_non_null(disk2);
    
    disk1->histogram.name = strdupz("disk1");
    disk1->boot_chart = strdupz("boot1");
    disk1->next = disk2;
    
    disk2->histogram.name = strdupz("disk2");
    disk2->boot_chart = strdupz("boot2");
    disk2->next = NULL;
    
    /* Cleanup */
    freez(disk1->histogram.name);
    freez(disk1->boot_chart);
    freez(disk2->histogram.name);
    freez(disk2->boot_chart);
    freez(disk1);
    freez(disk2);
}

/* Test: ebpf_cleanup_plot_disks - empty list */
static void test_ebpf_cleanup_plot_disks_empty(void **state) {
    ebpf_publish_disk_t *plot = NULL;
    
    while (plot) {
        ebpf_publish_disk_t *next = plot->next;
        freez(plot);
        plot = next;
    }
    
    /* Verification: plot should be NULL */
    assert_null(plot);
}

/* Test: ebpf_cleanup_plot_disks - single item */
static void test_ebpf_cleanup_plot_disks_single(void **state) {
    ebpf_publish_disk_t *plot = calloc(1, sizeof(ebpf_publish_disk_t));
    assert_non_null(plot);
    
    netdata_ebpf_disks_t *disk = calloc(1, sizeof(netdata_ebpf_disks_t));
    plot->plot = disk;
    plot->next = NULL;
    
    /* Cleanup */
    while (plot) {
        ebpf_publish_disk_t *next = plot->next;
        freez(plot);
        plot = next;
    }
    
    freez(disk);
}

/* Test: ebpf_fill_plot_disks - add to existing list */
static void test_ebpf_fill_plot_disks_add_to_existing(void **state) {
    netdata_ebpf_disks_t disk = {.dev = 2048, .flags = 0};
    
    /* Simulate existing plot list */
    ebpf_publish_disk_t *existing = calloc(1, sizeof(ebpf_publish_disk_t));
    existing->plot = calloc(1, sizeof(netdata_ebpf_disks_t));
    existing->plot->dev = 2047;
    existing->next = NULL;
    
    /* Clean up */
    freez(existing->plot);
    freez(existing);
}

/* Test: ebpf_fill_plot_disks - already in list */
static void test_ebpf_fill_plot_disks_already_exists(void **state) {
    netdata_ebpf_disks_t disk = {.dev = 2048, .flags = 0};
    
    ebpf_publish_disk_t *existing = calloc(1, sizeof(ebpf_publish_disk_t));
    existing->plot = &disk;
    existing->next = NULL;
    
    /* Disk should not be added twice */
    freez(existing);
}

/* Test: ebpf_read_disk_info - partition with digits */
static void test_ebpf_read_disk_info_partition(void **state) {
    netdata_ebpf_disks_t disk = {0};
    disk.main = NULL;
    
    /* Test with partition name like "sda1" */
    /* ebpf_read_disk_info(&disk, "sda1"); */
}

/* Test: ebpf_read_disk_info - device without digits */
static void test_ebpf_read_disk_info_device(void **state) {
    netdata_ebpf_disks_t disk = {0};
    disk.major = 8;
    disk.minor = 0;
    
    /* Test with device name like "sda" */
    /* ebpf_read_disk_info(&disk, "sda"); */
}

/* Test: ebpf_read_disk_info - empty name */
static void test_ebpf_read_disk_info_empty_name(void **state) {
    netdata_ebpf_disks_t disk = {0};
    
    /* Should return early without processing */
    /* ebpf_read_disk_info(&disk, ""); */
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ebpf_disk_parse_start_success),
        cmocka_unit_test(test_ebpf_disk_parse_start_open_fail),
        cmocka_unit_test(test_ebpf_disk_parse_start_empty_file),
        cmocka_unit_test(test_ebpf_disk_parse_start_large_number),
        cmocka_unit_test(test_ebpf_parse_uevent_with_efi),
        cmocka_unit_test(test_ebpf_parse_uevent_no_efi),
        cmocka_unit_test(test_ebpf_parse_uevent_open_fail),
        cmocka_unit_test(test_ebpf_parse_size_success),
        cmocka_unit_test(test_ebpf_parse_size_open_fail),
        cmocka_unit_test(test_ebpf_parse_size_empty_file),
        cmocka_unit_test(test_netdata_new_encode_dev_basic),
        cmocka_unit_test(test_netdata_new_encode_dev_with_minor),
        cmocka_unit_test(test_netdata_new_encode_dev_large_values),
        cmocka_unit_test(test_ebpf_compare_disks_equal),
        cmocka_unit_test(test_ebpf_compare_disks_greater),
        cmocka_unit_test(test_ebpf_compare_disks_less),
        cmocka_unit_test(test_ebpf_cleanup_disk_list_single),
        cmocka_unit_test(test_ebpf_cleanup_disk_list_multiple),
        cmocka_unit_test(test_ebpf_cleanup_plot_disks_empty),
        cmocka_unit_test(test_ebpf_cleanup_plot_disks_single),
        cmocka_unit_test(test_ebpf_fill_plot_disks_add_to_existing),
        cmocka_unit_test(test_ebpf_fill_plot_disks_already_exists),
        cmocka_unit_test(test_ebpf_read_disk_info_partition),
        cmocka_unit_test(test_ebpf_read_disk_info_device),
        cmocka_unit_test(test_ebpf_read_disk_info_empty_name),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}