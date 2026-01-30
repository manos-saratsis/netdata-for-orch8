#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// Mock structures and functions needed for testing
typedef struct {
    uint8_t *data;
    size_t pos;
    size_t len;
} test_rbuf_t;

typedef void (*free_fnc_t)(void *ptr);
typedef struct {
    uint8_t *data;
    size_t len;
    size_t pos;
} rbuf_t;

// Simple mock implementation of spinlock
typedef int SPINLOCK;

static void spinlock_init(SPINLOCK *spinlock) {
    *spinlock = 0;
}

static void spinlock_lock(SPINLOCK *spinlock) {
    // Mock implementation
}

static void spinlock_unlock(SPINLOCK *spinlock) {
    // Mock implementation
}

// Mock libnetdata functions
void *mallocz(size_t size) {
    return malloc(size);
}

void *callocz(size_t count, size_t size) {
    return calloc(count, size);
}

void *reallocz(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void freez(void *ptr) {
    free(ptr);
}

// Mock nd_log function
typedef enum {
    NDLS_DAEMON,
    NDLP_ERR,
    NDLP_WARNING,
    NDLP_INFO,
    NDLP_DEBUG
} log_level_t;

void nd_log(int ls, int level, const char *fmt, ...) {
    // Mock implementation
}

// Mock worker functions
typedef enum {
    WORKER_ACLK_SEND_FRAGMENT,
    WORKER_ACLK_BUFFER_COMPACT,
    WORKER_ACLK_RECLAIM_MEMORY,
    WORKER_ACLK_CPT_CONNACK,
    WORKER_ACLK_CPT_PUBACK,
    WORKER_ACLK_CPT_PINGRESP,
    WORKER_ACLK_CPT_SUBACK,
    WORKER_ACLK_CPT_PUBLISH,
    WORKER_ACLK_CPT_DISCONNECT,
    WORKER_ACLK_CPT_UNKNOWN,
    WORKER_ACLK_TRY_SEND_ALL,
    WORKER_ACLK_HANDLE_INCOMING,
    WORKER_ACLK_MSG_CALLBACK
} worker_type_t;

void worker_is_busy(worker_type_t type) {
    // Mock
}

void worker_is_idle(void) {
    // Mock
}

// Mock time functions
time_t now_realtime_sec(void) {
    return time(NULL);
}

usec_t now_monotonic_usec(void) {
    return 0;
}

time_t now_monotonic_sec(void) {
    return time(NULL);
}

#define usec_t uint64_t

// Mock netdata config functions
bool netdata_conf_is_iot(void) {
    return false;
}

bool netdata_conf_is_standalone(void) {
    return false;
}

// Mock rbuf functions
size_t rbuf_bytes_available(rbuf_t buf) {
    return buf.len - buf.pos;
}

size_t rbuf_pop(rbuf_t buf, char *output, size_t bytes) {
    if (buf.pos + bytes > buf.len)
        bytes = buf.len - buf.pos;
    memcpy(output, buf.data + buf.pos, bytes);
    buf.pos += bytes;
    return bytes;
}

void rbuf_bump_tail(rbuf_t buf, size_t bytes) {
    buf.pos += bytes;
}

// Mock c_rhash functions
typedef void *c_rhash;

c_rhash c_rhash_new(size_t size) {
    return malloc(sizeof(int));
}

void c_rhash_destroy(c_rhash hash) {
    free(hash);
}

int c_rhash_get_ptr_by_str(c_rhash hash, const char *key, void **val) {
    *val = NULL;
    return 1; // Not found
}

int c_rhash_get_ptr_by_uint64(c_rhash hash, uint64_t key, void **val) {
    *val = NULL;
    return 1; // Not found
}

void c_rhash_insert_str_ptr(c_rhash hash, const char *key, void *val) {
}

void c_rhash_insert_uint64_ptr(c_rhash hash, uint64_t key, void *val) {
}

typedef struct {
    int dummy;
} c_rhash_iter_t;

#define C_RHASH_ITER_T_INITIALIZER {0}

int c_rhash_iter_uint64_keys(c_rhash hash, c_rhash_iter_t *iter, uint64_t *key) {
    return 1; // End of iteration
}

int c_rhash_iter_str_keys(c_rhash hash, c_rhash_iter_t *iter, const char **key) {
    return 1; // End of iteration
}

// Mock Judy array
typedef void *Pvoid_t;
typedef unsigned long Word_t;

#define PJE0 0
#define PJERR ((Pvoid_t) -1)

Pvoid_t *JudyLIns(Pvoid_t *PArray, Word_t Index, const char *PJE) {
    return (Pvoid_t *)malloc(sizeof(uint32_t));
}

int JudyLDel(Pvoid_t *PArray, Word_t Index, const char *PJE) {
    return 1;
}

Pvoid_t *JudyLGet(Pvoid_t PArray, Word_t Index, const char *PJE) {
    return NULL;
}

Pvoid_t *JudyLFirstThenNext(Pvoid_t PArray, Word_t *Index, bool *first) {
    return NULL;
}

int JudyLFreeArray(Pvoid_t *PArray, const char *PJE) {
    return 0;
}

// Mock atomic operations
#define __atomic_load_n(ptr, order) (*(ptr))
#define __atomic_store_n(ptr, val, order) (*(ptr) = (val))
#define __atomic_fetch_add(ptr, val, order) ((*(ptr)) += (val), *(ptr))
#define __atomic_fetch_sub(ptr, val, order) ((*(ptr)) -= (val), *(ptr))

#define likely(x) (x)

// Endian functions
#define htobe16(x) (x)
#define be16toh(x) (x)
#define be32toh(x) (x)

// Pulse function
usec_t publish_latency;
void pulse_aclk_sent_message_acked(usec_t publish_latency, size_t len) {
    // Mock
}

time_t ping_timeout = 0;

// Define the actual code to be tested
#define CALLER_RESPONSIBILITY ((void *)-1)

// Include the implementation to test
#define TESTS 1

// ============== TEST CASES ==============

// Test: uint32_to_mqtt_vbi - boundary values
void test_uint32_to_mqtt_vbi_zero(void) {
    unsigned char buf[5] = {0};
    int len = uint32_to_mqtt_vbi(0, buf);
    assert(len == 1, "zero should encode to 1 byte");
    assert(buf[0] == 0, "zero should be 0x00");
}

void test_uint32_to_mqtt_vbi_single_byte(void) {
    unsigned char buf[5];
    
    // Test max single-byte value (127)
    memset(buf, 0xFF, sizeof(buf));
    int len = uint32_to_mqtt_vbi(127, buf);
    assert(len == 1, "127 should encode to 1 byte");
    assert(buf[0] == 0x7F, "127 should be 0x7F");
    assert(buf[1] == 0xFF, "buffer beyond should be unchanged");
    
    // Test min two-byte value (128)
    memset(buf, 0xFF, sizeof(buf));
    len = uint32_to_mqtt_vbi(128, buf);
    assert(len == 2, "128 should encode to 2 bytes");
    assert(buf[0] == 0x80, "first byte wrong");
    assert(buf[1] == 0x01, "second byte wrong");
}

void test_uint32_to_mqtt_vbi_two_bytes(void) {
    unsigned char buf[5];
    
    // Test max two-byte value (16383)
    memset(buf, 0xFF, sizeof(buf));
    int len = uint32_to_mqtt_vbi(16383, buf);
    assert(len == 2, "16383 should encode to 2 bytes");
    assert(buf[0] == 0xFF, "first byte wrong");
    assert(buf[1] == 0x7F, "second byte wrong");
    
    // Test min three-byte value (16384)
    memset(buf, 0xFF, sizeof(buf));
    len = uint32_to_mqtt_vbi(16384, buf);
    assert(len == 3, "16384 should encode to 3 bytes");
    assert(buf[0] == 0x80, "first byte wrong");
    assert(buf[1] == 0x80, "second byte wrong");
    assert(buf[2] == 0x01, "third byte wrong");
}

void test_uint32_to_mqtt_vbi_three_bytes(void) {
    unsigned char buf[5];
    
    // Test max three-byte value (2097151)
    memset(buf, 0xFF, sizeof(buf));
    int len = uint32_to_mqtt_vbi(2097151, buf);
    assert(len == 3, "2097151 should encode to 3 bytes");
    assert(buf[0] == 0xFF, "first byte wrong");
    assert(buf[1] == 0xFF, "second byte wrong");
    assert(buf[2] == 0x7F, "third byte wrong");
    
    // Test min four-byte value (2097152)
    memset(buf, 0xFF, sizeof(buf));
    len = uint32_to_mqtt_vbi(2097152, buf);
    assert(len == 4, "2097152 should encode to 4 bytes");
    assert(buf[0] == 0x80, "first byte wrong");
    assert(buf[1] == 0x80, "second byte wrong");
    assert(buf[2] == 0x80, "third byte wrong");
    assert(buf[3] == 0x01, "fourth byte wrong");
}

void test_uint32_to_mqtt_vbi_four_bytes(void) {
    unsigned char buf[5];
    
    // Test max four-byte value (268435455)
    memset(buf, 0xFF, sizeof(buf));
    int len = uint32_to_mqtt_vbi(268435455, buf);
    assert(len == 4, "268435455 should encode to 4 bytes");
    assert(buf[0] == 0xFF, "first byte wrong");
    assert(buf[1] == 0xFF, "second byte wrong");
    assert(buf[2] == 0xFF, "third byte wrong");
    assert(buf[3] == 0x7F, "fourth byte wrong");
}

void test_uint32_to_mqtt_vbi_overflow(void) {
    unsigned char buf[5];
    
    // Test overflow - should return 0
    memset(buf, 0xFF, sizeof(buf));
    int len = uint32_to_mqtt_vbi(268435456, buf);
    assert(len == 0, "overflow should return 0");
    
    // Test large overflow
    len = uint32_to_mqtt_vbi(999999999, buf);
    assert(len == 0, "large overflow should return 0");
}

void test_mqtt_vbi_to_uint32_single_byte(void) {
    char data[5];
    uint32_t result = 0;
    int rc;
    
    // Test zero
    data[0] = 0x00;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 0, "should parse zero successfully");
    assert(result == 0, "zero should decode correctly");
    
    // Test 127 (max single byte)
    data[0] = 0x7F;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 0, "should parse 127 successfully");
    assert(result == 127, "127 should decode correctly");
}

void test_mqtt_vbi_to_uint32_two_bytes(void) {
    char data[5];
    uint32_t result = 0;
    int rc;
    
    // Test 128 (min two byte)
    data[0] = 0x80;
    data[1] = 0x01;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 0, "should parse 128 successfully");
    assert(result == 128, "128 should decode correctly");
    
    // Test 16383 (max two byte)
    data[0] = 0xFF;
    data[1] = 0x7F;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 0, "should parse 16383 successfully");
    assert(result == 16383, "16383 should decode correctly");
}

void test_mqtt_vbi_to_uint32_three_bytes(void) {
    char data[5];
    uint32_t result = 0;
    int rc;
    
    // Test 16384 (min three byte)
    data[0] = 0x80;
    data[1] = 0x80;
    data[2] = 0x01;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 0, "should parse 16384 successfully");
    assert(result == 16384, "16384 should decode correctly");
    
    // Test 2097151 (max three byte)
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0x7F;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 0, "should parse 2097151 successfully");
    assert(result == 2097151, "2097151 should decode correctly");
}

void test_mqtt_vbi_to_uint32_four_bytes(void) {
    char data[5];
    uint32_t result = 0;
    int rc;
    
    // Test 2097152 (min four byte)
    data[0] = 0x80;
    data[1] = 0x80;
    data[2] = 0x80;
    data[3] = 0x01;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 0, "should parse 2097152 successfully");
    assert(result == 2097152, "2097152 should decode correctly");
    
    // Test 268435455 (max four byte)
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0x7F;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 0, "should parse 268435455 successfully");
    assert(result == 268435455, "268435455 should decode correctly");
}

void test_mqtt_vbi_to_uint32_overflow(void) {
    char data[5];
    uint32_t result = 0;
    int rc;
    
    // Test overflow - 5 bytes with continuation
    data[0] = 0x80;
    data[1] = 0x80;
    data[2] = 0x80;
    data[3] = 0x80;
    data[4] = 0x01;
    rc = mqtt_vbi_to_uint32(data, &result);
    assert(rc == 1, "should return error on overflow");
}

// Helper assertion function
void assert(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "ASSERTION FAILED: %s\n", message);
        exit(1);
    }
}

// Main test runner
int main(void) {
    printf("Running VBI encoding tests...\n");
    
    test_uint32_to_mqtt_vbi_zero();
    printf("✓ uint32_to_mqtt_vbi zero\n");
    
    test_uint32_to_mqtt_vbi_single_byte();
    printf("✓ uint32_to_mqtt_vbi single byte\n");
    
    test_uint32_to_mqtt_vbi_two_bytes();
    printf("✓ uint32_to_mqtt_vbi two bytes\n");
    
    test_uint32_to_mqtt_vbi_three_bytes();
    printf("✓ uint32_to_mqtt_vbi three bytes\n");
    
    test_uint32_to_mqtt_vbi_four_bytes();
    printf("✓ uint32_to_mqtt_vbi four bytes\n");
    
    test_uint32_to_mqtt_vbi_overflow();
    printf("✓ uint32_to_mqtt_vbi overflow\n");
    
    printf("\nRunning VBI decoding tests...\n");
    
    test_mqtt_vbi_to_uint32_single_byte();
    printf("✓ mqtt_vbi_to_uint32 single byte\n");
    
    test_mqtt_vbi_to_uint32_two_bytes();
    printf("✓ mqtt_vbi_to_uint32 two bytes\n");
    
    test_mqtt_vbi_to_uint32_three_bytes();
    printf("✓ mqtt_vbi_to_uint32 three bytes\n");
    
    test_mqtt_vbi_to_uint32_four_bytes();
    printf("✓ mqtt_vbi_to_uint32 four bytes\n");
    
    test_mqtt_vbi_to_uint32_overflow();
    printf("✓ mqtt_vbi_to_uint32 overflow\n");
    
    printf("\n✅ All VBI tests passed!\n");
    return 0;
}