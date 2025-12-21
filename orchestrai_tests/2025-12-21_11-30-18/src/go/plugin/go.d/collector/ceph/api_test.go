package ceph

import (
	"testing"
	"net/url"
)

func TestAPIConstants(t *testing.T) {
	// Test API path constants
	tests := []struct {
		name     string
		path     string
		expected string
	}{
		{"Auth Path", urlPathApiAuth, "/api/auth"},
		{"Auth Check Path", urlPathApiAuthCheck, "/api/auth/check"},
		{"Auth Logout Path", urlPathApiAuthLogout, "/api/auth/logout"},
		{"Health Minimal Path", urlPathApiHealthMinimal, "/api/health/minimal"},
		{"Monitor Path", urlPathApiMonitor, "/api/monitor"},
		{"OSD Path", urlPathApiOsd, "/api/osd"},
		{"Pool Path", urlPathApiPool, "/api/pool"},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if tt.path != tt.expected {
				t.Errorf("expected %s, got %s", tt.expected, tt.path)
			}
		})
	}

	// Test URL Query 
	if urlQueryApiPool != "stats=true" {
		t.Errorf("expected 'stats=true', got %s", urlQueryApiPool)
	}
}

func TestAPIResponseStructs(t *testing.T) {
	// Test Health Minimal Response
	healthResp := apiHealthMinimalResponse{}
	healthResp.Health.Status = "OK"
	if healthResp.Health.Status != "OK" {
		t.Errorf("expected 'OK', got %s", healthResp.Health.Status)
	}

	// Test OSD Response
	osdResp := apiOsdResponse{
		UUID: "test-uuid",
		ID:   1,
		Up:   1,
		In:   1,
	}
	if osdResp.UUID != "test-uuid" {
		t.Errorf("expected 'test-uuid', got %s", osdResp.UUID)
	}

	// Test Pool Response
	poolResp := apiPoolResponse{
		PoolName: "test-pool",
	}
	if poolResp.PoolName != "test-pool" {
		t.Errorf("expected 'test-pool', got %s", poolResp.PoolName)
	}
}