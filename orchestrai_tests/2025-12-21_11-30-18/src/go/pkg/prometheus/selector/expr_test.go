```go
package selector

import (
	"testing"
	"reflect"
)

func TestExprEmpty(t *testing.T) {
	testCases := []struct {
		name     string
		expr     Expr
		expected bool
	}{
		{
			name:     "Empty when no allow or deny",
			expr:     Expr{},
			expected: true,
		},
		{
			name:     "Not empty with allow",
			expr:     Expr{Allow: []string{"test"}},
			expected: false,
		},
		{
			name:     "Not empty with deny",
			expr:     Expr{Deny: []string{"test"}},
			expected: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := tc.expr.Empty()
			if result != tc.expected {
				t.Errorf("Expected %v, got %v", tc.expected, result)
			}
		})
	}
}

func TestExprParse(t *testing.T) {
	testCases := []struct {
		name        string
		expr        Expr
		expectError bool
		description string
	}{
		{
			name:        "Empty expression returns nil",
			expr:        Expr{},
			expectError: false,
			description: "Empty expression should return nil selector",
		},
		{
			name: "Valid single allow selector",
			expr: Expr{
				Allow: []string{"job=test"},
			},
			expectError: false,
			description: "Single valid allow selector",
		},
		{
			name: "Multiple allow selectors",
			expr: Expr{
				Allow: []string{"job=test1", "job=test2", "job=test3"},
			},
			expectError: false,
			description: "Multiple allow selectors should create OR selector",
		},
		{
			name: "Invalid allow selector",
			expr: Expr{
				Allow: []string{"invalid selector"},
			},
			expectError: true,
			description: "Invalid selector should return error",
		},
		{
			name: "Allow and Deny selectors",
			expr: Expr{
				Allow: []string{"job=test"},
				Deny:  []string{"instance=dev"},
			},
			expectError: false,
			description: "Mixed allow and deny selectors",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result, err := tc.expr.Parse()

			if tc.expectError {
				if err == nil {
					t.Errorf("%s: Expected error, got none", tc.description)
				}
				return
			}

			if err != nil {
				t.Errorf("%s: Unexpected error: %v", tc.description, err)
				return
			}

			if tc.expr.Empty() {
				if result != nil {
					t.Errorf("Empty expression should return nil selector")
				}
			} else {
				if result == nil {
					t.Errorf("Expected non-nil selector")
				}
			}
		})
	}
}
```