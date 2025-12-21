```c
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "atomic_flags.h"

void test_atomic_flags() {
    // Test atomic_flags_get
    uint64_t flags = 0b1010;
    assert(atomic_flags_get(&flags) == 0b1010);

    // Test atomic_flags_check
    assert(atomic_flags_check(&flags, 0b0010) == 0b0010);
    assert(atomic_flags_check(&flags, 0b0001) == 0);

    // Test atomic_flags_set
    uint64_t set_flags = 0;
    atomic_flags_set(&set_flags, 0b0101);
    assert(set_flags == 0b0101);

    // Test atomic_flags_clear
    uint64_t clear_flags = 0b1111;
    atomic_flags_clear(&clear_flags, 0b0011);
    assert(clear_flags == 0b1100);

    // Test atomic_flags_set_and_clear
    uint64_t complex_flags = 0b0101;
    uint64_t old_flags = atomic_flags_set_and_clear(&complex_flags, 0b1010, 0b0001);
    assert(old_flags == 0b0101);
    assert(complex_flags == 0b1110);

    printf("All atomic flags tests passed!\n");
}

int main() {
    test_atomic_flags();
    return 0;
}
```