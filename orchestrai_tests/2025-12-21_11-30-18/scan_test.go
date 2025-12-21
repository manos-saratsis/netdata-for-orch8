```go
package smartctl

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/tidwall/gjson"
)

func TestScanDevices(t *testing.T) {
	testCases := []struct {
		name         string
		setupMocks   func(*mockSmartctlCli, *Collector)
		extraDevices []ConfigExtraDevice
		expectedErr  bool
		expectCount  int
	}{
		{
			name: "Successful Scan with Multiple Devices",
			setupMocks: func(mcli *mockSmartctlCli, c *Collector) {
				mcli.On("scan", false).Return(&gjson.Result{Raw: `{
					"devices":[
						{"name":"/dev/sda","type":"sat","info_name":"sda"},
						{"name":"/dev/sdb","type":"scsi","info_name":"sdb"}
					]
				}`}, nil)
				mcli.On("deviceInfo", "/dev/sdb", "sat", mock.Anything).Return(&gjson.Result{Raw: `{}`}, nil)
			},
			expectedErr: false,
			expectCount: 2,
		},
		{
			name: "Scan with Extra Devices",
			setupMocks: func(mcli *mockSmartctlCli, c *Collector) {
				mcli.On("scan", false).Return(&gjson.Result{Raw: `{"devices":[]}`}, nil)
			},
			extraDevices: []ConfigExtraDevice{
				{Name: "/dev/sdc", Type: "sat"},
			},
			expectedErr: false,
			expectCount: 1,
		},
		{
			name: "No Devices Found",
			setupMocks: func(mcli *mockSmartctlCli, c *Collector) {
				mcli.On("scan", false).Return(&gjson.Result{Raw: `{"devices":[]}`}, nil)
			},
			expectedErr: true,
			expectCount: 0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockCli := &mockSmartctlCli{}
			c := New()
			c.exec = mockCli
			c.deviceSr = matcher.TRUE()
			c.ExtraDevices = tc.extraDevices

			tc.setupMocks(mockCli, c)

			devices, err := c.scanDevices()
			if tc.expectedErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Len(t, devices, tc.expectCount)
			}

			mockCli.AssertExpectations(t)
		})
	}
}

func TestScanDeviceKey(t *testing.T) {
	dev := &scanDevice{name: "/dev/sda", typ: "sat"}
	assert.Equal(t, "/dev/sda|sat", dev.key())
}

func TestScanDeviceShortName(t *testing.T) {
	dev := &scanDevice{name: "/dev/sda", typ: "sat"}
	assert.Equal(t, "sda", dev.shortName())
}
```