```go
package rabbitmq

import (
	"errors"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestCollector_collectNodes(t *testing.T) {
	testCases := []struct {
		name           string
		mockNodes      []apiNodeResp
		expectError    bool
		expectedMetrics map[string]int64
	}{
		{
			name: "Successful node collection with single node",
			mockNodes: []apiNodeResp{
				{
					Name:           "rabbit@test",
					Running:        true,
					OsPid:          "12345",
					Partitions:     []string{},
					MemAlarm:       false,
					DiskFreeAlarm:  false,
					FDTotal:        1000,
					FDUsed:         500,
					MemLimit:       2048,
					MemUsed:        1024,
					SocketsTotal:   100,
					SocketsUsed:    50,
					ProcTotal:      256,
					ProcUsed:       128,
					DiskFree:       1000000,
					RunQueue:       2,
					Uptime:         60000,
					ClusterLinks: []apiNodeClusterLink{
						{
							Name:      "peer1",
							RecvBytes: 1000,
							SendBytes: 2000,
						},
					},
				},
			},
			expectedMetrics: map[string]int64{
				"node_rabbit@test_avail_status_running":               1,
				"node_rabbit@test_avail_status_down":                  0,
				"node_rabbit@test_network_partition_status_clear":     1,
				"node_rabbit@test_network_partition_status_detected":  0,
				"node_rabbit@test_mem_alarm_status_clear":             1,
				"node_rabbit@test_mem_alarm_status_triggered":         0,
				"node_rabbit@test_disk_free_alarm_status_clear":       1,
				"node_rabbit@test_disk_free_alarm_status_triggered":   0,
				"node_rabbit@test_fds_available":                      500,
				"node_rabbit@test_fds_used":                           500,
				"node_rabbit@test_mem_available":                      1024,
				"node_rabbit@test_mem_used":                           1024,
				"node_rabbit@test_sockets_available":                  50,
				"node_rabbit@test_sockets_used":                       50,
				"node_rabbit@test_procs_available":                    128,
				"node_rabbit@test_procs_used":                         128,
				"node_rabbit@test_disk_free_bytes":                    1000000,
				"node_rabbit@test_run_queue":                          2,
				"node_rabbit@test_uptime":                             60,
				"node_rabbit@test_peer_peer1_cluster_link_recv_bytes": 1000,
				"node_rabbit@test_peer_peer1_cluster_link_send_bytes": 2000,
			},
		},
		{
			name: "Node with network partition and alarms",
			mockNodes: []apiNodeResp{
				{
					Name:           "rabbit@test",
					Running:        false,
					OsPid:          "12345",
					Partitions:     []string{"partition1"},
					MemAlarm:       true,
					DiskFreeAlarm:  true,
					FDTotal:        1000,
					FDUsed:         500,
					MemLimit:       2048,
					MemUsed:        1024,
					SocketsTotal:   100,
					SocketsUsed:    50,
					ProcTotal:      256,
					ProcUsed:       128,
					DiskFree:       1000000,
					RunQueue:       2,
					Uptime:         60000,
				},
			},
			expectedMetrics: map[string]int64{
				"node_rabbit@test_avail_status_running":               0,
				"node_rabbit@test_avail_status_down":                  1,
				"node_rabbit@test_network_partition_status_clear":     0,
				"node_rabbit@test_network_partition_status_detected":  1,
				"node_rabbit@test_mem_alarm_status_clear":             0,
				"node_rabbit@test_mem_alarm_status_triggered":         1,
				"node_rabbit@test_disk_free_alarm_status_clear":       0,
				"node_rabbit@test_disk_free_alarm_status_triggered":   1,
			},
		},
		{
			name: "Node without OS PID",
			mockNodes: []apiNodeResp{
				{
					Name:     "rabbit@test",
					Running:  true,
					OsPid:    "",
				},
			},
			expectedMetrics: map[string]int64{
				"node_rabbit@test_avail_status_running": 1,
				"node_rabbit@test_avail_status_down":    0,
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				RequestConfig: &web.RequestConfig{},
				cache:         &collectCache{},
			}

			mockClient := &mockWebClient{}
			mockClient.On("RequestJSON", mock.Anything, &[]apiNodeResp{}).Return(func(req *http.Request, v *[]apiNodeResp) error {
				*v = tc.mockNodes
				return nil
			})

			c.httpClient = &http.Client{}
			c.webClient = func() *web.Client {
				return &web.Client{
					Do: mockClient.RequestJSON,
				}
			}

			metrics := make(map[string]int64)
			err := c.collectNodes(metrics)

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			// Check metrics for each key that should exist
			for key, expectedValue := range tc.expectedMetrics {
				assert.Equal(t, expectedValue, metrics[key], "Metric mismatch for key: %s", key)
			}

			mockClient.AssertExpectations(t)
		})
	}

	t.Run("Web Client Request Error", func(t *testing.T) {
		c := &Collector{
			RequestConfig: &web.RequestConfig{},
		}

		mockClient := &mockWebClient{}
		mockClient.On("RequestJSON", mock.Anything, &[]apiNodeResp{}).Return(errors.New("request error"))

		c.httpClient = &http.Client{}
		c.webClient = func() *web.Client {
			return &web.Client{
				Do: mockClient.RequestJSON,
			}
		}

		metrics := make(map[string]int64)
		err := c.collectNodes(metrics)

		assert.Error(t, err)
		mockClient.AssertExpectations(t)
	})
}
```