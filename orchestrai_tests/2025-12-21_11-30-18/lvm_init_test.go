package lvm

import (
	"context"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestInitLVMCLIExec(t *testing.T) {
	testCases := []struct {
		name        string
		setupConfig func() *Collector
		expectError bool
	}{
		{
			name: "Successful Initialization",
			setupConfig: func() *Collector {
				c := New()
				c.Timeout = confopt.Duration(time.Second * 5)
				return c
			},
			expectError: false,
		},
		{
			name: "Zero Timeout",
			setupConfig: func() *Collector {
				c := New()
				c.Timeout = confopt.Duration(0)
				return c
			},
			expectError: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := tc.setupConfig()

			lvmExec, err := c.initLVMCLIExec()

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, lvmExec)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, lvmExec)
			}
		})
	}
}