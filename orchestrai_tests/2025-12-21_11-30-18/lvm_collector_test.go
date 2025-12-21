package lvm

import (
	"context"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
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

func TestNewCollector(t *testing.T) {
	c := New()
	assert.NotNil(t, c)
	assert.NotNil(t, c.charts)
	assert.NotNil(t, c.lvmThinPools)
	assert.Equal(t, time.Second*2, c.Timeout.Duration())
}

func TestCollectorConfiguration(t *testing.T) {
	c := New()
	config := c.Configuration()
	assert.NotNil(t, config)
}

func TestCollectorInit(t *testing.T) {
	t.Run("Successful Initialization", func(t *testing.T) {
		c := New()
		err := c.Init(context.Background())
		assert.NoError(t, err)
		assert.NotNil(t, c.exec)
	})
}

func TestCollectorCheck(t *testing.T) {
	testCases := []struct {
		name           string
		jsonResponse   string
		expectError    bool
	}{
		{
			name: "Successful Check",
			jsonResponse: `{
				"report": [{
					"lv": [{
						"vg_name": "testVG",
						"lv_name": "testLV", 
						"lv_attr": "t",
						"data_percent": "0.5"
					}]
				}]
			}`,
			expectError: false,
		},
		{
			name:           "No Metrics Collected",
			jsonResponse:   `{"report": [{"lv": []}]}`,
			expectError:    true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			mockExec := new(mockLVMCLI)
			mockExec.On("lvsReportJson").Return([]byte(tc.jsonResponse), nil)
			c.exec = mockExec

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
		name           string
		jsonResponse   string
		expectedMetrics map[string]int64
	}{
		{
			name: "Successful Metrics Collection",
			jsonResponse: `{
				"report": [{
					"lv": [{
						"vg_name": "testVG",
						"lv_name": "testLV", 
						"lv_attr": "t",
						"data_percent": "0.5",
						"metadata_percent": "0.3"
					}]
				}]
			}`,
			expectedMetrics: map[string]int64{
				"lv_testLV_vg_testVG_data_percent": 50,
				"lv_testLV_vg_testVG_metadata_percent": 30,
			},
		},
		{
			name:           "No Metrics",
			jsonResponse:   `{"report": [{"lv": []}]}`,
			expectedMetrics: nil,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			mockExec := new(mockLVMCLI)
			mockExec.On("lvsReportJson").Return([]byte(tc.jsonResponse), nil)
			c.exec = mockExec

			metrics := c.Collect(context.Background())
			assert.Equal(t, tc.expectedMetrics, metrics)
		})
	}
}

func TestCollectorCharts(t *testing.T) {
	c := New()
	charts := c.Charts()
	assert.NotNil(t, charts)
}

func TestCollectorCleanup(t *testing.T) {
	c := New()
	// Ensure no panic occurs
	assert.NotPanics(t, func() {
		c.Cleanup(context.Background())
	})
}