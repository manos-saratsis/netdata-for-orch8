```c
#ifndef TEST_DISK_AWAIT_H
#define TEST_DISK_AWAIT_H

#include <assert.h>
#include "disk-await.h"
#include "../../../libnetdata/libnetdata.h"

// Mock callback for testing
void mock_instance_label_callback(RRDSET *st, void *data) {
    // Simulate a callback
    assert(st != NULL);
}

void test_disk_await_struct() {
    // Test struct size and members
    ND_DISK_AWAIT await_struct = {0};
    assert(sizeof(await_struct.st_await) == sizeof(RRDSET*));
    assert(sizeof(await_struct.rd_await_reads) == sizeof(RRDDIM*));
    assert(sizeof(await_struct.rd_await_writes) == sizeof(RRDDIM*));
}

void test_common_disk_await() {
    ND_DISK_AWAIT await_struct = {0};
    
    // Test with valid inputs
    common_disk_await(
        &await_struct,      // Disk await struct
        "test_disk",        // ID
        "Test Disk",        // Name
        10.5,               // Read avg ms
        15.7,               // Write avg ms
        1,                  // Update every
        mock_instance_label_callback,  // Callback
        NULL                // Callback data
    );

    // After first call, st_await should be initialized
    assert(await_struct.st_await != NULL);
    assert(await_struct.rd_await_reads != NULL);
    assert(await_struct.rd_await_writes != NULL);

    // Test with zero values
    common_disk_await(
        &await_struct,
        "zero_disk",
        "Zero Disk",
        0.0,
        0.0,
        1,
        NULL,
        NULL
    );
}

#endif // TEST_DISK_AWAIT_H
```