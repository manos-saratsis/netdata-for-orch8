```go
package pulsar

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/prometheus"
	"github.com/stretchr/testify/assert"
)

func Test_isValidPulsarMetrics(t *testing.T) {
	tests := []struct {
		name    string
		metrics prometheus.Series
		want    bool
	}{
		{
			name: "Valid Pulsar metrics with topics count",
			metrics: prometheus.Series{
				{
					Name: metricPulsarTopicsCount,
				},
			},
			want: true,
		},
		{
			name:    "No Pulsar metrics",
			metrics: prometheus.Series{},
			want:    false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := isValidPulsarMetrics(tt.metrics)
			assert.Equal(t, tt.want, result)
		})
	}
}

func TestCollector_resetCurCache(t *testing.T) {
	c := New()
	c.curCache.namespaces["test-namespace"] = true
	c.curCache.topics["test-topic"] = true

	c.resetCurCache()

	assert.Empty(t, c.curCache.namespaces)
	assert.Empty(t, c.curCache.topics)
}

func TestCollector_collect(t *testing.T) {
	tests := []struct {
		name         string
		mockMetrics  prometheus.Series
		mockErr      error
		wantErr      bool
		wantEmpty    bool
	}{
		{
			name: "Successful collection with valid Pulsar metrics",
			mockMetrics: prometheus.Series{
				{
					Name: metricPulsarTopicsCount,
					Labels: prometheus.Labels{
						"cluster":   "test",
						"namespace": "default",
					},
					Value: 10,
				},
			},
			wantErr:   false,
			wantEmpty: false,
		},
		{
			name:         "Scrape series error",
			mockMetrics:  nil,
			mockErr:      assert.AnError,
			wantErr:      true,
			wantEmpty:    true,
		},
		{
			name:         "Invalid Pulsar metrics",
			mockMetrics:  prometheus.Series{},
			wantErr:      true,
			wantEmpty:    true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			mockClient := &mockPrometheusClient{}
			mockClient.On("ScrapeSeries").Return(tt.mockMetrics, tt.mockErr)
			c.prom = mockClient

			mx, err := c.collect()
			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			if tt.wantEmpty {
				assert.Empty(t, mx)
			} else {
				assert.NotEmpty(t, mx)
			}

			mockClient.AssertExpectations(t)
		})
	}
}

func TestCollector_collectMetrics(t *testing.T) {
	tests := []struct {
		name     string
		metrics  prometheus.Series
		wantKeys []string
	}{
		{
			name: "Collect broker metrics",
			metrics: prometheus.Series{
				{
					Name: metricPulsarTopicsCount,
					Labels: prometheus.Labels{
						"cluster":   "test",
						"namespace": "default",
					},
					Value: 10,
				},
			},
			wantKeys: []string{
				metricPulsarTopicsCount,
				metricPulsarTopicsCount + "_default",
				"pulsar_namespaces_count",
			},
		},
		{
			name:     "Empty metrics",
			metrics:  prometheus.Series{},
			wantKeys: []string{"pulsar_namespaces_count"},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			mx := c.collectMetrics(tt.metrics)

			for _, key := range tt.wantKeys {
				assert.Contains(t, mx, key)
			}
		})
	}
}

func Test_findPulsarMetrics(t *testing.T) {
	tests := []struct {
		name     string
		input    prometheus.Series
		wantLen  int
		wantName string
	}{
		{
			name: "Contains valid Pulsar metrics",
			input: prometheus.Series{
				{
					Name: "pulsar_storage_write_latency_le_0_5",
				},
				{
					Name: metricPulsarTopicsCount,
				},
			},
			wantLen:  2,
			wantName: metricPulsarTopicsCount,
		},
		{
			name:     "Empty input",
			input:    prometheus.Series{},
			wantLen:  0,
			wantName: "",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := findPulsarMetrics(tt.input)
			assert.Len(t, result, tt.wantLen)
			if tt.wantName != "" {
				assert.Contains(t, result, tt.input.FindByName(tt.wantName)[0])
			}
		})
	}
}

func Test_precision(t *testing.T) {
	tests := []struct {
		name     string
		metric   string
		expected float64
	}{
		{
			name:     "Metrics with 1000 precision",
			metric:   metricPulsarRateIn,
			expected: 1000,
		},
		{
			name:     "Metrics with default precision",
			metric:   metricPulsarTopicsCount,
			expected: 1,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := precision(tt.metric)
			assert.Equal(t, tt.expected, result)
		})
	}
}
```