```c
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "dictionary.h"

// Test insert callback
void test_insert_callback() {
    DICTIONARY *dict = dictionary_create(DICT_OPTION_NONE);
    
    int callback_called = 0;
    void insert_callback(const DICTIONARY_ITEM *item, void *value, void *data) {
        callback_called = 1;
        assert(value != NULL);
        assert(strcmp(dictionary_acquired_item_name(item), "key1") == 0);
    }
    
    dictionary_register_insert_callback(dict, insert_callback, NULL);
    
    int value = 42;
    dictionary_set(dict, "key1", &value, sizeof(int));
    
    assert(callback_called == 1);
    
    dictionary_destroy(dict);
}

// Test delete callback
void test_delete_callback() {
    DICTIONARY *dict = dictionary_create(DICT_OPTION_NONE);
    
    int callback_called = 0;
    void delete_callback(const DICTIONARY_ITEM *item, void *value, void *data) {
        callback_called = 1;
        assert(value != NULL);
        assert(strcmp(dictionary_acquired_item_name(item), "key1") == 0);
    }
    
    dictionary_register_delete_callback(dict, delete_callback, NULL);
    
    int value = 42;
    dictionary_set(dict, "key1", &value, sizeof(int));
    dictionary_del(dict, "key1");
    
    assert(callback_called == 1);
    
    dictionary_destroy(dict);
}

// Test conflict callback
void test_conflict_callback() {
    DICTIONARY *dict = dictionary_create(DICT_OPTION_DONT_OVERWRITE_VALUE);
    
    int callback_called = 0;
    bool conflict_callback(const DICTIONARY_ITEM *item, void *old_value, void *new_value, void *data) {
        callback_called = 1;
        assert(*(int*)old_value == 42);
        assert(*(int*)new_value == 100);
        return true;
    }
    
    dictionary_register_conflict_callback(dict, conflict_callback, NULL);
    
    int value1 = 42;
    int value2 = 100;
    dictionary_set(dict, "key1", &value1, sizeof(int));
    dictionary_set(dict, "key1", &value2, sizeof(int));
    
    assert(callback_called == 1);
    
    dictionary_destroy(dict);
}

// Test react callback
void test_react_callback() {
    DICTIONARY *dict = dictionary_create(DICT_OPTION_NONE);
    
    int callback_called = 0;
    void react_callback(const DICTIONARY_ITEM *item, void *value, void *data) {
        callback_called = 1;
        assert(value != NULL);
        assert(strcmp(dictionary_acquired_item_name(item), "key1") == 0);
    }
    
    dictionary_register_react_callback(dict, react_callback, NULL);
    
    int value = 42;
    dictionary_set(dict, "key1", &value, sizeof(int));
    
    assert(callback_called == 1);
    
    dictionary_destroy(dict);
}

// Main test runner for callbacks
int main() {
    test_insert_callback();
    test_delete_callback();
    test_conflict_callback();
    test_react_callback();
    
    printf("All dictionary callback tests passed!\n");
    return 0;
}
```