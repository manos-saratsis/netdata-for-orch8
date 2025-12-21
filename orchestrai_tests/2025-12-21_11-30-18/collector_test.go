```go
package tor

import (
	"context"
	"errors"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type mockControlConn struct {
	mock.Mock
}

func (m *mockControlConn) connect() error {
	args := m.Called()
	return args.Error(0)
}

func (m *mockControlConn) disconnect() {
	m.Called()
}

func (m *mockControlConn) getInfo(keywords ...string) ([]byte, error) {
	args := m.Called(keywords)
	return args.Get(0).([]byte), args.Error(1)
}

func TestCollector_Configuration(t *testing.T) {
	c := New()
	assert.Equal(t, c.Config, c.Configuration())
}

func TestCollector_Init(t *testing.T) {
	tests := []struct {
		name    string
		address string
		wantErr bool
	}{
		{
			name:    "Success with valid address",
			address: "127.0.0.1:9051",
			wantErr: false,
		},
		{
			name:    "Fail with empty address",
			address: "",
			wantErr: true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			c.Address = tt.address
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
		name          string
		mockCollect  map[string]int64
		mockCollectErr error
		wantErr       bool
	}{
		{
			name:          "Success with metrics",
			mockCollect:   map[string]int64{"traffic/read": 100},
			mockCollectErr: nil,
			wantErr:       false,
		},
		{
			name:          "Fail with no metrics",
			mockCollect:   map[string]int64{},
			mockCollectErr: nil,
			wantErr:       true,
		},
		{
			name:          "Fail with collect error",
			mockCollect:   nil,
			mockCollectErr: errors.New("collect error"),
			wantErr:       true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			c.collect = func() (map[string]int64, error) {
				return tt.mockCollect, tt.mockCollectErr
			}

			err := c.Check(context.Background())

			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Collect(t *testing.T) {
	tests := []struct {
		name          string
		mockCollect  map[string]int64
		mockCollectErr error
		wantResult    map[string]int64
	}{
		{
			name:          "Successful collection",
			mockCollect:   map[string]int64{"traffic/read": 100, "traffic/written": 200},
			mockCollectErr: nil,
			wantResult:    map[string]int64{"traffic/read": 100, "traffic/written": 200},
		},
		{
			name:          "Collection with error",
			mockCollect:   nil,
			mockCollectErr: errors.New("collect error"),
			wantResult:    nil,
		},
		{
			name:          "Collection with empty metrics",
			mockCollect:   map[string]int64{},
			mockCollectErr: nil,
			wantResult:    nil,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			c.collect = func() (map[string]int64, error) {
				return tt.mockCollect, tt.mockCollectErr
			}

			result := c.Collect(context.Background())

			assert.Equal(t, tt.wantResult, result)
		})
	}
}

func TestCollector_Cleanup(t *testing.T) {
	mockConn := &mockControlConn{}
	mockConn.On("disconnect").Return()

	c := New()
	c.conn = mockConn

	c.Cleanup(context.Background())

	assert.Nil(t, c.conn)
	mockConn.AssertExpectations(t)
}

func TestCollector_Charts(t *testing.T) {
	c := New()
	charts := c.Charts()

	assert.NotNil(t, charts)
	assert.Len(t, *charts, 2)
	assert.Contains(t, *charts, trafficChart)
	assert.Contains(t, *charts, uptimeChart)
}
```