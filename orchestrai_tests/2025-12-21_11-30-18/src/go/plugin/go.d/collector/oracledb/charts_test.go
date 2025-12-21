```go
package oracledb

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestCleanChartId(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{
			input:    "test chart",
			expected: "test_chart",
		},
		{
			input:    "test.chart",
			expected: "test_chart",
		},
		{
			input:    "complex test.chart name",
			expected: "complex_test_chart_name",
		},
		{
			input:    "",
			expected: "",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := cleanChartId(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestAddTablespaceCharts(t *testing.T) {
	collector := &Collector{
		charts: module.Charts{},
	}

	ts := tablespaceInfo{
		name:        "TEST_TABLESPACE",
		autoExtent:  "YES",
	}

	collector.addTablespaceCharts(ts)

	assert.Len(t, *collector.Charts(), 2, "Should add 2 charts")

	// Verify first chart (utilization)
	utilChart := (*collector.Charts())[0]
	assert.Equal(t, "tablespace_test_tablespace_utilization", utilChart.ID)
	assert.Equal(t, "Tablespace Utilization", utilChart.Title)
	assert.Contains(t, utilChart.Labels, module.Label{Key: "tablespace", Value: "TEST_TABLESPACE"})
	assert.Contains(t, utilChart.Labels, module.Label{Key: "autoextend_status", Value: "YES"})

	// Verify second chart (usage)
	usageChart := (*collector.Charts())[1]
	assert.Equal(t, "tablespace_test_tablespace_usage", usageChart.ID)
	assert.Equal(t, "Tablespace Usage", usageChart.Title)
	assert.Contains(t, usageChart.Labels, module.Label{Key: "tablespace", Value: "TEST_TABLESPACE"})
	assert.Contains(t, usageChart.Labels, module.Label{Key: "autoextend_status", Value: "YES"})
}

func TestAddWaitClassCharts(t *testing.T) {
	collector := &Collector{
		charts: module.Charts{},
	}

	waitClass := "TEST_WAIT_CLASS"

	collector.addWaitClassCharts(waitClass)

	assert.Len(t, *collector.Charts(), 1, "Should add 1 chart")

	waitClassChart := (*collector.Charts())[0]
	assert.Equal(t, "wait_class_test_wait_class_wait_time", waitClassChart.ID)
	assert.Equal(t, "Wait Class Wait Time", waitClassChart.Title)
	assert.Contains(t, waitClassChart.Labels, module.Label{Key: "wait_class", Value: "TEST_WAIT_CLASS"})
}

func TestRemoveTablespaceChart(t *testing.T) {
	// This is currently an empty function, so just ensure it doesn't panic
	collector := &Collector{}
	collector.removeTablespaceChart("TEST_TABLESPACE")
}

func TestGlobalChartsInitialization(t *testing.T) {
	assert.NotEmpty(t, globalCharts, "Global charts should be initialized")
	assert.Len(t, globalCharts, 15, "Expected 15 global charts")
}
```