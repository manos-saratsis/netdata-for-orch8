```go
package storcli

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestCollectMegaraidControllersInfo(t *testing.T) {
	testCases := []struct {
		name           string
		controllersResponse *controllersInfoResponse
		expectedMetrics    map[string]int64
	}{
		{
			name: "Healthy controller with BBU",
			controllersResponse: &controllersInfoResponse{
				Controllers: []struct {
					CommandStatus struct {
						Controller int    `json:"Controller"`
						Status     string `json:"Status"`
					} `json:"Command Status"`
					ResponseData controllerInfo `json:"Response Data"`
				}{
					{
						ResponseData: controllerInfo{
							Basics: struct {
								Controller   int    `json:"Controller"`
								Model        string `json:"Model"`
								SerialNumber string `json:"Serial Number"`
							}{
								Controller: 0,
							},
							Status: struct {
								ControllerStatus string      `json:"Controller Status"`
								BBUStatus        *storNumber `json:"BBU Status"`
							}{
								ControllerStatus: "Optimal",
								BBUStatus:        toStorNumber("0"),
							},
							BBUInfo: []struct {
								Model string `json:"Model"`
								State string `json:"State"`
								Temp  string `json:"Temp"`
							}{
								{
									Model: "Test BBU",
									Temp:  "30C",
								},
							},
						},
					},
				},
			},
			expectedMetrics: map[string]int64{
				"cntrl_0_health_status_healthy":    1,
				"cntrl_0_health_status_unhealthy":  0,
				"cntrl_0_status_optimal":           1,
				"cntrl_0_status_degraded":          0,
				"cntrl_0_status_partially_degraded": 0,
				"cntrl_0_status_failed":            0,
				"cntrl_0_bbu_status_healthy":       1,
				"cntrl_0_bbu_status_unhealthy":     0,
				"cntrl_0_bbu_status_na":            0,
				"bbu_0_cntrl_0_temperature":        30,
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			metrics := make(map[string]int64)

			err := c.collectMegaraidControllersInfo(metrics, tc.controllersResponse)

			assert.NoError(t, err)
			assert.Equal(t, tc.expectedMetrics, metrics)
		})
	}
}

func toStorNumber(s string) *storNumber {
	sn := storNumber(s)
	return &sn
}

func TestCollectMpt3sasControllersInfo(t *testing.T) {
	testCases := []struct {
		name           string
		controllersResponse *controllersInfoResponse
		expectedMetrics    map[string]int64
	}{
		{
			name: "OK controller with temperature",
			controllersResponse: &controllersInfoResponse{
				Controllers: []struct {
					CommandStatus struct {
						Controller int    `json:"Controller"`
						Status     string `json:"Status"`
					} `json:"Command Status"`
					ResponseData controllerInfo `json:"Response Data"`
				}{
					{
						ResponseData: controllerInfo{
							Basics: struct {
								Controller   int    `json:"Controller"`
								Model        string `json:"Model"`
								SerialNumber string `json:"Serial Number"`
							}{
								Controller: 0,
							},
							Status: struct {
								ControllerStatus string      `json:"Controller Status"`
								BBUStatus        *storNumber `json:"BBU Status"`
							}{
								ControllerStatus: "OK",
							},
							HwCfg: struct {
								TemperatureSensorForROC string `json:"Temperature Sensor for ROC"`
								ROCTemperatureC         int    `json:"ROC temperature(Degree Celsius)"`
							}{
								TemperatureSensorForROC: "present",
								ROCTemperatureC:         40,
							},
						},
					},
				},
			},
			expectedMetrics: map[string]int64{
				"cntrl_0_health_status_healthy":   1,
				"cntrl_0_health_status_unhealthy": 0,
				"cntrl_0_roc_temperature_celsius": 40,
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			metrics := make(map[string]int64)

			err := c.collectMpt3sasControllersInfo(metrics, tc.controllersResponse)

			assert.NoError(t, err)
			assert.Equal(t, tc.expectedMetrics, metrics)
		})
	}
}

func TestQueryControllersInfo(t *testing.T) {
	testCases := []struct {
		name                string
		mockControllersInfo []byte
		expectError         bool
	}{
		{
			name:                "Successful query",
			mockControllersInfo: []byte(`{"Controllers":[{"Command Status":{"Status":"Success"}, "Response Data":{}}]}`),
			expectError:         false,
		},
		{
			name:                "Empty response",
			mockControllersInfo: []byte{},
			expectError:         true,
		},
		{
			name:                "Invalid JSON",
			mockControllersInfo: []byte(`{invalid json}`),
			expectError:         true,
		},
		{
			name:                "No controllers",
			mockControllersInfo: []byte(`{"Controllers":[]}`),
			expectError:         true,
		},
		{
			name:                "Command status error",
			mockControllersInfo: []byte(`{"Controllers":[{"Command Status":{"Status":"Error"}}]}`),
			expectError:         true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			mockExec := &mockExec{}
			c.exec = mockExec

			mockExec.On("controllersInfo").Return(tc.mockControllersInfo, nil)

			resp, err := c.queryControllersInfo()

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
```