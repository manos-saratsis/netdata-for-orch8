```go
package pcf

import (
	"testing"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/ibm-messaging/mq-golang/v5/ibmmq"
)

// Mock Client for testing
type MockClient struct {
	mock.Mock
}

func (m *MockClient) sendPCFCommand(cmd int32, params []pcfParameter) ([]pcfParameter, error) {
	args := m.Called(cmd, params)
	return args.Get(0).([]pcfParameter), args.Error(1)
}

func (m *MockClient) parsePCFResponseFromParams(params []pcfParameter, prefix string) (map[int32]interface{}, error) {
	args := m.Called(params, prefix)
	return args.Get(0).(map[int32]interface{}), args.Error(1)
}

func (m *MockClient) parseChannelListResponseFromParams(params []pcfParameter) *ChannelParseResult {
	args := m.Called(params)
	return args.Get(0).(*ChannelParseResult)
}

func (m *MockClient) parseChannelInfoFromParams(params []pcfParameter) []ChannelInfo {
	args := m.Called(params)
	return args.Get(0).([]ChannelInfo)
}

func TestGetChannelList(t *testing.T) {
	testCases := []struct {
		name           string
		mockResponse   *ChannelParseResult
		expectedError  bool
		expectedResult []string
	}{
		{
			name: "Successful channel list retrieval",
			mockResponse: &ChannelParseResult{
				Channels: []string{"Channel1", "Channel2"},
				InternalErrors: 0,
				ErrorCounts:    map[int32]int{},
			},
			expectedResult: []string{"Channel1", "Channel2"},
		},
		{
			name: "Channel list retrieval with internal errors",
			mockResponse: &ChannelParseResult{
				Channels:       []string{"Channel1"},
				InternalErrors: 2,
				ErrorCounts:    map[int32]int{-1: 2},
			},
			expectedResult: []string{"Channel1"},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockClient := new(MockClient)
			c := &Client{
				protocol: &MockProtocol{},
				config:   &Config{QueueManager: "TestQueueManager"},
			}

			mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_CHANNEL", mock.Anything).
				Return([]pcfParameter{}, nil)
			mockClient.On("parseChannelListResponseFromParams", mock.Anything).
				Return(tc.mockResponse)

			channels, err := c.GetChannelList()

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedResult, channels)
			}

			mockClient.AssertExpectations(t)
		})
	}
}

func TestGetChannelMetrics(t *testing.T) {
	testCases := []struct {
		name           string
		channelName    string
		mockAttrs      map[int32]interface{}
		expectedMetrics *ChannelMetrics
		expectedError   bool
	}{
		{
			name:        "Successful channel metrics retrieval",
			channelName: "TestChannel",
			mockAttrs: map[int32]interface{}{
				ibmmq.MQIACH_CHANNEL_STATUS: int32(1),
				ibmmq.MQIACH_MSGS: int32(100),
				ibmmq.MQIACH_BYTES_SENT: int32(1000),
				ibmmq.MQIACH_BATCHES: int32(10),
				ibmmq.MQIACH_BUFFERS_SENT: int32(50),
				ibmmq.MQIACH_CURRENT_MSGS: int32(5),
			},
			expectedMetrics: &ChannelMetrics{
				Name: "TestChannel",
				Status: 1,
				Messages: ptr(int64(100)),
				Bytes: ptr(int64(1000)),
				Batches: ptr(int64(10)),
				BuffersSent: 50,
				CurrentMessages: 5,
				BuffersReceived: NotCollected,
				XmitQueueTime: NotCollected,
				MCAStatus: NotCollected,
				InDoubtStatus: NotCollected,
				SSLKeyResets: NotCollected,
				NPMSpeed: NotCollected,
				CurrentSharingConvs: NotCollected,
			},
		},
		{
			name:        "Channel metrics with partial data",
			channelName: "PartialChannel",
			mockAttrs: map[int32]interface{}{
				ibmmq.MQIACH_CHANNEL_STATUS: int32(2),
			},
			expectedMetrics: &ChannelMetrics{
				Name: "PartialChannel",
				Status: 2,
				// All other metrics will be NotCollected
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockClient := new(MockClient)
			c := &Client{
				protocol: &MockProtocol{},
				config:   &Config{QueueManager: "TestQueueManager"},
			}

			mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_CHANNEL_STATUS", mock.Anything).
				Return([]pcfParameter{}, nil)
			mockClient.On("parsePCFResponseFromParams", mock.Anything, "").
				Return(tc.mockAttrs, nil)

			metrics, err := c.GetChannelMetrics(tc.channelName)

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedMetrics, metrics)
			}
		})
	}
}

func TestGetChannelConfig(t *testing.T) {
	testCases := []struct {
		name          string
		channelName   string
		mockAttrs     map[int32]interface{}
		expectedConfig *ChannelConfig
		expectedError  bool
	}{
		{
			name:        "Successful channel config retrieval",
			channelName: "TestChannel",
			mockAttrs: map[int32]interface{}{
				ibmmq.MQIACH_CHANNEL_TYPE: int32(1),
				ibmmq.MQIACH_BATCH_SIZE: int32(100),
				ibmmq.MQIACH_BATCH_INTERVAL: int32(500),
			},
			expectedConfig: &ChannelConfig{
				Name: "TestChannel",
				Type: 1,
				BatchSize: 100,
				BatchInterval: 500,
				// Other attributes will be NotCollected
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockClient := new(MockClient)
			c := &Client{
				protocol: &MockProtocol{},
				config:   &Config{QueueManager: "TestQueueManager"},
			}

			mockClient.On("sendPCFCommand", ibmmq.MQCMD_INQUIRE_CHANNEL", mock.Anything).
				Return([]pcfParameter{}, nil)
			mockClient.On("parsePCFResponseFromParams", mock.Anything, "").
				Return(tc.mockAttrs, nil)

			config, err := c.GetChannelConfig(tc.channelName)

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedConfig, config)
			}
		})
	}
}

// Helper function to create pointer from value
func ptr[T any](v T) *T {
	return &v
}

// Mock protocol struct for logging
type MockProtocol struct{}

func (m *MockProtocol) Debugf(format string, args ...interface{}) {}
func (m *MockProtocol) Errorf(format string, args ...interface{}) {}
func (m *MockProtocol) Warningf(format string, args ...interface{}) {}
```