```go
package pulsar

import (
	"context"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/pkg/matcher"
	"github.com/netdata/netdata/go/plugins/pkg/prometheus"
	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestNew(t *testing.T) {
	collector := New()

	assert.NotNil(t, collector)
	assert.NotNil(t, collector.charts)
	assert.NotNil(t, collector.nsCharts)
	assert.NotNil(t, collector.cache)
	assert.NotNil(t, collector.curCache)
	assert.Equal(t, "http://127.0.0.1:8080/metrics", collector.URL)
	assert.Equal(t, 5*time.Second, time.Duration(collector.Timeout))
}

func TestCollector_Configuration(t *testing.T) {
	collector := New()
	config := collector.Configuration()

	assert.NotNil(t, config)
	assert.Equal(t, collector.Config, config)
}

func TestCollector_Init(t *testing.T) {
	tests := []struct {
		name           string
		prepareCollect func(*Collector)
		wantErr        bool
	}{
		{
			name: "Successful initialization",
			prepareCollect: func(c *Collector) {
				c.URL = "http://localhost:8080/metrics"
			},
			wantErr: false,
		},
		{
			name: "Empty URL validation error",
			prepareCollect: func(c *Collector) {
				c.URL = ""
			},
			wantErr: true,
		},
		{
			name: "Invalid topic filter",
			prepareCollect: func(c *Collector) {
				c.URL = "http://localhost:8080/metrics"
				c.TopicFilter = matcher.SimpleExpr{
					Includes: []string{"[invalid regex"},
				}
			},
			wantErr: true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			collector := New()
			tt.prepareCollect(collector)

			err := collector.Init(context.Background())
			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, collector.prom)
				assert.NotNil(t, collector.topicFilter)
			}
		})
	}
}

func TestCollector_Check(t *testing.T) {
	tests := []struct {
		name    string
		metrics prometheus.Series
		wantErr bool
	}{
		{
			name: "Successful check with metrics",
			metrics: prometheus.Series{
				{
					Name: metricPulsarTopicsCount,
					Labels: prometheus.Labels{
						"cluster": "test",
					},
					Value: 10,
				},
			},
			wantErr: false,
		},
		{
			name:    "No metrics collected",
			metrics: prometheus.Series{},
			wantErr: true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			collector := New()
			mockClient := &mockPrometheusClient{}

			mockClient.On("ScrapeSeries").Return(tt.metrics, nil)
			collector.prom = mockClient

			err := collector.Check(context.Background())
			if tt.wantErr {
				assert.Error(t, err)
				assert.Equal(t, "no metrics collected", err.Error())
			} else {
				assert.NoError(t, err)
			}

			mockClient.AssertExpectations(t)
		})
	}
}

func TestCollector_Charts(t *testing.T) {
	collector := New()
	charts := collector.Charts()

	assert.NotNil(t, charts)
	assert.Equal(t, collector.charts, charts)
}

func TestCollector_Collect(t *testing.T) {
	tests := []struct {
		name      string
		metrics   prometheus.Series
		wantEmpty bool
		wantErr   bool
	}{
		{
			name: "Successful collection with metrics",
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
			wantEmpty: false,
			wantErr:   false,
		},
		{
			name:      "No metrics collected",
			metrics:   prometheus.Series{},
			wantEmpty: true,
			wantErr:   true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			collector := New()
			mockClient := &mockPrometheusClient{}

			mockClient.On("ScrapeSeries").Return(tt.metrics, nil)
			collector.prom = mockClient

			metrics := collector.Collect(context.Background())

			if tt.wantEmpty {
				assert.Empty(t, metrics)
			} else {
				assert.NotEmpty(t, metrics)
			}

			mockClient.AssertExpectations(t)
		})
	}
}

func TestCollector_Cleanup(t *testing.T) {
	collector := New()

	mockClient := &mockPrometheusClient{}
	mockHTTPClient := &web.HTTPClient{}

	mockClient.On("HTTPClient").Return(mockHTTPClient)
	collector.prom = mockClient

	collector.Cleanup(context.Background())

	mockClient.AssertExpectations(t)
	mockClient.AssertCalled(t, "HTTPClient")
}
```