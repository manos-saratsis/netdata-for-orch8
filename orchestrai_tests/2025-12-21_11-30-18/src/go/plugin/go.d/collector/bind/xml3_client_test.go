```go
package bind

import (
	"net/http"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type MockXMLHTTPDoer struct {
	mock.Mock
}

func (m *MockXMLHTTPDoer) RequestXML(req *http.Request, v interface{}) error {
	args := m.Called(req, v)
	return args.Error(0)
}

func TestNewXML3Client(t *testing.T) {
	client := &http.Client{}
	requestConfig := web.RequestConfig{
		URL: "http://localhost:8653",
	}

	xml3Client := newXML3Client(client, requestConfig)

	assert.NotNil(t, xml3Client)
	assert.Equal(t, client, xml3Client.httpClient)
	assert.Equal(t, requestConfig, xml3Client.request)
}

func TestXML3Client_ServerStats(t *testing.T) {
	tests := []struct {
		name           string
		mockHTTPDoer  func(*MockXMLHTTPDoer)
		expectError   bool
	}{
		{
			name: "Successful server stats retrieval",
			mockHTTPDoer: func(m *MockXMLHTTPDoer) {
				m.On("RequestXML", mock.Anything, mock.Anything).Run(func(args mock.Arguments) {
					stats := args.Get(1).(*xml3Stats)
					stats.Server.CounterGroups = []xml3CounterGroup{
						{
							Type: "nsstat",
							Counters: []struct {
								Name  string
								Value int64
							}{
								{Name: "test", Value: 100},
							},
						},
					}
				}).Return(nil)
			},
			expectError: false,
		},
		{
			name: "HTTP request creation failure",
			mockHTTPDoer: func(m *MockXMLHTTPDoer) {
				m.On("RequestXML", mock.Anything, mock.Anything).Return(assert.AnError)
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
			xml3Client := newXML3Client(client, requestConfig)

			mockHTTPDoer := &MockXMLHTTPDoer{}
			tt.mockHTTPDoer(mockHTTPDoer)

			stats, err := xml3Client.serverStats()

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

func TestConvertXML(t *testing.T) {
	tests := []struct {
		name     string
		input    xml3Stats
		expected func(*serverStats)
	}{
		{
			name: "Convert server counter groups",
			input: xml3Stats{
				Server: xml3Server{
					CounterGroups: []xml3CounterGroup{
						{
							Type: "nsstat",
							Counters: []struct {
								Name  string
								Value int64
							}{
								{Name: "test_counter", Value: 100},
							},
						},
					},
				},
			},
			expected: func(stats *serverStats) {
				assert.Contains(t, stats.NSStats, "test_counter")
				assert.Equal(t, int64(100), stats.NSStats["test_counter"])
			},
		},
		{
			name: "Convert views with counter groups",
			input: xml3Stats{
				Views: []xml3View{
					{
						Name: "default_view",
						CounterGroups: []xml3CounterGroup{
							{
								Type: "resstats",
								Counters: []struct {
									Name  string
									Value int64
								}{
									{Name: "test_resolver_stat", Value: 200},
								},
							},
						},
					},
				},
			},
			expected: func(stats *serverStats) {
				assert.Contains(t, stats.Views, "default_view")
				resolverStats := stats.Views["default_view"].Resolver.Stats
				assert.Contains(t, resolverStats, "test_resolver_stat")
				assert.Equal(t, int64(200), resolverStats["test_resolver_stat"])
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := convertXML(tt.input)
			tt.expected(result)
		})
	}
}
```