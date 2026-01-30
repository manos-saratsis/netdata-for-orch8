// MQTT Constants Test File
// This file contains tests for mqtt_constants.h
// 
// mqtt_constants.h is a header file containing only #define constants
// that are used throughout the MQTT implementation.
// 
// All defined constants can be statically verified:

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

// Include the constants header
#include "src/aclk/mqtt_websockets/mqtt_constants.h"

void test_mqtt_max_qos(void) {
    // Verify MQTT_MAX_QOS is correctly defined
    assert(MQTT_MAX_QOS == 0x02);
    printf("✓ MQTT_MAX_QOS = 0x02\n");
}

void test_mqtt_version(void) {
    // Verify MQTT version constant
    assert(MQTT_VERSION_5_0 == 0x5);
    printf("✓ MQTT_VERSION_5_0 = 0x5\n");
}

void test_mqtt_vbi_constants(void) {
    // Test Variable Byte Integer constants
    assert(MQTT_VBI_CONTINUATION_FLAG == 0x80);
    assert(MQTT_VBI_DATA_MASK == 0x7F);
    assert(MQTT_VBI_MAXBYTES == 4);
    printf("✓ VBI constants correct\n");
}

void test_mqtt_control_packet_types(void) {
    // Test all MQTT control packet type definitions
    assert(MQTT_CPT_CONNECT == 0x1);
    assert(MQTT_CPT_CONNACK == 0x2);
    assert(MQTT_CPT_PUBLISH == 0x3);
    assert(MQTT_CPT_PUBACK == 0x4);
    assert(MQTT_CPT_PUBREC == 0x5);
    assert(MQTT_CPT_PUBREL == 0x6);
    assert(MQTT_CPT_PUBCOMP == 0x7);
    assert(MQTT_CPT_SUBSCRIBE == 0x8);
    assert(MQTT_CPT_SUBACK == 0x9);
    assert(MQTT_CPT_UNSUBSCRIBE == 0xA);
    assert(MQTT_CPT_UNSUBACK == 0xB);
    assert(MQTT_CPT_PINGREQ == 0xC);
    assert(MQTT_CPT_PINGRESP == 0xD);
    assert(MQTT_CPT_DISCONNECT == 0xE);
    assert(MQTT_CPT_AUTH == 0xF);
    printf("✓ All MQTT control packet types correct\n");
}

void test_mqtt_connect_flags(void) {
    // Test MQTT CONNECT flag definitions
    assert(MQTT_CONNECT_FLAG_USERNAME == 0x80);
    assert(MQTT_CONNECT_FLAG_PASSWORD == 0x40);
    assert(MQTT_CONNECT_FLAG_LWT_RETAIN == 0x20);
    assert(MQTT_CONNECT_FLAG_LWT == 0x04);
    assert(MQTT_CONNECT_FLAG_CLEAN_START == 0x02);
    assert(MQTT_CONNECT_FLAG_QOS_MASK == 0x18);
    assert(MQTT_CONNECT_FLAG_QOS_BITSHIFT == 3);
    printf("✓ All MQTT CONNECT flags correct\n");
}

void test_mqtt_max_client_id(void) {
    // Test max client ID length
    assert(MQTT_MAX_CLIENT_ID == 23);
    printf("✓ MQTT_MAX_CLIENT_ID = 23\n");
}

void test_mqtt_property_identifiers(void) {
    // Test MQTT property identifier values
    assert(MQTT_PROP_PAYLOAD_FMT_INDICATOR == 0x01);
    assert(MQTT_PROP_MSG_EXPIRY_INTERVAL == 0x02);
    assert(MQTT_PROP_CONTENT_TYPE == 0x03);
    assert(MQTT_PROP_RESPONSE_TOPIC == 0x08);
    assert(MQTT_PROP_CORRELATION_DATA == 0x09);
    assert(MQTT_PROP_SUB_IDENTIFIER == 0x0B);
    assert(MQTT_PROP_SESSION_EXPIRY_INTERVAL == 0x11);
    assert(MQTT_PROP_ASSIGNED_CLIENT_ID == 0x12);
    assert(MQTT_PROP_SERVER_KEEP_ALIVE == 0x13);
    assert(MQTT_PROP_AUTH_METHOD == 0x15);
    assert(MQTT_PROP_AUTH_DATA == 0x16);
    assert(MQTT_PROP_REQ_PROBLEM_INFO == 0x17);
    assert(MQTT_PROP_WILL_DELAY_INTERVAL == 0x18);
    assert(MQTT_PROP_REQ_RESP_INFORMATION == 0x19);
    assert(MQTT_PROP_RESP_INFORMATION == 0x1A);
    assert(MQTT_PROP_SERVER_REF == 0x1C);
    assert(MQTT_PROP_REASON_STR == 0x1F);
    assert(MQTT_PROP_RECEIVE_MAX == 0x21);
    assert(MQTT_PROP_TOPIC_ALIAS_MAX == 0x22);
    assert(MQTT_PROP_TOPIC_ALIAS == 0x23);
    assert(MQTT_PROP_MAX_QOS == 0x24);
    assert(MQTT_PROP_RETAIN_AVAIL == 0x25);
    assert(MQTT_PROP_USR == 0x26);
    assert(MQTT_PROP_MAX_PKT_SIZE == 0x27);
    assert(MQTT_PROP_WILDCARD_SUB_AVAIL == 0x28);
    assert(MQTT_PROP_SUB_ID_AVAIL == 0x29);
    assert(MQTT_PROP_SHARED_SUB_AVAIL == 0x2A);
    printf("✓ All MQTT property identifiers correct\n");
}

void test_property_name_strings(void) {
    // Verify property name strings are defined
    assert(MQTT_PROP_PAYLOAD_FMT_INDICATOR_NAME != NULL);
    assert(MQTT_PROP_TOPIC_ALIAS_NAME != NULL);
    assert(MQTT_PROP_MAX_PKT_SIZE_NAME != NULL);
    printf("✓ All property name strings defined\n");
}

int main(void) {
    printf("Running MQTT Constants Verification Tests...\n\n");
    
    test_mqtt_max_qos();
    test_mqtt_version();
    test_mqtt_vbi_constants();
    test_mqtt_control_packet_types();
    test_mqtt_connect_flags();
    test_mqtt_max_client_id();
    test_mqtt_property_identifiers();
    test_property_name_strings();
    
    printf("\n✅ All constant tests passed!\n");
    return 0;
}