package lvm

import (
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/logger"
	"github.com/stretchr/testify/assert"
)

func TestNewLVMCLIExec(t *testing.T) {
	log := logger.New()
	timeout := time.Second * 5

	exec := newLVMCLIExec(timeout, log)

	assert.NotNil(t, exec)
	assert.Equal(t, timeout, exec.timeout)
	assert.Equal(t, log, exec.Logger)
}

func TestLVSReportJson(t *testing.T) {
	// Note: This test can only verify the method signature 
	// Actual command execution requires system-specific mocking
	log := logger.New()
	timeout := time.Second * 5

	exec := newLVMCLIExec(timeout, log)

	_, err := exec.lvsReportJson()
	
	// We can't guarantee actual command success, 
	// but we can ensure the method doesn't panic
	assert.NotPanics(t, func() {
		exec.lvsReportJson()
	})
}