```go
package postgres

import (
	"testing"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestNewRunningTimeHistogramChart(t *testing.T) {
	t.Run("success case", func(t *testing.T) {
		tmpl := module.Chart{
			ID:       "test_template",
			Title:    "Test Histogram",
			Units:    "seconds",
			Priority: 1,
		}
		buckets := []float64{0.1, 0.5, 1.0}
		
		chart, err := newRunningTimeHistogramChart(tmpl, "test_prefix", buckets)
		
		assert.NoError(t, err)
		assert.NotNil(t, chart)
		assert.Equal(t, "test_template", chart.ID)
		assert.Len(t, chart.Dims, len(buckets)+1)
	})

	t.Run("error case with nil dimensions", func(t *testing.T) {
		tmpl := module.Chart{}
		buckets := []float64{}
		
		chart, err := newRunningTimeHistogramChart(tmpl, "test_prefix", buckets)
		
		assert.Error(t, err)
		assert.Nil(t, chart)
	})
}

func TestCollector_AddTransactionsRunTimeHistogramChart(t *testing.T) {
	collector := &Collector{
		XactTimeHistogram: []float64{0.1, 0.5, 1.0},
	}

	t.Run("successful chart addition", func(t *testing.T) {
		charts := module.Charts{}
		collector.Charts = &charts

		collector.addTransactionsRunTimeHistogramChart()

		assert.Len(t, charts, 1)
		assert.Equal(t, "transactions_duration", charts[0].ID)
	})
}

func TestNewDatabaseCharts(t *testing.T) {
	dbMetrics := &dbMetrics{
		name: "testdb",
	}

	t.Run("create database charts", func(t *testing.T) {
		tmpl := &module.Charts{
			&module.Chart{
				ID:       "db_%s_test_chart",
				Title:    "Test Chart",
				Dims: module.Dims{
					{ID: "db_%s_test_dim"},
				},
			},
		}

		charts := newDatabaseCharts(tmpl, dbMetrics)

		assert.Len(t, *charts, 1)
		chart := (*charts)[0]
		assert.Equal(t, "db_testdb_test_chart", chart.ID)
		assert.Equal(t, "testdb", chart.Labels[0].Value)
		assert.Equal(t, "db_testdb_test_dim", chart.Dims[0].ID)
	})
}

func TestNewTableCharts(t *testing.T) {
	tableMetrics := &tableMetrics{
		name:   "testtable",
		db:     "testdb",
		schema: "public",
	}

	t.Run("create table charts with bloat metrics", func(t *testing.T) {
		tableMetrics.bloatSize = &float64Value{} // Non-nil to test chart inclusion

		charts := newTableCharts(tableMetrics)

		assert.NotNil(t, charts)
		assert.Contains(t, (*charts)[0].ID, "testtable")
		assert.Contains(t, (*charts)[0].ID, "testdb")
		assert.Contains(t, (*charts)[0].ID, "public")
	})

	t.Run("create table charts without bloat metrics", func(t *testing.T) {
		tableMetrics.bloatSize = nil

		charts := newTableCharts(tableMetrics)

		assert.NotNil(t, charts)
		// Verify that bloat-related charts are removed
		for _, chart := range *charts {
			assert.NotContains(t, chart.ID, "bloat_size")
		}
	})
}

func TestCollector_AddChartMethods(t *testing.T) {
	collector := &Collector{
		Charts: module.NewCharts(),
	}

	dbMetrics := &dbMetrics{
		name: "testdb",
	}

	tableMetrics := &tableMetrics{
		name:   "testtable",
		db:     "testdb",
		schema: "public",
	}

	indexMetrics := &indexMetrics{
		name:    "testindex",
		table:   "testtable",
		db:      "testdb",
		schema:  "public",
	}

	t.Run("add database charts", func(t *testing.T) {
		collector.addNewDatabaseCharts(dbMetrics)
		assert.NotEmpty(t, *collector.Charts)
	})

	t.Run("add table charts", func(t *testing.T) {
		collector.addNewTableCharts(tableMetrics)
		assert.NotEmpty(t, *collector.Charts)
	})

	t.Run("add index charts", func(t *testing.T) {
		collector.addNewIndexCharts(indexMetrics)
		assert.NotEmpty(t, *collector.Charts)
	})
}

func TestCollector_RemoveChartMethods(t *testing.T) {
	collector := &Collector{
		Charts: module.NewCharts(),
	}

	// First add some charts
	dbMetrics := &dbMetrics{name: "testdb"}
	tableMetrics := &tableMetrics{
		name:   "testtable", 
		db:     "testdb", 
		schema: "public",
	}
	indexMetrics := &indexMetrics{
		name:    "testindex",
		table:   "testtable",
		db:      "testdb", 
		schema:  "public",
	}

	collector.addNewDatabaseCharts(dbMetrics)
	collector.addNewTableCharts(tableMetrics)
	collector.addNewIndexCharts(indexMetrics)

	t.Run("remove database charts", func(t *testing.T) {
		initialChartCount := len(*collector.Charts)
		collector.removeDatabaseCharts(dbMetrics)
		
		for _, chart := range *collector.Charts {
			if chart.ID == "db_testdb_test_chart" {
				assert.True(t, chart.IsRemoved())
			}
		}
	})

	t.Run("remove table charts", func(t *testing.T) {
		collector.removeTableCharts(tableMetrics)
		
		for _, chart := range *collector.Charts {
			if chart.ID == "table_testtable_db_testdb_schema_public_test_chart" {
				assert.True(t, chart.IsRemoved())
			}
		}
	})

	t.Run("remove index charts", func(t *testing.T) {
		collector.removeIndexCharts(indexMetrics)
		
		for _, chart := range *collector.Charts {
			if chart.ID == "index_testindex_table_testtable_db_testdb_schema_public_test_chart" {
				assert.True(t, chart.IsRemoved())
			}
		}
	})
}
```