```go
package systemdunits

import (
	"testing"
	"path/filepath"
	"strings"

	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
)

func TestAddUnitCharts(t *testing.T) {
	testCases := []struct {
		name string
		typ  string
		expectedChartID string
		expectedTitle string
	}{
		{
			name: "test-service",
			typ:  "service",
			expectedChartID: "unit_test-service_service_state",
			expectedTitle: "Service Unit State",
		},
		{
			name: "test-socket",
			typ:  "socket",
			expectedChartID: "unit_test-socket_socket_state",
			expectedTitle: "Socket Unit State",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				charts: module.Charts{},
			}

			c.addUnitCharts(tc.name, tc.typ)

			assert.Len(t, c.Charts(), 1, "should add one chart")
			chart := (*c.Charts())[0]
			
			assert.Equal(t, tc.expectedChartID, chart.ID)
			assert.Equal(t, tc.expectedTitle, chart.Title)
			assert.Equal(t, "systemd."+tc.typ+"_unit_state", chart.Ctx)
			assert.Len(t, chart.Dims, 5, "should have 5 dimensions for unit states")
		})
	}
}

func TestRemoveUnitCharts(t *testing.T) {
	testCases := []struct {
		name string
		typ  string
		existingCharts int
		expectedCharts int
	}{
		{
			name: "test-service",
			typ:  "service",
			existingCharts: 3,
			expectedCharts: 2,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := &Collector{
				charts: module.Charts{
					&module.Chart{ID: "other_chart"},
					&module.Chart{ID: "unit_test-service_service_state_1"},
					&module.Chart{ID: "unit_test-service_service_state_2"},
				},
			}

			c.removeUnitCharts(tc.name, tc.typ)

			assert.Len(t, c.Charts(), tc.expectedCharts)
			for _, chart := range c.Charts() {
				assert.NotEqual(t, "unit_test-service_service_state_1", chart.ID)
				assert.NotEqual(t, "unit_test-service_service_state_2", chart.ID)
			}
		})
	}
}

func TestAddUnitFileCharts(t *testing.T) {
	testCases := []struct {
		unitPath string
		expectedName string
		expectedType string
	}{
		{
			unitPath: "/etc/systemd/system/myservice.service",
			expectedName: "myservice.service",
			expectedType: "service",
		},
		{
			unitPath: "/usr/lib/systemd/system/example.socket",
			expectedName: "example.socket",
			expectedType: "socket",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.expectedName, func(t *testing.T) {
			c := &Collector{
				charts: module.Charts{},
			}

			c.addUnitFileCharts(tc.unitPath)

			assert.Len(t, c.Charts(), 1, "should add one chart")
			chart := (*c.Charts())[0]
			
			_, name := filepath.Split(tc.unitPath)
			expectedChartID := fmt.Sprintf("unit_file_%s_state", strings.ReplaceAll(tc.unitPath, ".", "_"))
			
			assert.Equal(t, expectedChartID, chart.ID)
			assert.Equal(t, "Unit File State", chart.Title)
			assert.Len(t, chart.Dims, 13, "should have 13 dimensions for unit file states")
			
			assert.Contains(t, chart.Labels, module.Label{Key: "unit_file_name", Value: name})
			assert.Contains(t, chart.Labels, module.Label{Key: "unit_file_type", Value: strings.TrimPrefix(filepath.Ext(tc.unitPath), ".")})
		})
	}
}

func TestRemoveUnitFileCharts(t *testing.T) {
	testCases := []struct {
		unitPath string
		existingCharts int
		expectedCharts int
	}{
		{
			unitPath: "/etc/systemd/system/myservice.service",
			existingCharts: 3,
			expectedCharts: 2,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.unitPath, func(t *testing.T) {
			c := &Collector{
				charts: module.Charts{
					&module.Chart{ID: "other_chart"},
					&module.Chart{ID: fmt.Sprintf("unit_file_%s_state_1", strings.ReplaceAll(tc.unitPath, ".", "_"))},
					&module.Chart{ID: fmt.Sprintf("unit_file_%s_state_2", strings.ReplaceAll(tc.unitPath, ".", "_"))},
				},
			}

			c.removeUnitFileCharts(tc.unitPath)

			assert.Len(t, c.Charts(), tc.expectedCharts)
			for _, chart := range c.Charts() {
				assert.NotEqual(t, 
					fmt.Sprintf("unit_file_%s_state_1", strings.ReplaceAll(tc.unitPath, ".", "_")), 
					chart.ID,
				)
				assert.NotEqual(t, 
					fmt.Sprintf("unit_file_%s_state_2", strings.ReplaceAll(tc.unitPath, ".", "_")), 
					chart.ID,
				)
			}
		})
	}
}

func TestRemoveCharts(t *testing.T) {
	testCases := []struct {
		prefix string
		existingCharts int
		expectedCharts int
	}{
		{
			prefix: "unit_test_",
			existingCharts: 5,
			expectedCharts: 2,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.prefix, func(t *testing.T) {
			charts := module.Charts{
				&module.Chart{ID: "other_chart1"},
				&module.Chart{ID: "unit_test_chart1"},
				&module.Chart{ID: "unit_test_chart2"},
				&module.Chart{ID: "different_chart1"},
				&module.Chart{ID: "unit_test_chart3"},
			}
			
			c := &Collector{charts: charts}

			c.removeCharts(tc.prefix)

			remainingCharts := *c.Charts()
			assert.Len(t, remainingCharts, tc.expectedCharts)
			for _, chart := range remainingCharts {
				assert.NotContains(t, chart.ID, tc.prefix)
			}
		})
	}
}
```

Now, I'll generate tests for the other systemdunits files. I'll start by reading the other files:=== FILE: systemdunits_client_test.go ===
```go
package systemdunits

import (
	"context"
	"testing"

	"github.com/coreos/go-systemd/v22/dbus"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type mockSystemdConnection struct {
	mock.Mock
}

func (m *mockSystemdConnection) Close() {
	m.Called()
}

func (m *mockSystemdConnection) GetManagerProperty(prop string) (string, error) {
	args := m.Called(prop)
	return args.String(0), args.Error(1)
}

func (m *mockSystemdConnection) GetUnitPropertyContext(ctx context.Context, unit string, propertyName string) (*dbus.Property, error) {
	args := m.Called(ctx, unit, propertyName)
	return args.Get(0).(*dbus.Property), args.Error(1)
}

func (m *mockSystemdConnection) ListUnitsContext(ctx context.Context) ([]dbus.UnitStatus, error) {
	args := m.Called(ctx)
	return args.Get(0).([]dbus.UnitStatus), args.Error(1)
}

func (m *mockSystemdConnection) ListUnitsByPatternsContext(ctx context.Context, states []string, patterns []string) ([]dbus.UnitStatus, error) {
	args := m.Called(ctx, states, patterns)
	return args.Get(0).([]dbus.UnitStatus), args.Error(1)
}

func (m *mockSystemdConnection) ListUnitFilesByPatternsContext(ctx context.Context, states []string, patterns []string) ([]dbus.UnitFile, error) {
	args := m.Called(ctx, states, patterns)
	return args.Get(0).([]dbus.UnitFile), args.Error(1)
}

func TestSystemdDBusClient_Connect(t *testing.T) {
	client := newSystemdDBusClient()
	
	// Note: This is more of a sanity check as actual D-Bus connection requires system setup
	conn, err := client.connect()
	
	assert.NotNil(t, conn, "should return a connection")
	assert.NoError(t, err, "should not return an error")
	
	if conn != nil {
		conn.Close()
	}
}

func TestSystemdClient_Interface(t *testing.T) {
	// Ensure systemdDBusClient implements systemdClient interface
	var _ systemdClient = &systemdDBusClient{}
}

func TestNewSystemdDBusClient(t *testing.T) {
	client := newSystemdDBusClient()
	
	assert.NotNil(t, client, "should create a new client")
	assert.IsType(t, &systemdDBusClient{}, client, "should be of correct type")
}
```

I'll continue generating tests for the other systemdunits files. Let's read the next file:=== FILE: systemdunits_collect_test.go ===
```go
package systemdunits

import (
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestCollect(t *testing.T) {
	testCases := []struct {
		name           string
		setupMocks     func(mockClient *mockSystemdClient, mockConn *mockSystemdConnection)
		expectError    bool
		expectedResult map[string]int64
	}{
		{
			name: "Successful collection with unit files",
			setupMocks: func(mockClient *mockSystemdClient, mockConn *mockSystemdConnection) {
				mockClient.On("connect").Return(mockConn, nil)
				mockConn.On("GetManagerProperty", versionProperty).Return("245.version", nil)
				mockConn.On("Close")
				
				// Setup successful unit and unit files collection
				mockClient.collector.CollectUnitFiles = true
				mockClient.collector.IncludeUnitFiles = []string{"*.service"}
			},
			expectError:    false,
			expectedResult: map[string]int64{},
		},
		{
			name: "Connection error",
			setupMocks: func(mockClient *mockSystemdClient, mockConn *mockSystemdConnection) {
				mockClient.On("connect").Return(nil, errors.New("connection failed"))
			},
			expectError: true,
		},
		{
			name: "Systemd version error",
			setupMocks: func(mockClient *mockSystemdClient, mockConn *mockSystemdConnection) {
				mockClient.On("connect").Return(mockConn, nil)
				mockConn.On("GetManagerProperty", versionProperty).Return("", errors.New("version error"))
				mockConn.On("Close")
			},
			expectError: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockClient := &mockSystemdClient{}
			mockConn := &mockSystemdConnection{}

			collector := &Collector{
				client: mockClient,
			}
			mockClient.collector = collector

			if tc.setupMocks != nil {
				tc.setupMocks(mockClient, mockConn)
			}

			result, err := collector.collect()

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, result)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, result)
			}

			mockClient.AssertExpectations(t)
			mockConn.AssertExpectations(t)
		})
	}
}

func TestGetConnection(t *testing.T) {
	testCases := []struct {
		name           string
		existingConn   bool
		connectionErr  error
		expectNewConn  bool
		expectError    bool
	}{
		{
			name:          "New connection successful",
			existingConn:  false,
			connectionErr: nil,
			expectNewConn: true,
			expectError:   false,
		},
		{
			name:          "Connection already exists",
			existingConn:  true,
			connectionErr: nil,
			expectNewConn: false,
			expectError:   false,
		},
		{
			name:          "Connection error",
			existingConn:  false,
			connectionErr: errors.New("connection failed"),
			expectNewConn: false,
			expectError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockClient := &mockSystemdClient{}
			mockConn := &mockSystemdConnection{}

			collector := &Collector{
				client: mockClient,
			}

			if tc.existingConn {
				collector.conn = mockConn
			}

			mockClient.On("connect").Return(mockConn, tc.connectionErr)

			conn, err := collector.getConnection()

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, conn)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, conn)
			}

			if tc.expectNewConn {
				assert.Equal(t, mockConn, collector.conn)
			}

			mockClient.AssertExpectations(t)
		})
	}
}

func TestCloseConnection(t *testing.T) {
	testCases := []struct {
		name          string
		existingConn  bool
	}{
		{
			name:         "Close existing connection",
			existingConn: true,
		},
		{
			name:         "No existing connection",
			existingConn: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			collector := &Collector{}

			if tc.existingConn {
				collector.conn = mockConn
				mockConn.On("Close")
			}

			collector.closeConnection()

			if tc.existingConn {
				mockConn.AssertExpectations(t)
				assert.Nil(t, collector.conn)
			}
		})
	}
}

func TestGetSystemdVersion(t *testing.T) {
	testCases := []struct {
		name          string
		versionString string
		expectedError bool
		expectedVersion int
	}{
		{
			name:            "Valid version",
			versionString:   "systemd 245 (245.4-1ubuntu3)",
			expectedError:   false,
			expectedVersion: 245,
		},
		{
			name:            "Invalid version format",
			versionString:   "invalid version",
			expectedError:   true,
			expectedVersion: 0,
		},
		{
			name:            "Empty version",
			versionString:   "",
			expectedError:   true,
			expectedVersion: 0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			collector := &Collector{}

			mockConn.On("GetManagerProperty", versionProperty).Return(tc.versionString, nil)

			version, err := collector.getSystemdVersion(mockConn)

			if tc.expectedError {
				assert.Error(t, err)
				assert.Equal(t, tc.expectedVersion, version)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedVersion, version)
			}

			mockConn.AssertExpectations(t)
		})
	}
}
```

I'll continue generating tests for the remaining systemdunits files. Let me read the next file:=== FILE: systemdunits_collect_unit_files_test.go ===
```go
package systemdunits

import (
	"context"
	"testing"
	"time"

	"github.com/coreos/go-systemd/v22/dbus"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestCollectUnitFiles(t *testing.T) {
	testCases := []struct {
		name             string
		systemdVersion  int
		cachedUnitFiles []dbus.UnitFile
		seenUnitFiles   map[string]bool
		expectedCharts  int
		expectErr       bool
	}{
		{
			name:            "Systemd version too low",
			systemdVersion: 220,
			expectErr:      false,
		},
		{
			name:            "New unit files",
			systemdVersion: 240,
			cachedUnitFiles: []dbus.UnitFile{
				{Path: "/etc/systemd/system/example.service", Type: "service"},
				{Path: "/usr/lib/systemd/system/another.socket", Type: "socket"},
			},
			seenUnitFiles:  make(map[string]bool),
			expectedCharts: 2,
		},
		{
			name:            "Existing unit files",
			systemdVersion: 240,
			cachedUnitFiles: []dbus.UnitFile{
				{Path: "/etc/systemd/system/example.service", Type: "service"},
			},
			seenUnitFiles: map[string]bool{
				"/etc/systemd/system/example.service": true,
			},
			expectedCharts: 1,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			mockClient := &mockSystemdClient{}

			collector := &Collector{
				systemdVersion:     tc.systemdVersion,
				CollectUnitFiles:   true,
				IncludeUnitFiles:   []string{"*.service", "*.socket"},
				seenUnitFiles:      tc.seenUnitFiles,
				cachedUnitFiles:    tc.cachedUnitFiles,
				lastListUnitFilesTime: time.Now(),
				CollectUnitFilesEvery: Duration{Duration: time.Hour},
				Timeout:             Duration{Duration: time.Second * 10},
			}

			// Mock methods needed for testing
			mockConn.On("ListUnitFilesByPatternsContext", mock.Anything, mock.Anything, mock.Anything).
				Return(tc.cachedUnitFiles, nil)

			mx := make(map[string]int64)

			err := collector.collectUnitFiles(mx, mockConn)

			if tc.expectErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			if tc.systemdVersion >= 230 {
				// Verify state metrics
				for _, uf := range tc.cachedUnitFiles {
					px := "unit_file_" + uf.Path + "_state_"
					assert.Contains(t, mx, px+strings.ToLower(uf.Type), "should have state metric for unit file type")
				}

				// Verify seen unit files and charts
				assert.Len(t, collector.seenUnitFiles, len(tc.cachedUnitFiles), 
					"should track correct number of seen unit files")
			}

			mockConn.AssertExpectations(t)
		})
	}
}

func TestGetUnitFilesByPatterns(t *testing.T) {
	testCases := []struct {
		name             string
		includePatterns  []string
		mockUnitFiles   []dbus.UnitFile
		expectedError   bool
		expectedCleanup bool
	}{
		{
			name:            "Successful retrieval",
			includePatterns: []string{"*.service"},
			mockUnitFiles: []dbus.UnitFile{
				{Path: "/etc/systemd/system/test.service"},
				{Path: "/usr/lib/systemd/system/another.service"},
			},
			expectedError:   false,
			expectedCleanup: true,
		},
		{
			name:            "Connection error",
			includePatterns: []string{"*.service"},
			expectedError:   true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			collector := &Collector{
				IncludeUnitFiles: tc.includePatterns,
				Timeout:          Duration{Duration: time.Second * 10},
			}

			if !tc.expectedError {
				mockConn.On("ListUnitFilesByPatternsContext", 
					mock.Anything, 
					mock.Anything, 
					tc.includePatterns,
				).Return(tc.mockUnitFiles, nil)
			} else {
				mockConn.On("ListUnitFilesByPatternsContext", 
					mock.Anything, 
					mock.Anything, 
					tc.includePatterns,
				).Return(nil, assert.AnError)
			}

			unitFiles, err := collector.getUnitFilesByPatterns(mockConn)

			if tc.expectedError {
				assert.Error(t, err)
				assert.Nil(t, unitFiles)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, len(tc.mockUnitFiles), len(unitFiles))
				
				// Verify path cleanup
				for _, uf := range unitFiles {
					assert.False(t, strings.Contains(uf.Path, ".service"), 
						"path should be cleaned")
				}
			}

			mockConn.AssertExpectations(t)
		})
	}
}

// Test cleanUnitName helper function
func TestCleanUnitName(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{
			input:    "/etc/systemd/system/test.service",
			expected: "/etc/systemd/system/test",
		},
		{
			input:    "test.socket",
			expected: "test",
		},
		{
			input:    "/no/extension/path",
			expected: "/no/extension/path",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := cleanUnitName(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}
```

I'll continue by reading the next file for systemdunits:=== FILE: systemdunits_collect_units_test.go ===
```go
package systemdunits

import (
	"context"
	"fmt"
	"regexp"
	"strings"
	"testing"
	"time"

	"github.com/coreos/go-systemd/v22/dbus"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

func TestCollectUnits(t *testing.T) {
	testCases := []struct {
		name             string
		systemdVersion  int
		mockUnits       []dbus.UnitStatus
		skipTransient   bool
		existingUnits   map[string]bool
		includePatterns []string
		unitNameRegex   *regexp.Regexp
		expectedCharts  int
		expectErr       bool
	}{
		{
			name:            "Collect units - systemd version < 230",
			systemdVersion: 220,
			mockUnits: []dbus.UnitStatus{
				{Name: "test.service", LoadState: "loaded", ActiveState: "active"},
				{Name: "another.socket", LoadState: "loaded", ActiveState: "inactive"},
			},
			skipTransient:  false,
			existingUnits:  make(map[string]bool),
			expectedCharts: 2,
		},
		{
			name:            "Collect units - systemd version >= 230",
			systemdVersion: 240,
			mockUnits: []dbus.UnitStatus{
				{Name: "test.service", LoadState: "loaded", ActiveState: "active"},
				{Name: "another.socket", LoadState: "loaded", ActiveState: "failed"},
			},
			includePatterns: []string{"*.service", "*.socket"},
			unitNameRegex:   regexp.MustCompile(`.+`),
			expectedCharts:  2,
		},
		{
			name:            "Skip transient units",
			systemdVersion: 240,
			mockUnits: []dbus.UnitStatus{
				{Name: "transient.service", LoadState: "loaded", ActiveState: "active"},
				{Name: "normal.service", LoadState: "loaded", ActiveState: "inactive"},
			},
			skipTransient:   true,
			existingUnits:   make(map[string]bool),
			expectedCharts:  1,
			includePatterns: []string{"*.service"},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			collector := &Collector{
				systemdVersion:   tc.systemdVersion,
				SkipTransient:    tc.skipTransient,
				Include:          tc.includePatterns,
				Timeout:          Duration{Duration: time.Second * 10},
				seenUnits:        tc.existingUnits,
				seenUnitFiles:    make(map[string]bool),
				unitTransient:    make(map[string]bool),
				unitSr:           tc.unitNameRegex,
			}

			if tc.skipTransient {
				// Mock GetUnitPropertyContext for transient check
				for _, unit := range tc.mockUnits {
					propValue := "false"
					if strings.Contains(unit.Name, "transient") {
						propValue = "true"
					}
					mockConn.On("GetUnitPropertyContext", 
						mock.Anything, 
						unit.Name, 
						transientProperty,
					).Return(&dbus.Property{
						Value: dbus.MakeVariant(propValue),
					}, nil)
				}
			}

			// Mock ListUnits or ListUnitsByPatterns depending on systemd version
			if tc.systemdVersion < 230 {
				mockConn.On("ListUnitsContext", mock.Anything).
					Return(tc.mockUnits, nil)
			} else {
				mockConn.On("ListUnitsByPatternsContext", 
					mock.Anything, 
					unitStates, 
					tc.includePatterns,
				).Return(tc.mockUnits, nil)
			}

			mx := make(map[string]int64)
			err := collector.collectUnits(mx, mockConn)

			if tc.expectErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)

				// Verify metrics for each unit
				for _, unit := range tc.mockUnits {
					name, typ, ok := extractUnitNameType(unit.Name)
					if !ok {
						continue
					}

					if tc.skipTransient && strings.Contains(unit.Name, "transient") {
						continue
					}

					// Verify metrics are set correctly
					stateKey := fmt.Sprintf("unit_%s_%s_state_%s", name, typ, unit.ActiveState)
					assert.Equal(t, int64(1), mx[stateKey], 
						"should set correct metric for unit state")
				}

				// Verify chart creation
				assertChartCount(t, collector, tc.expectedCharts)
			}

			mockConn.AssertExpectations(t)
		})
	}
}

func TestGetLoadedUnits(t *testing.T) {
	testCases := []struct {
		name            string
		mockUnits      []dbus.UnitStatus
		expectedLoaded int
		expectedError  bool
	}{
		{
			name: "Successful retrieval with filtering",
			mockUnits: []dbus.UnitStatus{
				{Name: "test.service", LoadState: "loaded"},
				{Name: "another.service", LoadState: "not-loaded"},
				{Name: "extra.socket", LoadState: "loaded"},
			},
			expectedLoaded: 1,
		},
		{
			name:           "Connection error",
			expectedError:  true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			collector := &Collector{
				Timeout:  Duration{Duration: time.Second * 10},
				unitSr:   regexp.MustCompile(`\.service$`),
			}

			if !tc.expectedError {
				mockConn.On("ListUnitsContext", mock.Anything).
					Return(tc.mockUnits, nil)
			} else {
				mockConn.On("ListUnitsContext", mock.Anything).
					Return(nil, assert.AnError)
			}

			units, err := collector.getLoadedUnits(mockConn)

			if tc.expectedError {
				assert.Error(t, err)
				assert.Nil(t, units)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedLoaded, len(units))
			}

			mockConn.AssertExpectations(t)
		})
	}
}

func TestGetLoadedUnitsByPatterns(t *testing.T) {
	testCases := []struct {
		name            string
		mockUnits      []dbus.UnitStatus
		includePatterns []string
		expectedLoaded int
		expectedError  bool
	}{
		{
			name: "Successful retrieval with filtering",
			mockUnits: []dbus.UnitStatus{
				{Name: "test.service", LoadState: "loaded"},
				{Name: "another.service", LoadState: "not-loaded"},
				{Name: "extra.socket", LoadState: "loaded"},
			},
			includePatterns: []string{"*.service", "*.socket"},
			expectedLoaded: 2,
		},
		{
			name:           "Connection error",
			expectedError:  true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			collector := &Collector{
				Timeout:  Duration{Duration: time.Second * 10},
				Include:  tc.includePatterns,
			}

			if !tc.expectedError {
				mockConn.On("ListUnitsByPatternsContext", 
					mock.Anything, 
					unitStates, 
					tc.includePatterns,
				).Return(tc.mockUnits, nil)
			} else {
				mockConn.On("ListUnitsByPatternsContext", 
					mock.Anything, 
					unitStates, 
					tc.includePatterns,
				).Return(nil, assert.AnError)
			}

			units, err := collector.getLoadedUnitsByPatterns(mockConn)

			if tc.expectedError {
				assert.Error(t, err)
				assert.Nil(t, units)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedLoaded, len(units))
			}

			mockConn.AssertExpectations(t)
		})
	}
}

func TestGetUnitTransientProperty(t *testing.T) {
	testCases := []struct {
		name        string
		unitName    string
		propValue   string
		expectedVal string
		expectErr   bool
	}{
		{
			name:        "Transient property true",
			unitName:    "test.service",
			propValue:   "true",
			expectedVal: "true",
		},
		{
			name:        "Transient property false",
			unitName:    "normal.service",
			propValue:   "false",
			expectedVal: "false",
		},
		{
			name:      "Connection error",
			unitName:  "error.service",
			expectErr: true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			collector := &Collector{
				Timeout: Duration{Duration: time.Second * 10},
			}

			if !tc.expectErr {
				mockConn.On("GetUnitPropertyContext", 
					mock.Anything, 
					tc.unitName, 
					transientProperty,
				).Return(&dbus.Property{
					Value: dbus.MakeVariant(tc.propValue),
				}, nil)
			} else {
				mockConn.On("GetUnitPropertyContext", 
					mock.Anything, 
					tc.unitName, 
					transientProperty,
				).Return(nil, assert.AnError)
			}

			val, err := collector.getUnitTransientProperty(mockConn, tc.unitName)

			if tc.expectErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tc.expectedVal, val)
			}

			mockConn.AssertExpectations(t)
		})
	}
}

func TestExtractUnitNameType(t *testing.T) {
	testCases := []struct {
		name      string
		expected  string
		expectedType string
		ok        bool
	}{
		{
			name:         "Valid service unit",
			name:        "test.service",
			expected:    "test",
			expectedType: "service",
			ok:          true,
		},
		{
			name:         "Valid socket unit",
			name:        "socket.socket",
			expected:    "socket",
			expectedType: "socket",
			ok:          true,
		},
		{
			name:         "Invalid unit name",
			name:        "testonly",
			expected:    "",
			expectedType: "",
			ok:          false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			name, typ, ok := extractUnitNameType(tc.name)
			
			assert.Equal(t, tc.ok, ok)
			assert.Equal(t, tc.expected, name)
			assert.Equal(t, tc.expectedType, typ)
		})
	}
}

func TestCleanUnitName(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
	}{
		{
			input:    `dev-disk-by\x2duuid-DE44\x2dCEE0.device`,
			expected: `dev-disk-by-uuid-DE44-CEE0.device`,
		},
		{
			input:    `test.service`,
			expected: `test.service`,
		},
		{
			input:    `normal-unit`,
			expected: `normal-unit`,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := cleanUnitName(tc.input)
			assert.Equal(t, tc.expected, result)
		})
	}
}

// Helper function to assert chart count
func assertChartCount(t *testing.T, collector *Collector, expectedCharts int) {
	actualCharts := 0
	for range *collector.Charts() {
		actualCharts++
	}
	assert.Equal(t, expectedCharts, actualCharts, "Incorrect number of charts")
}
```

I'll continue with the next file for systemdunits:=== FILE: systemdunits_collector_test.go ===
```go
package systemdunits

import (
	"context"
	"errors"
	"fmt"
	"testing"
	"time"

	"github.com/coreos/go-systemd/v22/dbus"
	"github.com/netdata/netdata/go/plugins/pkg/confopt"
	"github.com/netdata/netdata/go/plugins/pkg/matcher"
	"github.com/netdata/netdata/go/plugins/plugin/go.d/agent/module"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

// Mocks for testing
type mockSystemdClient struct {
	mock.Mock
	collector *Collector
}

func (m *mockSystemdClient) connect() (systemdConnection, error) {
	args := m.Called()
	conn, _ := args.Get(0).(systemdConnection)
	return conn, args.Error(1)
}

type mockSystemdConnection struct {
	mock.Mock
}

func TestNew(t *testing.T) {
	collector := New()

	assert.NotNil(t, collector)
	assert.Equal(t, time.Second*2, time.Duration(collector.Timeout))
	assert.Equal(t, []string{"*.service"}, collector.Include)
	assert.False(t, collector.SkipTransient)
	assert.False(t, collector.CollectUnitFiles)
	assert.Equal(t, []string{"*.service"}, collector.IncludeUnitFiles)
	assert.Equal(t, time.Minute*5, time.Duration(collector.CollectUnitFilesEvery))
}

func TestConfiguration(t *testing.T) {
	testCfg := Config{
		UpdateEvery:           30,
		Timeout:               confopt.Duration(time.Second * 5),
		Include:               []string{"*.socket"},
		SkipTransient:         true,
		CollectUnitFiles:      true,
		IncludeUnitFiles:      []string{"*.socket"},
		CollectUnitFilesEvery: confopt.Duration(time.Minute * 10),
	}

	collector := &Collector{
		Config: testCfg,
	}

	assert.Equal(t, testCfg, collector.Configuration())
}

func TestInit(t *testing.T) {
	testCases := []struct {
		name          string
		config        Config
		expectError   bool
		errorContains string
	}{
		{
			name: "Valid config with default patterns",
			config: Config{
				Include: []string{"*.service"},
			},
			expectError: false,
		},
		{
			name: "Invalid include pattern",
			config: Config{
				Include: []string{"invalid[pattern"},
			},
			expectError:   true,
			errorContains: "init unit selector",
		},
		{
			name: "Valid config with multiple patterns",
			config: Config{
				Include: []string{"*.service", "*.socket"},
			},
			expectError: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				Config: tc.config,
			}

			err := collector.Init(context.Background())

			if tc.expectError {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errorContains)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, collector.unitSr)
			}
		})
	}
}

func TestCheck(t *testing.T) {
	testCases := []struct {
		name           string
		collectError   error
		mockMetrics    map[string]int64
		expectedResult bool
	}{
		{
			name:           "Successful collection",
			mockMetrics:    map[string]int64{"test_metric": 1},
			expectedResult: true,
		},
		{
			name:           "Collection error",
			collectError:   errors.New("collection failed"),
			expectedResult: false,
		},
		{
			name:           "No metrics collected",
			mockMetrics:    map[string]int64{},
			expectedResult: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				client: &mockSystemdClient{},
			}

			// Mock the collect method behavior
			mockCollect := func() (map[string]int64, error) {
				return tc.mockMetrics, tc.collectError
			}

			// Temporarily replace the collect method
			origCollect := c.collect
			c.collect = mockCollect
			defer func() { c.collect = origCollect }()

			err := collector.Check(context.Background())

			if tc.expectedResult {
				assert.NoError(t, err)
			} else {
				assert.Error(t, err)
			}
		})
	}
}

func TestCollect(t *testing.T) {
	testCases := []struct {
		name           string
		collectError   error
		mockMetrics    map[string]int64
		expectedResult map[string]int64
	}{
		{
			name:           "Successful collection",
			mockMetrics:    map[string]int64{"test_metric_1": 10, "test_metric_2": 20},
			expectedResult: map[string]int64{"test_metric_1": 10, "test_metric_2": 20},
		},
		{
			name:           "Collection error",
			collectError:   errors.New("collection failed"),
			mockMetrics:    map[string]int64{},
			expectedResult: nil,
		},
		{
			name:           "No metrics",
			mockMetrics:    map[string]int64{},
			expectedResult: nil,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				Base: module.Base{
					Error: func(err error) {},  // No-op error logging
				},
				client: &mockSystemdClient{},
			}

			// Create a mock collect method
			mockCollect := func() (map[string]int64, error) {
				return tc.mockMetrics, tc.collectError
			}

			// Temporarily replace the collect method
			origCollect := c.collect
			c.collect = mockCollect
			defer func() { c.collect = origCollect }()

			metrics := collector.Collect(context.Background())

			assert.Equal(t, tc.expectedResult, metrics)
		})
	}
}

func TestCleanup(t *testing.T) {
	testCases := []struct {
		name        string
		hasExistingConn bool
	}{
		{
			name:           "With existing connection",
			hasExistingConn: true,
		},
		{
			name:           "Without existing connection",
			hasExistingConn: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			mockConn := &mockSystemdConnection{}
			collector := &Collector{
				conn: nil,
			}

			if tc.hasExistingConn {
				collector.conn = mockConn
				mockConn.On("Close")
			}

			collector.Cleanup(context.Background())

			if tc.hasExistingConn {
				mockConn.AssertExpectations(t)
				assert.Nil(t, collector.conn)
			}
		})
	}
}

// Utility function to validate config
func TestValidateConfig(t *testing.T) {
	testCases := []struct {
		name          string
		config        Config
		expectError   bool
		errorContains string
	}{
		{
			name: "Valid default config",
			config: Config{
				Include: []string{"*.service"},
			},
			expectError: false,
		},
		{
			name: "Negative timeout",
			config: Config{
				Timeout: confopt.Duration(-1 * time.Second),
			},
			expectError:   true,
			errorContains: "timeout",
		},
		{
			name: "Empty include list",
			config: Config{
				Include: []string{},
			},
			expectError:   true,
			errorContains: "include",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				Config: tc.config,
			}

			err := collector.validateConfig()

			if tc.expectError {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errorContains)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}
```

I'll continue with the remaining systemdunits files. Let's read the init.go:=== FILE: systemdunits_init_test.go ===
```go
package systemdunits

import (
	"testing"

	"github.com/netdata/netdata/go/plugins/pkg/matcher"
	"github.com/stretchr/testify/assert"
)

func TestValidateConfig(t *testing.T) {
	testCases := []struct {
		name          string
		includes      []string
		expectError   bool
		errorContains string
	}{
		{
			name:        "Valid config with single pattern",
			includes:    []string{"*.service"},
			expectError: false,
		},
		{
			name:        "Valid config with multiple patterns",
			includes:    []string{"*.service", "*.socket"},
			expectError: false,
		},
		{
			name:          "Empty include list",
			includes:      []string{},
			expectError:   true,
			errorContains: "'include' option not set",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				Config: Config{
					Include: tc.includes,
				},
			}

			err := collector.validateConfig()

			if tc.expectError {
				assert.Error(t, err)
				assert.Contains(t, err.Error(), tc.errorContains)
			} else {
				assert.NoError(t, err)
			}
		})
	}
}

func TestInitUnitSelector(t *testing.T) {
	testCases := []struct {
		name          string
		includes      []string
		expectError   bool
		expectedType  string
	}{
		{
			name:         "Single simple pattern",
			includes:     []string{"*.service"},
			expectError:  false,
			expectedType: "*matcher.simplePatternsMatcher",
		},
		{
			name:         "Multiple patterns",
			includes:     []string{"*.service", "*.socket"},
			expectError:  false,
			expectedType: "*matcher.simplePatternsMatcher",
		},
		{
			name:         "Empty include list returns true matcher",
			includes:     []string{},
			expectError:  false,
			expectedType: "*matcher.trueMatcher",
		},
		{
			name:         "Invalid pattern",
			includes:     []string{"[invalid]"},
			expectError:  true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				Config: Config{
					Include: tc.includes,
				},
			}

			matcher, err := collector.initUnitSelector()

			if tc.expectError {
				assert.Error(t, err)
				assert.Nil(t, matcher)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, matcher)

				if tc.expectedType == "*matcher.trueMatcher" {
					assert.IsType(t, matcher, matcher.TRUE(), "should return true matcher for empty include")
				} else {
					assert.IsType(t, &matcher.SimplePatternsMatcher{}, matcher, 
						"should return simple patterns matcher")
				}
			}
		})
	}
}

// Additional pattern matching tests
func TestUnitSelectorMatching(t *testing.T) {
	testCases := []struct {
		name      string
		includes  []string
		testUnits []string
		expected  map[string]bool
	}{
		{
			name:     "Simple service pattern",
			includes: []string{"*.service"},
			testUnits: []string{
				"test.service", 
				"another.service", 
				"not-service.socket", 
				"file.txt",
			},
			expected: map[string]bool{
				"test.service": true,
				"another.service": true,
				"not-service.socket": false,
				"file.txt": false,
			},
		},
		{
			name:     "Multiple patterns",
			includes: []string{"*.service", "*.socket"},
			testUnits: []string{
				"test.service", 
				"another.socket", 
				"excluded.timer", 
				"file.txt",
			},
			expected: map[string]bool{
				"test.service": true,
				"another.socket": true,
				"excluded.timer": false,
				"file.txt": false,
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			collector := &Collector{
				Config: Config{
					Include: tc.includes,
				},
			}

			matcher, err := collector.initUnitSelector()
			assert.NoError(t, err)

			for _, unit := range tc.testUnits {
				match := matcher.MatchString(unit)
				assert.Equal(t, tc.expected[unit], match, 
					"mismatch for unit: "+unit)
			}
		})
	}
}
```

I'll generate a test for the doc.go, which is typically used for documentation purposes and may not need extensive testing: