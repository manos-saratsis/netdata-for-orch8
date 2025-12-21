```go
package sql

import (
	"testing"
	"github.com/stretchr/testify/assert"
)

func TestCollectorValidateConfig(t *testing.T) {
	testCases := []struct {
		name    string
		config  *Collector
		wantErr bool
		errMsg  string
	}{
		{
			name: "Valid Config",
			config: &Collector{
				Driver: "mysql",
				DSN:    "user:pass@tcp(localhost:3306)/db",
				Metrics: []ConfigMetricBlock{
					{
						ID:     "test_metric",
						Query:  "SELECT 1",
						Mode:   "columns",
						Charts: []ConfigChartConfig{
							{
								Title:    "Test Chart",
								Context: "test_context",
								Family:  "test_family",
								Units:   "test_units",
								Dims: []ConfigDimConfig{
									{
										Name:   "test_dim",
										Source: "test_source",
									},
								},
							},
						},
					},
				},
			},
			wantErr: false,
		},
		{
			name: "Missing Driver",
			config: &Collector{
				DSN: "user:pass@tcp(localhost:3306)/db",
			},
			wantErr: true,
			errMsg: "driver required",
		},
		{
			name: "Unsupported Driver",
			config: &Collector{
				Driver: "unsupported",
				DSN:    "user:pass@tcp(localhost:3306)/db",
			},
			wantErr: true,
			errMsg: "unsupported driver \"unsupported\"",
		},
		{
			name: "Missing DSN",
			config: &Collector{
				Driver: "mysql",
			},
			wantErr: true,
			errMsg: "dsn required",
		},
		{
			name: "Missing Metrics",
			config: &Collector{
				Driver: "mysql",
				DSN:    "user:pass@tcp(localhost:3306)/db",
			},
			wantErr: true,
			errMsg: "missing metrics",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			err := tc.config.validateConfig()
			if tc.wantErr {
				assert.Error(t, err)
				if tc.errMsg != "" {
					assert.Contains(t, err.Error(), tc.errMsg)
				}
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestConfigQueryDefValidate(t *testing.T) {
	testCases := []struct {
		name    string
		query   ConfigQueryDef
		seen    map[string]bool
		wantErr bool
		errMsg  string
	}{
		{
			name: "Valid Query",
			query: ConfigQueryDef{
				ID:    "test_query",
				Query: "SELECT 1",
			},
			seen:    map[string]bool{},
			wantErr: false,
		},
		{
			name: "Missing Query ID",
			query: ConfigQueryDef{
				Query: "SELECT 1",
			},
			seen:    map[string]bool{},
			wantErr: true,
			errMsg: "missing id",
		},
		{
			name: "Missing Query",
			query: ConfigQueryDef{
				ID: "test_query",
			},
			seen:    map[string]bool{},
			wantErr: true,
			errMsg: "missing query",
		},
		{
			name: "Duplicate Query ID",
			query: ConfigQueryDef{
				ID:    "test_query",
				Query: "SELECT 1",
			},
			seen:    map[string]bool{"test_query": true},
			wantErr: true,
			errMsg: "duplicate id",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			errs := tc.query.validate(0, tc.seen)
			if tc.wantErr {
				assert.NotEmpty(t, errs)
				assert.Contains(t, errs[0].Error(), tc.errMsg)
			} else {
				assert.Empty(t, errs)
			}
		})
	}
}

func TestConfigMetricBlockValidate(t *testing.T) {
	testCases := []struct {
		name     string
		metric   ConfigMetricBlock
		queryIdx map[string]bool
		wantErr  bool
		errMsg   string
	}{
		{
			name: "Valid Columns Mode Metric",
			metric: ConfigMetricBlock{
				ID:      "test_metric",
				Query:   "SELECT 1",
				Mode:    "columns",
				Charts: []ConfigChartConfig{
					{
						Title:    "Test Chart",
						Context: "test_context",
						Family:  "test_family",
						Units:   "test_units",
						Dims: []ConfigDimConfig{
							{
								Name:   "test_dim",
								Source: "test_source",
							},
						},
					},
				},
			},
			queryIdx: map[string]bool{},
			wantErr:  false,
		},
		{
			name: "Valid KV Mode Metric",
			metric: ConfigMetricBlock{
				ID:     "test_metric",
				Query:  "SELECT 1",
				Mode:   "kv",
				KVMode: &ConfigKVMode{
					NameCol:  "name",
					ValueCol: "value",
				},
				Charts: []ConfigChartConfig{
					{
						Title:    "Test Chart",
						Context: "test_context",
						Family:  "test_family",
						Units:   "test_units",
						Dims: []ConfigDimConfig{
							{
								Name:   "test_dim",
								Source: "test_key",
							},
						},
					},
				},
			},
			queryIdx: map[string]bool{},
			wantErr:  false,
		},
		{
			name: "Missing Metric ID",
			metric: ConfigMetricBlock{
				Query: "SELECT 1",
				Mode:  "columns",
			},
			queryIdx: map[string]bool{},
			wantErr:  true,
			errMsg:   "missing id",
		},
		{
			name: "Missing Query and QueryRef",
			metric: ConfigMetricBlock{
				ID:   "test_metric",
				Mode: "columns",
			},
			queryIdx: map[string]bool{},
			wantErr:  true,
			errMsg:   "must set exactly one of query_ref or query",
		},
		{
			name: "Invalid Mode",
			metric: ConfigMetricBlock{
				ID:     "test_metric",
				Query:  "SELECT 1",
				Mode:   "invalid",
				Charts: []ConfigChartConfig{},
			},
			queryIdx: map[string]bool{},
			wantErr:  true,
			errMsg:   "invalid mode",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			errs := tc.metric.validate(0, tc.queryIdx)
			if tc.wantErr {
				assert.NotEmpty(t, errs)
				assert.Contains(t, errs[0].Error(), tc.errMsg)
			} else {
				assert.Empty(t, errs)
			}
		})
	}
}

func TestConfigStatusWhenValidate(t *testing.T) {
	testCases := []struct {
		name     string
		status   ConfigStatusWhen
		wantErr  bool
		errMsg   string
	}{
		{
			name: "Valid Equals Status",
			status: ConfigStatusWhen{
				Equals: "value",
			},
			wantErr: false,
		},
		{
			name: "Valid In Status",
			status: ConfigStatusWhen{
				In: []string{"value1", "value2"},
			},
			wantErr: false,
		},
		{
			name: "Valid Regex Status",
			status: ConfigStatusWhen{
				Match: "^test.*",
			},
			wantErr: false,
		},
		{
			name: "Invalid No Status Selector",
			status: ConfigStatusWhen{},
			wantErr: true,
			errMsg: "must have exactly one of equals|in|match",
		},
		{
			name: "Invalid Multiple Status Selectors",
			status: ConfigStatusWhen{
				Equals: "value",
				Match:  "^test.*",
			},
			wantErr: true,
			errMsg: "must not set multiple selectors",
		},
		{
			name: "Invalid Regex",
			status: ConfigStatusWhen{
				Match: "[invalid-regex",
			},
			wantErr: true,
			errMsg: "invalid regex in status_when.match",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			errs := tc.status.validate(0, "test_metric", 0, 0)
			if tc.wantErr {
				assert.NotEmpty(t, errs)
				assert.Contains(t, errs[0].Error(), tc.errMsg)
			} else {
				assert.Empty(t, errs)
			}
		})
	}
}
```