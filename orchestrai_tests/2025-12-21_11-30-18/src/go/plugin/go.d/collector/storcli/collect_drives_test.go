```go
package storcli

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestCollectMegaRaidDrives(t *testing.T) {
	testCases := []struct {
		name               string
		drivesResponse    *drivesInfoResponse
		expectedMetrics   map[string]int64
		expectError       bool
	}{
		{
			name: "Successful collection",
			drivesResponse: &drivesInfoResponse{
				Controllers: []struct {
					CommandStatus struct {
						Controller int    `json:"Controller"`
						Status     string `json:"Status"`
					} `json:"Command Status"`
					ResponseData map[string]json.RawMessage `json:"Response Data"`
				}{
					{
						CommandStatus: struct {
							Controller int    `json:"Controller"`
							Status     string `json:"Status"`
						}{
							Controller: 0,
							Status:     "Success",
						},
						ResponseData: map[string]json.RawMessage{
							"Drive /c0/e252/s0 - Detailed Information": json.RawMessage(`{
								"Drive /c0/e252/s0 State": {
									"Media Error Count": "10", 
									"Other Error Count": "5", 
									"Drive Temperature": "30C", 
									"Predictive Failure Count": "2",
									"S.M.A.R.T alert flagged by drive": "Yes"
								},
								"Drive /c0/e252/s0 Device attributes": {
									"WWN": "12345",
									"Device Speed": "6Gb/s"
								},
								"Drive /c0/e252/s0": {
									"EID:Slt": "1"
								}
							}`),
							"Drive /c0/e252/s0": json.RawMessage(`[{
								"EID:Slt": "1",
								"Med": "HDD"
							}]`),
						},
					},
				},
			},
			expectedMetrics: map[string]int64{
				"phys_drive_12345_cntrl_0_media_error_count":          10,
				"phys_drive_12345_cntrl_0_other_error_count":          5,
				"phys_drive_12345_cntrl_0_predictive_failure_count":   2,
				"phys_drive_12345_cntrl_0_temperature":                30,
				"phys_drive_12345_cntrl_0_smart_alert_status_active":   1,
				"phys_drive_12345_cntrl_0_smart_alert_status_inactive": 0,
			},
			expectError: false,
		},
		{
			name:             "Nil response",
			drivesResponse:   nil,
			expectedMetrics:  nil,
			expectError:      false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			metrics := make(map[string]int64)

			err := c.collectMegaRaidDrives(metrics, tc.drivesResponse)

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				if tc.expectedMetrics != nil {
					assert.Equal(t, tc.expectedMetrics, metrics)
				}
			}
		})
	}
}

func TestQueryDrivesInfo(t *testing.T) {
	testCases := []struct {
		name            string
		mockDrivesInfo  []byte
		expectError     bool
	}{
		{
			name:            "Successful query",
			mockDrivesInfo:  []byte(`{"Controllers":[{"Command Status":{"Status":"Success"}, "Response Data":{}}]}`),
			expectError:     false,
		},
		{
			name:            "Empty response",
			mockDrivesInfo:  []byte{},
			expectError:     true,
		},
		{
			name:            "Invalid JSON",
			mockDrivesInfo:  []byte(`{invalid json}`),
			expectError:     true,
		},
		{
			name:            "No controllers",
			mockDrivesInfo:  []byte(`{"Controllers":[]}`),
			expectError:     true,
		},
		{
			name:            "Command status error",
			mockDrivesInfo:  []byte(`{"Controllers":[{"Command Status":{"Status":"Error"}}]}`),
			expectError:     true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			mockExec := &mockExec{}
			c.exec = mockExec

			mockExec.On("drivesInfo").Return(tc.mockDrivesInfo, nil)

			resp, err := c.queryDrivesInfo()

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, resp)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, resp)
			}
		})
	}
}

func TestGetDriveInfo(t *testing.T) {
	testCases := []struct {
		name           string
		respData       map[string]json.RawMessage
		id             string
		expectError    bool
	}{
		{
			name: "Successful drive info retrieval",
			respData: map[string]json.RawMessage{
				"Drive /c0/e252/s0": json.RawMessage(`[{"EID:Slt":"1", "Med":"HDD"}]`),
			},
			id:          "/c0/e252/s0",
			expectError: false,
		},
		{
			name:           "Drive info not found",
			respData:       map[string]json.RawMessage{},
			id:             "/c0/e252/s0",
			expectError:    true,
		},
		{
			name: "Invalid JSON",
			respData: map[string]json.RawMessage{
				"Drive /c0/e252/s0": json.RawMessage(`{invalid json}`),
			},
			id:          "/c0/e252/s0",
			expectError: true,
		},
		{
			name: "Empty drive info array",
			respData: map[string]json.RawMessage{
				"Drive /c0/e252/s0": json.RawMessage(`[]`),
			},
			id:          "/c0/e252/s0",
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			driveInfo, err := getDriveInfo(tc.respData, tc.id)

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, driveInfo)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, driveInfo)
				assert.Equal(t, "1", driveInfo.EIDSlt)
			}
		})
	}
}

func TestGetDriveDetailedInfo(t *testing.T) {
	testCases := []struct {
		name           string
		respData       map[string]json.RawMessage
		id             string
		expectError    bool
	}{
		{
			name: "Successful detailed info retrieval",
			respData: map[string]json.RawMessage{
				"Drive /c0/e252/s0 - Detailed Information": json.RawMessage(`{"test":"data"}`),
			},
			id:          "/c0/e252/s0",
			expectError: false,
		},
		{
			name:           "Drive detailed info not found",
			respData:       map[string]json.RawMessage{},
			id:             "/c0/e252/s0",
			expectError:    true,
		},
		{
			name: "Invalid JSON",
			respData: map[string]json.RawMessage{
				"Drive /c0/e252/s0 - Detailed Information": json.RawMessage(`{invalid json}`),
			},
			id:          "/c0/e252/s0",
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			detailedInfo, err := getDriveDetailedInfo(tc.respData, tc.id)

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, detailedInfo)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, detailedInfo)
			}
		})
	}
}

func TestGetDriveState(t *testing.T) {
	testCases := []struct {
		name              string
		driveDetailedInfo map[string]json.RawMessage
		id                string
		expectError       bool
	}{
		{
			name: "Successful drive state retrieval",
			driveDetailedInfo: map[string]json.RawMessage{
				"Drive /c0/e252/s0 State": json.RawMessage(`{
					"Media Error Count": "10",
					"Other Error Count": "5",
					"Drive Temperature": "30C",
					"Predictive Failure Count": "2",
					"S.M.A.R.T alert flagged by drive": "Yes"
				}`),
			},
			id:          "/c0/e252/s0",
			expectError: false,
		},
		{
			name:              "Drive state not found",
			driveDetailedInfo: map[string]json.RawMessage{},
			id:                "/c0/e252/s0",
			expectError:       true,
		},
		{
			name: "Invalid JSON",
			driveDetailedInfo: map[string]json.RawMessage{
				"Drive /c0/e252/s0 State": json.RawMessage(`{invalid json}`),
			},
			id:          "/c0/e252/s0",
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			driveState, err := getDriveState(tc.driveDetailedInfo, tc.id)

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, driveState)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, driveState)
				assert.Equal(t, "10", string(driveState.MediaErrorCount))
			}
		})
	}
}

func TestGetDriveAttrs(t *testing.T) {
	testCases := []struct {
		name              string
		driveDetailedInfo map[string]json.RawMessage
		id                string
		expectError       bool
	}{
		{
			name: "Successful device attributes retrieval",
			driveDetailedInfo: map[string]json.RawMessage{
				"Drive /c0/e252/s0 Device attributes": json.RawMessage(`{
					"WWN": "12345",
					"Device Speed": "6Gb/s",
					"Link Speed": "6Gb/s"
				}`),
			},
			id:          "/c0/e252/s0",
			expectError: false,
		},
		{
			name:              "Device attributes not found",
			driveDetailedInfo: map[string]json.RawMessage{},
			id:                "/c0/e252/s0",
			expectError:       true,
		},
		{
			name: "Invalid JSON",
			driveDetailedInfo: map[string]json.RawMessage{
				"Drive /c0/e252/s0 Device attributes": json.RawMessage(`{invalid json}`),
			},
			id:          "/c0/e252/s0",
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			driveAttrs, err := getDriveAttrs(tc.driveDetailedInfo, tc.id)

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, driveAttrs)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, driveAttrs)
				assert.Equal(t, "12345", driveAttrs.WWN)
			}
		})
	}
}

func TestGetTemperature(t *testing.T) {
	testCases := []struct {
		name     string
		input    string
		expected string
	}{
		{
			name:     "Drive temperature format",
			input:    " 28C (82.40 F)",
			expected: "28",
		},
		{
			name:     "BBU temperature format",
			input:    "33C",
			expected: "33",
		},
		{
			name:     "No Celsius indicator",
			input:    "Invalid Temperature",
			expected: "",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := getTemperature(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestParseInt(t *testing.T) {
	testCases := []struct {
		name     string
		input    string
		expected int64
		ok       bool
	}{
		{
			name:     "Valid positive integer",
			input:    "123",
			expected: 123,
			ok:       true,
		},
		{
			name:     "Invalid integer",
			input:    "not a number",
			expected: 0,
			ok:       false,
		},
		{
			name:     "Empty string",
			input:    "",
			expected: 0,