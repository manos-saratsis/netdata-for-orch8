```go
package storcli

import (
	"context"
	"errors"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type mockStorCli struct {
	mock.Mock
}

func (m *mockStorCli) controllersInfo() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func (m *mockStorCli) drivesInfo() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func TestCollector_Configuration(t *testing.T) {
	c := New()
	cfg := c.Configuration()
	assert.NotNil(t, cfg)
	assert.IsType(t, &Config{}, cfg)
}

func TestCollector_Init(t *testing.T) {
	testCases := []struct {
		name        string
		createExec  func() (storCli, error)
		expectError bool
	}{
		{
			name: "Successful initialization",
			createExec: func() (storCli, error) {
				return &mockStorCli{}, nil
			},
			expectError: false,
		},
		{
			name: "Initialization with error",
			createExec: func() (storCli, error) {
				return nil, errors.New("init error")
			},
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			c.initStorCliExec = tc.createExec

			err := c.Init(context.Background())
			if tc.expectError {
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
		collectResult  map[string]int64
		collectError   error
		expectedResult bool
	}{
		{
			name:           "Successful check with metrics",
			collectResult:  map[string]int64{"test_metric": 1},
			collectError:   nil,
			expectedResult: true,
		},
		{
			name:           "Check fails with collection error",
			collectResult:  nil,
			collectError:   errors.New("collection error"),
			expectedResult: false,
		},
		{
			name:           "Check fails with no metrics",
			collectResult:  map[string]int64{},
			collectError:   nil,
			expectedResult: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			c.collect = func() (map[string]int64, error) {
				return tc.collectResult, tc.collectError
			}

			err := c.Check(context.Background())
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
		collectResult  map[string]int64
		collectError   error
		expectedResult map[string]int64
	}{
		{
			name:           "Successful collection",
			collectResult:  map[string]int64{"test_metric": 10},
			collectError:   nil,
			expectedResult: map[string]int64{"test_metric": 10},
		},
		{
			name:           "Collection with error",
			collectResult:  nil,
			collectError:   errors.New("collection error"),
			expectedResult: nil,
		},
		{
			name:           "Collection with empty metrics",
			collectResult:  map[string]int64{},
			collectError:   nil,
			expectedResult: nil,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			c.collect = func() (map[string]int64, error) {
				return tc.collectResult, tc.collectError
			}
			c.Error = func(err error) {}

			result := c.Collect(context.Background())
			assert.Equal(t, tc.expectedResult, result)
		})
	}
}

func TestCollector_New(t *testing.T) {
	c := New()
	assert.NotNil(t, c)
	assert.NotNil(t, c.charts)
	assert.NotNil(t, c.controllers)
	assert.NotNil(t, c.drives)
	assert.NotNil(t, c.bbu)
	assert.Equal(t, time.Second*2, c.Timeout.Duration())
}

func TestCollector_Cleanup(t *testing.T) {
	c := New()
	// Should not panic and not do anything
	assert.NotPanics(t, func() {
		c.Cleanup(context.Background())
	})
}

func TestCollector_Charts(t *testing.T) {
	c := New()
	charts := c.Charts()
	assert.NotNil(t, charts)
	assert.Equal(t, c.charts, charts)
}
```