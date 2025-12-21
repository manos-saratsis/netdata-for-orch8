```go
package httpcheck

import (
	"errors"
	"net"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
)

func TestDecodeReqError(t *testing.T) {
	testCases := []struct {
		name     string
		err      error
		expected reqErrCode
	}{
		{
			name:     "Redirect",
			err:      web.ErrRedirectAttempted,
			expected: codeRedirect,
		},
		{
			name:     "Timeout",
			err:      &timeoutNetError{},
			expected: codeTimeout,
		},
		{
			name:     "No Connection",
			err:      errors.New("some network error"),
			expected: codeNoConnection,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := decodeReqError(tc.err)
			assert.Equal(t, tc.expected, result)
		})
	}

	// Test panic case
	t.Run("Nil Error Panic", func(t *testing.T) {
		assert.Panics(t, func() {
			decodeReqError(nil)
		})
	})
}

type timeoutNetError struct{}

func (e *timeoutNetError) Error() string   { return "timeout error" }
func (e *timeoutNetError) Timeout() bool   { return true }
func (e *timeoutNetError) Temporary() bool { return false }

func TestIsError(t *testing.T) {
	testCases := []struct {
		name           string
		err            error
		resp           *http.Response
		acceptedStatus map[int]bool
		expected       bool
	}{
		{
			name: "Regular Error",
			err:  errors.New("some error"),
			resp: &http.Response{StatusCode: 200},
			expected: true,
		},
		{
			name:           "Redirect with Accepted Status",
			err:            web.ErrRedirectAttempted,
			resp:           &http.Response{StatusCode: 200},
			acceptedStatus: map[int]bool{200: true},
			expected:       false,
		},
		{
			name:           "Redirect with Unaccepted Status",
			err:            web.ErrRedirectAttempted,
			resp:           &http.Response{StatusCode: 404},
			acceptedStatus: map[int]bool{200: true},
			expected:       true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{acceptedStatuses: tc.acceptedStatus}
			result := c.isError(tc.err, tc.resp)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestCheckHeader(t *testing.T) {
	testCases := []struct {
		name    string
		headers http.Header
		matches []headerMatch
		expect  bool
	}{
		{
			name: "Header match with value",
			headers: http.Header{
				"Content-Type": []string{"application/json"},
			},
			matches: []headerMatch{
				{key: "Content-Type", valMatcher: nil, exclude: false},
			},
			expect: true,
		},
		{
			name:    "Empty header when expecting a match",
			headers: http.Header{},
			matches: []headerMatch{
				{key: "Content-Type", valMatcher: nil, exclude: false},
			},
			expect: false,
		},
		{
			name: "Regex header match",
			headers: http.Header{
				"Content-Type": []string{"application/json"},
			},
			matches: []headerMatch{
				{key: "Content-Type", valMatcher: nil, exclude: false},
			},
			expect: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{headerMatch: tc.matches}
			resp := &http.Response{Header: tc.headers}
			result := c.checkHeader(resp)
			assert.Equal(t, tc.expect, result)
		})
	}
}
```