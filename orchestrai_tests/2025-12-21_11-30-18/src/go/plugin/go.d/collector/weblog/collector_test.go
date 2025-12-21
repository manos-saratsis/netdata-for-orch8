package weblog

import (
	"context"
	"testing"
	"github.com/stretchr/testify/assert"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
)

func TestNew(t *testing.T) {
	collector := New()
	assert.NotNil(t, collector)
	assert.Equal(t, "*.gz", collector.ExcludePath)
	assert.True(t, collector.GroupRespCodes)
	assert.Equal(t, "auto", collector.ParserConfig.LogType)
}

func TestCollector_Configuration(t *testing.T) {
	collector := &Collector{
		Config: Config{
			UpdateEvery: 10,
			Path:        "/var/log/web.log",
			ExcludePath: "test*.log",
			GroupRespCodes: true,
		},
	}

	config := collector.Configuration()
	assert.NotNil(t, config)
	
	typedConfig, ok := config.(Config)
	assert.True(t, ok)
	assert.Equal(t, 10, typedConfig.UpdateEvery)
	assert.Equal(t, "/var/log/web.log", typedConfig.Path)
	assert.Equal(t, "test*.log", typedConfig.ExcludePath)
	assert.True(t, typedConfig.GroupRespCodes)
}

func TestCollector_Init(t *testing.T) {
	tests := []struct {
		name          string
		configureTest func(*Collector)
		wantErr       bool
	}{
		{
			name: "Minimal Config",
			configureTest: func(c *Collector) {
				c.Config.URLPatterns = []userPattern{
					{Name: "test_pattern", Match: "/test"},
				}
			},
			wantErr: false,
		},
		{
			name: "Invalid Custom Fields",
			configureTest: func(c *Collector) {
				c.Config.CustomFields = []customField{
					{Name: "", Patterns: []userPattern{}},
				}
			},
			wantErr: true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			collector := New()
			
			if tt.configureTest != nil {
				tt.configureTest(collector)
			}

			err := collector.Init(context.Background())

			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, collector.mx)
			}
		})
	}
}

func TestCollector_Check(t *testing.T) {
	tests := []struct {
		name          string
		configureTest func(*Collector)
		wantErr       bool
	}{
		{
			name: "Successful check with valid configuration",
			configureTest: func(c *Collector) {
				c.line = &logLine{} // Provide a minimal valid log line
				c.Config.Path = "testdata/sample.log" // Assuming you have a sample log file
			},
			wantErr: false,
		},
		{
			name: "Failed check due to invalid log reader",
			configureTest: func(c *Collector) {
				c.Config.Path = "/path/to/nonexistent/log" // Use an invalid path
			},
			wantErr: true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			collector := New()
			
			if tt.configureTest != nil {
				tt.configureTest(collector)
			}

			err := collector.Check(context.Background())

			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, collector.charts)
			}
		})
	}
}

func TestCollector_Collect(t *testing.T) {
	tests := []struct {
		name          string
		configureTest func(*Collector)
		wantNonEmpty  bool
	}{
		{
			name: "Successful collect with log data",
			configureTest: func(c *Collector) {
				// Setup a mock collect method to return some data
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{
						"requests": 100,
					}, nil
				}
			},
			wantNonEmpty: true,
		},
		{
			name: "Collect with no data",
			configureTest: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{}, nil
				}
			},
			wantNonEmpty: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			collector := New()
			
			if tt.configureTest != nil {
				tt.configureTest(collector)
			}

			result := collector.Collect(context.Background())

			if tt.wantNonEmpty {
				assert.NotEmpty(t, result)
			} else {
				assert.Empty(t, result)
			}
		})
	}
}

func TestCollector_Cleanup(t *testing.T) {
	collector := New()
	
	// Simulate log file creation
	collector.file = &logs.Reader{} // Mock or use an actual reader if needed

	// Run cleanup
	collector.Cleanup(context.Background())

	// Since current implementation doesn't expose much, just verify it doesn't panic
	assert.NotPanics(t, func() {
		collector.Cleanup(context.Background())
	})
}