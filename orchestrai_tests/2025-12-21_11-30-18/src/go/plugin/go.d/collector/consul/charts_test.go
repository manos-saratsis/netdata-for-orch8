package consul

import (
	"testing"

	"github.com/blang/semver/v4"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestAddGlobalCharts(t *testing.T) {
	testCases := []struct {
		name            string
		setupCollector func() *Collector
		expectedCharts []string
	}{
		{
			name: "Client without telemetry",
			setupCollector: func() *Collector {
				c := &Collector{
					cfg: &Config{
						Config: &config{
							Datacenter: "test-dc",
							NodeName:   "test-node",
						},
					},
				}
				return c
			},
			expectedCharts: nil,
		},
		{
			name: "Client with telemetry",
			setupCollector: func() *Collector {
				c := &Collector{
					cfg: &Config{
						Config: &config{
							Datacenter: "test-dc",
							NodeName:   "test-node",
						},
					},
					telemetryPrometheusEnabled: true,
				}
				return c
			},
			expectedCharts: []string{
				"client_rpc_requests_rate",
				"client_rpc_requests_exceeded_rate",
				"client_rpc_requests_failed_rate",
				"memory_allocated",
				"memory_sys",
				"gc_pause_time",
				"license_expiration_time",
			},
		},
		{
			name: "Server with version before 1.13",
			setupCollector: func() *Collector {
				v := semver.Version{Major: 1, Minor: 12, Patch: 0}
				c := &Collector{
					cfg: &Config{
						Config: &config{
							Datacenter: "test-dc",
							NodeName:   "test-node",
						},
					},
					telemetryPrometheusEnabled: true,
					isServerNode:               true,
					version:                    &v,
				}
				return c
			},
			expectedCharts: []string{
				"raft_commit_time",
				"raft_leader_last_contact_time",
				"raft_commits_rate",
				"raft_leader_oldest_log_age",
				"license_expiration_time",
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := tc.setupCollector()

			charts := module.Charts{}
			c.Charts = func() *module.Charts { return &charts }

			c.addGlobalCharts()

			if tc.expectedCharts == nil {
				assert.Empty(t, charts)
				return
			}

			for _, chartID := range tc.expectedCharts {
				assert.Contains(t, charts, charts.Get(chartID))
			}
		})
	}
}

func TestNewServiceHealthCheckChart(t *testing.T) {
	check := &agentCheck{
		CheckID:      "test-check",
		Node:         "test-node",
		Name:         "Test Check",
		ServiceName:  "test-service",
	}

	chart := newServiceHealthCheckChart(check)

	assert.Equal(t, "health_check_test-check_status", chart.ID)
	assert.Contains(t, chart.Labels, module.Label{Key: "node_name", Value: "test-node"})
	assert.Contains(t, chart.Labels, module.Label{Key: "check_name", Value: "Test Check"})
	assert.Contains(t, chart.Labels, module.Label{Key: "service_name", Value: "test-service"})
}

func TestNewNodeHealthCheckChart(t *testing.T) {
	check := &agentCheck{
		CheckID: "test-check",
		Node:    "test-node",
		Name:    "Test Check",
	}

	chart := newNodeHealthCheckChart(check)

	assert.Equal(t, "health_check_test-check_status", chart.ID)
	assert.Contains(t, chart.Labels, module.Label{Key: "node_name", Value: "test-node"})
	assert.Contains(t, chart.Labels, module.Label{Key: "check_name", Value: "Test Check"})
}

func TestAddHealthCheckCharts(t *testing.T) {
	testCases := []struct {
		name            string
		check           *agentCheck
		expectedChartID string
	}{
		{
			name: "Service health check chart",
			check: &agentCheck{
				CheckID:      "svc-check",
				Node:         "test-node",
				ServiceName:  "test-service",
			},
			expectedChartID: "health_check_svc-check_status",
		},
		{
			name: "Node health check chart",
			check: &agentCheck{
				CheckID: "node-check",
				Node:    "test-node",
			},
			expectedChartID: "health_check_node-check_status",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				cfg: &Config{
					Config: &config{
						Datacenter: "test-dc",
					},
				},
			}

			charts := module.Charts{}
			c.Charts = func() *module.Charts { return &charts }

			c.addHealthCheckCharts(tc.check)

			assert.NotNil(t, charts.Get(tc.expectedChartID))
			chart := charts.Get(tc.expectedChartID)
			assert.Contains(t, chart.Labels, module.Label{
				Key:   "datacenter",
				Value: "test-dc",
			})
		})
	}
}

func TestRemoveHealthCheckCharts(t *testing.T) {
	c := &Collector{}
	charts := module.Charts{
		&module.Chart{
			ID: "health_check_test-check_status",
		},
	}
	c.Charts = func() *module.Charts { return &charts }

	c.removeHealthCheckCharts("test-check")

	chart := charts[0]
	assert.True(t, chart.IsRemoved())
	assert.False(t, chart.IsCreated())
}

func TestAddRemoveLeaderCharts(t *testing.T) {
	c := &Collector{
		cfg: &Config{
			Config: &config{
				Datacenter: "test-dc",
				NodeName:   "test-node",
			},
		},
	}

	charts := module.Charts{}
	c.Charts = func() *module.Charts { return &charts }

	c.addLeaderCharts()
	assert.NotEmpty(t, charts)
	for _, chart := range charts {
		assert.Contains(t, chart.Labels, module.Label{
			Key:   "datacenter",
			Value: "test-dc",
		})
		assert.Contains(t, chart.Labels, module.Label{
			Key:   "node_name",
			Value: "test-node",
		})
	}

	c.removeLeaderCharts()
	for _, chart := range charts {
		assert.True(t, chart.IsRemoved())
		assert.False(t, chart.IsCreated())
	}
}

func TestAddRemoveFollowerCharts(t *testing.T) {
	testCases := []struct {
		name          string
		isCloudManaged bool
		expectedCharts int
	}{
		{
			name:           "Not cloud managed",
			isCloudManaged: false,
			expectedCharts: 2,
		},
		{
			name:           "Cloud managed",
			isCloudManaged: true,
			expectedCharts: 1,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				cfg: &Config{
					Config: &config{
						Datacenter: "test-dc",
						NodeName:   "test-node",
					},
				},
				isCloudManaged: tc.isCloudManaged,
			}

			charts := module.Charts{}
			c.Charts = func() *module.Charts { return &charts }

			c.addFollowerCharts()
			assert.Len(t, charts, tc.expectedCharts)
			for _, chart := range charts {
				assert.Contains(t, chart.Labels, module.Label{
					Key:   "datacenter",
					Value: "test-dc",
				})
				assert.Contains(t, chart.Labels, module.Label{
					Key:   "node_name",
					Value: "test-node",
				})
			}

			c.removeFollowerCharts()
			for _, chart := range charts {
				assert.True(t, chart.IsRemoved())
				assert.False(t, chart.IsCreated())
			}
		})
	}
}