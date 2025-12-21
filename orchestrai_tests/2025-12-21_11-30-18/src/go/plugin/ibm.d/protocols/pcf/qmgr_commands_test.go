```go
package pcf

import (
	"testing"
	"fmt"
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/ibm-messaging/mq-golang/v5/ibmmq"
)

// Mock client for testing
type MockClient struct {
	mock.Mock
}

func (m *MockClient) sendPCFCommand(cmd ibmmq.MQCommandType, params map[int32]interface{}) ([]ibmmq.PCFParameter, error) {
	args := m.Called(cmd, params)
	return args.Get(0).([]ibmmq.PCFParameter), args.Error(1)
}

func TestGetQueueManagerStatus_Success(t *testing.T) {
	mockClient := &MockClient{}
	mockProtocol := &MockProtocol{}
	
	client := &Client{
		config: &Config{
			QueueManager: "TestQMGR",
		},
		protocol: mockProtocol,
	}

	// Mock successful response with complete data
	mockResponse := []ibmmq.PCFParameter{
		{
			Type:      ibmmq.MQCFT_STRING,
			Parameter: int32(3175),  // Start Date
			String:    []string{"2023-05-01"},
		},
		{
			Type:      ibmmq.MQCFT_STRING,
			Parameter: int32(3176),  // Start Time
			String:    []string{"12:00:00"},
		},
		{
			Type:      ibmmq.MQCFT_INTEGER,
			Parameter: ibmmq.MQIACF_CONNECTION_COUNT,
			Int64Value: []int64{10},
		},
	}

	mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_Q_MGR_STATUS, mock.Anything).Return(mockResponse, nil)
	mockProtocol.On("Debugf", mock.Anything, mock.Anything).Return(nil)

	metrics, err := client.GetQueueManagerStatus()

	assert.NoError(t, err)
	assert.NotNil(t, metrics)
	assert.Equal(t, int32(1), metrics.Status)
	assert.Equal(t, AttributeValue(10), metrics.ConnectionCount)
}

func TestGetQueueManagerStatus_FallbackToBasicInquiry(t *testing.T) {
	mockClient := &MockClient{}
	mockProtocol := &MockProtocol{}
	
	client := &Client{
		config: &Config{
			QueueManager: "TestQMGR",
		},
		protocol: mockProtocol,
	}

	// Mock first inquiry fails, second succeeds
	mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_Q_MGR_STATUS, mock.Anything).Return([]ibmmq.PCFParameter{}, errors.New("first inquiry failed"))
	mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_Q_MGR, mock.Anything).Return([]ibmmq.PCFParameter{}, nil)
	mockProtocol.On("Debugf", mock.Anything, mock.Anything).Return(nil)
	mockProtocol.On("Errorf", mock.Anything, mock.Anything, mock.Anything).Return(nil)

	metrics, err := client.GetQueueManagerStatus()

	assert.NoError(t, err)
	assert.NotNil(t, metrics)
}

func TestGetQueueManagerStatus_BothInquiriesFail(t *testing.T) {
	mockClient := &MockClient{}
	mockProtocol := &MockProtocol{}
	
	client := &Client{
		config: &Config{
			QueueManager: "TestQMGR",
		},
		protocol: mockProtocol,
	}

	// Mock both inquiries fail
	mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_Q_MGR_STATUS, mock.Anything).Return([]ibmmq.PCFParameter{}, errors.New("first inquiry failed"))
	mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_Q_MGR, mock.Anything).Return([]ibmmq.PCFParameter{}, errors.New("second inquiry failed"))
	mockProtocol.On("Debugf", mock.Anything, mock.Anything).Return(nil)
	mockProtocol.On("Errorf", mock.Anything, mock.Anything, mock.Anything).Return(nil)

	metrics, err := client.GetQueueManagerStatus()

	assert.Error(t, err)
	assert.Nil(t, metrics)
}

func TestRefreshStaticDataFromPCF_Success(t *testing.T) {
	mockClient := &MockClient{}
	mockProtocol := &MockProtocol{}
	
	client := &Client{
		config: &Config{
			QueueManager: "TestQMGR",
		},
		protocol: mockProtocol,
	}

	mockResponse := []ibmmq.PCFParameter{
		{
			Type:      ibmmq.MQCFT_INTEGER,
			Parameter: ibmmq.MQIA_COMMAND_LEVEL,
			Int64Value: []int64{930},
		},
		{
			Type:      ibmmq.MQCFT_INTEGER,
			Parameter: ibmmq.MQIA_PLATFORM,
			Int64Value: []int64{1}, // Unix platform
		},
		{
			Type:      ibmmq.MQCFT_INTEGER,
			Parameter: int32(131), // MQIA_STATISTICS_INTERVAL
			Int64Value: []int64{15},
		},
	}

	mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_Q_MGR, mock.Anything).Return(mockResponse, nil)
	mockProtocol.On("Debugf", mock.Anything, mock.Anything).Return(nil)

	err := client.refreshStaticDataFromPCF()

	assert.NoError(t, err)
	assert.Equal(t, "9.3", client.cachedVersion)
	assert.Equal(t, int32(1), client.cachedPlatform)
	assert.Equal(t, int32(15), client.cachedStatisticsInterval)
}

func TestRefreshStaticDataFromPCF_Failure(t *testing.T) {
	mockClient := &MockClient{}
	mockProtocol := &MockProtocol{}
	
	client := &Client{
		config: &Config{
			QueueManager: "TestQMGR",
		},
		protocol: mockProtocol,
	}

	mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_Q_MGR, mock.Anything).Return([]ibmmq.PCFParameter{}, errors.New("PCF command failed"))
	mockProtocol.On("Debugf", mock.Anything, mock.Anything).Return(nil)

	err := client.refreshStaticDataFromPCF()

	assert.Error(t, err)
	assert.Contains(t, err.Error(), "failed to get queue manager info")
}
```