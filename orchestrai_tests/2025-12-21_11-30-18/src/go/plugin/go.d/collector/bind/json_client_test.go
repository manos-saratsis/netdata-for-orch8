```go
package bind

import (
	"net/http"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type MockHTTPDoer struct {
	mock.Mock
}

func (m *MockHTTPDoer) RequestJSON(req *http.Request, v interface{}) error {
	args := m.Called(req, v)
	return args.Error(0)
}

func TestNewJSONClient(t *testing.T) {
	client := &http.Client{}
	requestConfig := web.RequestConfig{
		URL: "http://localhost:8653",
	}

	jsonClient := newJSONClient(client, requestConfig)

	assert.NotNil(t, jsonClient)
	assert.Equal(t, client, jsonClient.httpClient)
	assert.Equal(t, requestConfig, jsonClient.request)
}

func TestJSONClient_ServerStats(t *testing.T) {
	tests := []struct {
		name           string
		mockHTTPDoer  func(*MockHTTPDoer)
		expectError   bool
	}{
		{
			name: "Successful server stats retrieval",
			mockHTTPDoer: func(m *MockHTTPDoer) {
				m.On("RequestJSON", mock.Anything, mock.Anything).Run(func(args mock.Arguments) {
					stats := args.Get(1).(*jsonServerStats)
					stats.NSStats = map[string]int64{"test": 100}
				}).Return(nil)
			},
			expectError: false,
		},
		{
			name: "HTTP request creation failure",
			mockHTTPDoer: func(m *MockHTTPDoer) {
				m.On("RequestJSON", mock.Anything, mock.Anything).Return(assert.AnError)
			},
			expectError: true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			client := &http.Client{}
			requestConfig := web.RequestConfig{
				URL: "http://localhost:8653",
			}
			jsonClient := newJSONClient(client, requestConfig)

			mockHTTPDoer := &MockHTTPDoer{}
			tt.mockHTTPDoer(mockHTTPDoer)

			stats, err := jsonClient.serverStats()

			if tt.expectError {
				assert.Error(t, err)
				assert.Nil(t, stats)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, stats)
			}
		})
	}
}
```