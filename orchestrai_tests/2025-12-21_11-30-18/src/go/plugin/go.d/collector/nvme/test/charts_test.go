package nvme

import (
	"testing"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestAddDeviceCharts(t *testing.T) {
	testCases := []struct {
		name          string
		devicePath    string
		model         string
		expectedError bool
	}{
		{
			name:       "Normal Device Path",
			devicePath: "/dev/nvme0",
			model:      "TestModel",
			expectedError: false,
		},
		{
			name:       "Empty Device Path",
			devicePath: "",
			model:      "TestModel",
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				charts: &module.Charts{},
			}

			collector.addDeviceCharts(tc.devicePath, tc.model)

			charts := collector.Charts()
			
			if !tc.expectedError {
				device := extractDeviceFromPath(tc.devicePath)
				assert.NotEmpty(t, *charts, "Charts should not be empty")
				
				for _, chart := range *charts {
					assert.Contains(t, chart.ID, device, "Chart ID should contain device")
					
					// Check labels
					assert.Len(t, chart.Labels, 2, "Should have two labels")
					assert.Equal(t, chart.Labels[0].Key, "device")
					assert.Equal(t, chart.Labels[0].Value, device)
					assert.Equal(t, chart.Labels[1].Key, "model_number")
					assert.Equal(t, chart.Labels[1].Value, tc.model)
					
					// Check dimensions
					for _, dim := range chart.Dims {
						assert.Contains(t, dim.ID, device, "Dimension ID should contain device")
					}
				}
			}
		})
	}
}

func TestRemoveDeviceCharts(t *testing.T) {
	testCases := []struct {
		name       string
		devicePath string
	}{
		{
			name:       "Remove Existing Device Charts",
			devicePath: "/dev/nvme0",
		},
		{
			name:       "Remove Non-Existing Device Charts",
			devicePath: "",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				charts: &module.Charts{
					&module.Chart{
						ID: "device_nvme0_estimated_endurance_perc",
					},
					&module.Chart{
						ID: "device_nvme1_available_spare_perc",
					},
				},
			}

			collector.removeDeviceCharts(tc.devicePath)

			device := extractDeviceFromPath(tc.devicePath)
			prefix := "device_" + device

			for _, chart := range *collector.Charts() {
				assert.False(t, chart.IsRemoved(), "Chart should remain if not matching prefix")
			}
		})
	}
}

func TestDeviceChartTemplates(t *testing.T) {
	assert.NotNil(t, deviceChartsTmpl, "Device charts template should not be nil")
	assert.Greater(t, len(deviceChartsTmpl), 0, "Device charts template should have charts")

	templateChecks := []struct {
		chartTmpl     module.Chart
		expectedID    string
		expectedTitle string
	}{
		{
			chartTmpl:     deviceEstimatedEndurancePercChartTmpl,
			expectedID:    "device_%s_estimated_endurance_perc",
			expectedTitle: "Estimated endurance",
		},
		{
			chartTmpl:     deviceAvailableSparePercChartTmpl,
			expectedID:    "device_%s_available_spare_perc",
			expectedTitle: "Remaining spare capacity",
		},
	}

	for _, check := range templateChecks {
		t.Run(check.expectedTitle, func(t *testing.T) {
			assert.Equal(t, check.expectedID, check.chartTmpl.ID, "Chart ID should match")
			assert.Equal(t, check.expectedTitle, check.chartTmpl.Title, "Chart Title should match")
			assert.NotEmpty(t, check.chartTmpl.Dims, "Chart should have dimensions")
		})
	}
}