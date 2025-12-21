```go
package storcli

import (
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

type mockExec struct {
	mock.Mock
}

func (m *mockExec) controllersInfo() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func (m *mockExec) drivesInfo() ([]byte, error) {
	args := m.Called()
	return args.Get(0).([]byte), args.Error(1)
}

func TestCollector_Collect(t *testing.T) {
	testCases := []struct {
		name               string
		mockControllersResponse []byte
		mockControllersError    error
		mockDrivesResponse      []byte
		mockDrivesError         error
		expectedError           bool
	}{
		{
			name:                    "Successful collection with MegaRaid driver",
			mockControllersResponse: []byte(`{"Controllers":[{"Command Status":{"Status":"Success"},"Response Data":{"Version":{"Driver Name":"megaraid_sas"},"PD LIST":[{"EID:Slt":"1"}],"Status":{"Controller Status":"Optimal"}}}]}`),
			mockControllersError:    nil,
			mockDrivesResponse:      []byte(`{"Controllers":[{"Command Status":{"Status":"Success"}, "Response Data":{"Drive /c0/e252/s0 - Detailed Information": {"Drive /c0/e252/s0 State":{"Media Error Count":"10", "Other Error Count":"5"}, "Drive /c0/e252/s0 Device attributes":{"WWN":"123456"}, "Drive /c0/e252/s0":{"EID:Slt":"1"}}}}]}`),
			mockDrivesError:         nil,
			expectedError:           false,
		},
		{
			name:                    "Successful collection with MPT3SAS driver",
			mockControllersResponse: []byte(`{"Controllers":[{"Command Status":{"Status":"Success"},"Response Data":{"Version":{"Driver Name":"mpt3sas"},"Status":{"Controller Status":"OK"},"HwCfg":{"Temperature Sensor for ROC":"present", "ROC temperature(Degree Celsius)":40}}}]}`),
			mockControllersError:    nil,
			mockDrivesResponse:      nil,
			mockDrivesError:         nil,
			expectedError:           false,
		},
		{
			name:                    "Controllers info query error",
			mockControllersResponse: nil,
			mockControllersError:    errors.New("controllers info error"),
			mockDrivesResponse:      nil,
			mockDrivesError:         nil,
			expectedError:           true,
		},
		{
			name:                    "Unknown driver",
			mockControllersResponse: []byte(`{"Controllers":[{"Command Status":{"Status":"Success"},"Response Data":{"Version":{"Driver Name":"unknown"}}}]}`),
			mockControllersError:    nil,
			mockDrivesResponse:      nil,
			mockDrivesError:         nil,
			expectedError:           true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			c := New()
			mockExec := new(mockExec)
			c.exec = mockExec

			mockExec.On("controllersInfo").Return(tc.mockControllersResponse, tc.mockControllersError)
			if tc.mockDrivesResponse != nil {
				mockExec.On("drivesInfo").Return(tc.mockDrivesResponse, tc.mockDrivesError)
			}

			metrics, err := c.collect()
			
			if tc.expectedError {
				assert.Error(t, err)
				assert.Nil(t, metrics)
			} else {
				assert.NoError(t, err)
				assert.NotEmpty(t, metrics)
			}

			mockExec.AssertExpectations(t)
		})
	}
}
```