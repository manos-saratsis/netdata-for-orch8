package zfspool_test

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestParseZpoolListOutput(t *testing.T) {
	testCases := []struct {
		name          string
		input         []byte
		expectError   bool
		expectedPools int
	}{
		{
			name: "Valid Single Pool",
			input: []byte(`NAME    SIZE       ALLOC    FREE  FRAG  CAP  HEALTH
rpool   10000000   5000000  5000000   10   50  ONLINE`),
			expectedPools: 1,
		},
		{
			name: "Valid Multiple Pools",
			input: []byte(`NAME    SIZE       ALLOC    FREE  FRAG  CAP  HEALTH
rpool   10000000   5000000  5000000   10   50  ONLINE
zion    20000000   8000000  12000000  20   40  DEGRADED`),
			expectedPools: 2,
		},
		{
			name:          "Empty Input",
			input:         []byte{},
			expectError:   true,
			expectedPools: 0,
		},
		{
			name: "Invalid Header",
			input: []byte(`INVALID HEADER
rpool   10000000   5000000  5000000   10   50  ONLINE`),
			expectError:   true,
			expectedPools: 0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			pools, err := zfspool.ParseZpoolListOutput(tc.input)

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Len(t, pools, tc.expectedPools)
			}
		})
	}
}

func TestParseInt(t *testing.T) {
	testCases := []struct {
		input    string
		expected int64
		ok       bool
	}{
		{"100", 100, true},
		{"-", 0, false},
		{"invalid", 0, false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result, ok := zfspool.ParseInt(tc.input)
			assert.Equal(t, tc.expected, result)
			assert.Equal(t, tc.ok, ok)
		})
	}
}

func TestParseFloat(t *testing.T) {
	testCases := []struct {
		input    string
		expected float64
		ok       bool
	}{
		{"50.5", 50.5, true},
		{"-", 0, false},
		{"invalid", 0, false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result, ok := zfspool.ParseFloat(tc.input)
			assert.Equal(t, tc.expected, result)
			assert.Equal(t, tc.ok, ok)
		})
	}
}