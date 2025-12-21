```go
package postgres

import (
	"testing"
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

// Mock Collector for testing
type MockCollector struct {
	mock.Mock
}

func (m *MockCollector) doQueryDatabaseStats() error {
	args := m.Called()
	return args.Error(0)
}

func (m *MockCollector) doQueryDatabaseSize() error {
	args := m.Called()
	return args.Error(0)
}

func (m *MockCollector) doQueryDatabaseConflicts() error {
	args := m.Called()
	return args.Error(0)
}

func (m *MockCollector) isPGInRecovery() bool {
	args := m.Called()
	return args.Bool(0)
}

func (m *MockCollector) doQueryDatabaseLocks() error {
	args := m.Called()
	return args.Error(0)
}

func TestDoQueryDatabasesMetrics_AllSuccessful(t *testing.T) {
	mockCollector := new(MockCollector)
	
	mockCollector.On("doQueryDatabaseStats").Return(nil)
	mockCollector.On("doQueryDatabaseSize").Return(nil)
	mockCollector.On("isPGInRecovery").Return(false)
	mockCollector.On("doQueryDatabaseLocks").Return(nil)

	err := mockCollector.doQueryDatabasesMetrics()
	assert.NoError(t, err)
	
	mockCollector.AssertExpectations(t)
}

func TestDoQueryDatabasesMetrics_DatabaseStatsError(t *testing.T) {
	mockCollector := new(MockCollector)
	
	mockCollector.On("doQueryDatabaseStats").Return(errors.New("database stats error"))
	
	err := mockCollector.doQueryDatabasesMetrics()
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "querying database stats error")
	
	mockCollector.AssertExpectations(t)
}

func TestDoQueryDatabasesMetrics_DatabaseSizeError(t *testing.T) {
	mockCollector := new(MockCollector)
	
	mockCollector.On("doQueryDatabaseStats").Return(nil)
	mockCollector.On("doQueryDatabaseSize").Return(errors.New("database size error"))
	
	err := mockCollector.doQueryDatabasesMetrics()
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "querying database size error")
	
	mockCollector.AssertExpectations(t)
}

func TestDoQueryDatabasesMetrics_DatabaseConflictsError(t *testing.T) {
	mockCollector := new(MockCollector)
	
	mockCollector.On("doQueryDatabaseStats").Return(nil)
	mockCollector.On("doQueryDatabaseSize").Return(nil)
	mockCollector.On("isPGInRecovery").Return(true)
	mockCollector.On("doQueryDatabaseConflicts").Return(errors.New("database conflicts error"))
	
	err := mockCollector.doQueryDatabasesMetrics()
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "querying database conflicts error")
	
	mockCollector.AssertExpectations(t)
}

func TestDoQueryDatabasesMetrics_DatabaseLocksError(t *testing.T) {
	mockCollector := new(MockCollector)
	
	mockCollector.On("doQueryDatabaseStats").Return(nil)
	mockCollector.On("doQueryDatabaseSize").Return(nil)
	mockCollector.On("isPGInRecovery").Return(false)
	mockCollector.On("doQueryDatabaseLocks").Return(errors.New("database locks error"))
	
	err := mockCollector.doQueryDatabasesMetrics()
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "querying database locks error")
	
	mockCollector.AssertExpectations(t)
}
```