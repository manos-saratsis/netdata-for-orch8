package zfspool_test

import (
	"errors"
	"testing"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/collector/zfspool"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type MockZpoolCli struct {
	mock.Mock
}

func (m *MockZpoolCli) List() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func (m *MockZpoolCli) ListWithVdev(pool string) ([]byte, error) {
	args := m.Called(pool)
	return args.Get(0).([]byte), args.Error(1)
}

func TestCollect(t *testing.T) {
	testCases := []struct {
		name           string
		listOutput     []byte
		vdevListOutput []byte
		expectError    bool
		expectedMetric func(mx map[string]int64) bool
	}{
		{
			name: "Successful Collection",
			listOutput: []byte(`NAME    SIZE   ALLOC   FREE  FRAG  CAP  DEDUP  HEALTH
testpool 1000000 500000  500000   10   50  1.00   ONLINE`),
			vdevListOutput: []byte(`NAME    HEALTH
testpool  ONLINE
vdev1     ONLINE`),
			expectedMetric: func(mx map[string]int64) bool {
				return mx["zpool_testpool_size"] == 1000000 &&
					mx["zpool_testpool_alloc"] == 500000 &&
					mx["zpool_testpool_health_state_online"] == 1
			},
		},
		{
			name:           "Zpool List Failure",
			listOutput:     []byte{},
			expectError:    true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockCli := &MockZpoolCli{}
			mockCli.On("List").Return(tc.listOutput, nil)
			mockCli.On("ListWithVdev", mock.Anything).Return(tc.vdevListOutput, nil)

			collector := &zfspool.Collector{
				Base: module.Base{},
				Exec: mockCli,
			}

			mx, err := collector.Collect()

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, mx)
				if tc.expectedMetric != nil {
					assert.True(t, tc.expectedMetric(mx))
				}
			}
		})
	}
}