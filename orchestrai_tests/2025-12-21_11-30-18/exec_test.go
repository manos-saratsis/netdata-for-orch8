```go
package smartctl

import (
	"context"
	"errors"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/tidwall/gjson"
)

func TestParseOutput(t *testing.T) {
	testCases := []struct {
		name           string
		input          string
		expectedErr    bool
		errorSubstring string
	}{
		{
			name:  "Valid JSON with Exit Status",
			input: `{"smartctl": {"exit_status": 0, "messages": []}}`,
		},
		{
			name:  "JSON with Error Message",
			input: `{"smartctl": {"exit_status": 0, "messages": [{"severity": "error", "string": "Test Error"}]}}`,
			expectedErr:    true,
			errorSubstring: "reported an error",
		},
		{
			name:           "Empty Input",
			input:          "",
			expectedErr:    true,
			errorSubstring: "returned no output",
		},
		{
			name:           "Invalid JSON",
			input:          "{invalid}",
			expectedErr:    true,
			errorSubstring: "returned invalid JSON",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result, err := parseOutput("test_cmd", []byte(tc.input), nil)
			if tc.expectedErr {
				assert.Error(t, err)
				if tc.errorSubstring != "" {
					assert.Contains(t, err.Error(), tc.errorSubstring)
				}
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, result)
			}
		})
	}
}

func TestExecExitCode(t *testing.T) {
	testCases := []struct {
		name     string
		err      error
		exitCode int
		expected bool
	}{
		{
			name:     "Matching Exit Code",
			err:      &exec.ExitError{Stderr: []byte("test")},
			exitCode: 1,
			expected: true,
		},
		{
			name:     "Non-Matching Exit Code",
			err:      &exec.ExitError{Stderr: []byte("test")},
			exitCode: 2,
			expected: false,
		},
		{
			name:     "Non-Exit Error",
			err:      errors.New("generic error"),
			exitCode: 1,
			expected: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := isExecExitCode(tc.err, tc.exitCode)
			assert.Equal(t, tc.expected, result)
		})
	}
}
```