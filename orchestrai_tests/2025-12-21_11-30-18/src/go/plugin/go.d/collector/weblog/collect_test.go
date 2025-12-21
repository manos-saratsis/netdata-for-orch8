package weblog

import (
	"testing"
	"strconv"
	"net/http"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type MockCollector struct {
	mock.Mock
}

func (m *MockCollector) logPanicStackIfAny() {
	m.Called()
}

func (m *MockCollector) collect() (map[string]int64, error) {
	args := m.Called()
	return args.Get(0).(map[string]int64), args.Error(1)
}

func TestCollector_collectLogLine(t *testing.T) {
	tests := []struct {
		name         string
		prepareMocks func(*Collector)
		line         *logLine
		wantCalls    []string
	}{
		{
			name: "Full log line",
			prepareMocks: func(c *Collector) {
				// Setup all possible mock expectations
				c.mx.Requests = &stmCounter{}
			},
			line: &logLine{
				vhost:         "example.com",
				port:          "80",
				reqScheme:     "https",
				reqClient:     "192.168.1.1",
				reqMethod:     "GET",
				reqURL:        "/test",
				reqProto:      "HTTP/1.1",
				respCode:      200,
				reqSize:       1024,
				respSize:      2048,
				reqProcTime:   0.5,
				upsRespTime:   0.3,
				sslProto:      "TLS1.2",
				sslCipherSuite: "TLS_RSA_WITH_AES_256_CBC_SHA256",
			},
			wantCalls: []string{
				"collectVhost",
				"collectPort",
				"collectReqScheme",
				"collectReqClient",
				"collectReqMethod",
				"collectReqURL",
				"collectReqProto",
				"collectRespCode",
				"collectReqSize",
				"collectRespSize",
				"collectReqProcTime",
				"collectUpsRespTime",
				"collectSSLProto",
				"collectSSLCipherSuite",
				"collectCustomFields",
			},
		},
		{
			name: "Minimal log line",
			prepareMocks: func(c *Collector) {
				c.mx.Requests = &stmCounter{}
			},
			line: &logLine{}, // Essentially an empty line
			wantCalls: []string{},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := &Collector{
				line: tt.line,
			}
			
			// Set up mocks as needed
			if tt.prepareMocks != nil {
				tt.prepareMocks(c)
			}

			c.collectLogLine()
		})
	}
}

func TestCollector_collectRespCode(t *testing.T) {
	tests := []struct {
		name     string
		code     int
		wantIncs []string
	}{
		{
			name: "Success - 200 OK",
			code: 200,
			wantIncs: []string{
				"ReqSuccess", 
				"Resp2xx",
			},
		},
		{
			name: "Redirect - 301 Moved Permanently",
			code: 301,
			wantIncs: []string{
				"ReqRedirect", 
				"Resp3xx",
			},
		},
		{
			name: "Client Error - 404 Not Found",
			code: 404,
			wantIncs: []string{
				"ReqBad", 
				"Resp4xx",
			},
		},
		{
			name: "Server Error - 500 Internal Server Error",
			code: 500,
			wantIncs: []string{
				"ReqError", 
				"Resp5xx",
			},
		},
		{
			name: "Informational - 100 Continue",
			code: 100,
			wantIncs: []string{
				"Resp1xx",
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := &Collector{
				mx: &metrics{
					Requests:  &stmCounter{},
					ReqSuccess: &stmCounter{},
					ReqBad:    &stmCounter{},
					ReqError:  &stmCounter{},
					ReqRedirect: &stmCounter{},
					Resp1xx:   &stmCounter{},
					Resp2xx:   &stmCounter{},
					Resp3xx:   &stmCounter{},
					Resp4xx:   &stmCounter{},
					Resp5xx:   &stmCounter{},
					RespCode:  &stmPersistentMap{},
				},
				line: &logLine{
					respCode: tt.code,
				},
			}

			c.collectRespCode()

			// Verify counters were incremented correctly
			for _, inc := range tt.wantIncs {
				assert.True(t, c.mx.Requests.called, fmt.Sprintf("%s not incremented", inc))
			}
		})
	}
}

func TestCollector_collectReqClient(t *testing.T) {
	tests := []struct {
		name         string
		reqClient    string
		wantIPv4Call bool
		wantIPv6Call bool
	}{
		{
			name:         "IPv4 Client",
			reqClient:    "192.168.1.1",
			wantIPv4Call: true,
		},
		{
			name:         "IPv6 Client",
			reqClient:    "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
			wantIPv6Call: true,
		},
		{
			name:         "Empty Client",
			reqClient:    "",
			wantIPv4Call: false,
			wantIPv6Call: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := &Collector{
				mx: &metrics{
					ReqIPv4:    &stmCounter{},
					ReqIPv6:    &stmCounter{},
					UniqueIPv4: &stmSet{},
					UniqueIPv6: &stmSet{},
				},
				line: &logLine{
					reqClient: tt.reqClient,
				},
			}

			c.collectReqClient()

			if tt.wantIPv4Call {
				assert.True(t, c.mx.ReqIPv4.called, "IPv4 counter not incremented")
			}
			if tt.wantIPv6Call {
				assert.True(t, c.mx.ReqIPv6.called, "IPv6 counter not incremented")
			}
		})
	}
}

func TestCollector_collectReqScheme(t *testing.T) {
	tests := []struct {
		name         string
		reqScheme    string
		wantHTTPCall bool
		wantHTTPSCall bool
	}{
		{
			name:         "HTTP Scheme",
			reqScheme:    "http",
			wantHTTPCall: true,
		},
		{
			name:          "HTTPS Scheme",
			reqScheme:     "https",
			wantHTTPSCall: true,
		},
		{
			name:         "Empty Scheme",
			reqScheme:    "",
			wantHTTPCall: false,
			wantHTTPSCall: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c := &Collector{
				mx: &metrics{
					ReqHTTPScheme:  &stmCounter{},
					ReqHTTPSScheme: &stmCounter{},
				},
				line: &logLine{
					reqScheme: tt.reqScheme,
				},
			}

			c.collectReqScheme()

			if tt.wantHTTPCall {
				assert.True(t, c.mx.ReqHTTPScheme.called, "HTTP counter not incremented")
			}
			if tt.wantHTTPSCall {
				assert.True(t, c.mx.ReqHTTPSScheme.called, "HTTPS counter not incremented")
			}
		})
	}
}

// Additional mock structs for testing
type stmCounter struct {
	called bool
}

func (c *stmCounter) Inc() {
	c.called = true
}

type stmPersistentMap struct{}

func (m *stmPersistentMap) GetP(key string) (*stmCounter, bool) {
	return &stmCounter{}, true
}

type stmSet struct{}

func (s *stmSet) Insert(string) {}