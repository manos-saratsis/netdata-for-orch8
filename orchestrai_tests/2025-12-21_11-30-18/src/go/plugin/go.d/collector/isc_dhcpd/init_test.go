package isc_dhcpd

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestValidateConfig(t *testing.T) {
	testCases := []struct {
		name    string
		config  Config
		wantErr bool
	}{
		{
			name: "valid config",
			config: Config{
				LeasesPath: "/var/lib/dhcp/dhcpd.leases",
				Pools: []PoolConfig{
					{Name: "test_pool", Networks: "192.168.1.0/24"},
				},
			},
			wantErr: false,
		},
		{
			name: "missing leases path",
			config: Config{
				Pools: []PoolConfig{
					{Name: "test_pool", Networks: "192.168.1.0/24"},
				},
			},
			wantErr: true,
		},
		{
			name: "empty pools",
			config: Config{
				LeasesPath: "/var/lib/dhcp/dhcpd.leases",
			},
			wantErr: true,
		},
		{
			name: "pool with missing name",
			config: Config{
				LeasesPath: "/var/lib/dhcp/dhcpd.leases",
				Pools: []PoolConfig{
					{Networks: "192.168.1.0/24"},
				},
			},
			wantErr: true,
		},
		{
			name: "pool with missing networks",
			config: Config{
				LeasesPath: "/var/lib/dhcp/dhcpd.leases",
				Pools: []PoolConfig{
					{Name: "test_pool"},
				},
			},
			wantErr: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				Config: tc.config,
			}
			err := c.validateConfig()
			if tc.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestInitPools(t *testing.T) {
	testCases := []struct {
		name    string
		config  Config
		wantErr bool
	}{
		{
			name: "valid config",
			config: Config{
				LeasesPath: "/var/lib/dhcp/dhcpd.leases",
				Pools: []PoolConfig{
					{Name: "test_pool", Networks: "192.168.1.0/24"},
				},
			},
			wantErr: false,
		},
		{
			name: "invalid network range",
			config: Config{
				LeasesPath: "/var/lib/dhcp/dhcpd.leases",
				Pools: []PoolConfig{
					{Name: "test_pool", Networks: "invalid_range"},
				},
			},
			wantErr: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				Config: tc.config,
			}
			pools, err := c.initPools()
			if tc.wantErr {
				assert.Error(t, err)
				assert.Empty(t, pools)
			} else {
				assert.NoError(t, err)
				assert.NotEmpty(t, pools)
			}
		})
	}
}

func TestInitCharts(t *testing.T) {
	testCases := []struct {
		name   string
		pools  []ipPool
		expect int
	}{
		{
			name: "single pool",
			pools: []ipPool{
				{
					name: "test_pool",
				},
			},
			expect: 3, // 1 total leases chart + 2 pool-specific charts
		},
		{
			name:   "no pools",
			pools:  []ipPool{},
			expect: 1, // only total leases chart
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{}
			charts, err := c.initCharts(tc.pools)
			assert.NoError(t, err)
			assert.NotNil(t, charts)
			assert.Len(t, *charts, tc.expect)
		})
	}
}

func TestCleanPoolNameForChart(t *testing.T) {
	testCases := []struct {
		name     string
		input    string
		expected string
	}{
		{"spaces", "test pool", "test_pool"},
		{"dots", "test.pool", "test_pool"},
		{"mixed", "test.pool name", "test_pool_name"},
		{"no changes", "testpool", "testpool"},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := cleanPoolNameForChart(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}