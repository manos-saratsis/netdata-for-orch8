```go
package jmx

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestNew(t *testing.T) {
	c := New()
	assert.NotNil(t, c, "New() should return a non-nil Collector")
	assert.NotNil(t, c.Collector, "Collector's base Collector should be initialized")
	assert.NotNil(t, c.Config, "Config should be initialized")
}

func TestConfiguration(t *testing.T) {
	c := New()
	cfg := c.Configuration()
	assert.NotNil(t, cfg, "Configuration() should return a non-nil config")
	
	// Type assertion to check returned config type
	configCopy, ok := cfg.(*Config)
	assert.True(t, ok, "Configuration() should return a *Config type")
	assert.Equal(t, c.Config, configCopy, "Returned config should be a copy of original")
}

func TestModuleRegistration(t *testing.T) {
	creator, exists := module.GetRegisteredModuleCreator("websphere_jmx")
	assert.True(t, exists, "websphere_jmx module should be registered")
	
	// Test creating a module
	mod := creator.Create()
	assert.NotNil(t, mod, "Module creator should create a non-nil module")
}
```