// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

/* Mock definitions for netdata types and functions */
typedef struct {
    unsigned char uuid[16];
} ND_UUID;

#define UUID_STR_LEN 37

typedef struct {
    ND_UUID claim_id_of_origin;
    ND_UUID claim_id_of_parent;
} ACLK_STRUCTURE;

typedef struct {
    ACLK_STRUCTURE aclk;
} RRDHOST;

typedef int SPINLOCK;
#define SPINLOCK_INITIALIZER 0

/* Global mock objects */
RRDHOST *localhost = NULL;
static int mock_aclk_online_value = 0;

/* Forward declarations from code under test */
extern void claim_id_clear_previous_working(void);
extern bool claim_id_set_str(const char *claim_id_str);
extern void claim_id_set(ND_UUID new_claim_id);
extern ND_UUID claim_id_get_uuid(void);
extern void claim_id_get_str(char str[UUID_STR_LEN]);
extern const char *claim_id_get_str_mallocz(void);
extern bool claim_id_is_set(ND_UUID uuid);
extern typedef struct {
    ND_UUID uuid;
    char str[UUID_STR_LEN];
} CLAIM_ID;
extern CLAIM_ID claim_id_get(void);
extern CLAIM_ID claim_id_get_last_working(void);
extern CLAIM_ID rrdhost_claim_id_get(RRDHOST *host);

/* Mock implementations */
static int spinlock_lock_count = 0;
static int spinlock_unlock_count = 0;

void spinlock_lock(SPINLOCK *spinlock) {
    spinlock_lock_count++;
    *spinlock = 1;
}

void spinlock_unlock(SPINLOCK *spinlock) {
    spinlock_unlock_count++;
    *spinlock = 0;
}

int uuid_parse(const char *uuid_str, unsigned char *uuid) {
    check_expected(uuid_str);
    return mock_type(int);
}

void uuid_unparse_lower(const unsigned char *uuid, char *str) {
    check_expected(uuid);
    const char *mock_str = mock_type(const char *);
    if (mock_str) {
        strcpy(str, mock_str);
    }
}

ND_UUID get_uuid_zero(void) {
    ND_UUID zero = { 0 };
    return zero;
}

#define UUID_ZERO get_uuid_zero()

int UUIDiszero(ND_UUID uuid) {
    for (int i = 0; i < 16; i++) {
        if (uuid.uuid[i] != 0) return 0;
    }
    return 1;
}

void *mallocz(size_t size) {
    return mock_type(void *);
}

int aclk_online(void) {
    return mock_type(int);
}

void memset_custom(void *s, int c, size_t n) {
    memset(s, c, n);
}

#undef memset
#define memset memset_custom

/* Test fixtures */

static int setup(void **state) {
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    return 0;
}

static int teardown(void **state) {
    if (localhost) {
        free(localhost);
        localhost = NULL;
    }
    return 0;
}

/* Tests for claim_id_clear_previous_working */

static void test_claim_id_clear_previous_working_basic(void **state) {
    /* Arrange - function will lock and unlock */
    /* Act */
    claim_id_clear_previous_working();
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_clear_previous_working_multiple_calls(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    claim_id_clear_previous_working();
    int first_lock_count = spinlock_lock_count;
    int first_unlock_count = spinlock_unlock_count;
    
    claim_id_clear_previous_working();
    
    /* Assert - each call should increment counts */
    assert_int_equal(spinlock_lock_count, first_lock_count + 1);
    assert_int_equal(spinlock_unlock_count, first_unlock_count + 1);
}

/* Tests for claim_id_set */

static void test_claim_id_set_with_zero_uuid(void **state) {
    /* Arrange */
    ND_UUID new_uuid = { { 0 } };
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    claim_id_set(new_uuid);
    
    /* Assert - should lock and unlock */
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_set_with_non_zero_uuid(void **state) {
    /* Arrange */
    ND_UUID new_uuid = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    claim_id_set(new_uuid);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_set_with_localhost_valid(void **state) {
    /* Arrange */
    localhost = (RRDHOST *)malloc(sizeof(RRDHOST));
    localhost->aclk.claim_id_of_origin = get_uuid_zero();
    
    ND_UUID new_uuid = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    
    /* Act */
    claim_id_set(new_uuid);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_set_with_localhost_null(void **state) {
    /* Arrange */
    localhost = NULL;
    ND_UUID new_uuid = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    
    /* Act */
    claim_id_set(new_uuid);
    
    /* Assert - should handle null localhost gracefully */
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

/* Tests for claim_id_set_str */

static void test_claim_id_set_str_with_null(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    bool result = claim_id_set_str(NULL);
    
    /* Assert */
    assert_true(result);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_set_str_with_empty_string(void **state) {
    /* Arrange */
    const char *empty_str = "";
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    bool result = claim_id_set_str(empty_str);
    
    /* Assert */
    assert_true(result);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_set_str_with_null_string(void **state) {
    /* Arrange */
    const char *null_str = "NULL";
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    bool result = claim_id_set_str(null_str);
    
    /* Assert */
    assert_true(result);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_set_str_with_valid_uuid(void **state) {
    /* Arrange */
    const char *valid_uuid = "550e8400-e29b-41d4-a716-446655440000";
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    expect_string(uuid_parse, uuid_str, valid_uuid);
    will_return(uuid_parse, 0);
    
    /* Act */
    bool result = claim_id_set_str(valid_uuid);
    
    /* Assert */
    assert_true(result);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_set_str_with_invalid_uuid(void **state) {
    /* Arrange */
    const char *invalid_uuid = "not-a-valid-uuid";
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    expect_string(uuid_parse, uuid_str, invalid_uuid);
    will_return(uuid_parse, -1);
    
    /* Act */
    bool result = claim_id_set_str(invalid_uuid);
    
    /* Assert */
    assert_false(result);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_set_str_with_malformed_uuid(void **state) {
    /* Arrange */
    const char *malformed = "550e8400-e29b-41d4";
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    expect_string(uuid_parse, uuid_str, malformed);
    will_return(uuid_parse, 1);
    
    /* Act */
    bool result = claim_id_set_str(malformed);
    
    /* Assert */
    assert_false(result);
}

/* Tests for claim_id_get_uuid */

static void test_claim_id_get_uuid_returns_uuid(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    ND_UUID result = claim_id_get_uuid();
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_get_uuid_thread_safety(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    claim_id_get_uuid();
    claim_id_get_uuid();
    
    /* Assert - each call should lock and unlock */
    assert_int_equal(spinlock_lock_count, 2);
    assert_int_equal(spinlock_unlock_count, 2);
}

/* Tests for claim_id_get_str */

static void test_claim_id_get_str_with_zero_uuid(void **state) {
    /* Arrange */
    char str[UUID_STR_LEN];
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    claim_id_get_str(str);
    
    /* Assert */
    assert_int_equal(str[0], 0);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_get_str_output_buffer(void **state) {
    /* Arrange */
    char str[UUID_STR_LEN];
    memset(str, 'A', UUID_STR_LEN);
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    claim_id_get_str(str);
    
    /* Assert - buffer should be cleared */
    assert_int_equal(str[0], 0);
}

/* Tests for claim_id_get_str_mallocz */

static void test_claim_id_get_str_mallocz_allocates_memory(void **state) {
    /* Arrange */
    char *allocated_str = (char *)malloc(UUID_STR_LEN);
    memset(allocated_str, 0, UUID_STR_LEN);
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    will_return(mallocz, (void *)allocated_str);
    
    /* Act */
    const char *result = claim_id_get_str_mallocz();
    
    /* Assert */
    assert_non_null(result);
    assert_ptr_equal(result, (const char *)allocated_str);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
    
    /* Cleanup */
    free((void *)result);
}

static void test_claim_id_get_str_mallocz_null_allocation_failure(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    will_return(mallocz, NULL);
    
    /* Act */
    const char *result = claim_id_get_str_mallocz();
    
    /* Assert */
    assert_null(result);
}

/* Tests for claim_id_get */

static void test_claim_id_get_with_zero_uuid(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    CLAIM_ID result = claim_id_get();
    
    /* Assert */
    assert_int_equal(result.str[0], 0);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_get_with_non_zero_uuid(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act - first set a uuid, then get it */
    ND_UUID new_uuid = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    claim_id_set(new_uuid);
    
    expect_any(uuid_unparse_lower, uuid);
    will_return(uuid_unparse_lower, "550e8400-e29b-41d4-a716-446655440000");
    
    CLAIM_ID result = claim_id_get();
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 2);
    assert_int_equal(spinlock_unlock_count, 2);
}

/* Tests for claim_id_get_last_working */

static void test_claim_id_get_last_working_with_zero_uuid(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    CLAIM_ID result = claim_id_get_last_working();
    
    /* Assert */
    assert_int_equal(result.str[0], 0);
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_claim_id_get_last_working_thread_safety(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    claim_id_get_last_working();
    claim_id_get_last_working();
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 2);
    assert_int_equal(spinlock_unlock_count, 2);
}

/* Tests for rrdhost_claim_id_get */

static void test_rrdhost_claim_id_get_with_localhost(void **state) {
    /* Arrange */
    localhost = (RRDHOST *)malloc(sizeof(RRDHOST));
    localhost->aclk.claim_id_of_origin = get_uuid_zero();
    localhost->aclk.claim_id_of_parent = get_uuid_zero();
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    CLAIM_ID result = rrdhost_claim_id_get(localhost);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

static void test_rrdhost_claim_id_get_with_localhost_and_claim_set(void **state) {
    /* Arrange */
    localhost = (RRDHOST *)malloc(sizeof(RRDHOST));
    localhost->aclk.claim_id_of_origin = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    localhost->aclk.claim_id_of_parent = get_uuid_zero();
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    ND_UUID new_uuid = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    claim_id_set(new_uuid);
    
    expect_any(uuid_unparse_lower, uuid);
    will_return(uuid_unparse_lower, "550e8400-e29b-41d4-a716-446655440000");
    
    /* Act */
    CLAIM_ID result = rrdhost_claim_id_get(localhost);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 2);
    assert_int_equal(spinlock_unlock_count, 2);
}

static void test_rrdhost_claim_id_get_with_remote_host_origin_set(void **state) {
    /* Arrange */
    RRDHOST remote_host = { 0 };
    remote_host.aclk.claim_id_of_origin = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    remote_host.aclk.claim_id_of_parent = get_uuid_zero();
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    expect_any(uuid_unparse_lower, uuid);
    will_return(uuid_unparse_lower, "550e8400-e29b-41d4-a716-446655440000");
    
    /* Act */
    CLAIM_ID result = rrdhost_claim_id_get(&remote_host);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 0);
    assert_int_equal(spinlock_unlock_count, 0);
}

static void test_rrdhost_claim_id_get_with_remote_host_origin_zero(void **state) {
    /* Arrange */
    RRDHOST remote_host = { 0 };
    remote_host.aclk.claim_id_of_origin = get_uuid_zero();
    remote_host.aclk.claim_id_of_parent = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    expect_any(uuid_unparse_lower, uuid);
    will_return(uuid_unparse_lower, "550e8400-e29b-41d4-a716-446655440001");
    
    /* Act */
    CLAIM_ID result = rrdhost_claim_id_get(&remote_host);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 0);
    assert_int_equal(spinlock_unlock_count, 0);
}

static void test_rrdhost_claim_id_get_with_remote_host_both_zero(void **state) {
    /* Arrange */
    RRDHOST remote_host = { 0 };
    remote_host.aclk.claim_id_of_origin = get_uuid_zero();
    remote_host.aclk.claim_id_of_parent = get_uuid_zero();
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    /* Act */
    CLAIM_ID result = rrdhost_claim_id_get(&remote_host);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 0);
    assert_int_equal(spinlock_unlock_count, 0);
    assert_int_equal(result.str[0], 0);
}

static void test_rrdhost_claim_id_get_localhost_aclk_offline_with_parent(void **state) {
    /* Arrange */
    localhost = (RRDHOST *)malloc(sizeof(RRDHOST));
    localhost->aclk.claim_id_of_origin = get_uuid_zero();
    localhost->aclk.claim_id_of_parent = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    
    ND_UUID new_uuid = { { 0 } };
    claim_id_set(new_uuid);
    
    will_return(aclk_online, 0);
    expect_any(uuid_unparse_lower, uuid);
    will_return(uuid_unparse_lower, "550e8400-e29b-41d4-a716-446655440001");
    
    /* Act */
    CLAIM_ID result = rrdhost_claim_id_get(localhost);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
}

/* Edge case tests */

static void test_claim_id_set_str_with_single_character(void **state) {
    /* Arrange */
    const char *single_char = "a";
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    expect_string(uuid_parse, uuid_str, single_char);
    will_return(uuid_parse, -1);
    
    /* Act */
    bool result = claim_id_set_str(single_char);
    
    /* Assert */
    assert_false(result);
}

static void test_claim_id_set_str_case_sensitivity(void **state) {
    /* Arrange */
    const char *null_lower = "null";
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    
    expect_string(uuid_parse, uuid_str, null_lower);
    will_return(uuid_parse, -1);
    
    /* Act - "null" (lowercase) should be parsed as UUID, not treated as NULL */
    bool result = claim_id_set_str(null_lower);
    
    /* Assert */
    assert_false(result);
}

static void test_claim_id_consecutive_operations(void **state) {
    /* Arrange */
    spinlock_lock_count = 0;
    spinlock_unlock_count = 0;
    localhost = NULL;
    
    /* Act */
    claim_id_clear_previous_working();
    assert_int_equal(spinlock_lock_count, 1);
    assert_int_equal(spinlock_unlock_count, 1);
    
    claim_id_get_uuid();
    assert_int_equal(spinlock_lock_count, 2);
    assert_int_equal(spinlock_unlock_count, 2);
    
    claim_id_get_last_working();
    assert_int_equal(spinlock_lock_count, 3);
    assert_int_equal(spinlock_unlock_count, 3);
    
    /* Assert */
    assert_int_equal(spinlock_lock_count, 3);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        /* claim_id_clear_previous_working tests */
        cmocka_unit_test_setup_teardown(test_claim_id_clear_previous_working_basic, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_clear_previous_working_multiple_calls, setup, teardown),
        
        /* claim_id_set tests */
        cmocka_unit_test_setup_teardown(test_claim_id_set_with_zero_uuid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_with_non_zero_uuid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_with_localhost_valid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_with_localhost_null, setup, teardown),
        
        /* claim_id_set_str tests */
        cmocka_unit_test_setup_teardown(test_claim_id_set_str_with_null, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_str_with_empty_string, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_str_with_null_string, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_str_with_valid_uuid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_str_with_invalid_uuid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_str_with_malformed_uuid, setup, teardown),
        
        /* claim_id_get_uuid tests */
        cmocka_unit_test_setup_teardown(test_claim_id_get_uuid_returns_uuid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_get_uuid_thread_safety, setup, teardown),
        
        /* claim_id_get_str tests */
        cmocka_unit_test_setup_teardown(test_claim_id_get_str_with_zero_uuid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_get_str_output_buffer, setup, teardown),
        
        /* claim_id_get_str_mallocz tests */
        cmocka_unit_test_setup_teardown(test_claim_id_get_str_mallocz_allocates_memory, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_get_str_mallocz_null_allocation_failure, setup, teardown),
        
        /* claim_id_get tests */
        cmocka_unit_test_setup_teardown(test_claim_id_get_with_zero_uuid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_get_with_non_zero_uuid, setup, teardown),
        
        /* claim_id_get_last_working tests */
        cmocka_unit_test_setup_teardown(test_claim_id_get_last_working_with_zero_uuid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_get_last_working_thread_safety, setup, teardown),
        
        /* rrdhost_claim_id_get tests */
        cmocka_unit_test_setup_teardown(test_rrdhost_claim_id_get_with_localhost, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rrdhost_claim_id_get_with_localhost_and_claim_set, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rrdhost_claim_id_get_with_remote_host_origin_set, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rrdhost_claim_id_get_with_remote_host_origin_zero, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rrdhost_claim_id_get_with_remote_host_both_zero, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rrdhost_claim_id_get_localhost_aclk_offline_with_parent, setup, teardown),
        
        /* Edge case tests */
        cmocka_unit_test_setup_teardown(test_claim_id_set_str_with_single_character, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_set_str_case_sensitivity, setup, teardown),
        cmocka_unit_test_setup_teardown(test_claim_id_consecutive_operations, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}