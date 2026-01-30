#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

// Mock structures and functions for testing
typedef struct {
    unsigned char data[1024];
    int pos;
} mock_rbuf_t;

typedef void (*free_fnc_t)(void *ptr);

// Define minimal mocks for external dependencies
#define CALLER_RESPONSIBILITY ((void*)-1)

int uint32_to_mqtt_vbi(uint32_t input, unsigned char *output);
int mqtt_vbi_to_uint32(char *input, uint32_t *output);

// Test uint32_to_mqtt_vbi with various input values
int test_uint32_to_mqtt_vbi_zero() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(0, output);
    assert(ret == 1);
    assert(output[0] == 0);
    return 0;
}

int test_uint32_to_mqtt_vbi_single_byte() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(127, output);
    assert(ret == 1);
    assert(output[0] == 127);
    return 0;
}

int test_uint32_to_mqtt_vbi_two_bytes() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(128, output);
    assert(ret == 2);
    assert(output[0] == 0x80);
    assert(output[1] == 0x01);
    return 0;
}

int test_uint32_to_mqtt_vbi_two_bytes_max() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(16383, output);
    assert(ret == 2);
    assert(output[0] == 0xFF);
    assert(output[1] == 0x7F);
    return 0;
}

int test_uint32_to_mqtt_vbi_three_bytes() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(16384, output);
    assert(ret == 3);
    assert(output[0] == 0x80);
    assert(output[1] == 0x80);
    assert(output[2] == 0x01);
    return 0;
}

int test_uint32_to_mqtt_vbi_three_bytes_max() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(2097151, output);
    assert(ret == 3);
    assert(output[0] == 0xFF);
    assert(output[1] == 0xFF);
    assert(output[2] == 0x7F);
    return 0;
}

int test_uint32_to_mqtt_vbi_four_bytes() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(2097152, output);
    assert(ret == 4);
    assert(output[0] == 0x80);
    assert(output[1] == 0x80);
    assert(output[2] == 0x80);
    assert(output[3] == 0x01);
    return 0;
}

int test_uint32_to_mqtt_vbi_four_bytes_max() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(268435455, output);
    assert(ret == 4);
    assert(output[0] == 0xFF);
    assert(output[1] == 0xFF);
    assert(output[2] == 0xFF);
    assert(output[3] == 0x7F);
    return 0;
}

int test_uint32_to_mqtt_vbi_overflow() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(268435456, output);
    assert(ret == 0); // Should return 0 for overflow
    return 0;
}

int test_uint32_to_mqtt_vbi_large_overflow() {
    unsigned char output[4];
    int ret = uint32_to_mqtt_vbi(UINT32_MAX, output);
    assert(ret == 0); // Should return 0 for overflow
    return 0;
}

// Test mqtt_vbi_to_uint32 with various input values
int test_mqtt_vbi_to_uint32_zero() {
    char input[] = { 0x00 };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 0);
    assert(output == 0);
    return 0;
}

int test_mqtt_vbi_to_uint32_single_byte() {
    char input[] = { 0x7F };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 0);
    assert(output == 127);
    return 0;
}

int test_mqtt_vbi_to_uint32_two_bytes() {
    char input[] = { 0x80, 0x01 };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 0);
    assert(output == 128);
    return 0;
}

int test_mqtt_vbi_to_uint32_two_bytes_max() {
    char input[] = { 0xFF, 0x7F };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 0);
    assert(output == 16383);
    return 0;
}

int test_mqtt_vbi_to_uint32_three_bytes() {
    char input[] = { 0x80, 0x80, 0x01 };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 0);
    assert(output == 16384);
    return 0;
}

int test_mqtt_vbi_to_uint32_three_bytes_max() {
    char input[] = { 0xFF, 0xFF, 0x7F };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 0);
    assert(output == 2097151);
    return 0;
}

int test_mqtt_vbi_to_uint32_four_bytes() {
    char input[] = { 0x80, 0x80, 0x80, 0x01 };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 0);
    assert(output == 2097152);
    return 0;
}

int test_mqtt_vbi_to_uint32_four_bytes_max() {
    char input[] = { 0xFF, 0xFF, 0xFF, 0x7F };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 0);
    assert(output == 268435455);
    return 0;
}

int test_mqtt_vbi_to_uint32_overflow() {
    char input[] = { 0x80, 0x80, 0x80, 0x80, 0x01 };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 1); // Should return error for overflow
    return 0;
}

int test_mqtt_vbi_to_uint32_multiplier_overflow() {
    // This should overflow the multiplier check
    char input[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x01 };
    uint32_t output;
    int ret = mqtt_vbi_to_uint32(input, &output);
    assert(ret == 1); // Should return error
    return 0;
}

int test_mqtt_vbi_roundtrip() {
    uint32_t test_values[] = { 0, 1, 127, 128, 16383, 16384, 2097151, 2097152, 268435455 };
    size_t num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        unsigned char vbi_buffer[4];
        uint32_t decoded;
        
        int len = uint32_to_mqtt_vbi(test_values[i], vbi_buffer);
        assert(len > 0);
        
        int ret = mqtt_vbi_to_uint32((char*)vbi_buffer, &decoded);
        assert(ret == 0);
        assert(decoded == test_values[i]);
    }
    return 0;
}

int main() {
    // Test uint32_to_mqtt_vbi
    if (test_uint32_to_mqtt_vbi_zero()) return 1;
    if (test_uint32_to_mqtt_vbi_single_byte()) return 1;
    if (test_uint32_to_mqtt_vbi_two_bytes()) return 1;
    if (test_uint32_to_mqtt_vbi_two_bytes_max()) return 1;
    if (test_uint32_to_mqtt_vbi_three_bytes()) return 1;
    if (test_uint32_to_mqtt_vbi_three_bytes_max()) return 1;
    if (test_uint32_to_mqtt_vbi_four_bytes()) return 1;
    if (test_uint32_to_mqtt_vbi_four_bytes_max()) return 1;
    if (test_uint32_to_mqtt_vbi_overflow()) return 1;
    if (test_uint32_to_mqtt_vbi_large_overflow()) return 1;
    
    // Test mqtt_vbi_to_uint32
    if (test_mqtt_vbi_to_uint32_zero()) return 1;
    if (test_mqtt_vbi_to_uint32_single_byte()) return 1;
    if (test_mqtt_vbi_to_uint32_two_bytes()) return 1;
    if (test_mqtt_vbi_to_uint32_two_bytes_max()) return 1;
    if (test_mqtt_vbi_to_uint32_three_bytes()) return 1;
    if (test_mqtt_vbi_to_uint32_three_bytes_max()) return 1;
    if (test_mqtt_vbi_to_uint32_four_bytes()) return 1;
    if (test_mqtt_vbi_to_mqtt_vbi_to_uint32_four_bytes_max()) return 1;
    if (test_mqtt_vbi_to_uint32_overflow()) return 1;
    if (test_mqtt_vbi_to_uint32_multiplier_overflow()) return 1;
    if (test_mqtt_vbi_roundtrip()) return 1;
    
    printf("All mqtt_ng VBI tests passed!\n");
    return 0;
}