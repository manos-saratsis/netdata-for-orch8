package metrix

import (
	"math"
	"testing"
)

func TestNewSummary(t *testing.T) {
	s := NewSummary()
	summary, ok := s.(*summary)
	if !ok {
		t.Fatal("NewSummary did not return *summary")
	}

	if summary.min != math.MaxFloat64 {
		t.Errorf("Initial min should be MaxFloat64, got %v", summary.min)
	}
	if summary.max != -math.MaxFloat64 {
		t.Errorf("Initial max should be -MaxFloat64, got %v", summary.max)
	}
	if summary.sum != 0 {
		t.Errorf("Initial sum should be 0, got %v", summary.sum)
	}
	if summary.count != 0 {
		t.Errorf("Initial count should be 0, got %v", summary.count)
	}
}

func TestSummaryObserve(t *testing.T) {
	s := NewSummary()
	summary := s.(*summary)

	testCases := []struct {
		value    float64
		expMin   float64
		expMax   float64
		expSum   float64
		expCount int64
	}{
		{1.0, 1.0, 1.0, 1.0, 1},
		{2.0, 1.0, 2.0, 3.0, 2},
		{0.5, 0.5, 2.0, 3.5, 3},
	}

	for _, tc := range testCases {
		s.Observe(tc.value)
		if summary.min != tc.expMin {
			t.Errorf("Unexpected min: got %v, want %v", summary.min, tc.expMin)
		}
		if summary.max != tc.expMax {
			t.Errorf("Unexpected max: got %v, want %v", summary.max, tc.expMax)
		}
		if summary.sum != tc.expSum {
			t.Errorf("Unexpected sum: got %v, want %v", summary.sum, tc.expSum)
		}
		if summary.count != tc.expCount {
			t.Errorf("Unexpected count: got %v, want %v", summary.count, tc.expCount)
		}
	}
}

func TestSummaryReset(t *testing.T) {
	s := NewSummary()
	summary := s.(*summary)

	s.Observe(10.0)
	s.Reset()

	if summary.min != math.MaxFloat64 {
		t.Errorf("After reset, min should be MaxFloat64, got %v", summary.min)
	}
	if summary.max != -math.MaxFloat64 {
		t.Errorf("After reset, max should be -MaxFloat64, got %v", summary.max)
	}
	if summary.sum != 0 {
		t.Errorf("After reset, sum should be 0, got %v", summary.sum)
	}
	if summary.count != 0 {
		t.Errorf("After reset, count should be 0, got %v", summary.count)
	}
}

func TestSummaryWriteTo(t *testing.T) {
	s := NewSummary()
	summary := s.(*summary)
	s.Observe(1.0)
	s.Observe(2.0)

	rv := make(map[string]int64)
	summary.WriteTo(rv, "test_key", 1, 1)

	expectedValues := map[string]int64{
		"test_key_min":   1,
		"test_key_max":   2,
		"test_key_sum":   3,
		"test_key_count": 2,
		"test_key_avg":   1,
	}

	for key, expectedVal := range expectedValues {
		if val, exists := rv[key]; !exists {
			t.Errorf("Expected key %s not found", key)
		} else if val != expectedVal {
			t.Errorf("For key %s, expected %v, got %v", key, expectedVal, val)
		}
	}
}

func TestSummaryVec(t *testing.T) {
	sv := NewSummaryVec()

	// Test Get - new key
	s1 := sv.Get("key1")
	if s1 == nil {
		t.Error("Get should create and return a new summary for a new key")
	}

	// Test Get - existing key
	s2 := sv.Get("key1")
	if s1 != s2 {
		t.Error("Get should return the same summary for an existing key")
	}

	// Test Reset
	s1.Observe(10.0)
	sv.Reset()

	summary1 := s1.(*summary)
	if summary1.count != 0 {
		t.Error("Reset should reset all summaries in the vector")
	}
}

func TestSummaryVecWriteTo(t *testing.T) {
	sv := NewSummaryVec()
	sv.Get("key1").Observe(1.0)
	sv.Get("key2").Observe(2.0)

	rv := make(map[string]int64)
	sv.WriteTo(rv, "test_summary", 1, 1)

	expectedKeys := []string{
		"test_summary_key1_min",
		"test_summary_key1_max",
		"test_summary_key1_sum",
		"test_summary_key1_count",
		"test_summary_key1_avg",
		"test_summary_key2_min",
		"test_summary_key2_max",
		"test_summary_key2_sum",
		"test_summary_key2_count",
		"test_summary_key2_avg",
	}

	for _, key := range expectedKeys {
		if _, exists := rv[key]; !exists {
			t.Errorf("Expected key %s not found", key)
		}
	}
}