```go
package matcher

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestMust(t *testing.T) {
	t.Run("Successful matcher", func(t *testing.T) {
		matcher, err := New(FmtString, "test")
		assert.NoError(t, err)

		assert.NotPanics(t, func() {
			Must(matcher, err)
		})
	})

	t.Run("Panic on error", func(t *testing.T) {
		assert.Panics(t, func() {
			Must(nil, assert.AnError)
		})
	})
}

func TestNew(t *testing.T) {
	testCases := []struct {
		name        string
		format      Format
		expr        string
		expectError bool
	}{
		{
			name:   "String matcher",
			format: FmtString,
			expr:   "hello",
		},
		{
			name:   "Glob matcher",
			format: FmtGlob,
			expr:   "test*",
		},
		{
			name:   "RegExp matcher",
			format: FmtRegExp,
			expr:   "^test.*",
		},
		{
			name:   "Simple Pattern matcher",
			format: FmtSimplePattern,
			expr:   "test",
		},
		{
			name:        "Unsupported format",
			format:      "invalid",
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			matcher, err := New(tc.format, tc.expr)
			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, matcher)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, matcher)
			}
		})
	}
}

func TestParse(t *testing.T) {
	testCases := []struct {
		name           string
		input          string
		expectError    bool
		expectedFormat Format
	}{
		// Short syntax tests
		{"String short syntax positive", "= hello", false, FmtString},
		{"String short syntax negative", "!= hello", false, FmtString},
		{"Glob short syntax positive", "* test*", false, FmtGlob},
		{"Glob short syntax negative", "!* test*", false, FmtGlob},
		{"RegExp short syntax positive", "~ ^test", false, FmtRegExp},
		{"RegExp short syntax negative", "!~ ^test", false, FmtRegExp},

		// Long syntax tests
		{"String long syntax positive", "string:hello", false, FmtString},
		{"String long syntax negative", "!string:hello", false, FmtString},
		{"Glob long syntax positive", "glob:test*", false, FmtGlob},
		{"Simple Pattern long syntax", "simple_patterns:test", false, FmtSimplePattern},

		// Error cases
		{"Invalid short syntax", "% invalid", true, ""},
		{"Invalid long syntax", "invalid:test", true, ""},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			matcher, err := Parse(tc.input)
			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, matcher)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, matcher)
			}
		})
	}
}

func TestParseShortFormat(t *testing.T) {
	testCases := []struct {
		name        string
		input       string
		expectError bool
	}{
		{"Valid string positive", "= hello", false},
		{"Valid string negative", "!= hello", false},
		{"Valid glob positive", "* test*", false},
		{"Valid regexp positive", "~ ^test", false},
		{"Invalid symbol", "# invalid", true},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			matcher, err := parseShortFormat(tc.input)
			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, matcher)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, matcher)
			}
		})
	}
}

func TestParseLongSyntax(t *testing.T) {
	testCases := []struct {
		name        string
		input       string
		expectError bool
	}{
		{"Valid string positive", "string:hello", false},
		{"Valid string negative", "!string:hello", false},
		{"Valid glob", "glob:test*", false},
		{"Valid regexp", "regexp:^test", false},
		{"Invalid format", "unknown:test", true},
		{"Invalid syntax", "string", true},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			matcher, err := parseLongSyntax(tc.input)
			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, matcher)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, matcher)
			}
		})
	}
}
```