```go
package httpcheck

import (
	"bytes"
	"errors"
	"io"
	"net/http"
	"os"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type MockHTTPClient struct {
	mock.Mock
}

func (m *MockHTTPClient) Do(req *http.Request) (*http.Response, error) {
	args := m.Called(req)
	return args.Get(0).(*http.Response), args.Error(1)
}

func TestCollect_Success(t *testing.T) {
	c := &Collector{
		RequestConfig: web.RequestConfig{
			URL: "http://example.com",
		},
		UpdateEvery:      10,
		acceptedStatuses: map[int]bool{200: true},
	}

	mockClient := new(MockHTTPClient)
	c.httpClient = mockClient

	body := io.NopCloser(bytes.NewReader([]byte("test response")))
	resp := &http.Response{
		StatusCode: 200,
		Body:       body,
		Header:     make(http.Header),
	}

	mockClient.On("Do", mock.Anything).Return(resp, nil)

	metrics, err := c.collect()

	assert.NoError(t, err)
	assert.NotNil(t, metrics)
	assert.True(t, metrics["status.success"] == 1)
}

func TestCollect_NetworkError(t *testing.T) {
	c := &Collector{
		RequestConfig: web.RequestConfig{
			URL: "http://example.com",
		},
		UpdateEvery: 10,
	}

	mockClient := new(MockHTTPClient)
	c.httpClient = mockClient

	mockClient.On("Do", mock.Anything).Return(&http.Response{}, errors.New("network error"))

	metrics, err := c.collect()

	assert.NoError(t, err)
	assert.NotNil(t, metrics)
	assert.True(t, metrics["status.no_connection"] == 1)
}

func TestCollect_Timeout(t *testing.T) {
	c := &Collector{
		RequestConfig: web.RequestConfig{
			URL: "http://example.com",
		},
		UpdateEvery: 10,
	}

	mockClient := new(MockHTTPClient)
	c.httpClient = mockClient

	timeoutError := &timeoutError{}
	mockClient.On("Do", mock.Anything).Return(&http.Response{}, timeoutError)

	metrics, err := c.collect()

	assert.NoError(t, err)
	assert.NotNil(t, metrics)
	assert.True(t, metrics["status.timeout"] == 1)
}

type timeoutError struct{}

func (e *timeoutError) Error() string   { return "timeout" }
func (e *timeoutError) Timeout() bool   { return true }
func (e *timeoutError) Temporary() bool { return false }

func TestCollect_BadStatus(t *testing.T) {
	c := &Collector{
		RequestConfig: web.RequestConfig{
			URL: "http://example.com",
		},
		UpdateEvery:      10,
		acceptedStatuses: map[int]bool{200: true},
	}

	mockClient := new(MockHTTPClient)
	c.httpClient = mockClient

	body := io.NopCloser(bytes.NewReader([]byte("test response")))
	resp := &http.Response{
		StatusCode: 404,
		Body:       body,
		Header:     make(http.Header),
	}

	mockClient.On("Do", mock.Anything).Return(resp, nil)

	metrics, err := c.collect()

	assert.NoError(t, err)
	assert.NotNil(t, metrics)
	assert.True(t, metrics["status.bad_status"] == 1)
}

func TestDurationToMs(t *testing.T) {
	testCases := []struct {
		name     string
		duration time.Duration
		expected int
	}{
		{"Zero duration", 0, 0},
		{"1 millisecond", time.Millisecond, 1},
		{"500 milliseconds", 500 * time.Millisecond, 500},
		{"1 second", time.Second, 1000},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := durationToMs(tc.duration)
			assert.Equal(t, tc.expected, result)
		})
	}
}
```