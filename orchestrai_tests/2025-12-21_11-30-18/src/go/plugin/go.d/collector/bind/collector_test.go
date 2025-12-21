```go
package bind

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

type MockBindAPIClient struct {
	mock.Mock
}

func (m *MockBindAPIClient) serverStats() (*serverStats, error) {
	args := m.Called()
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*serverStats), args.Error(1)
}

func TestCollector_Configuration(t *testing.T) {
	c := New()
	assert.NotNil(t, c.Configuration())
}

func TestCollector_Init(t *testing.T) {
	tests := []struct {
		name    string
		setup   func(*Collector)
		wantErr bool
	}{
		{
			name: "Successful initialization",
			setup: func(c *Collector) {
				c.URL = "http://localhost/json/v1"
			},
			wantErr: false,
		},
		{
			name: "Missing URL",
			setup: func(c *Collector) {
				c.URL = ""
			},
			wantErr: true,
		},
		{
			name: "Invalid permit view pattern",
			setup: func(c *Collector) {
				c.URL = "http://localhost/json/v1"
				c.PermitView = "["
			},
			wantErr: true,
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
				assert.NotNil(t, c.httpClient)
				assert.NotNil(t, c.bindAPIClient)
			}
		})
	}
}

func TestCollector_Check(t *testing.T) {
	tests := []struct {
		name       string
		setupMock  func(*MockBindAPIClient)
		wantErr    bool
		wantErrMsg string
	}{
		{
			name: "Successful collection",
			setupMock: func(m *MockBindAPIClient) {
				m.On("serverStats").Return(&serverStats{
					NSStats: map[string]int64{"Requestv4": 10},
				}, nil)
			},
			wantErr: false,
		},
		{
			name: "Collect error",
			setupMock: func(m *MockBindAPIClient) {
				m.On("serverStats").Return(nil, errors.New("test error"))
			},
			wantErr:    true,
			wantErrMsg: "test error",
		},
		{
			name: "No metrics collected",
			setupMock: func(m *MockBindAPIClient) {
				m.On("serverStats").Return(&serverStats{}, nil)
			},
			wantErr:    true,
			wantErrMsg: "no metrics collected",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			mockClient := &MockBindAPIClient{}
			tt.setupMock(mockClient)
			c.bindAPIClient = mockClient

			err := c.Check(context.Background())

			if tt.wantErr {
				assert.Error(t, err)
				if tt.wantErrMsg != "" {
					assert.Contains(t, err.Error(), tt.wantErrMsg)
				}
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Collect(t *testing.T) {
	tests := []struct {
		name       string
		setupMock  func(*MockBindAPIClient)
		wantMetrics bool
	}{
		{
			name: "Successful collection",
			setupMock: func(m *MockBindAPIClient) {
				m.On("serverStats").Return(&serverStats{
					NSStats: map[string]int64{"Requestv4": 10},
				}, nil)
			},
			wantMetrics: true,
		},
		{
			name: "Collection error",
			setupMock: func(m *MockBindAPIClient) {
				m.On("serverStats").Return(nil, errors.New("test error"))
			},
			wantMetrics: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			mockClient := &MockBindAPIClient{}
			tt.setupMock(mockClient)
			c.bindAPIClient = mockClient

			metrics := c.Collect(context.Background())

			if tt.wantMetrics {
				assert.NotEmpty(t, metrics)
			} else {
				assert.Empty(t, metrics)
			}
		})
	}
}

func TestCollector_Cleanup(t *testing.T) {
	c := New()
	c.httpClient = &http.Client{}
	c.Cleanup(context.Background())
	// Check if no panic occurs
}
```