package zfspool_test

import (
	"context"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/collector/zfspool"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestCollectorConfiguration(t *testing.T) {
	c := zfspool.New()
	config := c.Configuration()
	assert.NotNil(t, config)
}

func TestCollectorInit(t *testing.T) {
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
			errorContains: "no zpool binary path",
		},
		{
			name:          "Non-existent Binary Path",
			binaryPath:    "/nonexistent/zpool",
			expectError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := zfspool.New()
			c.BinaryPath = tc.binaryPath

			err := c.Init(context.Background())

			if tc.expectError {
				assert.Error(t, err)
				if tc.errorContains != "" {
					assert.Contains(t, err.Error(), tc.errorContains)
				}
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollectorCheck(t *testing.T) {
	testCases := []struct {
		name          string
		collectOutput map[string]int64
		collectError  error
		expectError   bool
	}{
		{
			name:          "Successful Check",
			collectOutput: map[string]int64{"test": 1},
		},
		{
			name:          "No Metrics Collected",
			collectOutput: map[string]int64{},
			expectError:   true,
		},
		{
			name:          "Collect Error",
			collectError:  errors.New("collection error"),
			expectError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := zfspool.New()
			mockExec := &MockZpoolCli{}
			mockExec.On("List").Return([]byte("NAME    SIZE    HEALTH\nrpool   1000   ONLINE"), nil)
			mockExec.On("ListWithVdev", mock.Anything).Return([]byte("NAME    SIZE    HEALTH\nrpool   1000   ONLINE"), nil)
			c.Exec = mockExec

			err := c.Check(context.Background())

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollectorCollect(t *testing.T) {
	testCases := []struct {
		name          string
		collectOutput map[string]int64
		collectError  error
		expectedLen   int
	}{
		{
			name:          "Metrics Collected",
			collectOutput: map[string]int64{"test": 1},
			expectedLen:   1,
		},
		{
			name:          "No Metrics",
			collectOutput: map[string]int64{},
			expectedLen:   0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := zfspool.New()
			mockExec := &MockZpoolCli{}
			mockExec.On("List").Return([]byte("NAME    SIZE    HEALTH\nrpool   1000   ONLINE"), nil)
			mockExec.On("ListWithVdev", mock.Anything).Return([]byte("NAME    SIZE    HEALTH\nrpool   1000   ONLINE"), nil)
			c.Exec = mockExec

			metrics := c.Collect(context.Background())
			assert.Len(t, metrics, tc.expectedLen)
		})
	}
}

func TestCollectorCleanup(t *testing.T) {
	c := zfspool.New()
	assert.NotPanics(t, func() {
		c.Cleanup(context.Background())
	})
}