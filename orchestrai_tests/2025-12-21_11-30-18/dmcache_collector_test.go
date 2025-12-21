```go
package dmcache

import (
	"context"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type mockDmsetupCli struct {
	mock.Mock
}

func (m *mockDmsetupCli) cacheStatus() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func TestCollectorConfiguration(t *testing.T) {
	collector := New()
	cfg := collector.Configuration()
	
	assert.NotNil(t, cfg)
	assert.IsType(t, &Config{}, cfg)
}

func TestCollectorInit(t *testing.T) {
	testCases := []struct {
		name          string
		mockSetup     func(*mockDmsetupCli)
		expectedError bool
	}{
		{
			name: "Successful init",
			mockSetup: func(m *mockDmsetupCli) {
				m.On("cacheStatus").Return([]byte("test-device: 0 0 cache 262144 0/1024 262144 0/1024 123456 7890 12345 6789 98765 54321 12345"), nil)
			},
			expectedError: false,
		},
		{
			name: "Initialization failure",
			mockSetup: func(m *mockDmsetupCli) {
				m.On("cacheStatus").Return([]byte{}, errors.New("init error"))
			},
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := New()
			mockCli := &mockDmsetupCli{}
			
			if tc.mockSetup != nil {
				tc.mockSetup(mockCli)
			}

			collector.exec = mockCli
			err := collector.Init(context.Background())

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollectorCheck(t *testing.T) {
	testCases := []struct {
		name          string
		mockSetup     func(*mockDmsetupCli)
		expectedError bool
	}{
		{
			name: "Successful check with metrics",
			mockSetup: func(m *mockDmsetupCli) {
				m.On("cacheStatus").Return([]byte("test-device: 0 0 cache 262144 0/1024 262144 0/1024 123456 7890 12345 6789 98765 54321 12345"), nil)
			},
			expectedError: false,
		},
		{
			name: "No metrics collected",
			mockSetup: func(m *mockDmsetupCli) {
				m.On("cacheStatus").Return([]byte{}, nil)
			},
			expectedError: true,
		},
		{
			name: "Collection error",
			mockSetup: func(m *mockDmsetupCli) {
				m.On("cacheStatus").Return([]byte{}, errors.New("collection error"))
			},
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := New()
			mockCli := &mockDmsetupCli{}
			
			if tc.mockSetup != nil {
				tc.mockSetup(mockCli)
			}

			collector.exec = mockCli
			err := collector.Check(context.Background())

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollectorCollect(t *testing.T) {
	testCases := []struct {
		name           string
		mockSetup      func(*mockDmsetupCli)
		validateResult func(map[string]int64)
	}{
		{
			name: "Successful collection",
			mockSetup: func(m *mockDmsetupCli) {
				m.On("cacheStatus").Return([]byte("test-device: 0 0 cache 262144 0/1024 262144 0/1024 123456 7890 12345 6789 98765 54321 12345"), nil)
			},
			validateResult: func(mx map[string]int64) {
				assert.Greater(t, len(mx), 0)
				assert.Contains(t, mx, "dmcache_device_test-device_metadata_free_bytes")
			},
		},
		{
			name: "Collection error",
			mockSetup: func(m *mockDmsetupCli) {
				m.On("cacheStatus").Return([]byte{}, errors.New("collection error"))
			},
			validateResult: func(mx map[string]int64) {
				assert.Len(t, mx, 0)
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := New()
			mockCli := &mockDmsetupCli{}
			
			if tc.mockSetup != nil {
				tc.mockSetup(mockCli)
			}

			collector.exec = mockCli
			result := collector.Collect(context.Background())

			if tc.validateResult != nil {
				tc.validateResult(result)
			}
		})
	}
}

func TestCollectorCharts(t *testing.T) {
	collector := New()
	charts := collector.Charts()

	assert.NotNil(t, charts)
	assert.IsType(t, &module.Charts{}, charts)
}
```