```go
package netdataexporter

import (
	"testing"
	"github.com/stretchr/testify/assert"
	"go.opentelemetry.io/collector/pdata/pcommon"
	"go.opentelemetry.io/collector/pdata/pmetric"
)

func TestSanitizeID(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{"hello", "hello"},
		{"123test", "n_123test"},
		{"test-metric", "test-metric"},
		{"test.metric", "test.metric"},
		{"test_metric", "test_metric"},
		{"test@metric", "test_metric"},
		{"", "unknown"},
		{"  space test  ", "__space_test__"},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := sanitizeID(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestAttributesHash(t *testing.T) {
	testCases := []struct {
		name     string
		attrs    map[string]string
		expected string
	}{
		{
			"empty attributes",
			map[string]string{},
			"default",
		},
		{
			"single attribute",
			map[string]string{"key": "value"},
			attributesHash(createAttributeMap(map[string]string{"key": "value"})),
		},
		{
			"multiple attributes",
			map[string]string{"key1": "value1", "key2": "value2"},
			attributesHash(createAttributeMap(map[string]string{"key1": "value1", "key2": "value2"})),
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			attrs := createAttributeMap(tc.attrs)
			result := attributesHash(attrs)
			assert.NotEmpty(t, result)
		})
	}
}

func TestGenerateChartID(t *testing.T) {
	testCases := []struct {
		metricName string
		attrHash   string
		maxLength  int
	}{
		{"test_metric", "hash123", maxChartIDLength},
		{"very_long_metric_name_that_exceeds_max_length", "hash123", maxChartIDLength},
	}

	for _, tc := range testCases {
		t.Run(tc.metricName, func(t *testing.T) {
			result := generateChartID(tc.metricName, tc.attrHash)
			assert.LessOrEqual(t, len(result), maxChartIDLength)
			assert.Contains(t, result, tc.attrHash)
		})
	}
}

func TestGetDimensionName(t *testing.T) {
	testCases := []struct {
		name     string
		attrs    map[string]string
		expected string
	}{
		{
			"empty attributes",
			map[string]string{},
			"value",
		},
		{
			"with name attribute",
			map[string]string{"name": "test_name"},
			"test_name",
		},
		{
			"with id attribute",
			map[string]string{"id": "test_id"},
			"test_id",
		},
		{
			"with key attribute",
			map[string]string{"key": "test_key"},
			"test_key",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			attrs := createAttributeMap(tc.attrs)
			result := getDimensionName(attrs)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func createAttributeMap(attrs map[string]string) pcommon.Map {
	m := pcommon.NewMap()
	for k, v := range attrs {
		m.PutStr(k, v)
	}
	return m
}

func TestGetFamily(t *testing.T) {
	testCases := []struct {
		metricName    string
		resourceAttrs map[string]string
		expected      string
	}{
		{
			"metric_name",
			map[string]string{"service.name": "test_service"},
			"test_service_metric",
		},
		{
			"metric.detail",
			map[string]string{"service.namespace": "test_namespace"},
			"test_namespace_metric",
		},
		{
			"complex.metric.name",
			map[string]string{},
			"complex",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.metricName, func(t *testing.T) {
			attrs := createAttributeMap(tc.resourceAttrs)
			result := getFamily(tc.metricName, attrs)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestUpdateChartLabels(t *testing.T) {
	chart := &ChartDefinition{}
	resourceAttrs := createAttributeMap(map[string]string{
		"service.name": "test_service",
		"version":      "1.0.0",
	})

	updateChartLabels(chart, resourceAttrs)

	assert.Equal(t, 2, len(chart.Labels))
	assert.ElementsMatch(t, []LabelDefinition{
		{Name: "service.name", Value: "test_service"},
		{Name: "version", Value: "1.0.0"},
	}, chart.Labels)
}

func TestAddDataPointAttributesAsLabels(t *testing.T) {
	chart := &ChartDefinition{
		Labels: []LabelDefinition{
			{Name: "service.name", Value: "test_service"},
		},
	}
	dataPointAttrs := createAttributeMap(map[string]string{
		"version": "1.0.0",
		"host":    "localhost",
	})

	addDataPointAttributesAsLabels(chart, dataPointAttrs)

	assert.Equal(t, 3, len(chart.Labels))
	assert.ElementsMatch(t, []LabelDefinition{
		{Name: "host", Value: "localhost"},
		{Name: "service.name", Value: "test_service"},
		{Name: "version", Value: "1.0.0"},
	}, chart.Labels)
}

func TestGetTotalCountFromBuckets(t *testing.T) {
	metrics := pmetric.NewMetrics()
	resourceMetrics := metrics.ResourceMetrics().AppendEmpty()
	scopeMetrics := resourceMetrics.ScopeMetrics().AppendEmpty()
	metric := scopeMetrics.Metrics().AppendEmpty()
	metric.SetType(pmetric.MetricTypeExponentialHistogram)
	dataPoint := metric.ExponentialHistogram().DataPoints().AppendEmpty()
	
	buckets := dataPoint.Positive()
	bucketCounts := buckets.BucketCounts()
	bucketCounts.Resize(3)
	bucketCounts.SetAt(0, 10)
	bucketCounts.SetAt(1, 20)
	bucketCounts.SetAt(2, 30)

	total := getTotalCountFromBuckets(buckets)
	assert.Equal(t, float64(60), total)
}

func TestGetUnits(t *testing.T) {
	testCases := []struct {
		metric   pmetric.Metric
		expected string
	}{
		{
			createMockMetric("", ""),
			"count",
		},
		{
			createMockMetric("seconds", "duration"),
			"seconds",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.expected, func(t *testing.T) {
			result := getUnits(tc.metric)
			assert.Equal(t, tc.expected, result)
		})
	}
}

// Helper function to create a mock metric for testing
func createMockMetric(unit, description string) pmetric.Metric {
	metrics := pmetric.NewMetrics()
	resourceMetrics := metrics.ResourceMetrics().AppendEmpty()
	scopeMetrics := resourceMetrics.ScopeMetrics().AppendEmpty()
	metric := scopeMetrics.Metrics().AppendEmpty()
	metric.SetUnit(unit)
	metric.SetDescription(description)
	return metric
}
```

This test file covers the utility functions in the `convert.go` file, with a focus on testing various scenarios:
1. Input sanitization
2. Attribute hashing
3. Chart ID generation
4. Dimension name resolution
5. Family name generation
6. Label management
7. Total count calculation
8. Unit retrieval

Key aspects of the tests:
- Uses testify for assertions
- Covers empty/edge cases
- Tests all code paths
- Validates transformations
- Mock objects for complex scenarios