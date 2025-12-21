```go
package megacli

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestCollectBBU(t *testing.T) {
	tests := []struct {
		name           string
		mockBBUInfo   []byte
		expectedError bool
		expectedCount int
	}{
		{
			name: "Multiple BBUs",
			mockBBUInfo: []byte(`BBU status for Adapter 0:
BatteryType: Smart
Temperature: 25 C
BBU Capacity Info for Adapter 0:
Relative State of Charge: 91%
Absolute State of charge: 100%
Full Charge Capacity: 100
Cycle Count: 50
BBU Design Info for Adapter 0:
Design Capacity: 90`),
			expectedError: false,
			expectedCount: 1,
		},
		{
			name:           "No BBUs",
			mockBBUInfo:    []byte{},
			expectedError:  false,
			expectedCount:  0,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := &Collector{
				exec: &mockExec{
					bbuInfoFunc: func() ([]byte, error) {
						return tt.mockBBUInfo, nil
					},
				},
				bbu: make(map[string]bool),
			}

			mx := make(map[string]int64)
			err := c.collectBBU(mx)

			if tt.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				// Check metrics or other expected behaviors
			}
		})
	}
}

func TestParseBBUInfo(t *testing.T) {
	tests := []struct {
		name           string
		input          []byte
		expectedCount  int
		expectedValues map[string]string
	}{
		{
			name: "Valid BBU Info",
			input: []byte(`BBU status for Adapter 0:
BatteryType: Smart
Temperature: 25 C
BBU Capacity Info for Adapter 0:
Relative State of Charge: 91%
Absolute State of charge: 100%
Full Charge Capacity: 100
Cycle Count: 50
BBU Design Info for Adapter 0:
Design Capacity: 90`),
			expectedCount: 1,
			expectedValues: map[string]string{
				"adapterNumber":  "0",
				"batteryType":    "Smart",
				"temperature":    "25",
				"rsoc":           "91",
				"asoc":           "100",
				"cycleCount":     "50",
				"fullChargeCap":  "100",
				"designCap":      "90",
			},
		},
		{
			name:           "Empty Input",
			input:          []byte{},
			expectedCount:  0,
			expectedValues: nil,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			bbus, err := parseBBUInfo(tt.input)

			assert.NoError(t, err)
			assert.Len(t, bbus, tt.expectedCount)

			if tt.expectedCount > 0 {
				bbu := bbus["0"]
				assert.Equal(t, tt.expectedValues["adapterNumber"], bbu.adapterNumber)
				assert.Equal(t, tt.expectedValues["batteryType"], bbu.batteryType)
				assert.Equal(t, tt.expectedValues["temperature"], bbu.temperature)
				assert.Equal(t, tt.expectedValues["rsoc"], bbu.rsoc)
				assert.Equal(t, tt.expectedValues["asoc"], bbu.asoc)
				assert.Equal(t, tt.expectedValues["cycleCount"], bbu.cycleCount)
				assert.Equal(t, tt.expectedValues["fullChargeCap"], bbu.fullChargeCap)
				assert.Equal(t, tt.expectedValues["designCap"], bbu.designCap)
			}
		})
	}
}

func TestCalcCapDegradationPerc(t *testing.T) {
	tests := []struct {
		name           string
		bbu            *megaBBU
		expectedDegradation int64
		expectedOK         bool
	}{
		{
			name: "Normal Degradation",
			bbu: &megaBBU{
				fullChargeCap: "90",
				designCap:     "100",
			},
			expectedDegradation: 10,
			expectedOK:          true,
		},
		{
			name: "Zero Full Charge Capacity",
			bbu: &megaBBU{
				fullChargeCap: "0",
				designCap:     "100",
			},
			expectedDegradation: 0,
			expectedOK:          false,
		},
		{
			name: "Zero Design Capacity",
			bbu: &megaBBU{
				fullChargeCap: "100",
				designCap:     "0",
			},
			expectedDegradation: 0,
			expectedOK:          false,
		},
		{
			name: "Invalid Number",
			bbu: &megaBBU{
				fullChargeCap: "invalid",
				designCap:     "invalid",
			},
			expectedDegradation: 0,
			expectedOK:          false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			degradation, ok := calcCapDegradationPerc(tt.bbu)
			assert.Equal(t, tt.expectedOK, ok)
			assert.Equal(t, tt.expectedDegradation, degradation)
		})
	}
}

// Mock executor for testing
type mockExec struct {
	bbuInfoFunc func() ([]byte, error)
}

func (m *mockExec) bbuInfo() ([]byte, error) {
	return m.bbuInfoFunc()
}
```