```go
package dmcache

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestCleanDeviceName(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{"device.name", "device_name"},
		{"simple-device", "simple-device"},
		{"", ""},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := cleanDeviceName(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestAddDeviceCharts(t *testing.T) {
	collector := &Collector{
		charts: &module.Charts{},
		Base:   module.Base{},
	}

	testDevices := []string{"test1", "test.device", "another_device"}

	for _, device := range testDevices {
		t.Run("Add charts for "+device, func(t *testing.T) {
			collector.addDeviceCharts(device)

			// Check that charts were added
			charts := *collector.Charts()
			assert.Greater(t, len(charts), 0)

			// Check chart IDs include the cleaned device name
			cleanDevName := cleanDeviceName(device)
			for _, chart := range charts {
				assert.Contains(t, chart.ID, cleanDevName)
				
				// Check labels
				hasDeviceLabel := false
				for _, label := range chart.Labels {
					if label.Key == "device" && label.Value == device {
						hasDeviceLabel = true
						break
					}
				}
				assert.True(t, hasDeviceLabel, "chart should have device label")
			}
		})
	}
}

func TestRemoveDeviceCharts(t *testing.T) {
	collector := &Collector{
		charts:  &module.Charts{},
		Base:    module.Base{},
		devices: make(map[string]bool),
	}

	// Add some charts first
	testDevice := "test.device"
	collector.addDeviceCharts(testDevice)
	initialChartCount := len(*collector.Charts())
	assert.Greater(t, initialChartCount, 0)

	// Remove device charts
	collector.removeDeviceCharts(testDevice)

	// Verify charts are marked for removal
	charts := *collector.Charts()
	for _, chart := range charts {
		if chart.ID == fmt.Sprintf("dmcache_device_%s_", cleanDeviceName(testDevice)) {
			assert.True(t, chart.ToRemove)
		}
	}
}
```