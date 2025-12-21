package isc_dhcpd

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestParseDHCPdLeasesFile(t *testing.T) {
	testCases := []struct {
		name           string
		leaseContent   string
		expectedLeases int
	}{
		{
			name: "valid DHCPv4 leases",
			leaseContent: `
lease 192.168.1.10 {
    binding state active;
}
lease 192.168.1.11 {
    binding state free;
}
			`,
			expectedLeases: 2,
		},
		{
			name: "valid DHCPv6 leases",
			leaseContent: `
iaaddr 1985:470:1f0b:c9a::001 {
    binding state active;
}
iaaddr 1985:470:1f0b:c9a::002 {
    binding state free;
}
			`,
			expectedLeases: 2,
		},
		{
			name:           "empty lease file",
			leaseContent:   "",
			expectedLeases: 0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			tmpFile, err := os.CreateTemp("", "test_leases")
			require.NoError(t, err)
			defer os.Remove(tmpFile.Name())

			_, err = tmpFile.WriteString(tc.leaseContent)
			require.NoError(t, err)
			tmpFile.Close()

			leases, err := parseDHCPdLeasesFile(tmpFile.Name())
			assert.NoError(t, err)
			assert.Len(t, leases, tc.expectedLeases)
		})
	}

	t.Run("non-existent file", func(t *testing.T) {
		leases, err := parseDHCPdLeasesFile("/non/existent/file")
		assert.Error(t, err)
		assert.Nil(t, leases)
	})
}

func TestLeaseEntry(t *testing.T) {
	testCases := []struct {
		name    string
		addr    string
		state   string
		isValid bool
	}{
		{
			name:    "valid entry",
			addr:    "192.168.1.1",
			state:   "active",
			isValid: true,
		},
		{
			name:    "invalid address",
			addr:    "invalid",
			state:   "active",
			isValid: false,
		},
		{
			name:    "empty state",
			addr:    "192.168.1.1",
			state:   "",
			isValid: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			var addr netip.Addr
			var err error
			if tc.addr != "invalid" {
				addr = netip.MustParseAddr(tc.addr)
			}
			
			l := leaseEntry{
				addr:         addr,
				bindingState: tc.state,
			}

			assert.Equal(t, tc.isValid, l.isAddrValid())
			assert.Equal(t, tc.state != "", l.hasBindingState())
		})
	}
}