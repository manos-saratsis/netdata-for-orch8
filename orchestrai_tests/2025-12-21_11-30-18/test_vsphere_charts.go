package vsphere

import (
	"testing"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	rs "github.com/netdata/netdata/go/plugins/plugin/go.d/collector/vsphere/resources"
	"github.com/stretchr/testify/assert"
)

func TestGetVMClusterName(t *testing.T) {
	testCases := []struct {
		name     string
		vm       *rs.VM
		expected string
	}{
		{
			name: "Different cluster and host names",
			vm: &rs.VM{
				Hier: rs.Hierarchy{
					Cluster: rs.Cluster{Name: "TestCluster"},
					Host:    rs.Host{Name: "TestHost"},
				},
			},
			expected: "TestCluster",
		},
		{
			name: "Cluster name same as host name",
			vm: &rs.VM{
				Hier: rs.Hierarchy{
					Cluster: rs.Cluster{Name: "TestHost"},
					Host:    rs.Host{Name: "TestHost"},
				},
			},
			expected: "",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := getVMClusterName(tc.vm)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestGetHostClusterName(t *testing.T) {
	testCases := []struct {
		name     string
		host     *rs.Host
		expected string
	}{
		{
			name: "Different cluster and host names",
			host: &rs.Host{
				Name: "TestHost",
				Hier: rs.Hierarchy{
					Cluster: rs.Cluster{Name: "TestCluster"},
				},
			},
			expected: "TestCluster",
		},
		{
			name: "Cluster name same as host name",
			host: &rs.Host{
				Name: "TestHost",
				Hier: rs.Hierarchy{
					Cluster: rs.Cluster{Name: "TestHost"},
				},
			},
			expected: "",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := getHostClusterName(tc.vm)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestNewVMCharts(t *testing.T) {
	// Setup mock VM
	vm := &rs.VM{
		ID: "test-vm-1",
		Name: "Test VM",
		Hier: rs.Hierarchy{
			DC:    rs.Datacenter{Name: "TestDC"},
			Cluster: rs.Cluster{Name: "TestCluster"},
			Host:   rs.Host{Name: "TestHost"},
		},
	}

	charts := newVMCHarts(vm)

	assert.NotNil(t, charts)
	assert.Greater(t, len(*charts), 0, "Should generate at least one chart")

	for _, chart := range *charts {
		// Verify chart ID is properly formatted
		assert.Contains(t, chart.ID, "test-vm-1", "Chart ID should contain VM ID")

		// Verify labels
		labelKeys := make(map[string]bool)
		for _, label := range chart.Labels {
			labelKeys[label.Key] = true
		}
		assert.Contains(t, labelKeys, "datacenter", "Chart should have datacenter label")
		assert.Contains(t, labelKeys, "cluster", "Chart should have cluster label")
		assert.Contains(t, labelKeys, "host", "Chart should have host label")
		assert.Contains(t, labelKeys, "vm", "Chart should have vm label")

		// Verify dimensions
		for _, dim := range chart.Dims {
			assert.Contains(t, dim.ID, "test-vm-1", "Dimension ID should contain VM ID")
		}
	}
}

func TestNewHostCharts(t *testing.T) {
	// Setup mock Host
	host := &rs.Host{
		ID: "test-host-1",
		Name: "Test Host",
		Hier: rs.Hierarchy{
			DC:    rs.Datacenter{Name: "TestDC"},
			Cluster: rs.Cluster{Name: "TestCluster"},
		},
	}

	charts := newHostCharts(host)

	assert.NotNil(t, charts)
	assert.Greater(t, len(*charts), 0, "Should generate at least one chart")

	for _, chart := range *charts {
		// Verify chart ID is properly formatted
		assert.Contains(t, chart.ID, "test-host-1", "Chart ID should contain Host ID")

		// Verify labels
		labelKeys := make(map[string]bool)
		for _, label := range chart.Labels {
			labelKeys[label.Key] = true
		}
		assert.Contains(t, labelKeys, "datacenter", "Chart should have datacenter label")
		assert.Contains(t, labelKeys, "cluster", "Chart should have cluster label")
		assert.Contains(t, labelKeys, "host", "Chart should have host label")

		// Verify dimensions
		for _, dim := range chart.Dims {
			assert.Contains(t, dim.ID, "test-host-1", "Dimension ID should contain Host ID")
		}
	}
}

func TestRemoveFromCharts(t *testing.T) {
	// Create a mock Collector with some charts
	collector := &Collector{
		charted: make(map[string]bool),
	}
	
	// Create a mock Charts
	mockCharts := &module.Charts{
		&module.Chart{ID: "test-vm-1_cpu_utilization"},
		&module.Chart{ID: "test-vm-2_cpu_utilization"},
		&module.Chart{ID: "test-host-1_cpu_usage_total"},
	}
	collector.Charts = func() *module.Charts { return mockCharts }

	// Test removing charts with prefix
	collector.removeFromCharts("test-vm-1")

	// Verify the specific chart was marked for removal
	for _, chart := range *mockCharts {
		if chart.ID == "test-vm-1_cpu_utilization" {
			assert.True(t, chart.IsMarkedToRemove(), "Chart with matching prefix should be marked for removal")
		} else {
			assert.False(t, chart.IsMarkedToRemove(), "Other charts should not be marked for removal")
		}
	}
}

func TestUpdateCharts(t *testing.T) {
	// Mock tests for updateCharts would require extensive mocking of Collector's internal state
	// These tests would verify the chart update logic, including:
	// 1. Handling of failed updates
	// 2. Removal of charts after too many failed updates
	// 3. Adding new charts for discovered hosts/VMs
}