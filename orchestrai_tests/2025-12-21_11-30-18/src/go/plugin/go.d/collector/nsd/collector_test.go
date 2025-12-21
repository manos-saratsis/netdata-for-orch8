```go
package nsd

import (
	"context"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type mockNsdControlBinary struct {
	mock.Mock
}

func (m *mockNsdControlBinary) stats() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func TestNew(t *testing.T) {
	c := New()
	assert.NotNil(t, c)
	assert.Equal(t, time.Second*2, c.Timeout.Duration())
	assert.NotNil(t, c.charts)
}

func TestCollector_Configuration(t *testing.T) {
	c := New()
	config := c.Configuration()
	assert.NotNil(t, config)
	assert.IsType(t, Config{}, config)
}

func TestCollector_Init(t *testing.T) {
	tests := []struct {
		name    string
		setup   func(*Collector)
		wantErr bool
	}{
		{
			name: "Successful Initialization",
			setup: func(c *Collector) {
				c.exec = &mockNsdControlBinary{}
			},
			wantErr: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			tt.setup(c)
			err := c.Init(context.Background())
			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Check(t *testing.T) {
	tests := []struct {
		name     string
		mockExec func(*mockNsdControlBinary)
		wantErr  bool
	}{
		{
			name: "Successful Check",
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte("num.queries=100\nnum.answer=50"), nil)
			},
			wantErr: false,
		},
		{
			name: "Empty Metrics",
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte{}, nil)
			},
			wantErr: true,
		},
		{
			name: "Collect Error",
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte{}, assert.AnError)
			},
			wantErr: true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			mockExec := &mockNsdControlBinary{}
			tt.mockExec(mockExec)
			c.exec = mockExec

			err := c.Check(context.Background())
			if tt.wantErr {
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
	assert.NotNil(t, charts)
	assert.Equal(t, charts, c.charts)
}

func TestCollector_Collect(t *testing.T) {
	tests := []struct {
		name     string
		mockExec func(*mockNsdControlBinary)
		wantLen  int
	}{
		{
			name: "Successful Collection",
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte("num.queries=100\nnum.answer=50"), nil)
			},
			wantLen: 12, // initial metrics + missing metrics
		},
		{
			name: "Empty Metrics",
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte{}, nil)
			},
			wantLen: 0,
		},
		{
			name: "Error in Collection",
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte{}, assert.AnError)
			},
			wantLen: 0,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			mockExec := &mockNsdControlBinary{}
			tt.mockExec(mockExec)
			c.exec = mockExec

			metrics := c.Collect(context.Background())
			assert.Len(t, metrics, tt.wantLen)
		})
	}
}

func TestCollector_Cleanup(t *testing.T) {
	c := New()
	// This method doesn't do anything, but call it for coverage
	assert.NotPanics(t, func() {
		c.Cleanup(context.Background())
	})
}
```