package zfspool_test

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/collector/zfspool"
	"github.com/stretchr/testify/assert"
)

func TestValidateConfig(t *testing.T) {
	testCases := []struct {
		name          string
		binaryPath    string
		expectError   bool
		errorContains string
	}{
		{
			name:       "Valid Binary Path",
			binaryPath: "/usr/bin/zpool",
		},
		{
			name:          "Empty Binary Path",
			binaryPath:    "",
			expectError:   true,
			errorContains: "no zpool binary path specified",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &zfspool.Collector{
				Config: zfspool.Config{
					BinaryPath: tc.binaryPath,
				},
			}

			err := c.ValidateConfig()

			if tc.expectError {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errorContains)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestInitZPoolCLIExec(t *testing.T) {
	testCases := []struct {
		name          string
		binaryPath    string
		expectError   bool
		errorContains string
	}{
		{
			name:       "Absolute Binary Path",
			binaryPath: "/usr/bin/zpool",
		},
		{
			name:          "Non-existent Binary Path",
			binaryPath:    "/nonexistent/zpool",
			expectError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &zfspool.Collector{
				Config: zfspool.Config{
					BinaryPath: tc.binaryPath,
				},
			}

			exec, err := c.InitZPoolCLIExec()

			if tc.expectError {
				assert.Error(t, err)
				if tc.errorContains != "" {
					assert.Contains(t, err.Error(), tc.errorContains)
				}
				assert.Nil(t, exec)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, exec)
			}
		})
	}
}