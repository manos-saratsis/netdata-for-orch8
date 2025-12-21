package metrix

import (
	"testing"

	"github.com/axiomhq/hyperloglog"
)

func TestNewUniqueCounter(t *testing.T) {
	testCases := []struct {
		name           string
		useHyperLogLog bool
		expectedType   string
	}{
		{"MapUniqueCounter", false, "mapUniqueCounter"},
		{"HyperLogLogUniqueCounter", true, "hyperLogLogUniqueCounter"},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			counter := NewUniqueCounter(tc.useHyperLogLog)

			switch tc.expectedType {
			case "mapUniqueCounter":
				if _, ok := counter.(mapUniqueCounter); !ok {
					t.Errorf("Expected mapUniqueCounter when useHyperLogLog is false")
				}
			case "hyperLogLogUniqueCounter":
				if _, ok := counter.(*hyperLogLogUniqueCounter); !ok {
					t.Errorf("Expected hyperLogLogUniqueCounter when useHyperLogLog is true")
				}
			}
		})
	}
}

func TestMapUniqueCounter(t *testing.T) {
	counter := NewUniqueCounter(false)

	// Test Insert and Value
	counter.Insert("item1")
	counter.Insert("item2")
	counter.Insert("item1")

	if counter.Value() != 2 {
		t.Errorf("Expected count 2, got %d", counter.Value())
	}

	// Test Reset
	counter.Reset()
	if counter.Value() != 0 {
		t.Errorf("Expected count 0 after reset, got %d", counter.Value())
	}

	// Test WriteTo
	rv := make(map[string]int64)
	counter.Insert("test")
	counter.WriteTo(rv, "test_key", 1, 1)

	expectedValue := int64(1)
	if rv["test_key"] != expectedValue {
		t.Errorf("WriteTo failed. Expected %v, got %v", expectedValue, rv["test_key"])
	}
}

func TestHyperLogLogUniqueCounter(t *testing.T) {
	counter := NewUniqueCounter(true)

	// Test Insert and Value with HyperLogLog
	counter.Insert("item1")
	counter.Insert("item2")
	counter.Insert("item1")

	if counter.Value() != 2 {
		t.Errorf("Expected approximate count 2, got %d", counter.Value())
	}

	// Test Reset
	counter.Reset()
	if counter.Value() != 0 {
		t.Errorf("Expected count 0 after reset, got %d", counter.Value())
	}

	// Test WriteTo
	rv := make(map[string]int64)
	counter.Insert("test")
	counter.WriteTo(rv, "test_key", 1, 1)

	expectedValue := int64(1)
	if rv["test_key"] != expectedValue {
		t.Errorf("WriteTo failed. Expected %v, got %v", expectedValue, rv["test_key"])
	}
}

func TestUniqueCounterVec(t *testing.T) {
	vec := NewUniqueCounterVec(false)

	// Test Get - new key
	c1 := vec.Get("key1")
	if c1 == nil {
		t.Error("Get should create and return a new unique counter")
	}

	// Test Get - existing key
	c2 := vec.Get("key1")
	if c1 != c2 {
		t.Error("Get should return the same unique counter for an existing key")
	}

	// Test Reset
	c1.Insert("test")
	vec.Reset()

	if c1.Value() != 0 {
		t.Error("Reset should clear all counters in the vector")
	}

	// Test WriteTo
	vec.Get("key1").Insert("item1")
	vec.Get("key2").Insert("item2")

	rv := make(map[string]int64)
	vec.WriteTo(rv, "test_unique", 1, 1)

	expectedKeys := []string{
		"test_unique_key1",
		"test_unique_key2",
	}

	for _, key := range expectedKeys {
		if _, exists := rv[key]; !exists {
			t.Errorf("Expected key %s not found", key)
		}
	}
}