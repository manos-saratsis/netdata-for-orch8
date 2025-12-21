package isc_dhcpd

import (
	"context"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestNew(t *testing.T) {
	c := New()
	assert.NotNil(t, c)
	assert.Equal(t, "/var/lib/dhcp/dhcpd.leases", c.LeasesPath)
	assert.NotNil(t, c.collected)
}

func TestConfiguration(t *testing.T) {
	c := New()
	config := c.Configuration()
	assert.NotNil(t, config)
}

func TestInit(t *testing.T) {
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
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				Config: tc.config,
			}

			err := c.Init(context.Background())
			if tc.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCheck(t *testing.T) {
	testCases := []struct {
		name    string
		config  Config
		wantErr bool
	}{
		{
			name: "valid config with metrics",
			config: Config{
				LeasesPath: "testdata/sample_leases.txt",
				Pools: []PoolConfig{
					{Name: "test_pool", Networks: "192.168.1.0/24"},
				},
			},
			wantErr: false,
		},
		{
			name: "invalid leases file",
			config: Config{
				LeasesPath: "/non/existent/file",
				Pools: []PoolConfig{
					{Name: "test_pool", Networks: "192.168.1.0/24"},
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
			err := c.Init(context.Background())
			if !tc.wantErr {
				assert.NoError(t, err)
			}

			err = c.Check(context.Background())
			if tc.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollect(t *testing.T) {
	testCases := []struct {
		name    string
		config  Config
		wantNil bool
	}{
		{
			name: "valid config with metrics",
			config: Config{
				LeasesPath: "testdata/sample_leases.txt",
				Pools: []PoolConfig{
					{Name: "test_pool", Networks: "192.168.1.0/24"},
				},
			},
			wantNil: false,
		},
		{
			name: "invalid leases file",
			config: Config{
				LeasesPath: "/non/existent/file",
				Pools: []PoolConfig{
					{Name: "test_pool", Networks: "192.168.1.0/24"},
				},
			},
			wantNil: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				Config: tc.config,
			}
			err := c.Init(context.Background())
			if !tc.wantNil {
				assert.NoError(t, err)
			}

			result := c.Collect(context.Background())
			if tc.wantNil {
				assert.Nil(t, result)
			} else {
				assert.NotNil(t, result)
			}
		})
	}
}