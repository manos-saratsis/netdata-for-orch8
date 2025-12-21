```go
package dmcache

import (
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestParseDmsetupStatusLine(t *testing.T) {
	testCases := []struct {
		name          string
		input         string
		expectedError bool
		validate      func(*dmCacheDevice, error)
	}{
		{
			name:          "Valid line",
			input:         "test-device: 0 0 cache 262144 0/1024 262144 0/1024 123456 7890 12345 6789 98765 54321 12345",
			expectedError: false,
			validate: func(dev *dmCacheDevice, err error) {
				require.NoError(t, err)
				assert.Equal(t, "test-device", dev.name)
				assert.Equal(t, int64(262144), dev.metaBlockSizeSectors)
				assert.Equal(t, int64(0), dev.metaUsedBlocks)
				assert.Equal(t, int64(1024), dev.metaTotalBlocks)
			},
		},
		{
			name:          "Too few fields",
			input:         "test-device: 0 0 cache",
			expectedError: true,
		},
		{
			name:          "Invalid numeric field",
			input:         "test-device: 0 0 cache 262144 0/1024 262144 0/1024 abc def ghi jkl mno pqr stu",
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			dev, err := parseDmsetupStatusLine(tc.input)

			if tc.expectedError {
				assert.Error(t, err)
				return
			}

			if tc.validate != nil {
				tc.validate(dev, err)
			}
		})
	}
}

func TestParseUsedTotalBlocks(t *testing.T) {
	testCases := []struct {
		name          string
		input         string
		expectedError bool
		expectedUsed  int64
		expectedTotal int64
	}{
		{
			name:          "Valid input",
			input:         "100/1000",
			expectedError: false,
			expectedUsed:  100,
			expectedTotal: 1000,
		},
		{
			name:          "Invalid format",
			input:         "100",
			expectedError: true,
		},
		{
			name:          "Invalid numeric values",
			input:         "abc/def",
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			used, total, err := parseUsedTotalBlocks(tc.input)

			if tc.expectedError {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			assert.Equal(t, tc.expectedUsed, used)
			assert.Equal(t, tc.expectedTotal, total)
		})
	}
}

func TestParseInt(t *testing.T) {
	testCases := []struct {
		name          string
		input         string
		expectedError bool
		expectedValue int64
	}{
		{
			name:          "Valid positive integer",
			input:         "12345",
			expectedError: false,
			expectedValue: 12345,
		},
		{
			name:          "Valid zero",
			input:         "0",
			expectedError: false,
			expectedValue: 0,
		},
		{
			name:          "Invalid input",
			input:         "abc",
			expectedError: true,
		},
		{
			name:          "Floating point",
			input:         "123.45",
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			val, err := parseInt(tc.input)

			if tc.expectedError {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			assert.Equal(t, tc.expectedValue, val)
		})
	}
}

func TestCollectCacheStatus(t *testing.T) {
	testCases := []struct {
		name           string
		input          []byte
		expectedError  bool
		validateMetrics func(map[string]int64)
	}{
		{
			name:          "Single device status",
			input:         []byte("test-device: 0 0 cache 262144 0/1024 262144 0/1024 123456 7890 12345 6789 98765 54321 12345\n"),
			expectedError: false,
			validateMetrics: func(mx map[string]int64) {
				assert.Greater(t, len(mx), 0)
				assert.Contains(t, mx, "dmcache_device_test-device_metadata_free_bytes")
				assert.Contains(t, mx, "dmcache_device_test-device_cache_used_bytes")
			},
		},
		{
			name:          "No devices",
			input:         []byte(""),
			expectedError: true,
		},
		{
			name:          "Invalid line",
			input:         []byte("invalid input\n"),
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				devices: make(map[string]bool),
				charts:  &module.Charts{},
			}

			mx := make(map[string]int64)
			err := collector.collectCacheStatus(mx, tc.input)

			if tc.expectedError {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			if tc.validateMetrics != nil {
				tc.validateMetrics(mx)
			}
		})
	}
}
```