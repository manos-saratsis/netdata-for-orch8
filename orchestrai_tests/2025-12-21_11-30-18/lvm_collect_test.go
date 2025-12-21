package lvm

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type mockLVMCLI struct {
	mock.Mock
}

func (m *mockLVMCLI) lvsReportJson() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func TestCollect(t *testing.T) {
	testCases := []struct {
		name           string
		jsonResponse   string
		expectedMetrics map[string]int64
		expectError    bool
	}{
		{
			name: "Successful Collection with Thin Pool",
			jsonResponse: `{
				"report": [{
					"lv": [{
						"vg_name": "testVG",
						"lv_name": "testLV", 
						"lv_size": "10G",
						"data_percent": "0.5",
						"metadata_percent": "0.3",
						"lv_attr": "t"
					}]
				}]
			}`,
			expectedMetrics: map[string]int64{
				"lv_testLV_vg_testVG_data_percent": 50,
				"lv_testLV_vg_testVG_metadata_percent": 30,
			},
			expectError: false,
		},
		{
			name: "Skips Non-Thin Pool",
			jsonResponse: `{
				"report": [{
					"lv": [{
						"vg_name": "testVG",
						"lv_name": "testLV", 
						"lv_size": "10G",
						"data_percent": "0.5",
						"metadata_percent": "0.3",
						"lv_attr": "s"
					}]
				}]
			}`,
			expectedMetrics: map[string]int64{},
			expectError: false,
		},
		{
			name: "Invalid JSON",
			jsonResponse: `{invalid json}`,
			expectedMetrics: nil,
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockExec := new(mockLVMCLI)
			mockExec.On("lvsReportJson").Return([]byte(tc.jsonResponse), nil)

			c := &Collector{
				charts:       nil,
				lvmThinPools: make(map[string]bool),
				exec:         mockExec,
			}

			metrics, err := c.collect()

			if tc.expectError {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			assert.Equal(t, tc.expectedMetrics, metrics)
		})
	}
}

func TestIsThinPool(t *testing.T) {
	testCases := []struct {
		lvAttr    string
		isThinPool bool
	}{
		{"t", true},
		{"T", false},
		{"s", false},
		{"", false},
	}

	for _, tc := range testCases {
		t.Run("Checking LV Type: " + tc.lvAttr, func(t *testing.T) {
			assert.Equal(t, tc.isThinPool, isThinPool(tc.lvAttr))
		})
	}
}

func TestParseFloat(t *testing.T) {
	testCases := []struct {
		input         string
		expectedValue float64
		expectedOK    bool
	}{
		{"0.5", 0.5, true},
		{"-", 0, false},
		{"invalid", 0, false},
		{"100.25", 100.25, true},
	}

	for _, tc := range testCases {
		t.Run("Parse Float: " + tc.input, func(t *testing.T) {
			val, ok := parseFloat(tc.input)
			assert.Equal(t, tc.expectedValue, val)
			assert.Equal(t, tc.expectedOK, ok)
		})
	}
}

func TestGetLVType(t *testing.T) {
	testCases := []struct {
		lvAttr    string
		expected string
	}{
		{"t", "thin_pool"},
		{"T", "thin_pool_data"},
		{"s", "snapshot"},
		{"", ""},
		{"C", "cache"},
		{"m", "mirrored"},
	}

	for _, tc := range testCases {
		t.Run("LV Type for " + tc.lvAttr, func(t *testing.T) {
			assert.Equal(t, tc.expected, getLVType(tc.lvAttr))
		})
	}
}