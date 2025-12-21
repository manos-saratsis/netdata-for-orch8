package confgroup

import (
	"testing"
)

func TestRegistryRegister(t *testing.T) {
	// Test registering with non-empty name
	t.Run("RegisterWithNonEmptyName", func(t *testing.T) {
		registry := make(Registry)
		def := Default{
			MinUpdateEvery:     10,
			UpdateEvery:        20,
			AutoDetectionRetry: 5,
			Priority:           1,
		}
		
		registry.Register("test", def)
		
		// Verify registration
		retrievedDef, ok := registry.Lookup("test")
		if !ok {
			t.Errorf("Expected 'test' to be registered")
		}
		if retrievedDef != def {
			t.Errorf("Registered default does not match original")
		}
	})
	
	// Test registering with empty name
	t.Run("RegisterWithEmptyName", func(t *testing.T) {
		registry := make(Registry)
		def := Default{
			MinUpdateEvery:     10,
			UpdateEvery:        20,
			AutoDetectionRetry: 5,
			Priority:           1,
		}
		
		registry.Register("", def)
		
		// Verify no registration occurred
		if len(registry) != 0 {
			t.Errorf("Expected empty registry when registering with empty name")
		}
	})
}

func TestRegistryLookup(t *testing.T) {
	// Test lookup with existing key
	t.Run("LookupExistingKey", func(t *testing.T) {
		registry := make(Registry)
		def := Default{
			MinUpdateEvery:     10,
			UpdateEvery:        20,
			AutoDetectionRetry: 5,
			Priority:           1,
		}
		
		registry.Register("test", def)
		
		retrievedDef, ok := registry.Lookup("test")
		if !ok {
			t.Errorf("Expected to find 'test' key")
		}
		if retrievedDef != def {
			t.Errorf("Retrieved default does not match registered default")
		}
	})
	
	// Test lookup with non-existing key
	t.Run("LookupNonExistingKey", func(t *testing.T) {
		registry := make(Registry)
		
		_, ok := registry.Lookup("nonexistent")
		if ok {
			t.Errorf("Expected lookup of non-existing key to return false")
		}
	})
}