```c
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "median.h"
#include "mocks.h"  // Assuming we'll create a mocks header

// Mock RRDR structure and related functions for testing
typedef struct {
    void *internal_owa;
    struct {
        void *data;
    } time_grouping;
    struct {
        long group;
    } view;
} MockRRDR;

// Test tg_median_create_internal
void test_tg_median_create_internal() {
    // Test with default parameters
    {
        MockRRDR r = {0};
        r.view.group = 5;
        
        tg_median_create_internal(&r, "", 0.0);
        
        struct tg_median *g = (struct tg_median *)r.time_grouping.data;
        assert(g != NULL);
        assert(g->series_size == 10);  // Minimum entries is 10
        assert(g->percent == 0.0);
        
        tg_median_free(&r);
    }
    
    // Test with custom percentage
    {
        MockRRDR r = {0};
        r.view.group = 20;
        
        tg_median_create_internal(&r, "10", 0.0);
        
        struct tg_median *g = (struct tg_median *)r.time_grouping.data;
        assert(g != NULL);
        assert(g->percent == 0.1);
        
        tg_median_free(&r);
    }
    
    // Test percentage boundary conditions
    {
        MockRRDR r = {0};
        r.view.group = 15;
        
        tg_median_create_internal(&r, "-5", 0.0);
        struct tg_median *g = (struct tg_median *)r.time_grouping.data;
        assert(g->percent == 0.0);
        
        tg_median_create_internal(&r, "60", 0.0);
        assert(g->percent == 0.5);
        
        tg_median_free(&r);
    }
}

// Test median creation functions
void test_median_create_variants() {
    MockRRDR r = {0};
    
    tg_median_create(&r, "");
    tg_median_create_trimmed_1(&r, "");
    tg_median_create_trimmed_2(&r, "");
    tg_median_create_trimmed_3(&r, "");
    tg_median_create_trimmed_5(&r, "");
    tg_median_create_trimmed_10(&r, "");
    tg_median_create_trimmed_15(&r, "");
    tg_median_create_trimmed_20(&r, "");
    tg_median_create_trimmed_25(&r, "");
    
    // We expect these not to crash
    assert(r.time_grouping.data != NULL);
    
    tg_median_free(&r);
}

// Test tg_median_add and tg_median_flush
void test_median_add_flush() {
    MockRRDR r = {0};
    r.view.group = 10;
    
    // Add less than series size
    tg_median_create(&r, "");
    struct tg_median *g = (struct tg_median *)r.time_grouping.data;
    
    for(int i = 0; i < 5; i++) {
        tg_median_add(&r, (NETDATA_DOUBLE)i);
    }
    
    RRDR_VALUE_FLAGS options = 0;
    NETDATA_DOUBLE result = tg_median_flush(&r, &options);
    assert(result == 2.0);  // Median of [0, 1, 2, 3, 4]
    
    // Test empty series
    g->next_pos = 0;
    options = 0;
    result = tg_median_flush(&r, &options);
    assert(result == 0.0);
    assert(options & RRDR_VALUE_EMPTY);
    
    tg_median_free(&r);
}

// Test tg_median_reset
void test_median_reset() {
    MockRRDR r = {0};
    r.view.group = 10;
    
    tg_median_create(&r, "");
    struct tg_median *g = (struct tg_median *)r.time_grouping.data;
    
    for(int i = 0; i < 5; i++) {
        tg_median_add(&r, (NETDATA_DOUBLE)i);
    }
    
    tg_median_reset(&r);
    assert(g->next_pos == 0);
    
    tg_median_free(&r);
}

int main() {
    test_tg_median_create_internal();
    test_median_create_variants();
    test_median_add_flush();
    test_median_reset();
    
    printf("All median tests passed!\n");
    return 0;
}
```