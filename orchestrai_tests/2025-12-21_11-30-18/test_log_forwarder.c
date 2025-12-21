```c
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "log-forwarder.h"

void test_log_forwarder_basic_flow() {
    // Test complete lifecycle of log forwarder
    LOG_FORWARDER *lf = log_forwarder_start();
    assert(lf != NULL);

    // Create a temporary file descriptor for testing
    int test_fd = open("/tmp/test_log.txt", O_CREAT | O_RDWR, 0644);
    assert(test_fd >= 0);

    // Test adding file descriptor
    log_forwarder_add_fd(lf, test_fd);

    // Test annotating file descriptor
    log_forwarder_annotate_fd_name(lf, test_fd, "test_process");
    log_forwarder_annotate_fd_pid(lf, test_fd, getpid());

    // Test removing file descriptor
    bool removed = log_forwarder_del_and_close_fd(lf, test_fd);
    assert(removed == true);

    // Stop the log forwarder
    log_forwarder_stop(lf);

    // Clean up
    unlink("/tmp/test_log.txt");
}

void test_log_forwarder_edge_cases() {
    // Test with invalid file descriptors
    LOG_FORWARDER *lf = log_forwarder_start();
    assert(lf != NULL);

    // Try operations with invalid fd
    log_forwarder_add_fd(lf, -1);  // Should handle gracefully
    log_forwarder_annotate_fd_name(lf, -1, NULL);  // NULL name
    log_forwarder_annotate_fd_pid(lf, -1, 0);  // Zero pid

    bool removed = log_forwarder_del_and_close_fd(lf, -1);
    assert(removed == false);  // Expect false for invalid fd

    log_forwarder_stop(lf);
}

int main() {
    test_log_forwarder_basic_flow();
    test_log_forwarder_edge_cases();
    printf("All log_forwarder tests passed!\n");
    return 0;
}
```