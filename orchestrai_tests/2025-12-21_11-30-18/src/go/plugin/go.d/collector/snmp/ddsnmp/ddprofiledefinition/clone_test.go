package ddprofiledefinition

import (
	"reflect"
	"testing"
)

// Mock cloneable type for testing
type testCloneable struct {
	value string
}

func (t testCloneable) Clone() testCloneable {
	return testCloneable{value: t.value}
}

func TestCloneSlice(t *testing.T) {
	// Test nil slice
	var nilSlice []testCloneable
	clonedNilSlice := CloneSlice(nilSlice)
	if clonedNilSlice != nil {
		t.Errorf("CloneSlice of nil slice should return nil")
	}

	// Test empty slice
	emptySlice := []testCloneable{}
	clonedEmptySlice := CloneSlice(emptySlice)
	if len(clonedEmptySlice) != 0 {
		t.Errorf("CloneSlice of empty slice should return empty slice")
	}

	// Test slice with elements
	originalSlice := []testCloneable{
		{value: "first"},
		{value: "second"},
	}
	clonedSlice := CloneSlice(originalSlice)

	// Check length
	if len(clonedSlice) != len(originalSlice) {
		t.Errorf("Cloned slice length should match original")
	}

	// Check each element is a clone (same value, different reference)
	for i := range originalSlice {
		if clonedSlice[i].value != originalSlice[i].value {
			t.Errorf("Cloned slice element value should match original")
		}
		if &clonedSlice[i] == &originalSlice[i] {
			t.Errorf("Cloned slice element should be a new instance")
		}
	}
}

func TestCloneMap(t *testing.T) {
	// Test nil map
	var nilMap map[string]testCloneable
	clonedNilMap := CloneMap(nilMap)
	if clonedNilMap != nil {
		t.Errorf("CloneMap of nil map should return nil")
	}

	// Test empty map
	emptyMap := map[string]testCloneable{}
	clonedEmptyMap := CloneMap(emptyMap)
	if len(clonedEmptyMap) != 0 {
		t.Errorf("CloneMap of empty map should return empty map")
	}

	// Test map with elements
	originalMap := map[string]testCloneable{
		"key1": {value: "first"},
		"key2": {value: "second"},
	}
	clonedMap := CloneMap(originalMap)

	// Check length
	if len(clonedMap) != len(originalMap) {
		t.Errorf("Cloned map length should match original")
	}

	// Check each element is a clone (same value, different reference)
	for k := range originalMap {
		if clonedMap[k].value != originalMap[k].value {
			t.Errorf("Cloned map element value should match original")
		}
		if &clonedMap[k] == &originalMap[k] {
			t.Errorf("Cloned map element should be a new instance")
		}
	}
}