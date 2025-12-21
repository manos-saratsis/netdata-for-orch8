```c
#include <assert.h>
#include <string.h>
#include "ebpf.h"

// Test suite for network parsing functions

void test_ebpf_parse_ip_list() {
    // Note: This would require mocking internal functions or creating test hooks
    ebpf_network_viewer_ip_list_t *test_list = NULL;
    
    // Test parsing various IP list formats
    ebpf_parse_ip_list_unsafe((void**)&test_list, "192.168.1.0/24");
    assert(test_list != NULL);
    
    // Clean up
    ebpf_clean_ip_structure(&test_list);
}

void test_ebpf_parse_port_list() {
    // Test port list parsing
    network_viewer_opt.included_port = NULL;
    
    // Test numeric port range
    ebpf_parse_port_list("80-100");
    assert(network_viewer_opt.included_port != NULL);
    
    // Test service name
    ebpf_parse_port_list("http");
    
    // Test wildcard
    ebpf_parse_port_list("*");
    
    // Clean up
    ebpf_clean_port_structure(&network_viewer_opt.included_port);
}

void test_ebpf_parse_ips() {
    // Reset existing lists
    network_viewer_opt.included_ips = NULL;
    network_viewer_opt.excluded_ips = NULL;
    
    // Test various IP parsing scenarios
    ebpf_parse_ips_unsafe("192.168.1.0/24 10.0.0.0/8");
    assert(network_viewer_opt.included_ips != NULL);
    
    // Test with exclusion
    ebpf_parse_ips_unsafe("!127.0.0.1/8 10.0.0.0/16");
    assert(network_viewer_opt.excluded_ips != NULL);
}

void test_ebpf_link_hostname() {
    // Test hostname linking mechanism
    ebpf_network_viewer_hostname_list_t *test_hostname = calloc(1, sizeof(ebpf_network_viewer_hostname_list_t));
    test_hostname->value = strdup("example.com");
    test_hostname->hash = simple_hash("example.com");
    
    network_viewer_opt.included_hostnames = NULL;
    ebpf_link_hostname(&network_viewer_opt.included_hostnames, test_hostname);
    
    assert(network_viewer_opt.included_hostnames != NULL);
}

int main() {
    test_ebpf_parse_ip_list();
    test_ebpf_parse_port_list();
    test_ebpf_parse_ips();
    test_ebpf_link_hostname();
    
    printf("All network parsing tests passed successfully!\n");
    return 0;
}
```