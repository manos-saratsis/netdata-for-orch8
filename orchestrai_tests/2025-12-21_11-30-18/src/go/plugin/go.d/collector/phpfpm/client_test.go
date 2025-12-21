```go
package phpfpm

import (
	"bytes"
	"encoding/json"
	"errors"
	"io"
	"net/http"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/logger"
	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
)

func TestRequestDurationUnmarshalJSON(t *testing.T) {
	testCases := []struct {
		input    string
		expected requestDuration
	}{
		{"123", 123},
		{"invalid", 0},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			var rd requestDuration
			err := rd.UnmarshalJSON([]byte(tc.input))
			assert.NoError(t, err)
			assert.Equal(t, tc.expected, rd)
		})
	}
}

func TestNewHTTPClient(t *testing.T) {
	tests := []struct {
		name    string
		url     string
		wantErr bool
	}{
		{"Valid URL without JSON", "http://localhost/status", false},
		{"Valid URL with JSON", "http://localhost/status?json", false},
		{"Invalid URL", "://invalid", true},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			client, err := newHTTPClient(&http.Client{}, web.RequestConfig{URL: tt.url})
			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, client)
			}
		})
	}
}

func TestHTTPClientGetStatus(t *testing.T) {
	tests := []struct {
		name           string
		mockResponse   string
		expectedStatus *status
		expectError    bool
	}{
		{
			"Successful JSON decoding",
			`{"active processes": 5, "max active processes": 10, "idle processes": 3, "accepted conn": 100, "max children reached": 2, "slow requests": 1, "processes": []}`,
			&status{
				Active:    5,
				MaxActive: 10,
				Idle:      3,
				Requests:  100,
				Reached:   2,
				Slow:      1,
				Processes: []proc{},
			},
			false,
		},
		{
			"Decode error",
			"invalid json",
			nil,
			true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			mockClient := &http.Client{
				Transport: &mockRoundTripper{
					response: tt.mockResponse,
				},
			}

			client, err := newHTTPClient(mockClient, web.RequestConfig{URL: "http://localhost/status"})
			assert.NoError(t, err)

			status, err := client.getStatus()
			if tt.expectError {
				assert.Error(t, err)
				assert.Nil(t, status)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tt.expectedStatus, status)
			}
		})
	}
}

type mockRoundTripper struct {
	response string
}

func (m *mockRoundTripper) RoundTrip(req *http.Request) (*http.Response, error) {
	return &http.Response{
		StatusCode: http.StatusOK,
		Body:       io.NopCloser(bytes.NewBufferString(m.response)),
	}, nil
}

func TestNewSocketClient(t *testing.T) {
	log := logger.New("test", "test")
	client := newSocketClient(log, "/tmp/php-fpm.sock", 5*time.Second, "/status.php")

	assert.NotNil(t, client)
	assert.Equal(t, "/tmp/php-fpm.sock", client.socket)
	assert.Equal(t, 5*time.Second, client.timeout)
	assert.Contains(t, client.env, "SCRIPT_NAME")
	assert.Contains(t, client.env, "SCRIPT_FILENAME")
}

func TestNewTCPClient(t *testing.T) {
	log := logger.New("test", "test")
	client := newTcpClient(log, "localhost:9000", 5*time.Second, "/status.php")

	assert.NotNil(t, client)
	assert.Equal(t, "localhost:9000", client.address)
	assert.Equal(t, 5*time.Second, client.timeout)
	assert.Contains(t, client.env, "SCRIPT_NAME")
	assert.Contains(t, client.env, "SCRIPT_FILENAME")
}
```

I'll continue generating tests for the remaining PHP-FPM related files. Would you like me to proceed with the next file in the same comprehensive manner?