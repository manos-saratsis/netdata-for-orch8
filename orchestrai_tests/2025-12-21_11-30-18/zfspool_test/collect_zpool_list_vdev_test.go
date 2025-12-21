package zfspool_test

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestParseZpoolListVdevOutput(t *testing.T) {
	testCases := []struct {
		name          string
		input         []byte
		expectError   bool
		expectedVdevs int
	}{
		{
			name: "Valid Single Vdev",
			input: []byte(`NAME    SIZE    HEALTH
rpool          ONLINE
 vdev1         ONLINE`),
			expectedVdevs: 1,
		},
		{
			name: "Multiple Vdevs",
			input: []byte(`NAME    SIZE    HEALTH
rpool          ONLINE
 mirror       ONLINE
  vdev1       ONLINE
  vdev2       ONLINE`),
			expectedVdevs: 3,
		},
		{
			name:          "Empty Input",
			input:         []byte{},
			expectError:   true,
			expectedVdevs: 0,
		},
		{
			name: "Invalid Header",
			input: []byte(`INVALID HEADER
rpool          ONLINE`),
			expectError:   true,
			expectedVdevs: 0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			vdevs, err := zfspool.ParseZpoolListVdevOutput(tc.input)

			if tc.expectError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Len(t, vdevs, tc.expectedVdevs)
			}
		})
	}
}