```c
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "refcount.h"

void test_refcount_basic_operations() {
    REFCOUNT count = 0;

    // Test initial reference count
    assert(refcount_references(&count) == 0);

    // Test reference increment
    assert(refcount_increment(&count) == 1);
    assert(refcount_references(&count) == 1);

    // Test reference decrement
    assert(refcount_decrement(&count) == 0);
    assert(refcount_references(&count) == 0);
}

void test_refcount_acquire() {
    REFCOUNT count = 0;

    // Test successful acquire
    assert(refcount_acquire(&count) == true);
    assert(refcount_references(&count) == 1);

    // Test multiple acquire
    assert(refcount_acquire(&count) == true);
    assert(refcount_references(&count) == 2);

    // Cleanup references
    refcount_release(&count);
    refcount_release(&count);
}

void test_refcount_deletion() {
    REFCOUNT count = 2;

    // Cannot acquire for deletion with active references
    assert(refcount_acquire_for_deletion(&count) == false);

    // Release references
    refcount_release(&count);
    refcount_release(&count);

    // Now can acquire for deletion
    assert(refcount_acquire_for_deletion(&count) == true);
    assert(refcount_references(&count) == REFCOUNT_DELETED);
}

void test_refcount_release_and_acquire_for_deletion() {
    REFCOUNT count = 1;

    // Release and acquire for deletion in one step
    assert(refcount_release_and_acquire_for_deletion(&count) == true);
    assert(refcount_references(&count) == REFCOUNT_DELETED);
}

void test_refcount_advanced_operations() {
    REFCOUNT count = 0;

    // Test advanced acquisition
    assert(refcount_acquire_advanced(&count) > 0);

    // Test maximum reference count limit
    count = REFCOUNT_MAX;
    assert(refcount_acquire_advanced(&count) == REFCOUNT_ERROR);
}

void test_refcount_error_conditions() {
    REFCOUNT count = -1; // Invalid reference count

    // Should trigger fatal error in debug mode
    // In this test, we'll just check if acquire fails
    assert(refcount_acquire(&count) == false);
}

int main() {
    test_refcount_basic_operations();
    test_refcount_acquire();
    test_refcount_deletion();
    test_refcount_release_and_acquire_for_deletion();
    test_refcount_advanced_operations();
    test_refcount_error_conditions();

    printf("All refcount tests passed!\n");
    return 0;
}
```