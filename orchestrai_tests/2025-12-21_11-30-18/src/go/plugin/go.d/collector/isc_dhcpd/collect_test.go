package isc_dhcpd

import (
	"os"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestCollect(t *testing.T) {
	t.Run("collect with no file", func(t *testing.T) {
		c := &Collector{
			LeasesPath: "/non/existent/path",
			collected:  make(map[string]int64),
		}
		_, err := c.collect()
		assert.Error(t, err)
	})

	t.Run("collect with no modification", func(t *testing.T) {
		tmpFile, err := os.CreateTemp("", "test_leases")
		require.NoError(t, err)
		defer os.Remove(tmpFile.Name())

		c := &Collector{
			LeasesPath:      tmpFile.Name(),
			collected:       map[string]int64{"existing": 42},
			leasesModTime:   tmpFile.ModTime(),
			pools: []ipPool{
				{
					name:      "test_pool",
					addresses: nil,
				},
			},
		}

		result, err := c.collect()
		assert.NoError(t, err)
		assert.Equal(t, map[string]int64{"existing": 42, "active_leases_total": 0}, result)
	})

	t.Run("collect with pools", func(t *testing.T) {
		leaseContent := `
lease 192.168.1.10 {
    binding state active;
}
lease 192.168.1.11 {
    binding state active;
}
`
		tmpFile, err := os.CreateTemp("", "test_leases")
		require.NoError(t, err)
		defer os.Remove(tmpFile.Name())

		_, err = tmpFile.WriteString(leaseContent)
		require.NoError(t, err)
		tmpFile.Close()

		c := &Collector{
			LeasesPath:    tmpFile.Name(),
			collected:     make(map[string]int64),
			leasesModTime: time.Time{},
			pools: []ipPool{
				{
					name:      "test_pool",
					addresses: nil,
				},
			},
		}

		result, err := c.collect()
		assert.NoError(t, err)
		assert.Contains(t, result, "active_leases_total")
	})
}

func TestCollectPool(t *testing.T) {
	t.Run("collect pool metrics", func(t *testing.T) {
		collected := make(map[string]int64)
		pool := ipPool{
			name:      "test_pool",
			addresses: nil,
		}
		leases := []leaseEntry{}

		collectPool(collected, pool, leases)

		assert.Contains(t, collected, "dhcp_pool_test_pool_active_leases")
		assert.Contains(t, collected, "dhcp_pool_test_pool_utilization")
	})
}

func TestCalcPoolActiveLeases(t *testing.T) {
	testCases := []struct {
		name     string
		pool     ipPool
		leases   []leaseEntry
		expected int64
	}{
		{
			name: "no matching leases",
			pool: ipPool{},
			leases: []leaseEntry{
				{addr: netip.MustParseAddr("192.168.1.1")},
			},
			expected: 0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := calcPoolActiveLeases(tc.pool, tc.leases)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestCalcPoolUtilizationPercentage(t *testing.T) {
	testCases := []struct {
		name     string
		pool     ipPool
		leases   int64
		expected float64
	}{
		{
			name:     "zero leases",
			pool:     ipPool{},
			leases:   0,
			expected: 0,
		},
		{
			name:     "full pool",
			pool:     ipPool{},
			leases:   100,
			expected: 100,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := calcPoolUtilizationPercentage(tc.pool, tc.leases)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestRemoveInactiveLeases(t *testing.T) {
	testCases := []struct {
		name     string
		leases   []leaseEntry
		expected int
	}{
		{
			name: "remove inactive leases",
			leases: []leaseEntry{
				{bindingState: "active"},
				{bindingState: "free"},
				{bindingState: "active"},
			},
			expected: 2,
		},
		{
			name: "no active leases",
			leases: []leaseEntry{
				{bindingState: "free"},
				{bindingState: "expired"},
			},
			expected: 0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := removeInactiveLeases(tc.leases)
			assert.Len(t, result, tc.expected)
			for _, lease := range result {
				assert.Equal(t, "active", lease.bindingState)
			}
		})
	}
}