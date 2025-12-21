```go
package sql

import (
	"context"
	"database/sql"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

// MockDatabase is a mock implementation of a database connection
type MockDatabase struct {
	mock.Mock
}

func (m *MockDatabase) QueryContext(ctx context.Context, query string, args ...interface{}) (*sql.Rows, error) {
	// Implement mock query logic
	return nil, nil
}

func (m *MockDatabase) Close() error {
	// Implement mock close logic
	return nil
}

func TestExecReusableQueries(t *testing.T) {
	testCases := []struct {
		name           string
		queries        []ConfigQueryDef
		setupMockQuery func(m *MockDatabase)
		wantErrContain string
	}{
		{
			name: "Successful Multiple Queries",
			queries: []ConfigQueryDef{
				{ID: "query1", Query: "SELECT 1"},
				{ID: "query2", Query: "SELECT 2"},
			},
			setupMockQuery: func(m *MockDatabase) {
				// Setup mock expectations for each query
			},
		},
		{
			name: "Missing Query ID",
			queries: []ConfigQueryDef{
				{Query: "SELECT 1"},
			},
			wantErrContain: "missing id",
		},
		{
			name: "Missing Query Text",
			queries: []ConfigQueryDef{
				{ID: "query1"},
			},
			wantErrContain: "missing query",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				Queries: tc.queries,
				db:      &sql.DB{}, // Replace with mock
			}

			cache, durations, err := c.execReusableQueries(context.Background())

			if tc.wantErrContain != "" {
				assert.ErrorContains(t, err, tc.wantErrContain)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, cache)
				assert.NotNil(t, durations)
			}
		})
	}
}

func TestExecMetricQueries(t *testing.T) {
	testCases := []struct {
		name           string
		metrics        []ConfigMetricBlock
		qcache         queryRowsCache
		wantErrContain string
	}{
		{
			name: "Successful Query Ref",
			metrics: []ConfigMetricBlock{
				{
					ID:       "metric1",
					QueryRef: "query1",
				},
			},
			qcache: queryRowsCache{
				"query1": []map[string]string{
					{"col1": "value1"},
				},
			},
		},
		{
			name: "Successful Inline Query",
			metrics: []ConfigMetricBlock{
				{
					ID:    "metric1",
					Query: "SELECT 1",
				},
			},
		},
		{
			name: "Missing Metric ID",
			metrics: []ConfigMetricBlock{
				{},
			},
			wantErrContain: "missing id",
		},
		{
			name: "Missing Query Reference",
			metrics: []ConfigMetricBlock{
				{
					ID: "metric1",
				},
			},
			wantErrContain: "must set one of query_ref or query",
		},
		{
			name: "Invalid Query Reference",
			metrics: []ConfigMetricBlock{
				{
					ID:       "metric1",
					QueryRef: "non_existent",
				},
			},
			qcache:         queryRowsCache{},
			wantErrContain: "not found in queries cache",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				Metrics: tc.metrics,
				db:      &sql.DB{}, // Replace with mock
			}

			cache, durations, err := c.execMetricQueries(context.Background(), tc.qcache)

			if tc.wantErrContain != "" {
				assert.ErrorContains(t, err, tc.wantErrContain)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, cache)
				assert.NotNil(t, durations)
			}
		})
	}
}

func TestRunSQL(t *testing.T) {
	testCases := []struct {
		name           string
		query          string
		timeout        time.Duration
		setupMockQuery func(m *MockDatabase)
		wantErrContain string
		wantRowCount   int
	}{
		{
			name:  "Successful Query",
			query: "SELECT 1",
			setupMockQuery: func(m *MockDatabase) {
				// Mock a single row query
			},
			wantRowCount: 1,
		},
		{
			name:         "Timeout Query",
			query:        "SELECT 1",
			timeout:      1 * time.Millisecond,
			wantErrContain: "context deadline exceeded",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				Timeout: confopt.Duration{Duration: tc.timeout},
				db:      &sql.DB{}, // Replace with mock
			}

			rows, duration, err := c.runSQL(context.Background(), tc.query)

			if tc.wantErrContain != "" {
				assert.ErrorContains(t, err, tc.wantErrContain)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.wantRowCount, len(rows))
				assert.Greater(t, duration, int64(0))
			}
		})
	}
}
```