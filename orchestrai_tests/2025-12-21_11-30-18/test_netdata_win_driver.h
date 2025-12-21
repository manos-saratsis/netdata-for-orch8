```c
#include <assert.h>
#include "netdata_win_driver.h"

void test_msr_request_structure() {
    MSR_REQUEST request = {
        .msr = 0x1234,
        .cpu = 0,
        .low = 0xABCD,
        .high = 0xEF01
    };

    assert(request.msr == 0x1234);
    assert(request.cpu == 0);
    assert(request.low == 0xABCD);
    assert(request.high == 0xEF01);
}

void test_cpu_detection_enum() {
    assert(NETDATA_CPU_WIN_UNKNOWN == 0);
    assert(NETDATA_CPU_WIN_INTEL == 1);
    assert(NETDATA_CPU_WIN_AMD == 2);
}

void test_driver_constants() {
    assert(strcmp(MSR_USER_PATH, "\\\\.\\NDDrv") == 0);
}

int main() {
    test_msr_request_structure();
    test_cpu_detection_enum();
    test_driver_constants();
    return 0;
}
```