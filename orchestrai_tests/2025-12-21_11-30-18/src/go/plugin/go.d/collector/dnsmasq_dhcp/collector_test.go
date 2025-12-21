```go
package dnsmasq_dhcp

import (
	"context"
	"errors"
	"os"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNew(t *testing.T) {
	c := New()
	
	assert.NotNil(t, c)
	assert.Equal(t, "/var/lib/misc/dnsmasq.leases", c.LeasesPath)
	assert.Equal(t, "/etc/dnsmasq.conf", c.ConfPath)
	assert.Equal(t, "/etc/dnsmasq.d,.dpkg-dist,.dpkg-old,.dpkg-new", c.ConfDir)
	assert.NotNil(t, c.mx)
	assert.NotNil(t, c.cacheDHCPRanges)
	assert.Equal(t, time.Minute, c.parseConfigEvery)
}

func TestCollector_Configuration(t *testing.T) {
	c := New()
	config := c.Configuration()
	assert.Equal(t, c.Config, config)
}

func TestCollector_Init(t *testing.T) {
	testCases := []struct {
		name           string
		setupConfig    func(*Collector)
		expectedError  bool
		errorSubstring string
	}{
		{
			name: "Valid configuration with existing lease file",
			setupConfig: func(c *Collector) {
				// Create a temporary lease file
				tmpfile, err := os.CreateTemp("", "dnsmasq.leases")
				require.NoError(t, err)
				defer os.Remove(tmpfile.Name())
				
				c.LeasesPath = tmpfile.Name()
				c.ConfPath = os.TempDir()
			},
		},
		{
			name: "Invalid lease file path",
			setupConfig: func(c *Collector) {
				c.LeasesPath = "/path/to/nonexistent/file"
			},
			expectedError:  true,
			errorSubstring: "leases path check",
		},
		{
			name: "Empty lease file path",
			setupConfig: func(c *Collector) {
				c.LeasesPath = ""
			},
			expectedError:  true,
			errorSubstring: "leases path check",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			if tc.setupConfig != nil {
				tc.setupConfig(c)
			}

			err := c.Init(context.Background())

			if tc.expectedError {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errorSubstring)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Check(t *testing.T) {
	testCases := []struct {
		name           string
		setupCollector func(*Collector)
		expectedError  bool
	}{
		{
			name: "Successful collection",
			setupCollector: func(c *Collector) {
				// Create a temporary lease file with some content
				tmpfile, err := os.CreateTemp("", "dnsmasq.leases")
				require.NoError(t, err)
				defer os.Remove(tmpfile.Name())
				
				_, err = tmpfile.Write([]byte("1560300536 08:00:27:61:3c:ee 192.168.1.3 debian8 *"))
				require.NoError(t, err)
				tmpfile.Close()

				c.LeasesPath = tmpfile.Name()
			},
			expectedError: false,
		},
		{
			name: "Collection error - no metrics",
			setupCollector: func(c *Collector) {
				// Override collect method to return no metrics
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{}, nil
				}
			},
			expectedError: true,
		},
		{
			name: "Collection error - with error",
			setupCollector: func(c *Collector) {
				// Override collect method to return an error
				c.collect = func() (map[string]int64, error) {
					return nil, errors.New("collection error")
				}
			},
			expectedError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			if tc.setupCollector != nil {
				tc.setupCollector(c)
			}

			err := c.Check(context.Background())

			if tc.expectedError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestCollector_Collect(t *testing.T) {
	testCases := []struct {
		name           string
		setupCollector func(*Collector)
		expectedMetric bool
	}{
		{
			name: "Successful collection with metrics",
			setupCollector: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{"test_metric": 100}, nil
				}
			},
			expectedMetric: true,
		},
		{
			name: "Collection with no metrics",
			setupCollector: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return map[string]int64{}, nil
				}
			},
			expectedMetric: false,
		},
		{
			name: "Collection with error",
			setupCollector: func(c *Collector) {
				c.collect = func() (map[string]int64, error) {
					return nil, errors.New("collection error")
				}
			},
			expectedMetric: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			if tc.setupCollector != nil {
				tc.setupCollector(c)
			}

			metrics := c.Collect(context.Background())

			if tc.expectedMetric {
				assert.NotEmpty(t, metrics)
			} else {
				assert.Empty(t, metrics)
			}
		})
	}
}

func TestCollector_Charts(t *testing.T) {
	c := New()
	charts := c.Charts()

	assert.NotNil(t, charts)
	assert.Equal(t, charts, c.charts)
}

func TestCollector_Cleanup(t *testing.T) {
	c := New()
	// This is essentially a no-op method, but we'll test it for completeness
	assert.NotPanics(t, func() {
		c.Cleanup(context.Background())
	})
}
```