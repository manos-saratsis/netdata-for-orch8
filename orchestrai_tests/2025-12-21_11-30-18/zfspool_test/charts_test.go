package zfspool_test

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/collector/zfspool"
	"github.com/stretchr/testify/assert"
)

func TestCleanVdev(t *testing.T) {
	testCases := []struct {
		name     string
		input    string
		expected string
	}{
		{"Basic replacement", "test.vdev", "test_vdev"},
		{"No dots", "testvdev", "testvdev"},
		{"Multiple dots", "test.vdev.1", "test_vdev_1"},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := zfspool.CleanVdev(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestAddRemoveZpoolCharts(t *testing.T) {
	c := &zfspool.Collector{
		Base: module.Base{},
		Charts: &module.Charts{},
	}

	// Test adding charts
	c.AddZpoolCharts("testpool")
	assert.NotEmpty(t, *c.Charts(), "Charts should be added for the pool")

	// Test chart IDs and labels
	for _, chart := range *c.Charts() {
		assert.Contains(t, chart.ID, "testpool")
		assert.Contains(t, chart.Labels[0].Value, "testpool")
	}

	// Test removing charts
	c.RemoveZpoolCharts("testpool")
	for _, chart := range *c.Charts() {
		if chart.ID != "" {
			assert.NotContains(t, chart.ID, "testpool")
		}
	}
}

func TestAddRemoveVdevCharts(t *testing.T) {
	c := &zfspool.Collector{
		Base: module.Base{},
		Charts: &module.Charts{},
	}

	// Test adding vdev charts
	c.AddVdevCharts("testpool", "vdev1")
	assert.NotEmpty(t, *c.Charts(), "Charts should be added for the vdev")

	// Test chart IDs, labels, and dimensions
	for _, chart := range *c.Charts() {
		assert.Contains(t, chart.ID, "vdev1")
		assert.Len(t, chart.Labels, 2)
		assert.Equal(t, "pool", chart.Labels[0].Key)
		assert.Equal(t, "testpool", chart.Labels[0].Value)
		assert.Equal(t, "vdev", chart.Labels[1].Key)
		assert.Equal(t, "vdev1", chart.Labels[1].Value)
	}

	// Test removing vdev charts
	c.RemoveVdevCharts("vdev1")
	for _, chart := range *c.Charts() {
		if chart.ID != "" {
			assert.NotContains(t, chart.ID, "vdev1")
		}
	}
}