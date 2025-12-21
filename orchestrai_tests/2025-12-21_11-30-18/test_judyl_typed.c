#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <Judy.h>
#include "src/libnetdata/libjudy/judyl-typed.h"

// Example type for demonstration
typedef struct {
    int x;
    char y;
} TestStruct;

// Define a JudyL typed set for TestStruct
DEFINE_JUDYL_TYPED(test_struct, TestStruct)

static void test_judyl_typed_macro_operations(void **state) {
    (void) state; // unused

    test_struct_JudyLSet set;
    test_struct_INIT(&set);

    // Test SET operation
    TestStruct test_value = {42, 'A'};
    bool set_result = test_struct_SET(&set, 100, test_value);
    assert_true(set_result);

    // Test GET operation
    TestStruct retrieved_value = test_struct_GET(&set, 100);
    assert_int_equal(retrieved_value.x, 42);
    assert_int_equal(retrieved_value.y, 'A');

    // Test DEL operation
    bool del_result = test_struct_DEL(&set, 100);
    assert_true(del_result);

    // Cleanup
    test_struct_FREE(&set, NULL, NULL);
}

static void test_judyl_typed_first_next_operations(void **state) {
    (void) state; // unused

    test_struct_JudyLSet set;
    test_struct_INIT(&set);

    // Insert multiple values
    test_struct_SET(&set, 10, (TestStruct){1, 'X'});
    test_struct_SET(&set, 20, (TestStruct){2, 'Y'});
    test_struct_SET(&set, 30, (TestStruct){3, 'Z'});

    // Test FIRST
    Word_t index = 0;
    TestStruct first_value = test_struct_FIRST(&set, &index);
    assert_int_equal(index, 10);
    assert_int_equal(first_value.x, 1);

    // Test NEXT
    index = 10;
    TestStruct next_value = test_struct_NEXT(&set, &index);
    assert_int_equal(index, 20);
    assert_int_equal(next_value.x, 2);

    // Cleanup
    test_struct_FREE(&set, NULL, NULL);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_judyl_typed_macro_operations),
        cmocka_unit_test(test_judyl_typed_first_next_operations),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}