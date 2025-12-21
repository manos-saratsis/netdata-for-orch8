```go
package jmx

import (
	"context"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/pkg/confopt"
	"github.com/stretchr/testify/assert"
)

func TestDefaultConfig(t *testing.T) {
	cfg := defaultConfig()
	
	// Check default values
	assert.Equal(t, int(defaultUpdateEvery/time.Second), cfg.UpdateEvery)
	assert.Equal(t, 60, cfg.ObsoletionIterations)
	assert.Equal(t, confopt.AutoBoolEnabled, cfg.CollectJVMMetrics)
	
	// Check timeouts and delays have valid defaults
	assert.Greater(t, cfg.JMXTimeout.Duration(), time.Duration(0))
	assert.Greater(t, cfg.InitTimeout.Duration(), time.Duration(0))
	assert.Greater(t, cfg.ShutdownDelay.Duration(), time.Duration(0))
}

func TestCollectorInit_MissingJMXURL(t *testing.T) {
	c := New()
	c.Config.JMXURL = ""
	
	err := c.Init(context.Background())
	assert.Error(t, err, "Init() should fail with empty JMXURL")
	assert.Contains(t, err.Error(), "jmx_url is required")
}

func TestCollectorInit_AdjustTimeouts(t *testing.T) {
	c := New()
	c.Config.JMXURL = "service:jmx:rmi:///jndi/rmi://localhost:9010/jmxrmi"
	c.Config.UpdateEvery = 0
	c.Config.JMXTimeout = 0
	c.Config.InitTimeout = 0
	c.Config.ShutdownDelay = 0
	
	err := c.Init(context.Background())
	
	assert.NoError(t, err, "Init() should succeed")
	assert.Equal(t, int(defaultUpdateEvery/time.Second), c.Config.UpdateEvery)
	assert.Equal(t, confopt.Duration(defaultScrapeTimeout), c.Config.JMXTimeout)
	assert.Equal(t, confopt.Duration(defaultInitTimeout), c.Config.InitTimeout)
	assert.Equal(t, confopt.Duration(defaultShutdownDelay), c.Config.ShutdownDelay)
}

func TestCollectorInit_CustomLabels(t *testing.T) {
	c := New()
	c.Config.JMXURL = "service:jmx:rmi:///jndi/rmi://localhost:9010/jmxrmi"
	c.Config.CustomLabels = map[string]string{
		"custom_key": "custom_value",
		"":           "invalid_key",
	}
	c.Config.ServerType = "test_server"
	
	err := c.Init(context.Background())
	
	assert.NoError(t, err, "Init() should succeed")
	
	// Validate global labels
	labels := c.GetGlobalLabels()
	assert.Equal(t, "custom_value", labels["custom_key"])
	assert.Equal(t, "test_server", labels["server_type"])
}

func TestCollectorCleanup(t *testing.T) {
	c := New()
	// Note: This test requires a mock implementation of JMX client
	// For now, just ensure no panic occurs
	c.Cleanup(context.Background())
}
```