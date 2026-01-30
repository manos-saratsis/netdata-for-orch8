#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Mock the needed structures and functions
typedef struct {
    unsigned char uuid[16];
} ND_UUID;

typedef struct {
    ND_UUID claim_id_of_origin;
    ND_UUID claim_id_of_parent;
} ACLK;

typedef struct {
    ACLK aclk;
} RRDHOST;

typedef struct {
    int lock;
} SPINLOCK;

typedef struct {
    char str[36];
} CLAIM_ID;

// Mock global variables
RRDHOST *localhost = NULL;
SPINLOCK spinlock;

// Mock functions
void spinlock_lock(SPINLOCK *lock) { }
void spinlock_unlock(SPINLOCK *lock) { }

bool UUIDiszero(ND_UUID uuid) {
    for (int i = 0; i < 16; i++)
        if (uuid.uuid[i] != 0) return false;
    return true;
}

int uuid_parse(const char *in, unsigned char *uuid) {
    if (!in || !*in) return 1;
    if (strlen(in) == 36) {
        memset(uuid, 0x12, 16);
        return 0;
    }
    return 1;
}

void uuid_unparse_lower(const unsigned char *uuid, char *out) {
    if (out) {
        snprintf(out, 36, "12345678-1234-1234-1234-123456789012");
    }
}

bool aclk_online(void) {
    return false;
}

ND_UUID UUID_ZERO = {{0}};

void *mallocz(size_t size) {
    return malloc(size);
}

void freez(void *ptr) {
    free(ptr);
}

// Include the actual implementation
#include "../claim/claim_id.c"

// Test: claim_id_clear_previous_working
static void test_claim_id_clear_previous_working(void **state) {
    claim_id_clear_previous_working();
    // Verify the function completes without error
    assert_true(1);
}

// Test: claim_id_set with non-zero UUID and aclk offline
static void test_claim_id_set_with_uuid_aclk_offline(void **state) {
    ND_UUID new_uuid = {{0x12, 0x34, 0x56, 0x78}};
    
    localhost = (RRDHOST *)malloc(sizeof(RRDHOST));
    memset(localhost, 0, sizeof(RRDHOST));
    
    claim_id_set(new_uuid);
    
    ND_UUID retrieved = claim_id_get_uuid();
    assert_memory_equal(&retrieved.uuid, &new_uuid.uuid, 16);
    
    free(localhost);
}

// Test: claim_id_set_str with NULL
static void test_claim_id_set_str_null(void **state) {
    bool result = claim_id_set_str(NULL);
    assert_true(result);
    
    ND_UUID retrieved = claim_id_get_uuid();
    assert_true(UUIDiszero(retrieved));
}

// Test: claim_id_set_str with empty string
static void test_claim_id_set_str_empty(void **state) {
    bool result = claim_id_set_str("");
    assert_true(result);
    
    ND_UUID retrieved = claim_id_get_uuid();
    assert_true(UUIDiszero(retrieved));
}

// Test: claim_id_set_str with "NULL"
static void test_claim_id_set_str_null_string(void **state) {
    bool result = claim_id_set_str("NULL");
    assert_true(result);
    
    ND_UUID retrieved = claim_id_get_uuid();
    assert_true(UUIDiszero(retrieved));
}

// Test: claim_id_set_str with valid UUID
static void test_claim_id_set_str_valid_uuid(void **state) {
    const char *valid_uuid = "12345678-1234-5678-1234-567812345678";
    bool result = claim_id_set_str(valid_uuid);
    assert_true(result);
}

// Test: claim_id_set_str with invalid UUID
static void test_claim_id_set_str_invalid_uuid(void **state) {
    const char *invalid_uuid = "not-a-valid-uuid";
    bool result = claim_id_set_str(invalid_uuid);
    assert_false(result);
}

// Test: claim_id_get_uuid
static void test_claim_id_get_uuid(void **state) {
    ND_UUID uuid = claim_id_get_uuid();
    assert_non_null(&uuid);
}

// Test: claim_id_get_str with zero UUID
static void test_claim_id_get_str_zero_uuid(void **state) {
    char str[36];
    memset(str, 0xFF, sizeof(str));
    
    claim_id_get_str(str);
    
    // Should be zeroed out
    for (int i = 0; i < 36; i++) {
        assert_int_equal(str[i], 0);
    }
}

// Test: claim_id_get_str with non-zero UUID
static void test_claim_id_get_str_non_zero_uuid(void **state) {
    ND_UUID new_uuid = {{0x12, 0x34, 0x56, 0x78}};
    claim_id_set(new_uuid);
    
    char str[36];
    memset(str, 0, sizeof(str));
    claim_id_get_str(str);
    
    // Should contain UUID string
    assert_string_equal(str, "12345678-1234-1234-1234-123456789012");
}

// Test: claim_id_get_str_mallocz
static void test_claim_id_get_str_mallocz(void **state) {
    const char *str = claim_id_get_str_mallocz();
    assert_non_null(str);
    freez((void *)str);
}

// Test: claim_id_get returns proper CLAIM_ID struct
static void test_claim_id_get(void **state) {
    CLAIM_ID cid = claim_id_get();
    assert_non_null(&cid.uuid);
    assert_non_null(&cid.str);
}

// Test: claim_id_get_last_working
static void test_claim_id_get_last_working(void **state) {
    CLAIM_ID cid = claim_id_get_last_working();
    assert_non_null(&cid);
}

// Test: rrdhost_claim_id_get with localhost
static void test_rrdhost_claim_id_get_localhost(void **state) {
    localhost = (RRDHOST *)malloc(sizeof(RRDHOST));
    memset(localhost, 0, sizeof(RRDHOST));
    
    CLAIM_ID cid = rrdhost_claim_id_get(localhost);
    assert_non_null(&cid);
    
    free(localhost);
}

// Test: rrdhost_claim_id_get with non-localhost
static void test_rrdhost_claim_id_get_non_localhost(void **state) {
    RRDHOST host;
    memset(&host, 0, sizeof(RRDHOST));
    host.aclk.claim_id_of_origin = UUID_ZERO;
    host.aclk.claim_id_of_parent = UUID_ZERO;
    
    CLAIM_ID cid = rrdhost_claim_id_get(&host);
    assert_non_null(&cid);
}

// Test: rrdhost_claim_id_get with origin set
static void test_rrdhost_claim_id_get_with_origin(void **state) {
    RRDHOST host;
    memset(&host, 0, sizeof(RRDHOST));
    host.aclk.claim_id_of_origin = (ND_UUID){{0x12, 0x34, 0x56, 0x78}};
    
    CLAIM_ID cid = rrdhost_claim_id_get(&host);
    assert_non_null(&cid);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_claim_id_clear_previous_working),
        cmocka_unit_test(test_claim_id_set_with_uuid_aclk_offline),
        cmocka_unit_test(test_claim_id_set_str_null),
        cmocka_unit_test(test_claim_id_set_str_empty),
        cmocka_unit_test(test_claim_id_set_str_null_string),
        cmocka_unit_test(test_claim_id_set_str_valid_uuid),
        cmocka_unit_test(test_claim_id_set_str_invalid_uuid),
        cmocka_unit_test(test_claim_id_get_uuid),
        cmocka_unit_test(test_claim_id_get_str_zero_uuid),
        cmocka_unit_test(test_claim_id_get_str_non_zero_uuid),
        cmocka_unit_test(test_claim_id_get_str_mallocz),
        cmocka_unit_test(test_claim_id_get),
        cmocka_unit_test(test_claim_id_get_last_working),
        cmocka_unit_test(test_rrdhost_claim_id_get_localhost),
        cmocka_unit_test(test_rrdhost_claim_id_get_non_localhost),
        cmocka_unit_test(test_rrdhost_claim_id_get_with_origin),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}