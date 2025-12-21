package vsphere

import (
	"testing"
	"errors"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/vmware/govmomi/performance"

	rs "github.com/netdata/netdata/go/plugins/plugin/go.d/collector/vsphere/resources"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/pkg/metrix"
)

type MockScraper struct {
	mock.Mock
}

func (m *MockScraper) ScrapeHosts(hosts rs.Hosts) []performance.EntityMetric {
	args := m.Called(hosts)
	return args.Get(0).([]performance.EntityMetric)
}

func (m *MockScraper) ScrapeVMs(vms rs.VMs) []performance.EntityMetric {
	args := m.Called(vms)
	return args.Get(0).([]performance.EntityMetric)
}

func TestCollect(t *testing.T) {
	testCases := []struct {
		name            string
		hostsMetrics    []performance.EntityMetric
		vmMetrics       []performance.EntityMetric
		expectedError   bool
	}{
		{
			name: "Successful Collection",
			hostsMetrics: []performance.EntityMetric{
				{
					Entity: struct{ Value string }{"host1"},
					Value: []performance.MetricSeries{
						{Name: "cpu.usage.average", Value: []int64{50}},
					},
				},
			},
			vmMetrics: []performance.EntityMetric{
				{
					Entity: struct{ Value string }{"vm1"},
					Value: []performance.MetricSeries{
						{Name: "mem.usage.average", Value: []int64{30}},
					},
				},
			},
			expectedError: false,
		},
		{
			name:            "No Hosts and VMs",
			hostsMetrics:    []performance.EntityMetric{},
			vmMetrics:       []performance.EntityMetric{},
			expectedError:   false,
		},
		{
			name: "Host Scrape Failure",
			hostsMetrics:    []performance.EntityMetric{},
			vmMetrics: []performance.EntityMetric{
				{
					Entity: struct{ Value string }{"vm1"},
					Value: []performance.MetricSeries{
						{Name: "mem.usage.average", Value: []int64{30}},
					},
				},
			},
			expectedError:   true,
		},
		{
			name: "VM Scrape Failure",
			hostsMetrics: []performance.EntityMetric{
				{
					Entity: struct{ Value string }{"host1"},
					Value: []performance.MetricSeries{
						{Name: "cpu.usage.average", Value: []int64{50}},
					},
				},
			},
			vmMetrics:       []performance.EntityMetric{},
			expectedError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			// Create mock collector
			collector := &Collector{
				resources: &rs.Resources{
					Hosts: rs.Hosts{
						"host1": &rs.Host{ID: "host1"},
					},
					VMs: rs.VMs{
						"vm1": &rs.VM{ID: "vm1"},
					},
				},
				discoveredHosts: make(map[string]int),
				discoveredVMs:   make(map[string]int),
			}

			// Mock scraper setup
			mockScraper := new(MockScraper)
			mockScraper.On("ScrapeHosts", mock.Anything).Return(tc.hostsMetrics)
			mockScraper.On("ScrapeVMs", mock.Anything).Return(tc.vmMetrics)

			// Replace the collector's scrapers with mock
			collector.ScrapeHosts = mockScraper.ScrapeHosts
			collector.ScrapeVMs = mockScraper.ScrapeVMs

			// Perform collect
			metrics, err := collector.collect()

			if tc.expectedError {
				assert.Error(t, err, "Should return an error")
			} else {
				assert.NoError(t, err, "Should not return an error")
				assert.NotNil(t, metrics, "Should return metrics")
			}

			mockScraper.AssertExpectations(t)
		})
	}
}

func TestCollectHostsMetrics(t *testing.T) {
	testCases := []struct {
		name         string
		hostMetrics  []performance.EntityMetric
		initialState map[string]int
		expectedMx   map[string]int64
	}{
		{
			name: "Successful Hosts Metrics Collection",
			hostMetrics: []performance.EntityMetric{
				{
					Entity: struct{ Value string }{"host1"},
					Value: []performance.MetricSeries{
						{Name: "cpu.usage.average", Value: []int64{50}},
						{Name: "disk.read.average", Value: []int64{1024}},
					},
				},
			},
			initialState: map[string]int{"host1": 0, "host2": 1},
			expectedMx: map[string]int64{
				"host1_cpu.usage.average": 50,
				"host1_disk.read.average": 1024,
				"host1_overall.status.green": 0,
				"host1_overall.status.red": 0,
				"host1_overall.status.yellow": 0,
				"host1_overall.status.gray": 0,
			},
		},
		{
			name:         "No Host Metrics",
			hostMetrics:  []performance.EntityMetric{},
			initialState: map[string]int{"host1": 1},
			expectedMx:   map[string]int64{},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			// Create mock collector
			collector := &Collector{
				resources: &rs.Resources{
					Hosts: rs.Hosts{
						"host1": &rs.Host{
							ID:             "host1",
							OverallStatus:  "", // Explicitly test status
						},
					},
				},
				discoveredHosts: tc.initialState,
			}

			// Metrics map to collect results
			mx := make(map[string]int64)

			// Call collectHostsMetrics
			collector.collectHostsMetrics(mx, tc.hostMetrics)

			// Validate results
			for k, v := range tc.expectedMx {
				assert.Equal(t, v, mx[k], fmt.Sprintf("Mismatch for key %s", k))
			}

			// Verify discovered hosts state
			if len(tc.hostMetrics) > 0 {
				assert.Equal(t, 0, collector.discoveredHosts["host1"], 
					"Discovered hosts counter should be reset for found hosts")
			}
		})
	}
}

func TestWriteHostMetrics(t *testing.T) {
	host := &rs.Host{
		ID:             "host1",
		OverallStatus:  "green",
	}

	metrics := []performance.MetricSeries{
		{Name: "cpu.usage.average", Value: []int64{50}},
		{Name: "disk.read.average", Value: []int64{1024}},
		{Name: "empty.metric", Value: []int64{}},
		{Name: "negative.metric", Value: []int64{-1}},
	}

	mx := make(map[string]int64)
	writeHostMetrics(mx, host, metrics)

	expectedMetrics := map[string]int64{
		"host1_cpu.usage.average": 50,
		"host1_disk.read.average": 1024,
		"host1_overall.status.green": 1,
		"host1_overall.status.red": 0,
		"host1_overall.status.yellow": 0,
		"host1_overall.status.gray": 0,
	}

	for k, v := range expectedMetrics {
		assert.Equal(t, v, mx[k], fmt.Sprintf("Mismatch for key %s", k))
	}
}

func TestCollectVMsMetrics(t *testing.T) {
	testCases := []struct {
		name         string
		vmMetrics    []performance.EntityMetric
		initialState map[string]int
		expectedMx   map[string]int64
	}{
		{
			name: "Successful VMs Metrics Collection",
			vmMetrics: []performance.EntityMetric{
				{
					Entity: struct{ Value string }{"vm1"},
					Value: []performance.MetricSeries{
						{Name: "mem.usage.average", Value: []int64{50}},
						{Name: "disk.write.average", Value: []int64{1024}},
					},
				},
			},
			initialState: map[string]int{"vm1": 0, "vm2": 1},
			expectedMx: map[string]int64{
				"vm1_mem.usage.average": 50,
				"vm1_disk.write.average": 1024,
				"vm1_overall.status.green": 0,
				"vm1_overall.status.red": 0,
				"vm1_overall.status.yellow": 0,
				"vm1_overall.status.gray": 0,
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			// Create mock collector
			collector := &Collector{
				resources: &rs.Resources{
					VMs: rs.VMs{
						"vm1": &rs.VM{
							ID:             "vm1",
							OverallStatus:  "", // Explicitly test status
						},
					},
				},
				discoveredVMs: tc.initialState,
			}

			// Metrics map to collect results
			mx := make(map[string]int64)

			// Call collectVMsMetrics
			collector.collectVMsMetrics(mx, tc.vmMetrics)

			// Validate results
			for k, v := range tc.expectedMx {
				assert.Equal(t, v, mx[k], fmt.Sprintf("Mismatch for key %s", k))
			}

			// Verify discovered VMs state
			if len(tc.vmMetrics) > 0 {
				assert.Equal(t, 0, collector.discoveredVMs["vm1"], 
					"Discovered VMs counter should be reset for found VMs")
			}
		})
	}
}

func TestWriteVMMetrics(t *testing.T) {
	vm := &rs.VM{
		ID:             "vm1",
		OverallStatus:  "green",
	}

	metrics := []performance.MetricSeries{
		{Name: "mem.usage.average", Value: []int64{50}},
		{Name: "disk.read.average", Value: []int64{1024}},
		{Name: "empty.metric", Value: []int64{}},
		{Name: "negative.metric", Value: []int64{-1}},
	}

	mx := make(map[string]int64)
	writeVMMetrics(mx, vm, metrics)

	expectedMetrics := map[string]int64{
		"vm1_mem.usage.average": 50,
		"vm1_disk.read.average": 1024,
		"vm1_overall.status.green": 1,
		"vm1_overall.status.red": 0,
		"vm1_overall.status.yellow": 0,
		"vm1_overall.status.gray": 0,
	}

	for k, v := range expectedMetrics {
		assert.Equal(t, v, mx[k], fmt.Sprintf("Mismatch for key %s", k))
	}
}