```go
package storcli

import (
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/logger"
	"github.com/stretchr/testify/assert"
)

func TestNewStorCliExec(t *testing.T) {
	timeout := time.Second * 5
	log := logger.New()

	exec := newStorCliExec(timeout, log)

	assert.NotNil(t, exec)
	assert.Equal(t, timeout, exec.timeout)
	assert.Equal(t, log, exec.Logger)
}
```