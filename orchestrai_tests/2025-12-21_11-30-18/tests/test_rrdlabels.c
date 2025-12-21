```c
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../src/database/rrdlabels.h"

// Test create and destroy
void test_rrdlabels_create_destroy() {
    RRDLABELS *labels = rrdlabels_create();
    assert(labels != NULL);
    rrdlabels_destroy(labels);
}

// Test adding labels
void test_rrdlabels_add() {
    RRDLABELS *labels = rrdlabels_create();
    
    // Test basic label addition
    rrdlabels_add(labels, "key1", "value1", RRDLABEL_SRC_CONFIG);
    assert(rrdlabels_exist(labels, "key1"));
    
    // Test label with different source
    rrdlabels_add(labels, "k8s_key", "k8s_value", RRDLABEL_SRC_K8S);
    assert(rrdlabels_exist(labels, "k8s_key"));
    
    rrdlabels_destroy(labels);
}

// Test label operations
void test_rrdlabels_operations() {
    RRDLABELS *labels = rrdlabels_create();
    
    // Add multiple labels
    rrdlabels_add(labels, "key1", "value1", RRDLABEL_SRC_AUTO);
    rrdlabels_add(labels, "key2", "value2", RRDLABEL_SRC_CONFIG);
    
    // Check entries count
    assert(rrdlabels_entries(labels) == 2);
    
    // Test flush
    rrdlabels_flush(labels);
    assert(rrdlabels_entries(labels) == 0);
    
    rrdlabels_destroy(labels);
}

// Test label value retrieval
void test_rrdlabels_get_value() {
    RRDLABELS *labels = rrdlabels_create();
    char *value = NULL;
    
    rrdlabels_add(labels, "test_key", "test_value", RRDLABEL_SRC_AUTO);
    
    // Get value by key
    rrdlabels_get_value_strdup_or_null(labels, &value, "test_key");
    assert(value != NULL);
    assert(strcmp(value, "test_value") == 0);
    free(value);
    
    rrdlabels_destroy(labels);
}

// Test marking and unmarking
void test_rrdlabels_mark_unmark() {
    RRDLABELS *labels = rrdlabels_create();
    
    rrdlabels_add(labels, "key1", "value1", RRDLABEL_SRC_AUTO);
    rrdlabels_add(labels, "key2", "value2", RRDLABEL_SRC_CONFIG);
    
    // Unmark all
    rrdlabels_unmark_all(labels);
    
    // Remove unmarked
    rrdlabels_remove_all_unmarked(labels);
    
    rrdlabels_destroy(labels);
}

int main() {
    test_rrdlabels_create_destroy();
    test_rrdlabels_add();
    test_rrdlabels_operations();
    test_rrdlabels_get_value();
    test_rrdlabels_mark_unmark();
    
    printf("All rrdlabels tests passed!\n");
    return 0;
}
```