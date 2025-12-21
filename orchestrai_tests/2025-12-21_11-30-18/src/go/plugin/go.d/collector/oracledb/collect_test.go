```go
package oracledb

import (
	"context"
	"database/sql"
	"errors"
	"testing"
	"time"

	"github.com/DATA-DOG/go-sqlmock"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestCollector_Collect(t *testing.T) {
	t.Run("successful collection", func(t *testing.T) {
		c := &Collector{
			DSN:     "test_dsn",
			Timeout: Duration{Duration: time.Second * 10},
		}

		// Mock database connection
		db, mock, err := sqlmock.New()
		require.NoError(t, err)
		defer db.Close()

		// Setup expectations
		mock.ExpectQuery(".*").WillReturnRows(sqlmock.NewRows([]string{"dummy"}).AddRow("value"))

		c.db = db

		// Temporarily replace the collection methods with mocks
		origSysMetrics := c.collectSysMetrics
		origSysStat := c.collectSysStat
		origWaitClass := c.collectWaitClass
		origTablespace := c.collectTablespace

		c.collectSysMetrics = func(mx map[string]int64) error { mx["sys_metrics"] = 1; return nil }
		c.collectSysStat = func(mx map[string]int64) error { mx["sys_stat"] = 2; return nil }
		c.collectWaitClass = func(mx map[string]int64) error { mx["wait_class"] = 3; return nil }
		c.collectTablespace = func(mx map[string]int64) error { mx["tablespace"] = 4; return nil }

		defer func() {
			c.collectSysMetrics = origSysMetrics
			c.collectSysStat = origSysStat
			c.collectWaitClass = origWaitClass
			c.collectTablespace = origTablespace
		}()

		mx, err := c.collect()

		assert.NoError(t, err)
		assert.Equal(t, map[string]int64{
			"sys_metrics": 1,
			"sys_stat":    2,
			"wait_class":  3,
			"tablespace":  4,
		}, mx)

		// Check mock expectations
		assert.NoError(t, mock.ExpectationsWereMet())
	})

	t.Run("no database connection", func(t *testing.T) {
		c := &Collector{
			DSN:     "test_dsn",
			Timeout: Duration{Duration: time.Second * 10},
		}

		mx, err := c.collect()

		assert.Nil(t, mx)
		assert.Error(t, err)
		assert.Contains(t, err.Error(), "failed to open connection")
	})

	t.Run("collection method failures", func(t *testing.T) {
		testCases := []struct {
			name           string
			mockCollection func(c *Collector)
			errorMessage   string
		}{
			{
				name: "sys metrics failure",
				mockCollection: func(c *Collector) {
					c.collectSysMetrics = func(map[string]int64) error {
						return errors.New("sys metrics error")
					}
				},
				errorMessage: "failed to collect system metrics",
			},
			{
				name: "sys stat failure",
				mockCollection: func(c *Collector) {
					c.collectSysStat = func(map[string]int64) error {
						return errors.New("sys stat error")
					}
				},
				errorMessage: "failed to collect activities",
			},
			{
				name: "wait class failure",
				mockCollection: func(c *Collector) {
					c.collectWaitClass = func(map[string]int64) error {
						return errors.New("wait class error")
					}
				},
				errorMessage: "failed to collect wait time",
			},
			{
				name: "tablespace failure",
				mockCollection: func(c *Collector) {
					c.collectTablespace = func(map[string]int64) error {
						return errors.New("tablespace error")
					}
				},
				errorMessage: "failed to collect tablespace",
			},
		}

		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				c := &Collector{
					DSN:     "test_dsn",
					Timeout: Duration{Duration: time.Second * 10},
				}

				// Mock database connection
				db, mock, err := sqlmock.New()
				require.NoError(t, err)
				defer db.Close()

				// Setup expectations
				mock.ExpectQuery(".*").WillReturnRows(sqlmock.NewRows([]string{"dummy"}).AddRow("value"))

				c.db = db

				// Apply mock collection method
				tc.mockCollection(c)

				mx, err := c.collect()

				assert.Nil(t, mx)
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errorMessage)

				// Check mock expectations
				assert.NoError(t, mock.ExpectationsWereMet())
			})
		}
	})
}

func TestDoQuery(t *testing.T) {
	t.Run("successful query", func(t *testing.T) {
		c := &Collector{
			Timeout: Duration{Duration: time.Second * 10},
		}

		// Mock database connection
		db, mock, err := sqlmock.New()
		require.NoError(t, err)
		defer db.Close()

		c.db = db

		// Setup expectations
		rows := sqlmock.NewRows([]string{"col1", "col2"}).
			AddRow("value1", "value2").
			AddRow("value3", "value4")
		mock.ExpectQuery("SELECT").WillReturnRows(rows)

		var results []string
		err = c.doQuery("SELECT", func(column, value string, lineEnd bool) error {
			results = append(results, column+":"+value)
			return nil
		})

		assert.NoError(t, err)
		assert.Equal(t, []string{
			"col1:value1", "col2:value2",
			"col1:value3", "col2:value4",
		}, results)

		assert.NoError(t, mock.ExpectationsWereMet())
	})

	t.Run("query error", func(t *testing.T) {
		c := &Collector{
			Timeout: Duration{Duration: time.Second * 10},
		}

		// Mock database connection
		db, mock, err := sqlmock.New()
		require.NoError(t, err)
		defer db.Close()

		c.db = db

		// Setup expectations
		mock.ExpectQuery("SELECT").WillReturnError(errors.New("query error"))

		err = c.doQuery("SELECT", func(column, value string, lineEnd bool) error {
			return nil
		})

		assert.Error(t, err)
		assert.Contains(t, err.Error(), "query error")

		assert.NoError(t, mock.ExpectationsWereMet())
	})

	t.Run("assign function error", func(t *testing.T) {
		c := &Collector{
			Timeout: Duration{Duration: time.Second * 10},
		}

		// Mock database connection
		db, mock, err := sqlmock.New()
		require.NoError(t, err)
		defer db.Close()

		c.db = db

		// Setup expectations
		rows := sqlmock.NewRows([]string{"col1"}).
			AddRow("value1")
		mock.ExpectQuery("SELECT").WillReturnRows(rows)

		err = c.doQuery("SELECT", func(column, value string, lineEnd bool) error {
			return errors.New("assign error")
		})

		assert.Error(t, err)
		assert.Contains(t, err.Error(), "assign error")

		assert.NoError(t, mock.ExpectationsWereMet())
	})
}

func TestOpenConnection(t *testing.T) {
	t.Run("successful connection", func(t *testing.T) {
		c := &Collector{
			DSN:     "test_dsn",
			Timeout: Duration{Duration: time.Second * 10},
		}

		// Mock database connection
		db, mock, err := sqlmock.New()
		require.NoError(t, err)
		defer db.Close()

		// Replace sql.Open with our mock
		origSqlOpen := sqlOpen
		defer func() { sqlOpen = origSqlOpen }()
		sqlOpen = func(driverName, dataSourceName string) (*sql.DB, error) {
			return db, nil
		}

		// Expect a ping
		mock.ExpectPing()

		err = c.openConnection()

		assert.NoError(t, err)
		assert.NotNil(t, c.db)

		assert.NoError(t, mock.ExpectationsWereMet())
	})

	t.Run("sql open error", func(t *testing.T) {
		c := &Collector{
			DSN:     "test_dsn",
			Timeout: Duration{Duration: time.Second * 10},
		}

		// Replace sql.Open with our mock
		origSqlOpen := sqlOpen
		defer func() { sqlOpen = origSqlOpen }()
		sqlOpen = func(driverName, dataSourceName string) (*sql.DB, error) {
			return nil, errors.New("open error")
		}

		err := c.openConnection()

		assert.Error(t, err)
		assert.Contains(t, err.Error(), "error on sql open")
		assert.Nil(t, c.db)
	})

	t.Run("ping error", func(t *testing.T) {
		c := &Collector{
			DSN:     "test_dsn",
			Timeout: Duration{Duration: time.Second * 10},
		}

		// Mock database connection
		db, mock, err := sqlmock.New()
		require.NoError(t, err)
		defer db.Close()

		// Replace sql.Open with our mock
		origSqlOpen := sqlOpen
		defer func() { sqlOpen = origSqlOpen }()
		sqlOpen = func(driverName, dataSourceName string) (*sql.DB, error) {
			return db, nil
		}

		// Expect a ping that fails
		mock.ExpectPing().WillReturnError(errors.New("ping error"))

		err = c.openConnection()

		assert.Error(t, err)
		assert.Contains(t, err.Error(), "error on pinging")
		assert.Nil(t, c.db)

		assert.NoError(t, mock.ExpectationsWereMet())
	})
}

func TestMakeValues(t *testing.T) {
	values := makeValues(3)

	assert.Len(t, values, 3)
	for _, v := range values {
		nullStr, ok := v.(*sql.NullString)
		assert.True(t, ok)
		assert.False(t, nullStr.Valid)
	}
}

func TestValueToString(t *testing.T) {
	testCases := []struct {
		name     string
		input    *sql.NullString
		expected string
	}{
		{
			name:     "Valid string",
			input:    &sql.NullString{String: "test", Valid: true},
			expected: "test",
		},
		{
			name:     "Invalid string",
			input:    &sql.NullString{Valid: false},
			expected: "",
		},
		{
			name:     "Wrong type",
			input:    nil,
			expected: "",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := valueToString(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}
```