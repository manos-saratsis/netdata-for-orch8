```go
package dbdriver

import (
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestConnectionConfig_SetDefaults(t *testing.T) {
	testCases := []struct {
		name     string
		input    *ConnectionConfig
		expected *ConnectionConfig
	}{
		{
			name: "Default values for empty config",
			input: &ConnectionConfig{},
			expected: &ConnectionConfig{
				Timeout:           30 * time.Second,
				MaxOpenConns:      1,
				ConnMaxLifetime:   10 * time.Minute,
				Port:              50000,
			},
		},
		{
			name: "Default values for AS/400",
			input: &ConnectionConfig{
				SystemType: "AS400",
			},
			expected: &ConnectionConfig{
				SystemType:        "AS400",
				Timeout:           30 * time.Second,
				MaxOpenConns:      1,
				ConnMaxLifetime:   10 * time.Minute,
				Port:              8471,
				Database:          "*SYSBAS",
			},
		},
		{
			name: "Do not override existing values",
			input: &ConnectionConfig{
				Timeout:           5 * time.Second,
				MaxOpenConns:      5,
				ConnMaxLifetime:   1 * time.Hour,
				Port:              1234,
				Database:          "TestDB",
			},
			expected: &ConnectionConfig{
				Timeout:           5 * time.Second,
				MaxOpenConns:      5,
				ConnMaxLifetime:   1 * time.Hour,
				Port:              1234,
				Database:          "TestDB",
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			tc.input.SetDefaults()
			assert.Equal(t, tc.expected, tc.input)
		})
	}
}
```