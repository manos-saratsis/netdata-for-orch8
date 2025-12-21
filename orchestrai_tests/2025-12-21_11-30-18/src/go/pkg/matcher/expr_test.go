```go
package matcher

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestSimpleExpr_Empty(t *testing.T) {
	testCases := []struct {
		name     string
		expr     *SimpleExpr
		expected bool
	}{
		{
			name:     "Empty when no includes and no excludes",
			expr:     &SimpleExpr{},
			expected: true,
		},
		{
			name:     "Not empty with includes",
			expr:     &SimpleExpr{Includes: []string{"test"}},
			expected: false,
		},
		{
			name:     "Not empty with excludes",
			expr:     &SimpleExpr{Excludes: []string{"test"}},
			expected: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			assert.Equal(t, tc.expected, tc.expr.Empty())
		})
	}
}

func TestSimpleExpr_Parse(t *testing.T) {
	testCases := []struct {
		name        string
		expr        *SimpleExpr
		expectError bool
		errorMsg    string
	}{
		{
			name:        "Error on empty expression",
			expr:        &SimpleExpr{},
			expectError: true,
			errorMsg:    "empty expression",
		},
		{
			name:        "Invalid include pattern",
			expr:        &SimpleExpr{Includes: []string{"[invalid"}},
			expectError: true,
		},
		{
			name:        "Invalid exclude pattern",
			expr:        &SimpleExpr{Excludes: []string{"[invalid"}},
			expectError: true,
		},
		{
			name:        "Valid includes and no excludes",
			expr:        &SimpleExpr{Includes: []string{"test*"}},
			expectError: false,
		},
		{
			name:        "No includes with valid excludes",
			expr:        &SimpleExpr{Excludes: []string{"test*"}},
			expectError: false,
		},
		{
			name:        "Valid includes and excludes",
			expr:        &SimpleExpr{Includes: []string{"test*"}, Excludes: []string{"excluded*"}},
			expectError: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			matcher, err := tc.expr.Parse()
			if tc.expectError {
				assert.Error(t, err)
				if tc.errorMsg != "" {
					assert.Contains(t, err.Error(), tc.errorMsg)
				}
				assert.Nil(t, matcher)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, matcher)
			}
		})
	}
}

func TestSimpleExpr_ParseMatcherValidation(t *testing.T) {
	testCases := []struct {
		name           string
		includes       []string
		excludes       []string
		matchingValues []string
		nonMatchingValues []string
	}{
		{
			name:           "Include matching",
			includes:       []string{"test*"},
			matchingValues: []string{"test123", "testABC"},
			nonMatchingValues: []string{"something", "else"},
		},
		{
			name:           "Exclude matching",
			excludes:       []string{"bad*"},
			matchingValues: []string{"good", "great"},
			nonMatchingValues: []string{"bad", "badstuff"},
		},
		{
			name:           "Include and Exclude combined",
			includes:       []string{"test*"},
			excludes:       []string{"*exclude"},
			matchingValues: []string{"test123"},
			nonMatchingValues: []string{"test_exclude", "something_else"},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			expr := &SimpleExpr{
				Includes: tc.includes,
				Excludes: tc.excludes,
			}
			
			matcher, err := expr.Parse()
			assert.NoError(t, err)

			for _, val := range tc.matchingValues {
				assert.True(t, matcher.Match(val), "Expected '%s' to match", val)
			}

			for _, val := range tc.nonMatchingValues {
				assert.False(t, matcher.Match(val), "Expected '%s' to not match", val)
			}
		})
	}
}
```