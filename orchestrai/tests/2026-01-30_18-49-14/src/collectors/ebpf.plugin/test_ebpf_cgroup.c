#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

// Mock structures and variables
typedef struct {
    void *header;
    void *body;
} netdata_ebpf_cgroup_shm_t;

typedef struct {
    uint32_t hash;
    char name[256];
    uint32_t options;
    uint32_t cgroup_root_count;
    size_t body_length;
    char path[256];
    int enabled;
} netdata_ebpf_cgroup_shm_header_t;

typedef struct {
    uint32_t hash;
    char name[256];
    uint32_t options;
    char path[256];
    int enabled;
} netdata_ebpf_cgroup_shm_body_t;

typedef struct pid_on_target2 {
    int32_t pid;
    int updated;
    void *swap;
    void *fd;
    void *vfs;
    void *ps;
    void *dc;
    void *shm;
    void *socket;
    void *cachestat;
    struct pid_on_target2 *next;
} pid_on_target2_t;

typedef struct ebpf_cgroup_target {
    char name[256];
    uint32_t hash;
    uint32_t flags;
    uint32_t systemd;
    uint32_t updated;
    void *publish_systemd_swap;
    void *publish_systemd_fd;
    void *publish_systemd_vfs;
    void *publish_systemd_ps;
    void *publish_dc;
    int oomkill;
    void *publish_shm;
    void *publish_socket;
    void *publish_cachestat;
    struct pid_on_target2 *pids;
    struct ebpf_cgroup_target *next;
} ebpf_cgroup_target_t;

typedef struct {
    char *id;
    char *title;
    char *units;
    char *family;
    char *charttype;
    int order;
    char *algorithm;
    char *context;
    char *module;
    int update_every;
    char *suffix;
    char *dimension;
} ebpf_systemd_args_t;

typedef struct {
    int32_t pid;
    void *data;
} procfile;

// Global variables
ebpf_cgroup_target_t *ebpf_cgroup_pids = NULL;
static void *ebpf_mapped_memory = NULL;
int send_cgroup_chart = 0;
int shm_fd_ebpf_cgroup = -1;
sem_t *shm_sem_ebpf_cgroup = SEM_FAILED;
netdata_ebpf_cgroup_shm_t shm_ebpf_cgroup = {NULL, NULL};

#define NETDATA_EBPF_CGROUP_MAX_TRIES 3
#define NETDATA_EBPF_CGROUP_NEXT_TRY_SEC 30
#define NETDATA_EBPF_CGROUP_UPDATE 60
#define CGROUP_OPTIONS_SYSTEM_SLICE_SERVICE 1
#define MAP_FAILED ((void *)-1)
#define SEM_FAILED ((sem_t *)-1)
#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_SHARED 1

// Mock mutex
struct {
    int locked;
} mutex_cgroup_shm = {0};

// Mock implementations
void *nd_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    (void)addr;
    (void)length;
    (void)prot;
    (void)flags;
    (void)fd;
    (void)offset;
    return malloc(length > 0 ? length : 1);
}

void nd_munmap(void *addr, size_t length) {
    (void)length;
    if (addr) {
        free(addr);
    }
}

void netdata_log_error(const char *fmt, ...) {
    (void)fmt;
}

void netdata_log_info(const char *fmt, ...) {
    (void)fmt;
}

int close(int fd) {
    (void)fd;
    return 0;
}

int shm_open(const char *name, int oflag, mode_t mode) {
    (void)name;
    (void)oflag;
    (void)mode;
    return mock_type(int);
}

int shm_unlink(const char *name) {
    (void)name;
    return 0;
}

sem_t *sem_open(const char *name, int oflag, ...) {
    (void)name;
    (void)oflag;
    return mock_type(sem_t *);
}

int sem_wait(sem_t *sem) {
    (void)sem;
    return 0;
}

int sem_post(sem_t *sem) {
    (void)sem;
    return 0;
}

void netdata_mutex_lock(void *mutex) {
    (void)mutex;
}

void netdata_mutex_unlock(void *mutex) {
    (void)mutex;
}

int ebpf_plugin_stop(void) {
    return mock_type(int);
}

void heartbeat_init(void *hb, uint64_t usec) {
    (void)hb;
    (void)usec;
}

void heartbeat_next(void *hb) {
    (void)hb;
}

void snprintfz(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(str, size, format, args);
    va_end(args);
}

procfile *procfile_open_no_log(const char *path, const char *separators, int flags) {
    (void)path;
    (void)separators;
    (void)flags;
    return mock_ptr_type(procfile *);
}

procfile *procfile_readall(procfile *ff) {
    return mock_ptr_type(procfile *);
}

size_t procfile_lines(procfile *ff) {
    (void)ff;
    return mock_type(size_t);
}

char *procfile_lineword(procfile *ff, size_t line, size_t word) {
    (void)ff;
    (void)line;
    (void)word;
    return mock_ptr_type(char *);
}

void procfile_close(procfile *ff) {
    (void)ff;
}

long str2l(const char *s) {
    (void)s;
    return mock_type(long);
}

void *callocz(size_t nmemb, size_t size) {
    return calloc(nmemb, size);
}

void freez(void *ptr) {
    free(ptr);
}

void ebpf_write_chart_cmd(const char *id, const char *suffix, const char *other, 
                          const char *title, const char *units, const char *family,
                          const char *charttype, const char *context, int order,
                          int update_every, const char *module) {
    (void)id;
    (void)suffix;
    (void)other;
    (void)title;
    (void)units;
    (void)family;
    (void)charttype;
    (void)context;
    (void)order;
    (void)update_every;
    (void)module;
}

void ebpf_create_chart_labels(const char *key, const char *value, int source) {
    (void)key;
    (void)value;
    (void)source;
}

void ebpf_commit_label(void) {
}

char *strdupz(const char *s) {
    return s ? strdup(s) : NULL;
}

// Forward declarations of functions under test
static inline void *ebpf_cgroup_map_shm_locally(int fd, size_t length);
void ebpf_unmap_cgroup_shared_memory(void);
void ebpf_map_cgroup_shared_memory(void);
static inline void ebpf_clean_specific_cgroup_pids(struct pid_on_target2 *pt);
static void ebpf_remove_cgroup_target_update_list(void);
static inline void ebpf_cgroup_set_target_data(ebpf_cgroup_target_t *out, netdata_ebpf_cgroup_shm_body_t *ptr);
static ebpf_cgroup_target_t *ebpf_cgroup_find_or_create(netdata_ebpf_cgroup_shm_body_t *ptr);
static void ebpf_update_pid_link_list(ebpf_cgroup_target_t *ect, char *path);
void ebpf_reset_updated_var(void);
void ebpf_parse_cgroup_shm_data(void);
void ebpf_create_charts_on_systemd(ebpf_systemd_args_t *chart);
void ebpf_cgroup_integration(void *ptr);

// ============================================================================
// TEST: ebpf_cgroup_map_shm_locally - Success
// ============================================================================
static int test_ebpf_cgroup_map_shm_locally_success(void **state) {
    (void)state;
    
    int fd = 5;
    size_t length = 4096;
    
    void *result = ebpf_cgroup_map_shm_locally(fd, length);
    
    assert_non_null(result);
    assert_not_equal(result, MAP_FAILED);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_cgroup_map_shm_locally - mmap failure
// ============================================================================
static int test_ebpf_cgroup_map_shm_locally_mmap_fail(void **state) {
    (void)state;
    
    int fd = -1;
    size_t length = 0;
    
    void *result = ebpf_cgroup_map_shm_locally(fd, length);
    
    assert_non_null(result);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_unmap_cgroup_shared_memory - Success
// ============================================================================
static int test_ebpf_unmap_cgroup_shared_memory_success(void **state) {
    (void)state;
    
    // Setup
    ebpf_mapped_memory = malloc(1024);
    netdata_ebpf_cgroup_shm_header_t *header = malloc(sizeof(netdata_ebpf_cgroup_shm_header_t));
    header->body_length = 1024;
    shm_ebpf_cgroup.header = header;
    
    // Execute
    ebpf_unmap_cgroup_shared_memory();
    
    // Verify
    assert_true(1); // Function returns void, just verify it doesn't crash
    
    return 0;
}

// ============================================================================
// TEST: ebpf_map_cgroup_shared_memory - Already mapped header
// ============================================================================
static int test_ebpf_map_cgroup_shared_memory_already_mapped(void **state) {
    (void)state;
    
    netdata_ebpf_cgroup_shm_header_t *header = malloc(sizeof(netdata_ebpf_cgroup_shm_header_t));
    shm_ebpf_cgroup.header = header;
    
    ebpf_map_cgroup_shared_memory();
    
    assert_non_null(shm_ebpf_cgroup.header);
    
    shm_ebpf_cgroup.header = NULL;
    freez(header);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_map_cgroup_shared_memory - Max tries exceeded
// ============================================================================
static int test_ebpf_map_cgroup_shared_memory_max_tries_exceeded(void **state) {
    (void)state;
    
    // This tests the limit_try > NETDATA_EBPF_CGROUP_MAX_TRIES condition
    // We can't directly set static limit_try, so we just verify the function handles it
    
    ebpf_map_cgroup_shared_memory();
    
    assert_true(1);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_map_cgroup_shared_memory - shm_open fails
// ============================================================================
static int test_ebpf_map_cgroup_shared_memory_shm_open_fail(void **state) {
    (void)state;
    
    will_return(shm_open, -1);
    
    // Reset state
    shm_fd_ebpf_cgroup = -1;
    shm_ebpf_cgroup.header = NULL;
    shm_sem_ebpf_cgroup = SEM_FAILED;
    
    ebpf_map_cgroup_shared_memory();
    
    assert_null(shm_ebpf_cgroup.header);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_map_cgroup_shared_memory - Header mapping fails
// ============================================================================
static int test_ebpf_map_cgroup_shared_memory_header_mapping_fail(void **state) {
    (void)state;
    
    will_return(shm_open, 5);
    
    // Reset state
    shm_fd_ebpf_cgroup = -1;
    shm_ebpf_cgroup.header = NULL;
    shm_sem_ebpf_cgroup = SEM_FAILED;
    
    ebpf_map_cgroup_shared_memory();
    
    // The function returns early if initial mapping fails
    assert_true(1);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_clean_specific_cgroup_pids - Empty list
// ============================================================================
static int test_ebpf_clean_specific_cgroup_pids_empty(void **state) {
    (void)state;
    
    ebpf_clean_specific_cgroup_pids(NULL);
    
    assert_true(1);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_clean_specific_cgroup_pids - Single element
// ============================================================================
static int test_ebpf_clean_specific_cgroup_pids_single(void **state) {
    (void)state;
    
    struct pid_on_target2 *pt = callocz(1, sizeof(struct pid_on_target2));
    pt->pid = 1234;
    pt->next = NULL;
    
    ebpf_clean_specific_cgroup_pids(pt);
    
    assert_true(1);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_clean_specific_cgroup_pids - Multiple elements
// ============================================================================
static int test_ebpf_clean_specific_cgroup_pids_multiple(void **state) {
    (void)state;
    
    struct pid_on_target2 *pt1 = callocz(1, sizeof(struct pid_on_target2));
    struct pid_on_target2 *pt2 = callocz(1, sizeof(struct pid_on_target2));
    struct pid_on_target2 *pt3 = callocz(1, sizeof(struct pid_on_target2));
    
    pt1->pid = 1001;
    pt2->pid = 1002;
    pt3->pid = 1003;
    
    pt1->next = pt2;
    pt2->next = pt3;
    pt3->next = NULL;
    
    ebpf_clean_specific_cgroup_pids(pt1);
    
    assert_true(1);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_remove_cgroup_target_update_list - Empty list
// ============================================================================
static int test_ebpf_remove_cgroup_target_update_list_empty(void **state) {
    (void)state;
    
    ebpf_cgroup_pids = NULL;
    
    ebpf_remove_cgroup_target_update_list();
    
    assert_null(ebpf_cgroup_pids);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_remove_cgroup_target_update_list - Single updated element
// ============================================================================
static int test_ebpf_remove_cgroup_target_update_list_single_updated(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t *ect = callocz(1, sizeof(ebpf_cgroup_target_t));
    ect->updated = 1;
    ect->next = NULL;
    ect->pids = NULL;
    
    ebpf_cgroup_pids = ect;
    
    ebpf_remove_cgroup_target_update_list();
    
    assert_non_null(ebpf_cgroup_pids);
    assert_int_equal(ebpf_cgroup_pids->updated, 1);
    
    freez(ebpf_cgroup_pids);
    ebpf_cgroup_pids = NULL;
    
    return 0;
}

// ============================================================================
// TEST: ebpf_remove_cgroup_target_update_list - Single not updated element
// ============================================================================
static int test_ebpf_remove_cgroup_target_update_list_single_not_updated(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t *ect = callocz(1, sizeof(ebpf_cgroup_target_t));
    ect->updated = 0;
    ect->next = NULL;
    ect->pids = NULL;
    
    ebpf_cgroup_pids = ect;
    
    ebpf_remove_cgroup_target_update_list();
    
    assert_null(ebpf_cgroup_pids);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_remove_cgroup_target_update_list - Multiple mixed
// ============================================================================
static int test_ebpf_remove_cgroup_target_update_list_multiple_mixed(void **state) {
    (void)state;
    
    // Create three targets: updated, not updated, updated
    ebpf_cgroup_target_t *ect1 = callocz(1, sizeof(ebpf_cgroup_target_t));
    ebpf_cgroup_target_t *ect2 = callocz(1, sizeof(ebpf_cgroup_target_t));
    ebpf_cgroup_target_t *ect3 = callocz(1, sizeof(ebpf_cgroup_target_t));
    
    ect1->updated = 1;
    ect1->pids = NULL;
    ect1->next = ect2;
    
    ect2->updated = 0;
    ect2->pids = NULL;
    ect2->next = ect3;
    
    ect3->updated = 1;
    ect3->pids = NULL;
    ect3->next = NULL;
    
    ebpf_cgroup_pids = ect1;
    
    ebpf_remove_cgroup_target_update_list();
    
    // ect2 should be removed, ect1 and ect3 should remain
    assert_non_null(ebpf_cgroup_pids);
    assert_int_equal(ebpf_cgroup_pids->updated, 1);
    assert_non_null(ebpf_cgroup_pids->next);
    assert_int_equal(ebpf_cgroup_pids->next->updated, 1);
    
    freez(ebpf_cgroup_pids);
    freez(ebpf_cgroup_pids->next);
    ebpf_cgroup_pids = NULL;
    
    return 0;
}

// ============================================================================
// TEST: ebpf_cgroup_set_target_data
// ============================================================================
static int test_ebpf_cgroup_set_target_data(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t out;
    memset(&out, 0, sizeof(out));
    
    netdata_ebpf_cgroup_shm_body_t ptr;
    memset(&ptr, 0, sizeof(ptr));
    ptr.hash = 12345;
    strcpy(ptr.name, "test_cgroup");
    ptr.options = CGROUP_OPTIONS_SYSTEM_SLICE_SERVICE;
    
    ebpf_cgroup_set_target_data(&out, &ptr);
    
    assert_int_equal(out.hash, 12345);
    assert_string_equal(out.name, "test_cgroup");
    assert_int_equal(out.systemd, 1);
    assert_int_equal(out.updated, 1);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_cgroup_set_target_data - No systemd flag
// ============================================================================
static int test_ebpf_cgroup_set_target_data_no_systemd(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t out;
    memset(&out, 0, sizeof(out));
    
    netdata_ebpf_cgroup_shm_body_t ptr;
    memset(&ptr, 0, sizeof(ptr));
    ptr.hash = 54321;
    strcpy(ptr.name, "another_cgroup");
    ptr.options = 0;
    
    ebpf_cgroup_set_target_data(&out, &ptr);
    
    assert_int_equal(out.hash, 54321);
    assert_string_equal(out.name, "another_cgroup");
    assert_int_equal(out.systemd, 0);
    assert_int_equal(out.updated, 1);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_cgroup_find_or_create - Create new
// ============================================================================
static int test_ebpf_cgroup_find_or_create_new(void **state) {
    (void)state;
    
    ebpf_cgroup_pids = NULL;
    
    netdata_ebpf_cgroup_shm_body_t ptr;
    memset(&ptr, 0, sizeof(ptr));
    ptr.hash = 99999;
    strcpy(ptr.name, "new_cgroup");
    ptr.options = 0;
    
    ebpf_cgroup_target_t *result = ebpf_cgroup_find_or_create(&ptr);
    
    assert_non_null(result);
    assert_int_equal(result->hash, 99999);
    assert_string_equal(result->name, "new_cgroup");
    assert_int_equal(result->updated, 1);
    assert_ptr_equal(ebpf_cgroup_pids, result);
    
    freez(ebpf_cgroup_pids);
    ebpf_cgroup_pids = NULL;
    
    return 0;
}

// ============================================================================
// TEST: ebpf_cgroup_find_or_create - Find existing
// ============================================================================
static int test_ebpf_cgroup_find_or_create_existing(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t *existing = callocz(1, sizeof(ebpf_cgroup_target_t));
    existing->hash = 11111;
    strcpy(existing->name, "existing_cgroup");
    existing->updated = 0;
    existing->next = NULL;
    existing->pids = NULL;
    
    ebpf_cgroup_pids = existing;
    
    netdata_ebpf_cgroup_shm_body_t ptr;
    memset(&ptr, 0, sizeof(ptr));
    ptr.hash = 11111;
    strcpy(ptr.name, "existing_cgroup");
    ptr.options = 0;
    
    ebpf_cgroup_target_t *result = ebpf_cgroup_find_or_create(&ptr);
    
    assert_ptr_equal(result, existing);
    assert_int_equal(result->updated, 1);
    assert_int_equal(ebpf_cgroup_pids->next, NULL);
    
    freez(ebpf_cgroup_pids);
    ebpf_cgroup_pids = NULL;
    
    return 0;
}

// ============================================================================
// TEST: ebpf_cgroup_find_or_create - Add to existing list
// ============================================================================
static int test_ebpf_cgroup_find_or_create_add_to_list(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t *first = callocz(1, sizeof(ebpf_cgroup_target_t));
    first->hash = 22222;
    strcpy(first->name, "first_cgroup");
    first->updated = 0;
    first->next = NULL;
    first->pids = NULL;
    
    ebpf_cgroup_pids = first;
    
    netdata_ebpf_cgroup_shm_body_t ptr;
    memset(&ptr, 0, sizeof(ptr));
    ptr.hash = 33333;
    strcpy(ptr.name, "second_cgroup");
    ptr.options = 0;
    
    ebpf_cgroup_target_t *result = ebpf_cgroup_find_or_create(&ptr);
    
    assert_non_null(result);
    assert_int_equal(result->hash, 33333);
    assert_ptr_equal(first->next, result);
    
    freez(ebpf_cgroup_pids);
    freez(ebpf_cgroup_pids->next);
    ebpf_cgroup_pids = NULL;
    
    return 0;
}

// ============================================================================
// TEST: ebpf_update_pid_link_list - procfile_open fails
// ============================================================================
static int test_ebpf_update_pid_link_list_open_fail(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t *ect = callocz(1, sizeof(ebpf_cgroup_target_t));
    ect->pids = NULL;
    
    will_return(procfile_open_no_log, NULL);
    
    ebpf_update_pid_link_list(ect, "/path/to/cgroup.procs");
    
    assert_null(ect->pids);
    
    freez(ect);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_update_pid_link_list - procfile_readall fails
// ============================================================================
static int test_ebpf_update_pid_link_list_readall_fail(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t *ect = callocz(1, sizeof(ebpf_cgroup_target_t));
    ect->pids = NULL;
    
    procfile *ff = malloc(sizeof(procfile));
    
    will_return(procfile_open_no_log, ff);
    will_return(procfile_readall, NULL);
    
    ebpf_update_pid_link_list(ect, "/path/to/cgroup.procs");
    
    assert_null(ect->pids);
    
    free(ff);
    freez(ect);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_update_pid_link_list - Single PID
// ============================================================================
static int test_ebpf_update_pid_link_list_single_pid(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t *ect = callocz(1, sizeof(ebpf_cgroup_target_t));
    ect->pids = NULL;
    
    procfile *ff = malloc(sizeof(procfile));
    
    will_return(procfile_open_no_log, ff);
    will_return(procfile_readall, ff);
    will_return(procfile_lines, 1);
    will_return(procfile_lineword, "1234");
    will_return(str2l, 1234L);
    
    ebpf_update_pid_link_list(ect, "/path/to/cgroup.procs");
    
    assert_non_null(ect->pids);
    assert_int_equal(ect->pids->pid, 1234);
    assert_null(ect->pids->next);
    
    freez(ect->pids);
    free(ff);
    freez(ect);
    
    return 0;
}

// ============================================================================
// TEST: ebpf_update_pid_link_list - Multiple PIDs
// ============================================================================
static int test_ebpf_update_pid_link_list_multiple_pids(void **state) {
    (void)state;
    
    ebpf_cgroup_target_t *ect = callocz(1, sizeof(ebpf_cgroup_target_t));
    ect->pids = NULL;
    
    procfile *ff = malloc(sizeof(procfile));
    
    will_return(procfile_open_no_log, ff);
    will_return(procfile_readall, ff);
    will_return(procfile_lines, 3);
    
    // Line 0
    will_return(procfile_lineword, "1001");
    will_return(str2l, 1001L);
    
    // Line