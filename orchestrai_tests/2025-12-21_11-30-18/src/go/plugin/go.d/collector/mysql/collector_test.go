```go
package mysql

import (
	"context"
	"database/sql"
	"errors"
	"testing"
	"time"

	"github.com/blang/semver/v4"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestNew(t *testing.T) {
	c := New()
	
	assert.NotNil(t, c)
	assert.Equal(t, "root@tcp(localhost:3306)/", c.DSN)
	assert.Equal(t, time.Second, time.Duration(c.Timeout))
	assert.NotNil(t, c.charts)
}

func TestCollector_Configuration(t *testing.T) {
	c := New()
	config := c.Configuration()
	
	assert.NotNil(t, config)
	assert.IsType(t, &Config{}, config)
}

func TestCollector_Init(t *testing.T) {
	testCases := []struct {
		name        string
		dsn         string
		myCNF       string
		expectedErr bool
		errMsg      string
	}{
		{
			name:        "Valid DSN",
			dsn:         "root:password@tcp(localhost:3306)/",
			expectedErr: false,
		},
		{
			name:        "Empty DSN",
			dsn:         "",
			expectedErr: true,
			errMsg:      "config: dsn not set",
		},
		{
			name:        "Invalid DSN",
			dsn:         "invalid-dsn",
			expectedErr: true,
			errMsg:      "error on parsing DSN:",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			c.DSN = tc.dsn
			c.MyCNF = tc.myCNF

			err := c.Init(context.Background())

			if tc.expectedErr {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errMsg)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Check(t *testing.T) {
	testCases := []struct {
		name        string
		mockCollect func() (map[string]int64, error)
		expectedErr bool
	}{
		{
			name: "Successful collection",
			mockCollect: func() (map[string]int64, error) {
				return map[string]int64{"metric1": 100}, nil
			},
			expectedErr: false,
		},
		{
			name: "Collection error",
			mockCollect: func() (map[string]int64, error) {
				return nil, errors.New("collection error")
			},
			expectedErr: true,
		},
		{
			name: "No metrics collected",
			mockCollect: func() (map[string]int64, error) {
				return map[string]int64{}, nil
			},
			expectedErr: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			c.collect = tc.mockCollect

			err := c.Check(context.Background())

			if tc.expectedErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Collect(t *testing.T) {
	testCases := []struct {
		name             string
		mockCollect      func() (map[string]int64, error)
		expectedMetrics map[string]int64
	}{
		{
			name: "Successful collection",
			mockCollect: func() (map[string]int64, error) {
				return map[string]int64{"metric1": 100}, nil
			},
			expectedMetrics: map[string]int64{"metric1": 100},
		},
		{
			name: "Collection error",
			mockCollect: func() (map[string]int64, error) {
				return nil, errors.New("collection error")
			},
			expectedMetrics: nil,
		},
		{
			name: "No metrics collected",
			mockCollect: func() (map[string]int64, error) {
				return map[string]int64{}, nil
			},
			expectedMetrics: nil,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			c.collect = tc.mockCollect

			metrics := c.Collect(context.Background())

			assert.Equal(t, tc.expectedMetrics, metrics)
		})
	}
}

func TestCollector_Cleanup(t *testing.T) {
	testCases := []struct {
		name         string
		setupDB      bool
		expectedErr  bool
	}{
		{
			name:         "Nil DB",
			setupDB:      false,
			expectedErr:  false,
		},
		{
			name:         "Successful DB Close",
			setupDB:      true,
			expectedErr:  false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			
			if tc.setupDB {
				mockDB, mock, err := createMockDB()
				assert.NoError(t, err)
				
				mock.ExpectClose()
				c.db = mockDB
			}

			c.Cleanup(context.Background())

			assert.Nil(t, c.db)
		})
	}
}

func createMockDB() (*sql.DB, *mock.MockDB, error) {
	mockDB, mock, err := createMockSQLDB()
	if err != nil {
		return nil, nil, err
	}
	return mockDB, mock, nil
}
```