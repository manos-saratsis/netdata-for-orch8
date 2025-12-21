package nvme

import (
	"errors"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/pkg/metrix"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

// Mock for NVMeExec
type MockNVMeExec struct {
	mock.Mock
}

func (m *MockNVMeExec) smartLog(devicePath string) (SmartLog, error) {
	args := m.Called(devicePath)
	return args.Get(0).(SmartLog), args.Error(1)
}

func (m *MockNVMeExec) list() (DeviceList, error) {
	args := m.Called()
	return args.Get(0).(DeviceList), args.Error(1)
}

func TestCollectNVMeDevice(t *testing.T) {
	testCases := []struct {
		name        string
		devicePath  string
		mockSmartLog SmartLog
		expectError  bool
		expectedMetrics map[string]int64
	}{
		{
			name:       "Successful Device Collection",
			devicePath: "/dev/nvme0n1",
			mockSmartLog: SmartLog{
				Temperature:     nvmeNumber("298.15"), // 25°C
				PercentUsed:     nvmeNumber("10"),
				AvailSpare:      nvmeNumber("90"),
				DataUnitsRead:   nvmeNumber("100"),
				DataUnitsWritten: nvmeNumber("50"),
				HostReadCommands: nvmeNumber("1000"),
				HostWriteCommands: nvmeNumber("500"),
				PowerCycles:     nvmeNumber("42"),
				PowerOnHours:    nvmeNumber("100"),
				UnsafeShutdowns: nvmeNumber("5"),
				MediaErrors:     nvmeNumber("2"),
				NumErrLogEntries: nvmeNumber("3"),
				ControllerBusyTime: nvmeNumber("30"),
				WarningTempTime:    nvmeNumber("15"),
				CriticalCompTime:   nvmeNumber("10"),
				ThmTemp1TransCount: nvmeNumber("20"),
				ThmTemp2TransCount: nvmeNumber("10"),
				ThmTemp1TotalTime:  nvmeNumber("100"),
				ThmTemp2TotalTime:  nvmeNumber("50"),
				CriticalWarningValue: nvmeNumber("42"), // Mix of warning flags
			},
			expectError: false,
			expectedMetrics: map[string]int64{
				"device_nvme0n1_temperature":        int64(-248), // 298.15K - 273.15 = 25°C
				"device_nvme0n1_percentage_used":    10,
				"device_nvme0n1_available_spare":    90,
				"device_nvme0n1_data_units_read":    100 * 1000 * 512,
				"device_nvme0n1_data_units_written": 50 * 1000 * 512,
				"device_nvme0n1_host_read_commands":  1000,
				"device_nvme0n1_host_write_commands": 500,
				"device_nvme0n1_power_cycles": 42,
				"device_nvme0n1_power_on_time": 360000, // 100 * 3600 seconds
			},
		},
		{
			name:       "SmartLog Failure",
			devicePath: "/dev/nvme1n1",
			mockSmartLog: SmartLog{},
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockExec := new(MockNVMeExec)
			mockExec.On("smartLog", tc.devicePath).Return(tc.mockSmartLog, nil)

			collector := &Collector{
				exec:         mockExec,
				devicePaths:  make(map[string]bool),
				charts:       nil,
			}

			mx := make(map[string]int64)
			err := collector.collectNVMeDevice(mx, tc.devicePath)

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				
				// Validate basic metrics
				for key, expectedVal := range tc.expectedMetrics {
					assert.Equal(t, expectedVal, mx[key], "Mismatch in metric: "+key)
				}
				
				// Validate critical warning metrics
				critWarningMetrics := map[string]int64{
					"device_nvme0n1_critical_warning_available_spare": metrix.Bool(true),
					"device_nvme0n1_critical_warning_temp_threshold": metrix.Bool(true),
					"device_nvme0n1_critical_warning_nvm_subsystem_reliability": metrix.Bool(false),
				}

				for key, expectedVal := range critWarningMetrics {
					assert.Equal(t, expectedVal, mx[key], "Mismatch in critical warning metric: "+key)
				}
			}

			mockExec.AssertExpectations(t)
		})
	}
}

func TestListNVMeDevices(t *testing.T) {
	testCases := []struct {
		name           string
		listResponse   DeviceList
		expectedPaths  map[string]bool
		expectError    bool
	}{
		{
			name: "Successful Device Discovery",
			listResponse: DeviceList{
				Devices: []Device{
					{DevicePath: "/dev/nvme0n1", ModelNumber: "TestModel1"},
					{DevicePath: "/dev/nvme1n1", ModelNumber: "TestModel2"},
				},
			},
			expectedPaths: map[string]bool{
				"/dev/nvme0": true,
				"/dev/nvme1": true,
			},
			expectError: false,
		},
		{
			name: "Device List Failure",
			listResponse: DeviceList{},
			expectedPaths: map[string]bool{},
			expectError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockExec := new(MockNVMeExec)
			mockExec.On("list").Return(tc.listResponse, nil)

			collector := &Collector{
				exec:              mockExec,
				devicePaths:       make(map[string]bool),
				listDevicesTime:   time.Now().Add(-2 * time.Hour),
				listDevicesEvery:  time.Hour,
				forceListDevices:  true,
			}

			err := collector.listNVMeDevices()

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedPaths, collector.devicePaths)
			}

			mockExec.AssertExpectations(t)
		})
	}
}

func TestExtractControllerPathFromDevicePath(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{"/dev/nvme0n1", "/dev/nvme0"},
		{"/dev/nvme10n1", "/dev/nvme10"},
		{"/dev/sda", ""},
		{"/dev/nvme0", "/dev/nvme0"},
		{"/dev/nvmea", ""},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := extractControllerPathFromDevicePath(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestExtractDeviceFromPath(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{"/dev/nvme0n1", "nvme0n1"},
		{"/path/to/device/nvme1", "nvme1"},
		{"", ""},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := extractDeviceFromPath(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestParseValue(t *testing.T) {
	testCases := []struct {
		input    nvmeNumber
		expected int64
	}{
		{"42", 42},
		{"3.14", 3},
		{"-10", -10},
		{"", 0},
	}

	for _, tc := range testCases {
		t.Run(string(tc.input), func(t *testing.T) {
			result := parseValue(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}