```go
package clickhouse

import (
	"bytes"
	"encoding/csv"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"strings"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestCollector_collect(t *testing.T) {
	testCases := []struct {
		name                  string
		mockSystemEvents     error
		mockSystemMetrics    error
		mockSystemAsyncMetrics error
		mockSystemParts      error
		mockSystemDisks      error
		mockLongestQuery     error
		expectedError        bool
	}{
		{
			name:                  "Successful collection",
			expectedError:         false,
		},
		{
			name:                  "System Events Collection Error",
			mockSystemEvents:      errors.New("system events error"),
			expectedError:         true,
		},
		{
			name:                  "System Metrics Collection Error",
			mockSystemMetrics:     errors.New("system metrics error"),
			expectedError:         true,
		},
		{
			name:                  "System Async Metrics Collection Error",
			mockSystemAsyncMetrics: errors.New("system async metrics error"),
			expectedError:         true,
		},
		{
			name:                  "System Parts Collection Error",
			mockSystemParts:       errors.New("system parts error"),
			expectedError:         true,
		},
		{
			name:                  "System Disks Collection Error",
			mockSystemDisks:       errors.New("system disks error"),
			expectedError:         true,
		},
		{
			name:                  "Longest Running Query Time Error",
			mockLongestQuery:      errors.New("longest query error"),
			expectedError:         true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()

			// Create mock functions
			originalSystemEvents := c.collectSystemEvents
			originalSystemMetrics := c.collectSystemMetrics
			originalSystemAsyncMetrics := c.collectSystemAsyncMetrics
			originalSystemParts := c.collectSystemParts
			originalSystemDisks := c.collectSystemDisks
			originalLongestQuery := c.collectLongestRunningQueryTime

			defer func() {
				c.collectSystemEvents = originalSystemEvents
				c.collectSystemMetrics = originalSystemMetrics
				c.collectSystemAsyncMetrics = originalSystemAsyncMetrics
				c.collectSystemParts = originalSystemParts
				c.collectSystemDisks = originalSystemDisks
				c.collectLongestRunningQueryTime = originalLongestQuery
			}()

			c.collectSystemEvents = func(m map[string]int64) error {
				return tc.mockSystemEvents
			}
			c.collectSystemMetrics = func(m map[string]int64) error {
				return tc.mockSystemMetrics
			}
			c.collectSystemAsyncMetrics = func(m map[string]int64) error {
				return tc.mockSystemAsyncMetrics
			}
			c.collectSystemParts = func(m map[string]int64) error {
				return tc.mockSystemParts
			}
			c.collectSystemDisks = func(m map[string]int64) error {
				return tc.mockSystemDisks
			}
			c.collectLongestRunningQueryTime = func(m map[string]int64) error {
				return tc.mockLongestQuery
			}

			mx, err := c.collect()

			if tc.expectedError {
				assert.Error(t, err)
				assert.Nil(t, mx)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, mx)
			}
		})
	}
}

func TestReadCSVResponseData(t *testing.T) {
	testCases := []struct {
		name           string
		csvData        string
		expectedError  bool
		expectedCalls  int
	}{
		{
			name: "Valid CSV Data",
			csvData: `column1,column2,column3
value1,value2,value3
value4,value5,value6`,
			expectedError: false,
			expectedCalls: 6,
		},
		{
			name: "Invalid CSV Data (column mismatch)",
			csvData: `column1,column2
value1
value2,value3,value4`,
			expectedError: true,
			expectedCalls: 0,
		},
		{
			name:           "Empty CSV Data",
			csvData:        "",
			expectedError:  false,
			expectedCalls:  0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			var calls int
			assign := func(column, value string, lineEnd bool) {
				calls++
			}

			reader := strings.NewReader(tc.csvData)
			err := readCSVResponseData(reader, assign)

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			assert.Equal(t, tc.expectedCalls, calls)
		})
	}
}

func TestMakeURLQuery(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{
			input:    "SELECT * FROM system.metrics",
			expected: "query=SELECT+*+FROM+system.metrics",
		},
		{
			input:    "",
			expected: "query=",
		},
		{
			input:    "SELECT 'test query'",
			expected: "query=SELECT+%27test+query%27",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := makeURLQuery(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestCollector_doHTTP(t *testing.T) {
	testCases := []struct {
		name          string
		httpResponse  string
		httpError     error
		expectedError bool
	}{
		{
			name:          "Successful HTTP Request",
			httpResponse:  "column1,column2\nvalue1,value2",
			expectedError: false,
		},
		{
			name:          "HTTP Request Error",
			httpError:     errors.New("http error"),
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			
			// Create a test HTTP client that returns controlled response
			client := &http.Client{
				Transport: &mockRoundTripper{
					response: tc.httpResponse,
					err:      tc.httpError,
				},
			}
			c.httpClient = client

			req, _ := http.NewRequest("GET", "http://test.com", nil)
			var calls int
			
			assign := func(column, value string, lineEnd bool) {
				calls++
			}

			err := c.doHTTP(req, assign)

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Greater(t, calls, 0)
			}
		})
	}
}

// mockRoundTripper simulates HTTP responses for testing
type mockRoundTripper struct {
	response string
	err      error
}

func (m *mockRoundTripper) RoundTrip(req *http.Request) (*http.Response, error) {
	if m.err != nil {
		return nil, m.err
	}

	resp := &http.Response{
		StatusCode: http.StatusOK,
		Body:       io.NopCloser(strings.NewReader(m.response)),
		Request:    req,
	}
	return resp, nil
}
```