package lvm

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestAddLVMThinPoolCharts(t *testing.T) {
	testCases := []struct {
		name         string
		lvName       string
		vgName       string
		expectedID   string
		expectedLen  int
		expectWarning bool
	}{
		{
			name:         "Normal Thin Pool Chart Creation",
			lvName:       "testLV",
			vgName:       "testVG",
			expectedID:   "lv_testLV_vg_testVG_lv_data_space_utilization",
			expectedLen:  2,
			expectWarning: false,
		},
		{
			name:         "Empty Names",
			lvName:       "",
			vgName:       "",
			expectedID:   "lv__vg__lv_data_space_utilization",
			expectedLen:  2,
			expectWarning: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				charts:       &module.Charts{},
				lvmThinPools: make(map[string]bool),
			}

			c.addLVMThinPoolCharts(tc.lvName, tc.vgName)

			// Check the number of charts added
			assert.Equal(t, tc.expectedLen, len(*c.charts))

			// Verify chart details
			if len(*c.charts) > 0 {
				firstChart := (*c.charts)[0]
				assert.Equal(t, tc.expectedID, firstChart.ID)
				
				// Verify labels
				assert.Equal(t, []module.Label{
					{Key: "lv_name", Value: tc.lvName},
					{Key: "vg_name", Value: tc.vgName},
					{Key: "volume_type", Value: "thin_pool"},
				}, firstChart.Labels)
			}
		})
	}
}

// Test chart templates to ensure they are defined correctly
func TestLVMChartTemplates(t *testing.T) {
	t.Run("Data Space Utilization Chart Template", func(t *testing.T) {
		assert.NotNil(t, lvDataSpaceUtilizationChartTmpl)
		assert.Equal(t, "percentage", lvDataSpaceUtilizationChartTmpl.Units)
		assert.Equal(t, "lv space usage", lvDataSpaceUtilizationChartTmpl.Fam)
		assert.Equal(t, module.Area, lvDataSpaceUtilizationChartTmpl.Type)
	})

	t.Run("Metadata Space Utilization Chart Template", func(t *testing.T) {
		assert.NotNil(t, lvMetadataSpaceUtilizationChartTmpl)
		assert.Equal(t, "percentage", lvMetadataSpaceUtilizationChartTmpl.Units)
		assert.Equal(t, "lv space usage", lvMetadataSpaceUtilizationChartTmpl.Fam)
		assert.Equal(t, module.Area, lvMetadataSpaceUtilizationChartTmpl.Type)
	})
}