#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "stacktrace.h"

// Test stacktrace_unittest()
void test_stacktrace_unittest() {
    int result = stacktrace_unittest();
    assert(result == 0);  // Expect successful test
}

int main() {
    test_stacktrace_unittest();
    printf("All stacktrace-unittest tests passed!\n");
    return 0;
}