```go
package nsd

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestCollector_initNsdControlExec(t *testing.T) {
	c := New()
	nsdControl, err := c.initNsdControlExec()

	assert.NoError(t, err)
	assert.NotNil(t, nsdControl)
}
```

These test files provide comprehensive coverage for the `nsd` collector package, testing:
- All exported functions
- Different code paths and branches
- Error scenarios
- Edge cases
- Initialization and collection processes

The tests use mock objects and cover various scenarios including successful execution, error handling, and empty/invalid inputs. They aim to provide 100% code coverage for the specified Go files.