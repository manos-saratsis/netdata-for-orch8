```go
package clickhouse

import (
	"context"
	"errors"
	"net/http"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestNew(t *testing.T) {
	c := New()
	
	assert.NotNil(t, c)
	assert.NotNil(t, c.charts)
	assert.NotNil(t, c.seenDisks)
	assert.NotNil(t, c.seenDbTables)
	assert.Equal(t, "http://127.0.0.1:8123", c.URL)
	assert.Equal(t, time.Second, time.Duration(c.Timeout))
}

func TestCollector_Configuration(t *testing.T) {
	c := New()
	config := c.Configuration()
	
	assert.NotNil(t, config)
	assert.Equal(t, c.Config, config)
}

func TestCollector_Init(t *testing.T) {
	testCases := []struct {
		name          string
		configFunc    func(*Collector)
		expectedError bool
	}{
		{
			name: "Successful initialization",
			configFunc: func(c *Collector) {
				c.URL = "http://localhost:8123"
			},
			expectedError: false,
		},
		{
			name: "Invalid URL",
			configFunc: func(c *Collector) {
				c.URL = ""
			},
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			tc.configFunc(c)

			err := c.Init(context.Background())

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, c.httpClient)
			}
		})
	}
}

func TestCollector_Check(t *testing.T) {
	testCases := []struct {
		name          string
		collectResult map[string]int64
		collectErr    error
		expectedError bool
	}{
		{
			name:          "Successful collection",
			collectResult: map[string]int64{"metric1": 100},
			collectErr:    nil,
			expectedError: false,
		},
		{
			name:          "No metrics collected",
			collectResult: map[string]int64{},
			collectErr:    nil,
			expectedError: true,
		},
		{
			name:          "Collection error",
			collectResult: nil,
			collectErr:    errors.New("collection error"),
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			
			// Mock collect method
			originalCollect := c.collect
			defer func() { c.collect = originalCollect }()
			c.collect = func() (map[string]int64, error) {
				return tc.collectResult, tc.collectErr
			}

			err := c.Check(context.Background())

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Charts(t *testing.T) {
	c := New()
	charts := c.Charts()
	
	assert.Equal(t, c.charts, charts)
}

func TestCollector_Collect(t *testing.T) {
	testCases := []struct {
		name          string
		collectResult map[string]int64
		collectErr    error
		expectedLen   int
	}{
		{
			name:          "Successful collection",
			collectResult: map[string]int64{"metric1": 100, "metric2": 200},
			collectErr:    nil,
			expectedLen:   2,
		},
		{
			name:          "Collection error",
			collectResult: nil,
			collectErr:    errors.New("collection error"),
			expectedLen:   0,
		},
		{
			name:          "No metrics",
			collectResult: map[string]int64{},
			collectErr:    nil,
			expectedLen:   0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			
			// Mock collect method
			originalCollect := c.collect
			defer func() { c.collect = originalCollect }()
			c.collect = func() (map[string]int64, error) {
				return tc.collectResult, tc.collectErr
			}

			result := c.Collect(context.Background())

			assert.Len(t, result, tc.expectedLen)
		})
	}
}

func TestCollector_Cleanup(t *testing.T) {
	c := New()
	
	// Mock HTTP client
	mockClient := &http.Client{}
	c.httpClient = mockClient

	c.Cleanup(context.Background())
	// Verify no panic occurs
}
```