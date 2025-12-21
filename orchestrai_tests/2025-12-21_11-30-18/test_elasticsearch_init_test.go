```go
package elasticsearch

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
	"github.com/stretchr/testify/assert"
)

func TestCollector_ValidateConfig(t *testing.T) {
	testCases := []struct {
		name    string
		config  Config
		wantErr bool
		errMsg  string
	}{
		{
			name: "Valid Config with Node Stats",
			config: Config{
				HTTPConfig: web.HTTPConfig{
					RequestConfig: web.RequestConfig{
						URL: "http://test.com",
					},
				},
				DoNodeStats: true,
			},
			wantErr: false,
		},
		{
			name: "Valid Config with Cluster Health",
			config: Config{
				HTTPConfig: web.HTTPConfig{
					RequestConfig: web.RequestConfig{
						URL: "http://test.com",
					},
				},
				DoClusterHealth: true,
			},
			wantErr: false,
		},
		{
			name: "Empty URL",
			config: Config{
				HTTPConfig: web.HTTPConfig{
					RequestConfig: web.RequestConfig{
						URL: "",
					},
				},
			},
			wantErr: true,
			errMsg:  "URL not set",
		},
		{
			name: "All API calls disabled",
			config: Config{
				HTTPConfig: web.HTTPConfig{
					RequestConfig: web.RequestConfig{
						URL: "http://test.com",
					},
				},
			},
			wantErr: true,
			errMsg:  "all API calls are disabled",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{Config: tc.config}
			err := c.validateConfig()

			if tc.wantErr {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errMsg)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_InitHTTPClient(t *testing.T) {
	c := New()
	client, err := c.initHTTPClient()

	assert.NoError(t, err)
	assert.NotNil(t, client)
}
```