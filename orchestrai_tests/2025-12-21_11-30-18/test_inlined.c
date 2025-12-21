```c
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "libnetdata/inlined.h"

void test_str2u() {
    // Test basic positive number
    assert(str2u("123") == 123);
    
    // Test leading spaces
    assert(str2u("  456") == 456);
    
    // Test zero
    assert(str2u("0") == 0);
    
    // Test max value
    char max_str[20];
    snprintf(max_str, sizeof(max_str), "%u", UINT_MAX);
    assert(str2u(max_str) == UINT_MAX);
}

void test_str2i() {
    // Test positive number
    assert(str2i("123") == 123);
    
    // Test negative number
    assert(str2i("-456") == -456);
    
    // Test leading spaces
    assert(str2i("  -789") == -789);
    
    // Test zero
    assert(str2i("0") == 0);
    
    // Test max/min values
    char max_str[20], min_str[20];
    snprintf(max_str, sizeof(max_str), "%d", INT_MAX);
    snprintf(min_str, sizeof(min_str), "%d", INT_MIN);
    
    assert(str2i(max_str) == INT_MAX);
    assert(str2i(min_str) == INT_MIN);
}

void test_streq() {
    // Test equal strings
    assert(streq("hello", "hello") == true);
    
    // Test different strings
    assert(streq("hello", "world") == false);
    
    // Test NULL cases
    assert(streq(NULL, NULL) == true);
    assert(streq("hello", NULL) == false);
    assert(streq(NULL, "hello") == false);
}

void test_strstartswith() {
    // Test valid prefix
    assert(strstartswith("hello world", "hello") == true);
    
    // Test no match
    assert(strstartswith("world", "hello") == false);
    
    // Test prefix longer than string
    assert(strstartswith("hi", "hello") == false);
    
    // Test NULL cases
    assert(strstartswith(NULL, "hello") == false);
    assert(strstartswith("hello", NULL) == false);
}

void test_strendswith() {
    // Test valid suffix
    assert(strendswith("hello world", "world") == true);
    
    // Test no match
    assert(strendswith("hello", "world") == false);
    
    // Test suffix longer than string
    assert(strendswith("hi", "hello") == false);
    
    // Test NULL cases
    assert(strendswith(NULL, "world") == false);
    assert(strendswith("hello", NULL) == false);
}

void test_trim() {
    char buffer1[] = "  hello world  ";
    char *result1 = trim(buffer1);
    assert(strcmp(result1, "hello world") == 0);
    
    char buffer2[] = "   ";
    char *result2 = trim(buffer2);
    assert(result2 == NULL);
    
    char buffer3[] = "";
    char *result3 = trim(buffer3);
    assert(result3 == NULL);
}

void test_trim_all() {
    char buffer1[] = "  hello   world  test  ";
    trim_all(buffer1);
    assert(strcmp(buffer1, "hello world test") == 0);
    
    char buffer2[] = "   ";
    trim_all(buffer2);
    assert(strcmp(buffer2, "") == 0);
}

void test_str2ndd() {
    // Test basic decimal
    assert(fabs(str2ndd("123.45", NULL) - 123.45) < DBL_EPSILON);
    
    // Test scientific notation
    assert(fabs(str2ndd("1.23e2", NULL) - 123.0) < DBL_EPSILON);
    
    // Test negative number
    assert(fabs(str2ndd("-123.45", NULL) - (-123.45)) < DBL_EPSILON);
    
    // Test NaN
    assert(isnan(str2ndd("nan", NULL)));
    
    // Test infinity
    assert(isinf(str2ndd("inf", NULL)));
}

int main() {
    test_str2u();
    test_str2i();
    test_streq();
    test_strstartswith();
    test_strendswith();
    test_trim();
    test_trim_all();
    test_str2ndd();
    
    printf("All tests passed!\n");
    return 0;
}
```