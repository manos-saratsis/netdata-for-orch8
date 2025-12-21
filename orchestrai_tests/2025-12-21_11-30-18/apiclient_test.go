```go
package fluentd

import (
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNewAPIClient(t *testing.T) {
	client := &http.Client{}
	config := web.RequestConfig{URL: "http://example.com"}
	
	apiClient := newAPIClient(client, config)
	
	assert.NotNil(t, apiClient)
	assert.Equal(t, client, apiClient.httpClient)
	assert.Equal(t, config, apiClient.request)
}

func TestAPIClientCreateRequest(t *testing.T) {
	tests := []struct {
		name           string
		baseURL        string
		path           string
		expectedURL    string
		expectedError  bool
	}{
		{
			name:           "Valid URL and Path",
			baseURL:        "http://example.com/base",
			path:           "/api/plugins.json",
			expectedURL:    "http://example.com/base/api/plugins.json",
			expectedError:  false,
		},
		{
			name:           "Invalid Base URL",
			baseURL:        "::invalid::",
			path:           "/api/plugins.json",
			expectedError:  true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			config := web.RequestConfig{URL: tt.baseURL}
			client := newAPIClient(&http.Client{}, config)

			req, err := client.createRequest(tt.path)

			if tt.expectedError {
				assert.Error(t, err)
				return
			}

			require.NoError(t, err)
			assert.Equal(t, tt.expectedURL, req.URL.String())
		})
	}
}

func TestAPIClientGetPluginsInfo(t *testing.T) {
	tests := []struct {
		name               string
		mockServerResponse string
		expectedError      bool
	}{
		{
			name: "Successful Response",
			mockServerResponse: `{
				"plugins": [
					{
						"plugin_id": "test_plugin",
						"type": "output",
						"plugin_category": "forward",
						"retry_count": 5,
						"buffer_total_queued_size": 1024,
						"buffer_queue_length": 10
					}
				]
			}`,
			expectedError: false,
		},
		{
			name:               "Invalid JSON Response",
			mockServerResponse: `{invalid json}`,
			expectedError:      true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
				w.Header().Set("Content-Type", "application/json")
				w.Write([]byte(tt.mockServerResponse))
			}))
			defer server.Close()

			config := web.RequestConfig{URL: server.URL}
			client := newAPIClient(server.Client(), config)

			info, err := client.getPluginsInfo()

			if tt.expectedError {
				assert.Error(t, err)
				return
			}

			require.NoError(t, err)
			assert.NotNil(t, info)
			assert.Len(t, info.Payload, 1)
		})
	}
}

func TestPluginDataHelpers(t *testing.T) {
	retryCount := int64(5)
	bufferQueueLength := int64(10)
	bufferTotalQueuedSize := int64(1024)

	tests := []struct {
		name            string
		plugin          pluginData
		expectCategory  bool
		expectQueueLen  bool
		expectQueueSize bool
	}{
		{
			name: "Plugin with all metrics",
			plugin: pluginData{
				RetryCount:            &retryCount,
				BufferQueueLength:     &bufferQueueLength,
				BufferTotalQueuedSize: &bufferTotalQueuedSize,
			},
			expectCategory:  true,
			expectQueueLen:  true,
			expectQueueSize: true,
		},
		{
			name: "Plugin with no metrics",
			plugin: pluginData{
				RetryCount:            nil,
				BufferQueueLength:     nil,
				BufferTotalQueuedSize: nil,
			},
			expectCategory:  false,
			expectQueueLen:  false,
			expectQueueSize: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			assert.Equal(t, tt.expectCategory, tt.plugin.hasCategory())
			assert.Equal(t, tt.expectQueueLen, tt.plugin.hasBufferQueueLength())
			assert.Equal(t, tt.expectQueueSize, tt.plugin.hasBufferTotalQueuedSize())
		})
	}
}
```