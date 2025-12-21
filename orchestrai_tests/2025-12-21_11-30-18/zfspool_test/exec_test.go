package zfspool_test

import (
	"errors"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/logger"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/collector/zfspool"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type MockRunUnprivileged struct {
	mock.Mock
}

func (m *MockRunUnprivileged) RunUnprivileged(l *logger.Logger, timeout time.Duration, cmd string, args ...string) ([]byte, error) {
	fullArgs := append([]string{cmd}, args...)
	result := m.Called(l, timeout, fullArgs)
	return result.Get(0).([]byte), result.Error(1)
}

func TestZpoolCLIExec_List(t *testing.T) {
	testCases := []struct {
		name          string
		mockOutput    []byte
		mockError     error
		expectError   bool
	}{
		{
			name:       "Successful List",
			mockOutput: []byte("test output"),
		},
		{
			name:          "List Error",
			mockError:     errors.New("list error"),
			expectError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockRunner := &MockRunUnprivileged{}
			exec := &zfspool.ZpoolCLIExec{
				Logger:   &logger.Logger{},
				BinPath:  "/usr/bin/zpool",
				Timeout:  time.Second,
				Executor: mockRunner,
			}

			expectedArgs := []string{"zpool", "list", "-p"}
			mockRunner.On("RunUnprivileged", mock.Anything, mock.Anything, expectedArgs).Return(tc.mockOutput, tc.mockError)

			output, err := exec.List()

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.mockOutput, output)
			}

			mockRunner.AssertExpectations(t)
		})
	}
}

func TestZpoolCLIExec_ListWithVdev(t *testing.T) {
	testCases := []struct {
		name          string
		pool          string
		mockOutput    []byte
		mockError     error
		expectError   bool
	}{
		{
			name:       "Successful List With Vdev",
			pool:       "testpool",
			mockOutput: []byte("test output"),
		},
		{
			name:          "List With Vdev Error",
			pool:          "testpool",
			mockError:     errors.New("list vdev error"),
			expectError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockRunner := &MockRunUnprivileged{}
			exec := &zfspool.ZpoolCLIExec{
				Logger:   &logger.Logger{},
				BinPath:  "/usr/bin/zpool",
				Timeout:  time.Second,
				Executor: mockRunner,
			}

			expectedArgs := []string{"zpool", "list", "-p", "-v", "-L", tc.pool}
			mockRunner.On("RunUnprivileged", mock.Anything, mock.Anything, expectedArgs).Return(tc.mockOutput, tc.mockError)

			output, err := exec.ListWithVdev(tc.pool)

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.mockOutput, output)
			}

			mockRunner.AssertExpectations(t)
		})
	}
}