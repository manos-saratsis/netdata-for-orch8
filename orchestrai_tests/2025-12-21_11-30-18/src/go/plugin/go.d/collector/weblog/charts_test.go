package weblog

import (
	"testing"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestNewReqProcTimeHistChart(t *testing.T) {
	tests := []struct {
		name      string
		histogram []float64
		wantErr   bool
	}{
		{
			name:      "Valid histogram",
			histogram: []float64{0.1, 0.5, 1.0},
			wantErr:   false,
		},
		{
			name:      "Empty histogram",
			histogram: []float64{},
			wantErr:   false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			chart, err := newReqProcTimeHistChart(tt.histogram)

			if tt.wantErr {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			assert.NotNil(t, chart)
			
			// Verify number of dimensions matches histogram length + 1
			assert.Equal(t, len(tt.histogram)+1, len(chart.Dims))
		})
	}
}

func TestNewUpsRespTimeHistChart(t *testing.T) {
	tests := []struct {
		name      string
		histogram []float64
		wantErr   bool
	}{
		{
			name:      "Valid histogram",
			histogram: []float64{0.1, 0.5, 1.0},
			wantErr:   false,
		},
		{
			name:      "Empty histogram",
			histogram: []float64{},
			wantErr:   false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			chart, err := newUpsRespTimeHistChart(tt.histogram)

			if tt.wantErr {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			assert.NotNil(t, chart)
			
			// Verify number of dimensions matches histogram length + 1
			assert.Equal(t, len(tt.histogram)+1, len(chart.Dims))
		})
	}
}

func TestNewURLPatternChart(t *testing.T) {
	tests := []struct {
		name      string
		patterns  []userPattern
		wantErr   bool
	}{
		{
			name: "Valid patterns",
			patterns: []userPattern{
				{Name: "pattern1"},
				{Name: "pattern2"},
			},
			wantErr: false,
		},
		{
			name:      "Empty patterns",
			patterns:  []userPattern{},
			wantErr:   false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			chart, err := newURLPatternChart(tt.patterns)

			if tt.wantErr {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			assert.NotNil(t, chart)
			
			// Verify number of dimensions matches number of patterns
			assert.Equal(t, len(tt.patterns), len(chart.Dims))
		})
	}
}

func TestNewCustomFieldChart(t *testing.T) {
	tests := []struct {
		name        string
		customField customField
		wantErr     bool
	}{
		{
			name: "Valid custom field with patterns",
			customField: customField{
				Name: "test_field",
				Patterns: []customFieldPattern{
					{Name: "pattern1"},
					{Name: "pattern2"},
				},
			},
			wantErr: false,
		},
		{
			name: "Custom field with no patterns",
			customField: customField{
				Name:     "empty_field",
				Patterns: []customFieldPattern{},
			},
			wantErr: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			chart, err := newCustomFieldChart(tt.customField)

			if tt.wantErr {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			assert.NotNil(t, chart)
			
			// Verify chart properties
			assert.Equal(t, fmt.Sprintf("custom_field_%s", tt.customField.Name), chart.ID)
			assert.Equal(t, len(tt.customField.Patterns), len(chart.Dims))
		})
	}
}

func TestAddMethodCharts(t *testing.T) {
	tests := []struct {
		name      string
		charts    *Charts
		patterns  []userPattern
		wantErr   bool
	}{
		{
			name:     "With patterns",
			charts:   &Charts{},
			patterns: []userPattern{{Name: "test_pattern"}},
			wantErr:  false,
		},
		{
			name:     "Without patterns",
			charts:   &Charts{},
			patterns: []userPattern{},
			wantErr:  false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := addMethodCharts(tt.charts, tt.patterns)

			if tt.wantErr {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			
			// Verify base method chart exists
			methodChartExists := false
			for _, chart := range *tt.charts {
				if chart.ID == "requests_by_http_method" {
					methodChartExists = true
					break
				}
			}
			assert.True(t, methodChartExists)
			
			// Verify number of pattern charts matches number of patterns
			patternCharts := 0
			for _, chart := range *tt.charts {
				if len(chart.ID) > 20 && chart.ID[:20] == "url_pattern_" {
					patternCharts++
				}
			}
			assert.Equal(t, len(tt.patterns), patternCharts)
		})
	}
}

func TestAddCustomNumericFieldsCharts(t *testing.T) {
	tests := []struct {
		name     string
		charts   *module.Charts
		fields   []customNumericField
		wantErr  bool
	}{
		{
			name:   "With numeric fields",
			charts: &module.Charts{},
			fields: []customNumericField{
				{Name: "field1", Units: "bytes"},
				{Name: "field2", Units: "count"},
			},
			wantErr: false,
		},
		{
			name:     "Empty numeric fields",
			charts:   &module.Charts{},
			fields:   []customNumericField{},
			wantErr:  false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := addCustomNumericFieldsCharts(tt.charts, tt.fields)

			if tt.wantErr {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)
			
			// Verify number of charts matches number of fields
			assert.Equal(t, len(tt.fields), len(*tt.charts))
			
			// Verify chart properties
			for i, field := range tt.fields {
				chart := (*tt.charts)[i]
				assert.Equal(t, fmt.Sprintf("custom_numeric_field_%s_summary", field.Name), chart.ID)
				assert.Equal(t, field.Units, chart.Units)
			}
		})
	}
}