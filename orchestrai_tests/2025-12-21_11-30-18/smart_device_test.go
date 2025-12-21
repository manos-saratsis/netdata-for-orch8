```go
package smartctl

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/tidwall/gjson"
)

func TestNewSmartDevice(t *testing.T) {
	result := gjson.Parse(`{
		"device": {"name": "/dev/sda", "type": "sat"},
		"serial_number": "abc123",
		"model_name": "Test Model",
		"power_on_time": {"hours": 100, "minutes": 30},
		"temperature": {"current": 45},
		"power_cycle_count": 500,
		"smart_status": {"passed": true},
		"ata_smart_error_log": {"summary": {"count": 2}},
		"ata_smart_attributes": {"table": [
			{"id": "1", "name": "Raw Read Error Rate", "value": "100", "raw": {"value": "50", "string": "100 100 100"}}
		]}
	}`)

	device := newSmartDevice(&result)

	// Test all getter methods
	assert.Equal(t, "sda", device.deviceName())
	assert.Equal(t, "sat", device.deviceType())
	assert.Equal(t, "abc123", device.serialNumber())
	assert.Equal(t, "Test Model", device.modelName())

	// Test power on time
	powerOnTime, ok := device.powerOnTime()
	assert.True(t, ok)
	assert.Equal(t, int64(100*3600+30*60), powerOnTime)

	// Test temperature
	temp, ok := device.temperature()
	assert.True(t, ok)
	assert.Equal(t, int64(45), temp)

	// Test power cycle count
	count, ok := device.powerCycleCount()
	assert.True(t, ok)
	assert.Equal(t, int64(500), count)

	// Test smart status
	status, ok := device.smartStatusPassed()
	assert.True(t, ok)
	assert.True(t, status)

	// Test smart error log count
	errorCount, ok := device.ataSmartErrorLogCount()
	assert.True(t, ok)
	assert.Equal(t, int64(2), errorCount)

	// Test smart attribute table
	attrs, ok := device.ataSmartAttributeTable()
	assert.True(t, ok)
	assert.Len(t, attrs, 1)

	// Test smart attribute details
	attr := attrs[0]
	assert.Equal(t, "1", attr.id())
	assert.Equal(t, "Raw Read Error Rate", attr.name())
	assert.Equal(t, "100", attr.value())
	assert.Equal(t, "50", attr.rawValue())
	assert.Equal(t, "100 100 100", attr.rawString())
}
```