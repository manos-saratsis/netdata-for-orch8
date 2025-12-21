```go
package logger

import (
	"log/slog"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestLevelEnabled(t *testing.T) {
	testCases := []struct {
		name           string
		setLevel       slog.Level
		checkLevel     slog.Level
		expectedResult bool
	}{
		{"Error Enabled", slog.LevelError, slog.LevelError, true},
		{"Error Disabled", slog.LevelError, slog.LevelDebug, false},
		{"Notice Enabled", levelNotice, levelNotice, true},
		{"Notice Disabled", levelNotice, slog.LevelDebug, false},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			Level.Set(tc.setLevel)
			result := Level.Enabled(tc.checkLevel)
			assert.Equal(t, tc.expectedResult, result)
		})
	}
}

func TestLevelSetByName(t *testing.T) {
	testCases := []struct {
		name     string
		levelStr string
		expected slog.Level
	}{
		{"Set Error", "error", slog.LevelError},
		{"Set Warn", "warning", slog.LevelWarn},
		{"Set Notice", "notice", levelNotice},
		{"Set Info", "info", slog.LevelInfo},
		{"Set Debug", "debug", slog.LevelDebug},
		{"Set Emergency", "emergency", levelDisable},
		{"Set Alert", "alert", levelDisable},
		{"Set Critical", "critical", levelDisable},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			originalLevel := Level.lvl.Level()
			
			Level.SetByName(tc.levelStr)
			assert.Equal(t, tc.expected, Level.lvl.Level())
			
			// Restore original level
			Level.Set(originalLevel)
		})
	}
}
```