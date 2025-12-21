```go
package logger

import (
	"context"
	"log/slog"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestNewLogger(t *testing.T) {
	logger := New()
	assert.NotNil(t, logger)
	assert.NotNil(t, logger.sl)
}

func TestLoggerMethods(t *testing.T) {
	testCases := []struct {
		name    string
		logFunc func(*Logger)
		method  string
	}{
		{"Error", func(l *Logger) { l.Error("test error") }, "Error"},
		{"Warning", func(l *Logger) { l.Warning("test warning") }, "Warning"},
		{"Notice", func(l *Logger) { l.Notice("test notice") }, "Notice"},
		{"Info", func(l *Logger) { l.Info("test info") }, "Info"},
		{"Debug", func(l *Logger) { l.Debug("test debug") }, "Debug"},
	}

	for _, tc := range testCases {
		t.Run(tc.method, func(t *testing.T) {
			logger := New()
			tc.logFunc(logger)
		})
	}
}

func TestLoggerFormattedMethods(t *testing.T) {
	testCases := []struct {
		name    string
		logFunc func(*Logger)
		method  string
	}{
		{"Errorf", func(l *Logger) { l.Errorf("test %s", "error") }, "Errorf"},
		{"Warningf", func(l *Logger) { l.Warningf("test %s", "warning") }, "Warningf"},
		{"Noticef", func(l *Logger) { l.Noticef("test %s", "notice") }, "Noticef"},
		{"Infof", func(l *Logger) { l.Infof("test %s", "info") }, "Infof"},
		{"Debugf", func(l *Logger) { l.Debugf("test %s", "debug") }, "Debugf"},
	}

	for _, tc := range testCases {
		t.Run(tc.method, func(t *testing.T) {
			logger := New()
			tc.logFunc(logger)
		})
	}
}

func TestLoggerMute(t *testing.T) {
	logger := New()
	
	// Test Mute
	logger.Mute()
	assert.True(t, logger.muted.Load())
	
	// Test Unmute
	logger.Unmute()
	assert.False(t, logger.muted.Load())
}

func TestLoggerWith(t *testing.T) {
	logger := New()
	
	// Test With method with arguments
	newLogger := logger.With("key", "value")
	assert.NotNil(t, newLogger)
	assert.NotEqual(t, logger, newLogger)
}

func TestLoggerNilHandling(t *testing.T) {
	var nilLogger *Logger

	// Test various methods on nil logger
	nilLogger.Error("test")
	nilLogger.Warning("test")
	nilLogger.Notice("test")
	nilLogger.Info("test")
	nilLogger.Debug("test")

	// Test mute on nil logger
	nilLogger.Mute()
	nilLogger.Unmute()
}

func TestLoggerInternalLog(t *testing.T) {
	// Test internal log method with different scenarios
	logger := New()

	// Test when logger is muted
	logger.Mute()
	logger.log(slog.LevelInfo, "muted message")

	// Test when logger is unmuted
	logger.Unmute()
	logger.log(slog.LevelInfo, "unmuted message")
}
```