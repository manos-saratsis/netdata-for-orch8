```go
package k8s_state

import (
	"context"
	"testing"
	"time"

	"github.com/netdata/netdata/go/plugins/logger"
	"github.com/stretchr/testify/assert"
	"k8s.io/client-go/tools/cache"
	"k8s.io/client-go/util/workqueue"
)

func TestNewCronJobDiscoverer(t *testing.T) {
	// Test panic on nil shared informer
	assert.Panics(t, func() {
		newCronJobDiscoverer(nil, logger.New())
	}, "Should panic when shared informer is nil")

	// Test successful creation
	mockInformer := cache.NewSharedInformer(nil, nil, time.Minute)
	discoverer := newCronJobDiscoverer(mockInformer, logger.New())

	assert.NotNil(t, discoverer, "Discoverer should be created")
	assert.NotNil(t, discoverer.queue, "Work queue should be initialized")
	assert.NotNil(t, discoverer.readyCh, "Ready channel should be initialized")
	assert.NotNil(t, discoverer.stopCh, "Stop channel should be initialized")
}

func TestCronJobResource(t *testing.T) {
	testCases := []struct {
		name     string
		src      string
		val      interface{}
		expected struct {
			source string
			kind   kubeResourceKind
		}
	}{
		{
			name: "Valid CronJob Resource",
			src:  "test-source",
			val:  struct{}{},
			expected: struct {
				source string
				kind   kubeResourceKind
			}{
				source: "test-source",
				kind:   kubeResourceCronJob,
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			r := cronjobResource{src: tc.src, val: tc.val}

			assert.Equal(t, tc.expected.source, r.source(), "Source should match")
			assert.Equal(t, tc.expected.kind, r.kind(), "Kind should be CronJob")
			assert.Equal(t, tc.val, r.value(), "Value should match")
		})
	}
}

func TestCronJobSource(t *testing.T) {
	testCases := []struct {
		namespace string
		name      string
		expected  string
	}{
		{"default", "job1", "k8s/cj/default/job1"},
		{"kube-system", "job2", "k8s/cj/kube-system/job2"},
		{"", "job3", "k8s/cj//job3"},
	}

	for _, tc := range testCases {
		t.Run(tc.namespace+"/"+tc.name, func(t *testing.T) {
			result := cronjobSource(tc.namespace, tc.name)
			assert.Equal(t, tc.expected, result)
		})
	}
}

func TestCronJobDiscovererReadyStopped(t *testing.T) {
	mockInformer := cache.NewSharedInformer(nil, nil, time.Minute)
	discoverer := newCronJobDiscoverer(mockInformer, logger.New())

	// Test initial state
	assert.False(t, discoverer.ready())
	assert.False(t, discoverer.stopped())

	// Simulate running
	ctx, cancel := context.WithCancel(context.Background())
	resultChan := make(chan resource)
	go discoverer.run(ctx, resultChan)

	// Wait a bit for channels to be closed
	time.Sleep(100 * time.Millisecond)

	// Cancel context
	cancel()

	// Wait for stop
	time.Sleep(100 * time.Millisecond)

	assert.True(t, discoverer.ready())
	assert.True(t, discoverer.stopped())
}
```