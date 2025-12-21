```c
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include "ebpf.h"

// Test suite for helper functions in ebpf.h

void test_ebpf_netmask() {
    // Test various prefix values
    assert(ntohl(ebpf_netmask(0)) == 0);  // all bits set to 0
    assert(ntohl(ebpf_netmask(32)) == 0xFFFFFFFF);  // all bits set to 1
    assert(ntohl(ebpf_netmask(16)) == 0xFFFF0000);  // half bits set
    assert(ntohl(ebpf_netmask(24)) == 0xFFFFFF00);  // 3/4 bits set
}

void test_ebpf_broadcast() {
    in_addr_t addr = inet_addr("192.168.1.0");
    
    // Test various broadcast calculations
    assert(ntohl(ebpf_broadcast(htonl(addr), 24)) == 0xC0A801FF);  // /24 network
    assert(ntohl(ebpf_broadcast(htonl(addr), 16)) == 0xC0A8FFFF);  // /16 network
    assert(ntohl(ebpf_broadcast(htonl(addr), 0)) == 0xFFFFFFFF);   // Full broadcast
}

void test_ebpf_ipv4_network() {
    in_addr_t addr = inet_addr("192.168.1.100");
    
    // Test network address calculations
    assert(ntohl(ebpf_ipv4_network(htonl(addr), 24)) == 0xC0A80100);  // /24 network
    assert(ntohl(ebpf_ipv4_network(htonl(addr), 16)) == 0xC0A80000);  // /16 network
    assert(ntohl(ebpf_ipv4_network(htonl(addr), 0)) == 0x00000000);   // No network
}

void test_ebpf_ip2nl() {
    uint8_t dst[16];
    
    // Test valid IPv4 conversion
    assert(ebpf_ip2nl(dst, "192.168.1.1", AF_INET, "test_ipv4") == 0);
    assert(ebpf_ip2nl(dst, "255.255.255.255", AF_INET, "test_ipv4_max") == 0);
    
    // Test valid IPv6 conversion
    assert(ebpf_ip2nl(dst, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", AF_INET6, "test_ipv6") == 0);
    
    // Test invalid IP conversion (expects -1)
    assert(ebpf_ip2nl(dst, "invalid_ip", AF_INET, "test_invalid") == -1);
}

int main() {
    test_ebpf_netmask();
    test_ebpf_broadcast();
    test_ebpf_ipv4_network();
    test_ebpf_ip2nl();
    
    printf("All tests passed successfully!\n");
    return 0;
}
```