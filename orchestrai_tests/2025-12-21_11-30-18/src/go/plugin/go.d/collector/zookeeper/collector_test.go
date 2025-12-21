```go
package zookeeper

import (
	"context"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/pkg/confopt"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type MockFetcher struct {
	mock.Mock
}

func (m *MockFetcher) collect() (map[string]int64, error) {
	args := m.Called()
	return args.Get(0).(map[string]int64), args.Error(1)
}

func TestCollector_Configuration(t *testing.T) {
	collector := New()
	config := collector.Configuration()
	assert.NotNil(t, config)
	assert.IsType(t, &Config{}, config)
}

func TestCollector_Init(t *testing.T) {
	testCases := []struct {
		name    string
		config  Config
		wantErr bool
	}{
		{
			name: "Valid Configuration",
			config: Config{
				Address: "localhost:2181",
				Timeout: confopt.Duration(time.Second),
				UseTLS:  false,
			},
			wantErr: false,
		},
		{
			name: "Invalid Address",
			config: Config{
				Address: "",
				Timeout: confopt.Duration(time.Second),
			},
			wantErr: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{Config: tc.config}
			err := collector.Init(context.Background())

			if tc.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Check(t *testing.T) {
	testCases := []struct {
		name           string
		mockMetrics    map[string]int64
		mockErr        error
		expectedResult bool
	}{
		{
			name:           "Successful Check",
			mockMetrics:    map[string]int64{"metric1": 100},
			mockErr:        nil,
			expectedResult: true,
		},
		{
			name:           "No Metrics Collected",
			mockMetrics:    map[string]int64{},
			mockErr:        nil,
			expectedResult: false,
		},
		{
			name:           "Collection Error",
			mockMetrics:    nil,
			mockErr:        assert.AnError,
			expectedResult: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := New()
			mockFetcher := &MockFetcher{}
			mockFetcher.On("collect").Return(tc.mockMetrics, tc.mockErr)
			collector.fetcher = mockFetcher

			err := collector.Check(context.Background())

			if tc.expectedResult {
				assert.NoError(t, err)
			} else {
				assert.Error(t, err)
			}
		})
	}
}

func TestCollector_Collect(t *testing.T) {
	testCases := []struct {
		name           string
		mockMetrics    map[string]int64
		mockErr        error
		expectedResult map[string]int64
	}{
		{
			name:           "Successful Collection",
			mockMetrics:    map[string]int64{"metric1": 100, "metric2": 200},
			mockErr:        nil,
			expectedResult: map[string]int64{"metric1": 100, "metric2": 200},
		},
		{
			name:           "No Metrics Collected",
			mockMetrics:    map[string]int64{},
			mockErr:        nil,
			expectedResult: nil,
		},
		{
			name:           "Collection Error",
			mockMetrics:    nil,
			mockErr:        assert.AnError,
			expectedResult: nil,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := New()
			mockFetcher := &MockFetcher{}
			mockFetcher.On("collect").Return(tc.mockMetrics, tc.mockErr)
			collector.fetcher = mockFetcher

			result := collector.Collect(context.Background())
			assert.Equal(t, tc.expectedResult, result)
		})
	}
}

func TestCollector_Charts(t *testing.T) {
	collector := New()
	chartsCopy := collector.Charts()
	assert.NotNil(t, chartsCopy)
}

func TestCollector_Cleanup(t *testing.T) {
	collector := New()
	// Currently no-op function, but testing for potential future implementations
	assert.NotPanics(t, func() {
		collector.Cleanup(context.Background())
	})
}

func TestNew(t *testing.T) {
	collector := New()
	assert.NotNil(t, collector)
	assert.Equal(t, "127.0.0.1:2181", collector.Address)
	assert.Equal(t, confopt.Duration(time.Second), collector.Timeout)
	assert.False(t, collector.UseTLS)
}
```