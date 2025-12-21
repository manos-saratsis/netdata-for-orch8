```go
package pulsar

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/prometheus"
	"github.com/stretchr/testify/assert"
)

func TestCollector_adjustCharts(t *testing.T) {
	tests := []struct {
		name     string
		metrics  prometheus.Series
		wantMaps []string
	}{
		{
			name: "Remove optional storage charts",
			metrics: prometheus.Series{
				{
					Name: metricPulsarTopicsCount,
					Labels: prometheus.Labels{
						"cluster": "test",
					},
				},
			},
			wantMaps: []string{
				"topicStorageReadRateChart.ID",
				"topicStorageWriteRateChart.ID",
			},
		},
		{
			name: "Remove optional subscription charts",
			metrics: prometheus.Series{
				{
					Name: metricPulsarTopicsCount,
					Labels: prometheus.Labels{
						"cluster": "test",
					},
				},
			},
			wantMaps: []string{
				"topicSubsMsgRateRedeliverChart.ID",
				"topicSubsBlockedOnUnackedMsgChart.ID",
			},
		},
		{
			name: "Remove optional replication charts",
			metrics: prometheus.Series{
				{
					Name: metricPulsarTopicsCount,
					Labels: prometheus.Labels{
						"cluster": "test",
					},
				},
			},
			wantMaps: []string{
				"topicReplicationRateInChart.ID",
				"topicReplicationRateOutChart.ID",
				"topicReplicationThroughputRateInChart.ID",
				"topicReplicationThroughputRateOutChart.ID",
				"topicReplicationBacklogChart.ID",
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			collector := New()

			collector.adjustCharts(tt.metrics)

			for _, chartKey := range tt.wantMaps {
				assert.NotContains(t, collector.topicChartsMapping, chartKey)
			}
		})
	}
}

func TestCollector_removeSummaryChart(t *testing.T) {
	collector := New()
	chartID := "test_chart"

	collector.charts.Add(&Chart{
		ID: chartID,
	})

	collector.removeSummaryChart(chartID)

	removedChart := collector.charts.Get(chartID)
	assert.Nil(t, removedChart)
}

func TestCollector_removeNamespaceChart(t *testing.T) {
	collector := New()
	chartID := "namespace_chart"

	collector.nsCharts.Add(&Chart{
		ID: chartID,
	})

	collector.removeNamespaceChart(chartID)

	removedChart := collector.nsCharts.Get(chartID)
	assert.Nil(t, removedChart)
}

func TestCollector_addNamespaceCharts(t *testing.T) {
	collector := New()
	ns := namespace{name: "test-namespace"}

	collector.addNamespaceCharts(ns)

	// Verify added charts
	for _, chart := range *collector.nsCharts {
		expectedID := chart.ID
		addedChart := collector.charts.Get(expectedID)
		assert.NotNil(t, addedChart)
	}
}

func TestCollector_removeNamespaceFromCharts(t *testing.T) {
	collector := New()
	ns := namespace{name: "test-namespace"}

	collector.addNamespaceCharts(ns)
	collector.removeNamespaceFromCharts(ns)

	// Verify charts are marked for removal
	for _, chart := range *collector.nsCharts {
		expectedID := chart.ID
		addedChart := collector.charts.Get(expectedID)
		assert.NotNil(t, addedChart)
		assert.True(t, addedChart.Removed)
	}
}

func TestCollector_extractTopicName(t *testing.T) {
	tests := []struct {
		name     string
		topic    topic
		expected string
	}{
		{
			name: "Full topic name",
			topic: topic{
				name: "persistent://sample/ns1/demo-1",
			},
			expected: "p:demo-1",
		},
		{
			name: "Short topic name",
			topic: topic{
				name: "demo-1",
			},
			expected: "demo-1",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := extractTopicName(tt.topic)
			assert.Equal(t, tt.expected, result)
		})
	}
}

func TestCollector_topicChartsMapping(t *testing.T) {
	mapping := topicChartsMapping()

	assert.NotEmpty(t, mapping)
	assert.Contains(t, mapping, topicSubscriptionsChart.ID)
	assert.Contains(t, mapping, topicProducersChart.ID)
	assert.Contains(t, mapping, topicConsumersChart.ID)
}
```