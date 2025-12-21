```go
package nsd

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_collectStatsLine(t *testing.T) {
	tests := []struct {
		name     string
		line     string
		expected map[string]int64
	}{
		{
			name:     "Empty Line",
			line:     "   ",
			expected: map[string]int64{},
		},
		{
			name: "Normal Integer Metric",
			line: "num.queries=100",
			expected: map[string]int64{
				"num.queries": 100,
			},
		},
		{
			name: "Boot Time Float Metric",
			line: "time.boot=1234.56",
			expected: map[string]int64{
				"time.boot": 1234,
			},
		},
		{
			name:     "Invalid Line Format",
			line:     "invalid_line",
			expected: map[string]int64{},
		},
		{
			name:     "Invalid Value",
			line:     "num.queries=invalid",
			expected: map[string]int64{},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := &Collector{}
			mx := make(map[string]int64)
			c.collectStatsLine(mx, tt.line)

			for k, v := range tt.expected {
				assert.Equal(t, v, mx[k])
			}
		})
	}
}

func Test_addMissingMetrics(t *testing.T) {
	tests := []struct {
		name     string
		mx       map[string]int64
		prefix   string
		values   []string
		expected map[string]int64
	}{
		{
			name:     "No Existing Metrics",
			mx:       make(map[string]int64),
			prefix:   "num.rcode.",
			values:   []string{"NOERROR", "FORMERR"},
			expected: map[string]int64{"num.rcode.NOERROR": 0, "num.rcode.FORMERR": 0},
		},
		{
			name:     "Some Existing Metrics",
			mx:       map[string]int64{"num.rcode.NOERROR": 10},
			prefix:   "num.rcode.",
			values:   []string{"NOERROR", "FORMERR"},
			expected: map[string]int64{"num.rcode.NOERROR": 10, "num.rcode.FORMERR": 0},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			addMissingMetrics(tt.mx, tt.prefix, tt.values)
			assert.Equal(t, tt.expected, tt.mx)
		})
	}
}

func Test_collect(t *testing.T) {
	tests := []struct {
		name        string
		stats       []byte
		mockExec    func(*mockNsdControlBinary)
		wantMetrics bool
		wantErr     bool
	}{
		{
			name:  "Successful Collection",
			stats: []byte("num.queries=100\nnum.answer=50"),
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte("num.queries=100\nnum.answer=50"), nil)
			},
			wantMetrics: true,
			wantErr:     false,
		},
		{
			name:  "Empty Stats",
			stats: []byte{},
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte{}, nil)
			},
			wantMetrics: false,
			wantErr:     true,
		},
		{
			name:  "No Metrics Found",
			stats: []byte("invalid data"),
			mockExec: func(mock *mockNsdControlBinary) {
				mock.On("stats").Return([]byte("invalid data"), nil)
			},
			wantMetrics: false,
			wantErr:     true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := New()
			mockExec := &mockNsdControlBinary{}
			tt.mockExec(mockExec)
			c.exec = mockExec

			metrics, err := c.collect()

			if tt.wantErr {
				assert.Error(t, err)
				assert.Nil(t, metrics)
			} else {
				assert.NoError(t, err)
				if tt.wantMetrics {
					assert.NotEmpty(t, metrics)
				} else {
					assert.Empty(t, metrics)
				}
			}
		})
	}
}
```