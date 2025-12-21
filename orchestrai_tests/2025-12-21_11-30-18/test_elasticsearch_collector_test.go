```go
package elasticsearch

import (
	"context"
	"errors"
	"net/http"
	"sync"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestNewCollector(t *testing.T) {
	c := New()

	assert.NotNil(t, c)
	assert.Equal(t, "http://127.0.0.1:9200", c.URL)
	assert.Equal(t, time.Second*2, time.Duration(c.Timeout))
	assert.True(t, c.DoNodeStats)
	assert.True(t, c.DoClusterStats)
	assert.True(t, c.DoClusterHealth)
	assert.False(t, c.ClusterMode)
	assert.False(t, c.DoIndicesStats)

	assert.NotNil(t, c.charts)
	assert.NotNil(t, c.addClusterHealthChartsOnce)
	assert.NotNil(t, c.addClusterStatsChartsOnce)
	assert.NotNil(t, c.nodes)
	assert.NotNil(t, c.indices)
}

func TestCollector_Configuration(t *testing.T) {
	c := New()
	config := c.Configuration()

	assert.Equal(t, c.Config, config)
}

func TestCollector_Init_InvalidConfig(t *testing.T) {
	testCases := []struct {
		name   string
		config Config
		error  string
	}{
		{
			name: "Empty URL",
			config: Config{
				HTTPConfig: web.HTTPConfig{
					RequestConfig: web.RequestConfig{
						URL: "",
					},
				},
			},
			error: "check configuration: URL not set",
		},
		{
			name: "All API calls disabled",
			config: Config{
				HTTPConfig: web.HTTPConfig{
					RequestConfig: web.RequestConfig{
						URL: "http://test.com",
					},
				},
				DoNodeStats:     false,
				DoClusterHealth: false,
				DoClusterStats:  false,
				DoIndicesStats:  false,
			},
			error: "check configuration: all API calls are disabled",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{Config: tc.config}
			err := c.Init(context.Background())

			assert.EqualError(t, err, tc.error)
		})
	}
}

func TestCollector_Init_Success(t *testing.T) {
	c := New()
	c.URL = "http://test.com"
	c.Timeout = 5 * time.Second

	err := c.Init(context.Background())

	assert.NoError(t, err)
	assert.NotNil(t, c.httpClient)
}

func TestCollector_Check(t *testing.T) {
	testCases := []struct {
		name         string
		prepareMocks func(*Collector)
		wantErr      bool
		errContains  string
	}{
		{
			name: "Collect error",
			prepareMocks: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return nil, errors.New("collect error")
				}
			},
			wantErr:     true,
			errContains: "collect error",
		},
		{
			name: "No metrics collected",
			prepareMocks: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{}, nil
				}
			},
			wantErr:     true,
			errContains: "no metrics collected",
		},
		{
			name: "Metrics collected successfully",
			prepareMocks: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{"test": 123}, nil
				}
			},
			wantErr: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			tc.prepareMocks(c)

			err := c.Check(context.Background())

			if tc.wantErr {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errContains)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Collect(t *testing.T) {
	testCases := []struct {
		name         string
		prepareMocks func(*Collector)
		expected     map[string]int64
	}{
		{
			name: "Successful collection",
			prepareMocks: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{"test": 123}, nil
				}
			},
			expected: map[string]int64{"test": 123},
		},
		{
			name: "Collect error",
			prepareMocks: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return nil, errors.New("collect error")
				}
			},
			expected: nil,
		},
		{
			name: "No metrics",
			prepareMocks: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{}, nil
				}
			},
			expected: nil,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			tc.prepareMocks(c)

			result := c.Collect(context.Background())

			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestCollector_Cleanup(t *testing.T) {
	c := New()
	mockClient := &http.Client{}
	c.httpClient = mockClient

	c.Cleanup(context.Background())

	// Verify CloseIdleConnections was called (no easy way to assert this directly)
	assert.NotPanics(t, func() {
		c.Cleanup(context.Background())
	})
}

func TestCollector_Charts(t *testing.T) {
	c := New()
	charts := c.Charts()

	assert.Equal(t, c.charts, charts)
	assert.NotNil(t, charts)
}
```