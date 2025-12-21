package metrix

import (
	"testing"
)

func TestBool(t *testing.T) {
	tests := []struct {
		name     string
		input    bool
		expected int64
	}{
		{
			name:     "True value",
			input:    true,
			expected: 1,
		},
		{
			name:     "False value",
			input:    false,
			expected: 0,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := Bool(tt.input)
			if result != tt.expected {
				t.Errorf("Bool(%v) = %v, want %v", tt.input, result, tt.expected)
			}
		})
	}
}

// Verify Observer interface implementation
var _ Observer = (*stm.Value)(nil)