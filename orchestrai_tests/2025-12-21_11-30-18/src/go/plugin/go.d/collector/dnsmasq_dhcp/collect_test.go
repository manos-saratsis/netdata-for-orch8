```go
package dnsmasq_dhcp

import (
	"bytes"
	"math/big"
	"net/netip"
	"os"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/pkg/iprange"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestFindLeases(t *testing.T) {
	testCases := []struct {
		name     string
		input    string
		expected []netip.Addr
	}{
		{
			name: "Valid leases with IPv4",
			input: `1560300536 08:00:27:61:3c:ee 2.2.2.3 debian8 *
1560300414 08:00:27:61:3c:dd 192.168.1.100 host1 *`,
			expected: []netip.Addr{
				netip.MustParseAddr("2.2.2.3"),
				netip.MustParseAddr("192.168.1.100"),
			},
		},
		{
			name: "Valid leases with IPv6",
			input: `1560300536 08:00:27:61:3c:ee 1234::20b debian8 *
1560300414 08:00:27:61:3c:dd 2001:db8::1 host1 *`,
			expected: []netip.Addr{
				netip.MustParseAddr("1234::20b"),
				netip.MustParseAddr("2001:db8::1"),
			},
		},
		{
			name:     "Empty input",
			input:    "",
			expected: []netip.Addr{},
		},
		{
			name: "Invalid lease lines",
			input: `incomplete line
1560300536 08:00:27:61:3c:ee invalid_ip host *
garbage data`,
			expected: []netip.Addr{},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			reader := bytes.NewBufferString(tc.input)
			result := findLeases(reader)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestCalcPercent(t *testing.T) {
	testCases := []struct {
		name     string
		ips      int64
		hosts    *big.Int
		expected float64
	}{
		{
			name:     "Normal case",
			ips:      50,
			hosts:    big.NewInt(100),
			expected: 50.0,
		},
		{
			name:     "Zero ips",
			ips:      0,
			hosts:    big.NewInt(100),
			expected: 0.0,
		},
		{
			name:     "Zero hosts",
			ips:      50,
			hosts:    big.NewInt(0),
			expected: 0.0,
		},
		{
			name:     "Large numbers",
			ips:      1000000,
			hosts:    big.NewInt(2000000),
			expected: 50.0,
		},
		{
			name:     "Non-int64 hosts",
			ips:      50,
			hosts:    new(big.Int).Exp(big.NewInt(2), big.NewInt(100), nil),
			expected: 0.0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := calcPercent(tc.ips, tc.hosts)
			assert.InDelta(t, tc.expected, result, 0.001)
		})
	}
}

type mockCollector struct {
	Collector
	mx map[string]int64
}

func TestCollector_collect(t *testing.T) {
	testCases := []struct {
		name             string
		setupCollector   func(*mockCollector)
		expectedError    bool
		expectedMetrics  map[string]int64
		mockLeaseContent string
	}{
		{
			name: "Successful lease collection",
			setupCollector: func(mc *mockCollector) {
				mc.LeasesPath = "/tmp/test_leases"
				mc.mx = make(map[string]int64)
				mc.dhcpRanges = []iprange.Range{
					iprange.MustNewRange("192.168.1.0/24"),
				}
				mc.dhcpHosts = []netip.Addr{
					netip.MustParseAddr("192.168.1.5"),
				}
				mc.cacheDHCPRanges = make(map[string]bool)
				mc.parseConfigEvery = time.Hour
			},
			mockLeaseContent: `1560300536 08:00:27:61:3c:ee 192.168.1.3 debian8 *
1560300414 660684014 192.168.1.4 * 00:01:00:01:24:90:cf:a3:08:00:27:61:3c:ee`,
			expectedMetrics: map[string]int64{
				"ipv4_dhcp_ranges":   1,
				"ipv4_dhcp_hosts":    1,
				"ipv6_dhcp_ranges":   0,
				"ipv6_dhcp_hosts":    0,
				"dhcp_range_192.168.1.0/24_allocated_leases": 3,
				"dhcp_range_192.168.1.0/24_utilization": 3, // Approximation of utilization
			},
		},
		{
			name: "File not found error",
			setupCollector: func(mc *mockCollector) {
				mc.LeasesPath = "/path/to/nonexistent/file"
				mc.mx = make(map[string]int64)
			},
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			// Create a temporary file for testing
			if tc.mockLeaseContent != "" {
				tmpfile, err := os.CreateTemp("", "leases")
				assert.NoError(t, err)
				defer os.Remove(tmpfile.Name())

				_, err = tmpfile.Write([]byte(tc.mockLeaseContent))
				assert.NoError(t, err)
				tmpfile.Close()
			}

			c := &mockCollector{
				mx: make(map[string]int64),
			}
			if tc.setupCollector != nil {
				tc.setupCollector(c)
			}

			// Mock or adjust file path if needed
			if tc.mockLeaseContent != "" {
				c.LeasesPath = tmpfile.Name()
			}

			metrics, err := c.collect()

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				// Check each expected metric exists and matches
				for k, v := range tc.expectedMetrics {
					assert.Contains(t, metrics, k)
					assert.Equal(t, v, metrics[k], "Metric %s did not match", k)
				}
			}
		})
	}
}

func TestCollector_collectV4V6Stats(t *testing.T) {
	testCases := []struct {
		name            string
		dhcpRanges      []iprange.Range
		dhcpHosts       []netip.Addr
		expectedMetrics map[string]int64
	}{
		{
			name: "Mixed IPv4 and IPv6",
			dhcpRanges: []iprange.Range{
				iprange.MustNewRange("192.168.1.0/24"),
				iprange.MustNewRange("2001:db8::/64"),
			},
			dhcpHosts: []netip.Addr{
				netip.MustParseAddr("192.168.1.5"),
				netip.MustParseAddr("2001:db8::1"),
			},
			expectedMetrics: map[string]int64{
				"ipv4_dhcp_ranges": 1,
				"ipv6_dhcp_ranges": 1,
				"ipv4_dhcp_hosts":  1,
				"ipv6_dhcp_hosts":  1,
			},
		},
		{
			name:            "No ranges or hosts",
			dhcpRanges:      []iprange.Range{},
			dhcpHosts:       []netip.Addr{},
			expectedMetrics: map[string]int64{"ipv4_dhcp_ranges": 0, "ipv6_dhcp_ranges": 0, "ipv4_dhcp_hosts": 0, "ipv6_dhcp_hosts": 0},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				mx:          make(map[string]int64),
				dhcpRanges:  tc.dhcpRanges,
				dhcpHosts:   tc.dhcpHosts,
			}

			c.collectV4V6Stats()

			for k, v := range tc.expectedMetrics {
				assert.Equal(t, v, c.mx[k], "Metric %s did not match", k)
			}
		})
	}
}
```