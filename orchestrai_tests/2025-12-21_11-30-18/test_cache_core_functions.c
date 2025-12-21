#include <assert.h>
#include "cache.h"
#include "mrg.h"

// Mock functions to replace actual dependencies
static void mock_free_clean_page(PGC *cache, PGC_ENTRY entry) {
    // Stub implementation
}

static void mock_save_dirty_page(PGC *cache, PGC_ENTRY *entries, PGC_PAGE **pages, size_t entries_count) {
    // Stub implementation
}

// Test fixture
typedef struct {
    PGC *cache;
} TestFixture;

static void setup(TestFixture *fixture) {
    fixture->cache = pgc_create(
        "test_cache",               // name
        32 * 1024 * 1024,           // clean_size_bytes 
        mock_free_clean_page,       // free callback
        64,                         // max dirty pages per flush
        NULL,                       // save init callback
        mock_save_dirty_page,       // save dirty callback
        10,                         // max pages per inline eviction
        10,                         // max inline evictors
        1000,                       // max skip pages per inline eviction
        10,                         // max flushes inline
        PGC_OPTIONS_DEFAULT,        // options
        1,                          // partitions
        0                           // additional bytes per page
    );
    assert(fixture->cache != NULL);
}

static void teardown(TestFixture *fixture) {
    pgc_destroy(fixture->cache, true);
}

// Test adding a clean page
void test_pgc_page_add_clean_page() {
    TestFixture fixture;
    setup(&fixture);

    bool added;
    PGC_ENTRY entry = {
        .section = 1,
        .metric_id = 10,
        .start_time_s = 100,
        .end_time_s = 200,
        .update_every_s = 10,
        .size = 4096,
        .data = NULL,
        .hot = false
    };

    PGC_PAGE *page = pgc_page_add_and_acquire(fixture.cache, entry, &added);
    assert(page != NULL);
    assert(added == true);
    assert(pgc_is_page_clean(page) == true);
    assert(pgc_page_section(page) == 1);
    assert(pgc_page_metric(page) == 10);

    pgc_page_release(fixture.cache, page);
    teardown(&fixture);
}

// Test adding a hot page
void test_pgc_page_add_hot_page() {
    TestFixture fixture;
    setup(&fixture);

    bool added;
    PGC_ENTRY entry = {
        .section = 1,
        .metric_id = 10,
        .start_time_s = 100,
        .end_time_s = 200,
        .update_every_s = 10,
        .size = 4096,
        .data = NULL,
        .hot = true
    };

    PGC_PAGE *page = pgc_page_add_and_acquire(fixture.cache, entry, &added);
    assert(page != NULL);
    assert(added == true);
    assert(pgc_is_page_hot(page) == true);
    assert(pgc_page_section(page) == 1);
    assert(pgc_page_metric(page) == 10);

    pgc_page_release(fixture.cache, page);
    teardown(&fixture);
}

// Test page state transitions
void test_pgc_page_state_transitions() {
    TestFixture fixture;
    setup(&fixture);

    // Add a hot page
    PGC_PAGE *hot_page = pgc_page_add_and_acquire(fixture.cache, (PGC_ENTRY){
        .section = 1,
        .metric_id = 10,
        .start_time_s = 100,
        .end_time_s = 200,
        .size = 4096,
        .hot = true
    }, NULL);

    // Transition hot page to dirty
    pgc_page_hot_to_dirty_and_release(fixture.cache, hot_page, false);

    // Reacquire the page and check its state
    PGC_PAGE *dirty_page = pgc_page_get_and_acquire(fixture.cache, 1, 10, 100, PGC_SEARCH_EXACT);
    assert(dirty_page != NULL);
    assert(pgc_is_page_dirty(dirty_page) == true);

    pgc_page_release(fixture.cache, dirty_page);
    teardown(&fixture);
}

// Test page search methods
void test_pgc_page_search_methods() {
    TestFixture fixture;
    setup(&fixture);

    // Add multiple pages for the same metric
    pgc_page_add_and_acquire(fixture.cache, (PGC_ENTRY){
        .section = 1,
        .metric_id = 20,
        .start_time_s = 100,
        .end_time_s = 200,
        .size = 4096,
    }, NULL);

    pgc_page_add_and_acquire(fixture.cache, (PGC_ENTRY){
        .section = 1,
        .metric_id = 20,
        .start_time_s = 300,
        .end_time_s = 400,
        .size = 4096,
    }, NULL);

    // Test exact search
    PGC_PAGE *exact_page = pgc_page_get_and_acquire(fixture.cache, 1, 20, 100, PGC_SEARCH_EXACT);
    assert(exact_page != NULL);
    assert(pgc_page_start_time_s(exact_page) == 100);
    pgc_page_release(fixture.cache, exact_page);

    // Test closest search
    PGC_PAGE *closest_page = pgc_page_get_and_acquire(fixture.cache, 1, 20, 250, PGC_SEARCH_CLOSEST);
    assert(closest_page != NULL);
    assert(pgc_page_start_time_s(closest_page) == 300);
    pgc_page_release(fixture.cache, closest_page);

    teardown(&fixture);
}

int main() {
    test_pgc_page_add_clean_page();
    test_pgc_page_add_hot_page();
    test_pgc_page_state_transitions();
    test_pgc_page_search_methods();
    return 0;
}