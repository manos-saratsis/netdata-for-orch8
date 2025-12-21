```go
package rabbitmq

import (
	"errors"
	"net/http"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type mockWebClient struct {
	mock.Mock
}

func (m *mockWebClient) RequestJSON(req *http.Request, v interface{}) error {
	args := m.Called(req, v)
	return args.Error(0)
}

func TestCollector_collect(t *testing.T) {
	testCases := []struct {
		name           string
		queryCluster   bool
		collectQueues  bool
		mockClusterMeta func(*Collector)
		expectError    bool
	}{
		{
			name:           "Successful collection with cluster meta and queues",
			queryCluster:   true,
			collectQueues:  true,
			mockClusterMeta: func(c *Collector) {
				c.clusterId = "test-cluster-id"
				c.clusterName = "test-cluster"
			},
			expectError: false,
		},
		{
			name:           "Failed cluster meta collection",
			queryCluster:   true,
			collectQueues:  false,
			mockClusterMeta: func(c *Collector) {
				c.getClusterMeta = func() (string, string, error) {
					return "", "", errors.New("cluster meta error")
				}
			},
			expectError: true,
		},
		{
			name:           "Partial collection error",
			queryCluster:   false,
			collectQueues:  false,
			mockClusterMeta: func(c *Collector) {},
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				queryClusterMeta: tc.queryCluster,
				CollectQueues:    tc.collectQueues,
				cache:            &collectCache{},
			}

			if tc.mockClusterMeta != nil {
				tc.mockClusterMeta(c)
			}

			mx, err := c.collect()

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, mx)
			}
		})
	}
}

func TestCollector_getClusterMeta(t *testing.T) {
	testCases := []struct {
		name               string
		mockUserResp       apiWhoamiResp
		mockDefinitionsResp apiDefinitionsResp
		expectError        bool
		expectedID         string
		expectedName       string
	}{
		{
			name: "Successful cluster meta retrieval",
			mockUserResp: apiWhoamiResp{
				Name: "admin",
				Tags: []string{"administrator"},
			},
			mockDefinitionsResp: apiDefinitionsResp{
				GlobalParams: []apiGlobalParam{
					{Name: "cluster_name", Value: "test-cluster"},
					{Name: "internal_cluster_id", Value: "rabbitmq-cluster-id-12345"},
				},
			},
			expectError:  false,
			expectedID:   "12345",
			expectedName: "test-cluster",
		},
		{
			name: "Non-admin user",
			mockUserResp: apiWhoamiResp{
				Name: "user",
				Tags: []string{"monitoring"},
			},
			expectError: false,
			expectedID:  "",
			expectedName: "",
		},
		{
			name: "Empty user name",
			mockUserResp: apiWhoamiResp{
				Name: "",
				Tags: []string{},
			},
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				RequestConfig: &web.RequestConfig{},
			}

			mockClient := &mockWebClient{}
			mockClient.On("RequestJSON", mock.Anything, &apiWhoamiResp{}).Return(func(req *http.Request, v *apiWhoamiResp) error {
				*v = tc.mockUserResp
				return nil
			})
			mockClient.On("RequestJSON", mock.Anything, &apiDefinitionsResp{}).Return(func(req *http.Request, v *apiDefinitionsResp) error {
				*v = tc.mockDefinitionsResp
				return nil
			})

			c.httpClient = &http.Client{}
			c.webClient = func() *web.Client {
				return &web.Client{
					Do: mockClient.RequestJSON,
				}
			}

			id, name, err := c.getClusterMeta()

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedID, id)
				assert.Equal(t, tc.expectedName, name)
			}

			mockClient.AssertExpectations(t)
		})
	}
}

func TestCollector_webClient(t *testing.T) {
	c := &Collector{}
	client := c.webClient()

	assert.NotNil(t, client, "webClient should return a non-nil client")
}
```