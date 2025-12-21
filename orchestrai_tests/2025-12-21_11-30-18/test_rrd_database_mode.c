```c
#include <assert.h>
#include <string.h>
#include "src/database/rrd-database-mode.h"

void test_rrd_memory_mode_name() {
    // Test all defined memory modes
    assert(strcmp(rrd_memory_mode_name(RRD_DB_MODE_NONE), RRD_DB_MODE_NONE_NAME) == 0);
    assert(strcmp(rrd_memory_mode_name(RRD_DB_MODE_RAM), RRD_DB_MODE_RAM_NAME) == 0);
    assert(strcmp(rrd_memory_mode_name(RRD_DB_MODE_ALLOC), RRD_DB_MODE_ALLOC_NAME) == 0);
    assert(strcmp(rrd_memory_mode_name(RRD_DB_MODE_DBENGINE), RRD_DB_MODE_DBENGINE_NAME) == 0);
}

void test_rrd_memory_mode_id() {
    // Test converting names back to modes
    assert(rrd_memory_mode_id(RRD_DB_MODE_NONE_NAME) == RRD_DB_MODE_NONE);
    assert(rrd_memory_mode_id(RRD_DB_MODE_RAM_NAME) == RRD_DB_MODE_RAM);
    assert(rrd_memory_mode_id(RRD_DB_MODE_ALLOC_NAME) == RRD_DB_MODE_ALLOC);
    assert(rrd_memory_mode_id(RRD_DB_MODE_DBENGINE_NAME) == RRD_DB_MODE_DBENGINE);
}

void test_default_rrd_memory_mode() {
    // Ensure default mode is defined
    assert(default_rrd_memory_mode != RRD_DB_MODE_NONE);
}

int main() {
    test_rrd_memory_mode_name();
    test_rrd_memory_mode_id();
    test_default_rrd_memory_mode();
    return 0;
}
```