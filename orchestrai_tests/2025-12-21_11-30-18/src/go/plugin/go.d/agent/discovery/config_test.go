package discovery

import (
	"testing"
	
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/confgroup"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/discovery/dummy"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/discovery/file"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/discovery/sd"
)

func TestValidateConfig(t *testing.T) {
	// Test successful validation
	t.Run("ValidConfigWithRegistry", func(t *testing.T) {
		cfg := Config{
			Registry: make(confgroup.Registry),
			File: file.Config{
				Read:  []string{"/path/to/read"},
				Watch: []string{"/path/to/watch"},
			},
			Dummy: dummy.Config{},
			SD:    sd.Config{},
		}
		
		// Add an entry to the registry
		cfg.Registry.Register("test", confgroup.Default{})
		
		err := validateConfig(cfg)
		if err != nil {
			t.Errorf("Expected no error, got: %v", err)
		}
	})
	
	// Test validation with file discoverers
	t.Run("ValidConfigWithFileDiscoverers", func(t *testing.T) {
		cfg := Config{
			Registry: make(confgroup.Registry),
			File: file.Config{
				Read: []string{"/path/to/read"},
			},
			Dummy: dummy.Config{},
			SD:    sd.Config{},
		}
		
		// Add an entry to the registry
		cfg.Registry.Register("test", confgroup.Default{})
		
		err := validateConfig(cfg)
		if err != nil {
			t.Errorf("Expected no error, got: %v", err)
		}
	})
	
	// Test validation with dummy discoverers
	t.Run("ValidConfigWithDummyDiscoverers", func(t *testing.T) {
		cfg := Config{
			Registry: make(confgroup.Registry),
			File:     file.Config{},
			Dummy: dummy.Config{
				Names: []string{"dummy1"},
			},
			SD: sd.Config{},
		}
		
		// Add an entry to the registry
		cfg.Registry.Register("test", confgroup.Default{})
		
		err := validateConfig(cfg)
		if err != nil {
			t.Errorf("Expected no error, got: %v", err)
		}
	})
	
	// Test validation with empty registry
	t.Run("EmptyRegistryConfig", func(t *testing.T) {
		cfg := Config{
			Registry: make(confgroup.Registry),
			File:     file.Config{},
			Dummy:    dummy.Config{},
			SD:       sd.Config{},
		}
		
		err := validateConfig(cfg)
		if err == nil {
			t.Errorf("Expected error for empty registry")
		}
	})
	
	// Test validation with no discoverers
	t.Run("NoDiscoverersConfig", func(t *testing.T) {
		cfg := Config{
			Registry: confgroup.Registry{"test": confgroup.Default{}},
			File:     file.Config{},
			Dummy:    dummy.Config{},
			SD:       sd.Config{},
		}
		
		err := validateConfig(cfg)
		if err == nil {
			t.Errorf("Expected error for no discoverers")
		}
	})
}