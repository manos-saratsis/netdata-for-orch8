```go
package mysql

import (
	"context"
	"database/sql"
	"errors"
	"testing"
	"time"

	"github.com/blang/semver/v4"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestCollector_OpenConnection(t *testing.T) {
	testCases := []struct {
		name         string
		dsn          string
		mockPingErr  error
		expectedErr  bool
	}{
		{
			name:        "Successful Connection",
			dsn:         "user:pass@tcp(localhost:3306)/",
			expectedErr: false,
		},
		{
			name:         "Connection Open Error",
			dsn:          "invalid-dsn",
			expectedErr:  true,
		},
		{
			name:         "Ping Error",
			dsn:          "user:pass@tcp(localhost:3306)/",
			mockPingErr: errors.New("ping failed"),
			expectedErr:  true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			c.DSN = tc.dsn
			c.safeDSN = tc.dsn

			mockDB, mock, err := createMockSQLDB()
			assert.NoError(t, err)

			if tc.mockPingErr != nil {
				mock.ExpectPing().WillReturnError(tc.mockPingErr)
			} else {
				mock.ExpectPing()
			}

			err = c.openConnection()

			if tc.expectedErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, c.db)
			}
		})
	}
}

func TestCalcThreadCacheMisses(t *testing.T) {
	testCases := []struct {
		name               string
		threads            int64
		connections        int64
		expectedMisses     int64
	}{
		{
			name:            "Normal Calculation",
			threads:         100,
			connections:     1000,
			expectedMisses:  1000,
		},
		{
			name:            "Zero Threads",
			threads:         0,
			connections:     1000,
			expectedMisses:  0,
		},
		{
			name:            "Zero Connections",
			threads:         100,
			connections:     0,
			expectedMisses:  0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collected := map[string]int64{
				"threads_created": tc.threads,
				"connections":     tc.connections,
			}

			calcThreadCacheMisses(collected)

			assert.Equal(t, tc.expectedMisses, collected["thread_cache_misses"])
		})
	}
}

func TestHasMetricCheckers(t *testing.T) {
	testCases := []struct {
		name               string
		metricCheckerFunc  func(map[string]int64) bool
		metrics            map[string]int64
		expected           bool
	}{
		{
			name:               "HasInnodbOSLog - Exists",
			metricCheckerFunc:  hasInnodbOSLog,
			metrics:            map[string]int64{"innodb_os_log_fsyncs": 100},
			expected:           true,
		},
		{
			name:               "HasInnodbOSLog - Not Exists",
			metricCheckerFunc:  hasInnodbOSLog,
			metrics:            map[string]int64{},
			expected:           false,
		},
		{
			name:               "HasGaleraMetrics - Exists",
			metricCheckerFunc:  hasGaleraMetrics,
			metrics:            map[string]int64{"wsrep_received": 50},
			expected:           true,
		},
		{
			name:               "HasGaleraMetrics - Not Exists",
			metricCheckerFunc:  hasGaleraMetrics,
			metrics:            map[string]int64{},
			expected:           false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := tc.metricCheckerFunc(tc.metrics)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestValueConversionFunctions(t *testing.T) {
	t.Run("ParseInt", func(t *testing.T) {
		testCases := []struct {
			input    string
			expected int64
		}{
			{"123", 123},
			{"-456", -456},
			{"0", 0},
			{"invalid", 0},
		}

		for _, tc := range testCases {
			result := parseInt(tc.input)
			assert.Equal(t, tc.expected, result)
		}
	})

	t.Run("ParseFloat", func(t *testing.T) {
		testCases := []struct {
			input    string
			expected float64
		}{
			{"123.45", 123.45},
			{"-456.78", -456.78},
			{"0", 0},
			{"invalid", 0},
		}

		for _, tc := range testCases {
			result := parseFloat(tc.input)
			assert.Equal(t, tc.expected, result)
		}
	})
}

// Mock function to create SQL mock database
func createMockSQLDB() (*sql.DB, *mock.MockDB, error) {
	// Implement mock DB creation logic
	// This is a placeholder implementation
	return nil, nil, errors.New("not implemented")
}
```