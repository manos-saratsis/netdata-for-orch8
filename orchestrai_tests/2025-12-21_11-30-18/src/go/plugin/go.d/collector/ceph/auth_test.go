package ceph

import (
	"bytes"
	"encoding/json"
	"errors"
	"io"
	"net/http"
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/web"
)

// Mock web client to simulate HTTP requests
type mockWebClient struct {
	requestJSONFunc func(req *http.Request, v interface{}) error
	requestFunc     func(req *http.Request, v interface{}) error
}

func (m *mockWebClient) RequestJSON(req *http.Request, v interface{}) error {
	if m.requestJSONFunc != nil {
		return m.requestJSONFunc(req, v)
	}
	return nil
}

func (m *mockWebClient) Request(req *http.Request, v interface{}) error {
	if m.requestFunc != nil {
		return m.requestFunc(req, v)
	}
	return nil
}

func TestAuthLogin(t *testing.T) {
	// Test successful login
	{
		c := &Collector{
			Username: "testuser",
			Password: "testpass",
			webClient: func(codes ...int) web.HTTPClient {
				return &mockWebClient{
					requestJSONFunc: func(req *http.Request, v interface{}) error {
						tok := v.(*authLoginResp)
						tok.Token = "test-token"
						return nil
					},
				}
			},
		}

		token, err := c.authLogin()
		if err != nil {
			t.Errorf("unexpected error: %v", err)
		}
		if token != "test-token" {
			t.Errorf("expected 'test-token', got %s", token)
		}
	}

	// Test login with empty token
	{
		c := &Collector{
			Username: "testuser",
			Password: "testpass",
			webClient: func(codes ...int) web.HTTPClient {
				return &mockWebClient{
					requestJSONFunc: func(req *http.Request, v interface{}) error {
						tok := v.(*authLoginResp)
						tok.Token = ""
						return nil
					},
				}
			},
		}

		_, err := c.authLogin()
		if err == nil || err.Error() != "empty token" {
			t.Errorf("expected 'empty token' error, got %v", err)
		}
	}

	// Test login with JSON marshaling error
	{
		c := &Collector{
			Username: struct{}{}, // Cause JSON marshaling error
			Password: "testpass",
		}

		_, err := c.authLogin()
		if err == nil {
			t.Errorf("expected JSON marshaling error")
		}
	}
}

func TestAuthCheck(t *testing.T) {
	// Test successful auth check
	{
		c := &Collector{
			token: "test-token",
			webClient: func(codes ...int) web.HTTPClient {
				return &mockWebClient{
					requestJSONFunc: func(req *http.Request, v interface{}) error {
						resp := v.(*authCheckResp)
						resp.Username = "testuser"
						return nil
					},
				}
			},
		}

		valid, err := c.authCheck()
		if err != nil {
			t.Errorf("unexpected error: %v", err)
		}
		if !valid {
			t.Errorf("expected auth check to be valid")
		}
	}

	// Test auth check with empty token
	{
		c := &Collector{
			token: "",
		}

		valid, err := c.authCheck()
		if err != nil {
			t.Errorf("unexpected error: %v", err)
		}
		if valid {
			t.Errorf("expected auth check to be invalid")
		}
	}

	// Test auth check with request error
	{
		c := &Collector{
			token: "test-token",
			webClient: func(codes ...int) web.HTTPClient {
				return &mockWebClient{
					requestJSONFunc: func(req *http.Request, v interface{}) error {
						return errors.New("request error")
					},
				}
			},
		}

		_, err := c.authCheck()
		if err == nil {
			t.Errorf("expected request error")
		}
	}
}

func TestAuthLogout(t *testing.T) {
	// Test successful logout
	{
		c := &Collector{
			token: "test-token",
			webClient: func(codes ...int) web.HTTPClient {
				return &mockWebClient{
					requestFunc: func(req *http.Request, v interface{}) error {
						return nil
					},
				}
			},
		}

		err := c.authLogout()
		if err != nil {
			t.Errorf("unexpected error: %v", err)
		}
		if c.token != "" {
			t.Errorf("token should be cleared after logout")
		}
	}

	// Test logout with empty token
	{
		c := &Collector{
			token: "",
		}

		err := c.authLogout()
		if err != nil {
			t.Errorf("unexpected error: %v", err)
		}
	}

	// Test logout with request error
	{
		c := &Collector{
			token: "test-token",
			webClient: func(codes ...int) web.HTTPClient {
				return &mockWebClient{
					requestFunc: func(req *http.Request, v interface{}) error {
						return errors.New("request error")
					},
				}
			},
		}

		err := c.authLogout()
		if err == nil {
			t.Errorf("expected request error")
		}
	}
}