```go
package sql

import (
	"testing"
	"github.com/stretchr/testify/assert"
)

func TestSupportedDrivers(t *testing.T) {
	expectedDrivers := []string{
		"mysql",
		"oracle", 
		"pgx", 
		"sqlserver",
	}

	for _, driver := range expectedDrivers {
		t.Run("Check Driver: " + driver, func(t *testing.T) {
			assert.True(t, supportedDrivers[driver], "Driver %s should be supported", driver)
		})
	}

	// Test unsupported drivers
	unsupportedDrivers := []string{
		"sqlite", 
		"mongodb", 
		"cassandra",
	}

	for _, driver := range unsupportedDrivers {
		t.Run("Check Unsupported Driver: " + driver, func(t *testing.T) {
			assert.False(t, supportedDrivers[driver], "Driver %s should not be supported", driver)
		})
	}
}
```