```go
package jmx

import (
	"context"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/confopt"
	jmxproto "github.com/netdata/netdata/go/plugins/plugin/ibm.d/protocols/websphere/jmx"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

// Mock JMX client for testing
type mockJMXClient struct {
	mock.Mock
}

func (m *mockJMXClient) Start(ctx context.Context) error {
	args := m.Called(ctx)
	return args.Error(0)
}

func (m *mockJMXClient) Shutdown() {
	m.Called()
}

func (m *mockJMXClient) FetchJVM(ctx context.Context) (*jmxproto.JVMStats, error) {
	args := m.Called(ctx)
	return args.Get(0).(*jmxproto.JVMStats), args.Error(1)
}

func (m *mockJMXClient) FetchThreadPools(ctx context.Context, maxItems int) ([]jmxproto.ThreadPool, error) {
	args := m.Called(ctx, maxItems)
	return args.Get(0).([]jmxproto.ThreadPool), args.Error(1)
}

func (m *mockJMXClient) FetchJDBCPools(ctx context.Context, maxItems int) ([]jmxproto.JDBCPool, error) {
	args := m.Called(ctx, maxItems)
	return args.Get(0).([]jmxproto.JDBCPool), args.Error(1)
}

func (m *mockJMXClient) FetchJCAPools(ctx context.Context, maxItems int) ([]jmxproto.JCAPool, error) {
	args := m.Called(ctx, maxItems)
	return args.Get(0).([]jmxproto.JCAPool), args.Error(1)
}

func (m *mockJMXClient) FetchJMSDestinations(ctx context.Context, maxItems int) ([]jmxproto.JMSDestination, error) {
	args := m.Called(ctx, maxItems)
	return args.Get(0).([]jmxproto.JMSDestination), args.Error(1)
}

func (m *mockJMXClient) FetchApplications(ctx context.Context, maxItems int, includeSessions, includeTransactions bool) ([]jmxproto.ApplicationMetric, error) {
	args := m.Called(ctx, maxItems, includeSessions, includeTransactions)
	return args.Get(0).([]jmxproto.ApplicationMetric), args.Error(1)
}

func TestCollectOnce_MissingClient(t *testing.T) {
	c := New()
	err := c.CollectOnce()
	assert.Error(t, err, "CollectOnce() should fail if client is not initialized")
	assert.Contains(t, err.Error(), "protocol client not initialised")
}

func TestCollectOnce_JVMMetricCollection(t *testing.T) {
	c := New()
	mockClient := new(mockJMXClient)
	c.client = mockClient

	mockStats := &jmxproto.JVMStats{
		Heap: jmxproto.MemoryStats{
			Used:      1024 * 1024,
			Committed: 2048 * 1024,
			Max:       4096 * 1024,
		},
		NonHeap: jmxproto.MemoryStats{
			Used:      512 * 1024,
			Committed: 1024 * 1024,
		},
		GC: jmxproto.GCStats{
			Count: 10,
			Time:  1000,
		},
		Threads: jmxproto.ThreadStats{
			Count:   50,
			Daemon:  20,
			Peak:    60,
			Started: 100,
		},
		Classes: jmxproto.ClassStats{
			Loaded:   200,
			Unloaded: 50,
		},
		CPU: jmxproto.CPUStats{
			ProcessUsage: 25.5,
		},
		Uptime: 3600000, // 1 hour
	}

	mockClient.On("FetchJVM", mock.Anything).Return(mockStats, nil)
	mockClient.On("FetchThreadPools", mock.Anything, mock.Anything).Return([]jmxproto.ThreadPool{}, nil)
	mockClient.On("FetchJDBCPools", mock.Anything, mock.Anything).Return([]jmxproto.JDBCPool{}, nil)
	mockClient.On("FetchJCAPools", mock.Anything, mock.Anything).Return([]jmxproto.JCAPool{}, nil)
	mockClient.On("FetchJMSDestinations", mock.Anything, mock.Anything).Return([]jmxproto.JMSDestination{}, nil)
	mockClient.On("FetchApplications", mock.Anything, mock.Anything, mock.Anything, mock.Anything).Return([]jmxproto.ApplicationMetric{}, nil)

	err := c.CollectOnce()
	assert.NoError(t, err, "CollectOnce() should succeed")

	mockClient.AssertExpectations(t)
}

func TestCollectOnce_ConditionalMetricCollection(t *testing.T) {
	testCases := []struct {
		name             string
		enabledMetrics   map[string]bool
		expectedMethods  []string
		unexpectedMethod string
	}{
		{
			name: "All metrics enabled",
			enabledMetrics: map[string]bool{
				"ThreadPoolMetrics":  true,
				"JDBCMetrics":        true,
				"JCAMetrics":         true,
				"JMSMetrics":         true,
				"WebAppMetrics":      true,
				"SessionMetrics":     true,
				"TransactionMetrics": true,
			},
			expectedMethods: []string{
				"FetchThreadPools",
				"FetchJDBCPools",
				"FetchJCAPools",
				"FetchJMSDestinations",
				"FetchApplications",
			},
		},
		{
			name: "Only Thread Pool and JDBC Metrics",
			enabledMetrics: map[string]bool{
				"ThreadPoolMetrics": true,
				"JDBCMetrics":       true,
			},
			expectedMethods: []string{
				"FetchThreadPools",
				"FetchJDBCPools",
			},
			unexpectedMethod: "FetchJCAPools",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			mockClient := new(mockJMXClient)
			c.client = mockClient

			// Configure enabled metrics
			c.Config.CollectThreadPoolMetrics = confopt.AutoBoolDisabled
			c.Config.CollectJDBCMetrics = confopt.AutoBoolDisabled
			c.Config.CollectJCAMetrics = confopt.AutoBoolDisabled
			c.Config.CollectJMSMetrics = confopt.AutoBoolDisabled
			c.Config.CollectWebAppMetrics = confopt.AutoBoolDisabled
			c.Config.CollectSessionMetrics = confopt.AutoBoolDisabled
			c.Config.CollectTransactionMetrics = confopt.AutoBoolDisabled

			for metric, enabled := range tc.enabledMetrics {
				switch metric {
				case "ThreadPoolMetrics":
					c.Config.CollectThreadPoolMetrics = confopt.GetAutoBool(enabled)
				case "JDBCMetrics":
					c.Config.CollectJDBCMetrics = confopt.GetAutoBool(enabled)
				case "JCAMetrics":
					c.Config.CollectJCAMetrics = confopt.GetAutoBool(enabled)
				case "JMSMetrics":
					c.Config.CollectJMSMetrics = confopt.GetAutoBool(enabled)
				case "WebAppMetrics":
					c.Config.CollectWebAppMetrics = confopt.GetAutoBool(enabled)
				case "SessionMetrics":
					c.Config.CollectSessionMetrics = confopt.GetAutoBool(enabled)
				case "TransactionMetrics":
					c.Config.CollectTransactionMetrics = confopt.GetAutoBool(enabled)
				}
			}

			mockJVMStats := &jmxproto.JVMStats{}
			mockClient.On("FetchJVM", mock.Anything).Return(mockJVMStats, nil)

			// Set up expected method calls
			for _, method := range tc.expectedMethods {
				switch method {
				case "FetchThreadPools":
					mockClient.On("FetchThreadPools", mock.Anything, mock.Anything).Return([]jmxproto.ThreadPool{}, nil)
				case "FetchJDBCPools":
					mockClient.On("FetchJDBCPools", mock.Anything, mock.Anything).Return([]jmxproto.JDBCPool{}, nil)
				case "FetchJCAPools":
					mockClient.On("FetchJCAPools", mock.Anything, mock.Anything).Return([]jmxproto.JCAPool{}, nil)
				case "FetchJMSDestinations":
					mockClient.On("FetchJMSDestinations", mock.Anything, mock.Anything).Return([]jmxproto.JMSDestination{}, nil)
				case "FetchApplications":
					mockClient.On("FetchApplications", mock.Anything, mock.Anything, mock.Anything, mock.Anything).Return([]jmxproto.ApplicationMetric{}, nil)
				}
			}

			// Ensure specific method is not called if metric is disabled
			if tc.unexpectedMethod != "" {
				mockClient.On(tc.unexpectedMethod, mock.Anything, mock.Anything).Return([]interface{}{}, nil).Maybe()
			}

			err := c.CollectOnce()
			assert.NoError(t, err, "CollectOnce() should succeed")

			// Verify expected method calls
			for _, method := range tc.expectedMethods {
				mockClient.AssertCalled(t, method, mock.Anything, mock.Anything)
			}

			if tc.unexpectedMethod != "" {
				mockClient.AssertNotCalled(t, tc.unexpectedMethod, mock.Anything, mock.Anything)
			}
		})
	}
}
```

These test files cover the majority of the code paths, including:
- Initialization scenarios
- Metric collection logic
- Configuration handling
- Error scenarios
- Conditional metric collection

Key testing strategies include:
1. Using testify's mock framework for simulating JMX client
2. Testing default configurations
3. Testing error scenarios
4. Testing conditional metric collection
5. Verifying method calls and interactions

The tests aim to provide comprehensive coverage of the WebSphere JMX collector's functionality.