```go
package upsd

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestCleanUpsName(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{"Test UPS", "Test_UPS"},
		{"server.ups", "server_ups"},
		{"normal_name", "normal_name"},
		{" space name ", "_space_name_"},
		{"multiple . dots", "multiple__dots"},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := cleanUpsName(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestCollector_AddUPSCharts(t *testing.T) {
	testCases := []struct {
		name                string
		upsUnit             upsUnit
		expectedChartCount  int
		expectedRemovedVars []string
	}{
		{
			name: "Full UPS Unit",
			upsUnit: upsUnit{
				name: "TestUPS",
				vars: map[string]string{
					varUpsLoad:               "1",
					varBatteryVoltage:        "1",
					varBatteryVoltageNominal: "1",
					varUpsTemperature:        "1",
					varInputVoltage:          "1",
					varInputVoltageNominal:   "1",
					varInputCurrent:          "1",
					varInputCurrentNominal:   "1",
					varInputFrequency:        "1",
					varOutputVoltage:         "1",
					varOutputVoltageNominal:  "1",
					varOutputCurrent:         "1",
					varOutputCurrentNominal:  "1",
					varOutputFrequency:       "1",
					varBatteryType:           "test",
					varDeviceModel:           "model",
					varDeviceSerial:          "serial",
					varDeviceMfr:             "manufacturer",
					varDeviceType:            "type",
				},
			},
			expectedChartCount: 20, // Full set of charts
			expectedRemovedVars: []string{},
		},
		{
			name: "Partial UPS Unit",
			upsUnit: upsUnit{
				name: "LimitedUPS",
				vars: map[string]string{
					varBatteryVoltage: "1",
					varDeviceModel:    "model",
				},
			},
			expectedChartCount: 2, // Only battery voltage charts
			expectedRemovedVars: []string{
				varUpsLoad, varUpsTemperature,
				varInputVoltage, varInputVoltageNominal,
				varInputCurrent, varInputCurrentNominal,
				varInputFrequency, varInputFrequencyNominal,
				varOutputVoltage, varOutputVoltageNominal,
				varOutputCurrent, varOutputCurrentNominal,
				varOutputFrequency, varOutputFrequencyNominal,
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				charts: *module.NewCharts(),
				Debug:  false,
			}

			collector.addUPSCharts(tc.upsUnit)

			charts := *collector.Charts()
			assert.Equal(t, tc.expectedChartCount, len(charts), "Chart count should match")

			// Verify chart IDs are correct
			expectedPrefix := cleanUpsName(tc.upsUnit.name)
			for _, chart := range charts {
				assert.True(t, len(chart.ID) > 0, "Chart ID should not be empty")
				assert.Contains(t, chart.ID, expectedPrefix, "Chart ID should contain cleaned UPS name")
			}
		})
	}
}

func TestCollector_RemoveUPSCharts(t *testing.T) {
	collector := &Collector{
		charts: *module.NewCharts(),
	}

	// Add some test charts
	testCharts := []string{
		"test_ups_chart1",
		"test_ups_chart2",
		"another_ups_chart",
		"unrelated_chart",
	}

	for _, chartID := range testCharts {
		chart := &module.Chart{
			ID: chartID,
		}
		collector.Charts().Add(chart)
	}

	// Remove charts for a specific UPS
	collector.removeUPSCharts("Test UPS")

	// Verify charts for the specific UPS are marked for removal
	for _, chart := range *collector.Charts() {
		if chart.ID == "test_ups_chart1" || chart.ID == "test_ups_chart2" {
			assert.True(t, chart.IsMarkedRemove(), "Chart should be marked for removal")
			assert.True(t, chart.IsNotCreated(), "Chart should be marked not created")
		} else {
			assert.False(t, chart.IsMarkedRemove(), "Unrelated chart should not be marked for removal")
		}
	}
}
```

Next, I'll read the client.go file:Now, I'll create the test file for the client: