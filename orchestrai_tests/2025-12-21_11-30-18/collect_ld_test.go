```go
package adaptecraid

import (
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type MockExec struct {
	mock.Mock
}

func (m *MockExec) logicalDevicesInfo() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func (m *MockExec) physicalDevicesInfo() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func TestCollector_CollectLogicalDevices(t *testing.T) {
	testCases := []struct {
		name           string
		mockLogicalDevInfo func() ([]byte, error)
		expectedErrMsg     string
		verifyMetrics      func(mx map[string]int64)
	}{
		{
			name: "Successful collection - Optimal device",
			mockLogicalDevInfo: func() ([]byte, error) {
				return []byte(`Logical device number 0
Logical device name: Test Device
RAID level: RAID 5
Status of logical device: Optimal`), nil
			},
			verifyMetrics: func(mx map[string]int64) {
				assert.Equal(t, int64(1), mx["ld_0_health_state_ok"])
				assert.Equal(t, int64(0), mx["ld_0_health_state_critical"])
			},
		},
		{
			name: "Successful collection - Non-Optimal device",
			mockLogicalDevInfo: func() ([]byte, error) {
				return []byte(`Logical device number 0
Logical device name: Test Device
RAID level: RAID 5
Status of logical device: Degraded`), nil
			},
			verifyMetrics: func(mx map[string]int64) {
				assert.Equal(t, int64(0), mx["ld_0_health_state_ok"])
				assert.Equal(t, int64(1), mx["ld_0_health_state_critical"])
			},
		},
		{
			name: "Logical devices info retrieval error",
			mockLogicalDevInfo: func() ([]byte, error) {
				return nil, errors.New("failed to retrieve devices")
			},
			expectedErrMsg: "failed to retrieve devices",
		},
		{
			name: "No logical devices found",
			mockLogicalDevInfo: func() ([]byte, error) {
				return []byte{}, nil
			},
			expectedErrMsg: "no logical devices found",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			mockExec := &MockExec{}
			mockExec.On("logicalDevicesInfo").Return(tc.mockLogicalDevInfo())
			c.exec = mockExec
			c.lds = make(map[string]bool)

			mx := make(map[string]int64)
			err := c.collectLogicalDevices(mx)

			if tc.expectedErrMsg != "" {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.expectedErrMsg)
			} else {
				assert.NoError(t, err)
				tc.verifyMetrics(mx)
			}
		})
	}
}

func TestIsOkLDStatus(t *testing.T) {
	testCases := []struct {
		name       string
		ldStatus   string
		expectOk   bool
	}{
		{
			name:     "Optimal status",
			ldStatus: "Optimal",
			expectOk: true,
		},
		{
			name:     "Non-Optimal status",
			ldStatus: "Degraded",
			expectOk: false,
		},
		{
			name:     "Empty status",
			ldStatus: "",
			expectOk: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			ld := &logicalDevice{status: tc.ldStatus}
			result := isOkLDStatus(ld)
			assert.Equal(t, tc.expectOk, result)
		})
	}
}

func TestParseLogicDevInfo(t *testing.T) {
	testCases := []struct {
		name            string
		input           []byte
		expectedDevices map[string]*logicalDevice
		expectedErr     bool
	}{
		{
			name: "Single device parsing",
			input: []byte(`Logical device number 0
Logical device name: Test Device
RAID level: RAID 5
Status of logical device: Optimal
Failed stripes: 0`),
			expectedDevices: map[string]*logicalDevice{
				"0": {
					number:        "0",
					name:          "Test Device",
					raidLevel:     "RAID 5",
					status:        "Optimal",
					failedStripes: "0",
				},
			},
			expectedErr: false,
		},
		{
			name: "Multiple devices parsing",
			input: []byte(`Logical device number 0
Logical device name: Device 0
RAID level: RAID 5
Status of logical device: Optimal

Logical device number 1
Logical device name: Device 1
RAID level: RAID 1
Status of logical device: Degraded`),
			expectedDevices: map[string]*logicalDevice{
				"0": {
					number:    "0",
					name:      "Device 0",
					raidLevel: "RAID 5",
					status:    "Optimal",
				},
				"1": {
					number:    "1",
					name:      "Device 1",
					raidLevel: "RAID 1",
					status:    "Degraded",
				},
			},
			expectedErr: false,
		},
		{
			name:            "Empty input",
			input:           []byte{},
			expectedDevices: map[string]*logicalDevice{},
			expectedErr:     false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			devices, err := parseLogicDevInfo(tc.input)

			if tc.expectedErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			assert.Equal(t, len(tc.expectedDevices), len(devices))
			for k, expectedDevice := range tc.expectedDevices {
				actualDevice, exists := devices[k]
				assert.True(t, exists)
				assert.Equal(t, expectedDevice.number, actualDevice.number)
				assert.Equal(t, expectedDevice.name, actualDevice.name)
				assert.Equal(t, expectedDevice.raidLevel, actualDevice.raidLevel)
				assert.Equal(t, expectedDevice.status, actualDevice.status)
			}
		})
	}
}

func TestGetColonSepValue(t *testing.T) {
	testCases := []struct {
		name     string
		input    string
		expected string
	}{
		{
			name:     "Normal case",
			input:    "Key: Value",
			expected: "Value",
		},
		{
			name:     "No value",
			input:    "Key:",
			expected: "",
		},
		{
			name:     "No colon",
			input:    "KeyValue",
			expected: "",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := getColonSepValue(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}
```