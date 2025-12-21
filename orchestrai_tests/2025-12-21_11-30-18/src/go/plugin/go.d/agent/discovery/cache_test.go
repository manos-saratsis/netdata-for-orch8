package discovery

import (
	"testing"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/confgroup"
)

func TestNewCache(t *testing.T) {
	t.Run("CreateEmptyCache", func(t *testing.T) {
		c := newCache()
		if c == nil {
			t.Errorf("Expected non-nil cache")
		}
		if len(*c) != 0 {
			t.Errorf("Expected empty cache")
		}
	})
}

func TestCacheUpdate(t *testing.T) {
	// Test updating with non-empty groups
	t.Run("UpdateWithGroups", func(t *testing.T) {
		c := newCache()
		groups := []*confgroup.Group{
			{Source: "source1"},
			{Source: "source2"},
		}
		
		c.update(groups)
		
		if len(*c) != 2 {
			t.Errorf("Expected 2 groups, got %d", len(*c))
		}
		
		if _, ok := (*c)["source1"]; !ok {
			t.Errorf("Expected source1 to be in cache")
		}
		if _, ok := (*c)["source2"]; !ok {
			t.Errorf("Expected source2 to be in cache")
		}
	})
	
	// Test updating with empty groups slice
	t.Run("UpdateWithEmptyGroups", func(t *testing.T) {
		c := newCache()
		groups := []*confgroup.Group{}
		
		c.update(groups)
		
		if len(*c) != 0 {
			t.Errorf("Expected empty cache after updating with empty groups")
		}
	})
	
	// Test updating with nil groups
	t.Run("UpdateWithNilGroups", func(t *testing.T) {
		c := newCache()
		groups := []*confgroup.Group{nil, nil}
		
		c.update(groups)
		
		if len(*c) != 0 {
			t.Errorf("Expected empty cache after updating with nil groups")
		}
	})
}

func TestCacheReset(t *testing.T) {
	t.Run("ResetNonEmptyCache", func(t *testing.T) {
		c := newCache()
		groups := []*confgroup.Group{
			{Source: "source1"},
			{Source: "source2"},
		}
		
		c.update(groups)
		c.reset()
		
		if len(*c) != 0 {
			t.Errorf("Expected empty cache after reset")
		}
	})
}

func TestCacheGroups(t *testing.T) {
	// Test retrieving groups from non-empty cache
	t.Run("RetrieveGroups", func(t *testing.T) {
		c := newCache()
		groups := []*confgroup.Group{
			{Source: "source1"},
			{Source: "source2"},
		}
		
		c.update(groups)
		
		retrievedGroups := c.groups()
		
		if len(retrievedGroups) != 2 {
			t.Errorf("Expected 2 groups, got %d", len(retrievedGroups))
		}
		
		// Check if retrieved groups match original
		found := make(map[string]bool)
		for _, g := range retrievedGroups {
			found[g.Source] = true
		}
		
		if !found["source1"] || !found["source2"] {
			t.Errorf("Not all source groups retrieved")
		}
	})
	
	// Test retrieving groups from empty cache
	t.Run("RetrieveFromEmptyCache", func(t *testing.T) {
		c := newCache()
		
		retrievedGroups := c.groups()
		
		if len(retrievedGroups) != 0 {
			t.Errorf("Expected empty groups list")
		}
	})
}