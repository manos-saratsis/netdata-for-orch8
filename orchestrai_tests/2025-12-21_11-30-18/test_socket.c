```c
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../libnetdata.h"
#include "socket.h"
#include "security.h"

// Mocking for SSL functions
typedef struct {
    int dummy;  // Placeholder for SSL connection state
} MOCK_SSL;

MOCK_SSL mock_ssl;

int SSL_connection(void* ssl) {
    return 1;  // Always connected for testing
}

int netdata_ssl_has_pending(void* ssl) {
    return 0;
}

int netdata_ssl_write(void* ssl, const void* buf, size_t len) {
    return len;
}

// Test cases for ip_to_hostname
void test_ip_to_hostname() {
    char hostname[256];

    // Test valid IPv4
    assert(ip_to_hostname("8.8.8.8", hostname, sizeof(hostname)) == true);
    printf("IPv4 hostname test passed\n");

    // Test valid IPv6
    assert(ip_to_hostname("2001:4860:4860::8888", hostname, sizeof(hostname)) == true);
    printf("IPv6 hostname test passed\n");

    // Test invalid IP
    assert(ip_to_hostname("not_an_ip", hostname, sizeof(hostname)) == false);
    printf("Invalid IP hostname test passed\n");

    // Test NULL destination
    assert(ip_to_hostname("8.8.8.8", NULL, 0) == false);
    printf("NULL destination test passed\n");
}

// Test cases for socket utility functions
void test_socket_utilities() {
    int test_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert(test_socket >= 0);

    // Test fd_is_socket
    assert(fd_is_socket(test_socket) == true);
    assert(fd_is_socket(-1) == false);

    // Test is_socket_closed
    assert(is_socket_closed(-1) == true);

    // Test sock_setnonblock
    int nonblock_result = sock_setnonblock(test_socket, true);
    assert(nonblock_result == 1);

    nonblock_result = sock_setnonblock(test_socket, false);
    assert(nonblock_result == 0);

    // Test sock_setcloexec
    int cloexec_result = sock_setcloexec(test_socket, true);
    assert(cloexec_result == 1);

    cloexec_result = sock_setcloexec(test_socket, false);
    assert(cloexec_result == 0);

    // Cleanup
    close(test_socket);
}

// Test sock_setreuse functions (addr and port)
void test_socket_reuse() {
    int test_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert(test_socket >= 0);

    // Test SO_REUSEADDR
    int reuse_addr_result = sock_setreuse_addr(test_socket, true);
    assert(reuse_addr_result != -1);

    // Test SO_REUSEPORT
    int reuse_port_result = sock_setreuse_port(test_socket, true);
    assert(reuse_port_result != -1);

    close(test_socket);
}

// Test send_timeout function
void test_send_timeout() {
    int test_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert(test_socket >= 0);

    char test_buf[] = "test data";
    
    // Test send with mock SSL 
    ssize_t result = send_timeout((NETDATA_SSL*)&mock_ssl, test_socket, test_buf, sizeof(test_buf), 0, 1);
    assert(result > 0);

    // Test send without SSL
    result = send_timeout(NULL, test_socket, test_buf, sizeof(test_buf), 0, 1);
    assert(result > 0);

    close(test_socket);
}

// Test connection_allowed function (simplified mock)
void test_connection_allowed() {
    int test_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert(test_socket >= 0);

    char client_ip[256] = "127.0.0.1";
    char client_host[256] = "";
    SIMPLE_PATTERN* access_list = NULL;

    // Test with no access list
    int result = connection_allowed(test_socket, client_ip, client_host, sizeof(client_host), access_list, "test", 1);
    assert(result == 1);

    close(test_socket);
}

// Test TCP_DEFER_ACCEPT
void test_sock_set_tcp_defer_accept() {
    int test_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert(test_socket >= 0);

    // Test setting defer to true
    int defer_result = sock_set_tcp_defer_accept(test_socket, true);
    #ifdef TCP_DEFER_ACCEPT
    assert(defer_result != -1);
    #endif

    // Test setting defer to false
    defer_result = sock_set_tcp_defer_accept(test_socket, false);
    #ifdef TCP_DEFER_ACCEPT
    assert(defer_result != -1);
    #endif

    close(test_socket);
}

int main() {
    test_ip_to_hostname();
    test_socket_utilities();
    test_socket_reuse();
    test_send_timeout();
    test_connection_allowed();
    test_sock_set_tcp_defer_accept();

    printf("All socket tests passed successfully!\n");
    return 0;
}
```