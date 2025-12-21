```go
package sql

import (
	"context"
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestToInt64(t *testing.T) {
	testCases := []struct {
		input    string
		expected int64
	}{
		{"", 0},
		{"123", 123},
		{"-456", -456},
		{"3.14", 3},
		{"true", 1},
		{"True", 1},
		{"yes", 1},
		{"Y", 1},
		{"on", 1},
		{"false", 0},
		{"False", 0},
		{"no", 0},
		{"N", 0},
		{"off", 0},
		{"invalid", 0},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			assert.Equal(t, tc.expected, toInt64(tc.input))
		})
	}
}

func TestNormalizeID(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{"Hello World", "hello_world"},
		{"Test.ID", "test_id"},
		{"UPPERCASE", "uppercase"},
		{"multiple spaces   test", "multiple_spaces_test"},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			assert.Equal(t, tc.expected, normalizeID(tc.input))
		})
	}
}

func TestRedactDSN(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{"", ""},
		{"mysql://user:password@localhost:3306/db", "mysql://user:****@localhost:3306/db"},
		{"postgres://myuser:mypass@localhost/mydb", "postgres://myuser:****@localhost/mydb"},
		{"user:password@tcp(localhost:3306)/db", "****@tcp(localhost:3306)/db"},
		{"user@tcp(localhost:3306)/db", "user@tcp(localhost:3306)/db"},
		{"localhost:3306", "localhost:3306"},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			assert.Equal(t, tc.expected, redactDSN(tc.input))
		})
	}
}

func TestEvalStatusWhen(t *testing.T) {
	testCases := []struct {
		name   string
		status *ConfigStatusWhen
		value  string
		expect bool
	}{
		{
			name: "Equals Match",
			status: &ConfigStatusWhen{
				Equals: "active",
			},
			value:  "active",
			expect: true,
		},
		{
			name: "Equals No Match",
			status: &ConfigStatusWhen{
				Equals: "active",
			},
			value:  "inactive",
			expect: false,
		},
		{
			name: "In List Match",
			status: &ConfigStatusWhen{
				In: []string{"green", "blue"},
			},
			value:  "green",
			expect: true,
		},
		{
			name: "In List No Match",
			status: &ConfigStatusWhen{
				In: []string{"green", "blue"},
			},
			value:  "red",
			expect: false,
		},
		{
			name: "Regex Match",
			status: &ConfigStatusWhen{
				re: func() *regexp.Regexp {
					r, _ := regexp.Compile("^test")
					return r
				}(),
			},
			value:  "test_value",
			expect: true,
		},
		{
			name: "Regex No Match",
			status: &ConfigStatusWhen{
				re: func() *regexp.Regexp {
					r, _ := regexp.Compile("^test")
					return r
				}(),
			},
			value:  "example_test",
			expect: false,
		},
		{
			name: "No Conditions",
			status: &ConfigStatusWhen{},
			value:  "any_value",
			expect: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{}
			result := c.evalStatusWhen(tc.status, tc.value)
			assert.Equal(t, tc.expect, result)
		})
	}
}

func TestBuildMetricChartID(t *testing.T) {
	collector := &Collector{Driver: "mysql"}

	testCases := []struct {
		name   string
		metric ConfigMetricBlock
		chart  ConfigChartConfig
		row    map[string]string
		expect string
	}{
		{
			name: "Simple Chart ID",
			metric: ConfigMetricBlock{
				ID: "test_metric",
				LabelsFromRow: []ConfigLabelFromRow{},
			},
			chart: ConfigChartConfig{
				Context: "test_context",
			},
			row:    map[string]string{},
			expect: "mysql_test_metric_test_context",
		},
		{
			name: "Chart ID with Label from Row",
			metric: ConfigMetricBlock{
				ID: "test_metric",
				LabelsFromRow: []ConfigLabelFromRow{
					{Source: "hostname", Name: "server"},
				},
			},
			chart: ConfigChartConfig{
				Context: "test_context",
			},
			row: map[string]string{
				"hostname": "web01",
			},
			expect: "mysql_test_metric_test_context_web01",
		},
		{
			name: "Missing Label Source",
			metric: ConfigMetricBlock{
				ID: "test_metric",
				LabelsFromRow: []ConfigLabelFromRow{
					{Source: "hostname", Name: "server"},
				},
			},
			chart: ConfigChartConfig{
				Context: "test_context",
			},
			row:    map[string]string{},
			expect: "",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := collector.buildMetricChartID(tc.metric, tc.chart, tc.row)
			assert.Equal(t, tc.expect, result)
		})
	}
}
```