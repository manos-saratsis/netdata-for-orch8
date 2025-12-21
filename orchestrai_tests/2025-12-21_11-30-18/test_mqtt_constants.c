#include <assert.h>
#include "mqtt_constants.h"

void test_mqtt_constants() {
    // Test QoS-related constants
    assert(MQTT_MAX_QOS == 0x02);

    // Test MQTT version
    assert(MQTT_VERSION_5_0 == 0x5);

    // Test Variable Byte Integer constants
    assert(MQTT_VBI_CONTINUATION_FLAG == 0x80);
    assert(MQTT_VBI_DATA_MASK == 0x7F);
    assert(MQTT_VBI_MAXBYTES == 4);

    // Test Control Packet Types
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

    // Test Connect Flags
    assert(MQTT_CONNECT_FLAG_USERNAME == 0x80);
    assert(MQTT_CONNECT_FLAG_PASSWORD == 0x40);
    assert(MQTT_CONNECT_FLAG_LWT_RETAIN == 0x20);
    assert(MQTT_CONNECT_FLAG_LWT == 0x04);
    assert(MQTT_CONNECT_FLAG_CLEAN_START == 0x02);
    
    // Test QoS-related flags
    assert(MQTT_CONNECT_FLAG_QOS_MASK == 0x18);
    assert(MQTT_CONNECT_FLAG_QOS_BITSHIFT == 3);

    // Test Client ID max length
    assert(MQTT_MAX_CLIENT_ID == 23);

    // Optional: Test a few property identifiers
    assert(MQTT_PROP_PAYLOAD_FMT_INDICATOR == 0x01);
    assert(MQTT_PROP_MSG_EXPIRY_INTERVAL == 0x02);
}

int main() {
    test_mqtt_constants();
    return 0;
}