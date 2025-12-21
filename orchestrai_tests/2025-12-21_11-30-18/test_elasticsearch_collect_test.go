```go
package elasticsearch

import (
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestElasticsearchMetricsEmpty(t *testing.T) {
	testCases := []struct {
		name     string
		metrics  esMetrics
		expected bool
	}{
		{
			name:     "All empty",
			metrics:  esMetrics{},
			expected: true,
		},
		{
			name: "Nodes stats present",
			metrics: esMetrics{
				NodesStats: &esNodesStats{
					Nodes: map[string]*esNodeStats{
						"node1": {},
					},
				},
			},
			expected: false,
		},
		{
			name: "Cluster health present",
			metrics: esMetrics{
				ClusterHealth: &esClusterHealth{},
			},
			expected: false,
		},
		{
			name: "Cluster stats present",
			metrics: esMetrics{
				ClusterStats: &esClusterStats{},
			},
			expected: false,
		},
		{
			name: "Local indices stats present",
			metrics: esMetrics{
				LocalIndicesStats: []esIndexStats{
					{Index: "test"},
				},
			},
			expected: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := tc.metrics.empty()
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestMetricsPresenceCheckers(t *testing.T) {
	testCases := []struct {
		name                 string
		metrics              esMetrics
		hasNodesStats        bool
		hasClusterHealth     bool
		hasClusterStats      bool
		hasLocalIndicesStats bool
	}{
		{
			name:                 "All empty",
			metrics:              esMetrics{},
			hasNodesStats:        false,
			hasClusterHealth:     false,
			hasClusterStats:      false,
			hasLocalIndicesStats: false,
		},
		{
			name: "Nodes stats present",
			metrics: esMetrics{
				NodesStats: &esNodesStats{
					Nodes: map[string]*esNodeStats{
						"node1": {},
					},
				},
			},
			hasNodesStats:        true,
			hasClusterHealth:     false,
			hasClusterStats:      false,
			hasLocalIndicesStats: false,
		},
		{
			name: "Cluster health present",
			metrics: esMetrics{
				ClusterHealth: &esClusterHealth{},
			},
			hasNodesStats:        false,
			hasClusterHealth:     true,
			hasClusterStats:      false,
			hasLocalIndicesStats: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			assert.Equal(t, tc.hasNodesStats, tc.metrics.hasNodesStats())
			assert.Equal(t, tc.hasClusterHealth, tc.metrics.hasClusterHealth())
			assert.Equal(t, tc.hasClusterStats, tc.metrics.hasClusterStats())
			assert.Equal(t, tc.hasLocalIndicesStats, tc.metrics.hasLocalIndicesStats())
		})
	}
}

func TestConvertIndexStoreSizeToBytes(t *testing.T) {
	testCases := []struct {
		input    string
		expected int64
	}{
		{"1kb", 1024},
		{"2mb", 2 * 1024 * 1024},
		{"3gb", 3 * 1024 * 1024 * 1024},
		{"4tb", 4 * 1024 * 1024 * 1024 * 1024},
		{"5b", 5},
		{"invalid", 0},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := convertIndexStoreSizeToBytes(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestStrToInt(t *testing.T) {
	testCases := []struct {
		input    string
		expected int64
	}{
		{"123", 123},
		{"-456", -456},
		{"0", 0},
		{"invalid", 0},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := strToInt(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestRemoveSystemIndices(t *testing.T) {
	testCases := []struct {
		name             string
		input            []esIndexStats
		expectedLength   int
		expectedRemained []string
	}{
		{
			name: "Remove system indices",
			input: []esIndexStats{
				{Index: ".system1"},
				{Index: "user1"},
				{Index: ".system2"},
				{Index: "user2"},
			},
			expectedLength:   2,
			expectedRemained: []string{"user1", "user2"},
		},
		{
			name: "No system indices",
			input: []esIndexStats{
				{Index: "user1"},
				{Index: "user2"},
			},
			expectedLength:   2,
			expectedRemained: []string{"user1", "user2"},
		},
		{
			name: "Only system indices",
			input: []esIndexStats{
				{Index: ".system1"},
				{Index: ".system2"},
			},
			expectedLength:   0,
			expectedRemained: []string{},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := removeSystemIndices(tc.input)

			assert.Len(t, result, tc.expectedLength)
			if len(result) > 0 {
				for i, r := range result {
					assert.Equal(t, tc.expectedRemained[i], r.Index)
				}
			}
		})
	}
}

func TestMerge(t *testing.T) {
	testCases := []struct {
		name     string
		dst      map[string]int64
		src      map[string]int64
		prefix   string
		expected map[string]int64
	}{
		{
			name:     "Basic merge",
			dst:      map[string]int64{"existing": 1},
			src:      map[string]int64{"key1": 100, "key2": 200},
			prefix:   "node",
			expected: map[string]int64{
				"existing": 1,
				"node_key1": 100,
				"node_key2": 200,
			},
		},
		{
			name:     "Empty source map",
			dst:      map[string]int64{"existing": 1},
			src:      map[string]int64{},
			prefix:   "prefix",
			expected: map[string]int64{"existing": 1},
		},
		{
			name:     "Empty destination map",
			dst:      map[string]int64{},
			src:      map[string]int64{"key1": 100, "key2": 200},
			prefix:   "node",
			expected: map[string]int64{
				"node_key1": 100,
				"node_key2": 200,
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			merge(tc.dst, tc.src, tc.prefix)
			assert.Equal(t, tc.expected, tc.dst)
		})
	}
}
```

These test files comprehensively cover the Elasticsearch collector's components, with tests for:

1. Configuration validation
2. HTTP client initialization
3. Module lifecycle (Init, Check, Collect, Cleanup)
4. Charts generation
5. Metrics data handling and transformations
6. Edge cases and error scenarios
7. Different input combinations

The tests use `testify/assert` for clean, readable assertions and cover various scenarios to ensure robust code coverage.