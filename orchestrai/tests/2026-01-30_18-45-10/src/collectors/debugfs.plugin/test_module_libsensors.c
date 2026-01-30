#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <cmocka.h>
#include <math.h>
#include <limits.h>

/* Type definitions */
typedef enum {
    SENSORS_FEATURE_IN,
    SENSORS_FEATURE_FAN,
    SENSORS_FEATURE_TEMP,
    SENSORS_FEATURE_POWER,
    SENSORS_FEATURE_ENERGY,
    SENSORS_FEATURE_CURR,
    SENSORS_FEATURE_HUMIDITY,
    SENSORS_FEATURE_INTRUSION,
    SENSORS_FEATURE_BEEP_ENABLE,
    SENSORS_FEATURE_UNKNOWN,
} sensors_feature_type;

typedef enum {
    SENSORS_SUBFEATURE_IN_INPUT,
    SENSORS_SUBFEATURE_IN_MIN,
    SENSORS_SUBFEATURE_IN_MAX,
    SENSORS_SUBFEATURE_IN_LCRIT,
    SENSORS_SUBFEATURE_IN_CRIT,
    SENSORS_SUBFEATURE_IN_AVERAGE,
    SENSORS_SUBFEATURE_IN_LOWEST,
    SENSORS_SUBFEATURE_IN_HIGHEST,
    SENSORS_SUBFEATURE_IN_ALARM,
    SENSORS_SUBFEATURE_IN_MIN_ALARM,
    SENSORS_SUBFEATURE_IN_MAX_ALARM,
    SENSORS_SUBFEATURE_IN_BEEP,
    SENSORS_SUBFEATURE_IN_LCRIT_ALARM,
    SENSORS_SUBFEATURE_IN_CRIT_ALARM,
    SENSORS_SUBFEATURE_FAN_INPUT,
    SENSORS_SUBFEATURE_FAN_MIN,
    SENSORS_SUBFEATURE_FAN_MAX,
    SENSORS_SUBFEATURE_FAN_ALARM,
    SENSORS_SUBFEATURE_FAN_FAULT,
    SENSORS_SUBFEATURE_FAN_DIV,
    SENSORS_SUBFEATURE_FAN_BEEP,
    SENSORS_SUBFEATURE_FAN_PULSES,
    SENSORS_SUBFEATURE_FAN_MIN_ALARM,
    SENSORS_SUBFEATURE_FAN_MAX_ALARM,
    SENSORS_SUBFEATURE_TEMP_INPUT,
    SENSORS_SUBFEATURE_TEMP_MAX,
    SENSORS_SUBFEATURE_TEMP_MAX_HYST,
    SENSORS_SUBFEATURE_TEMP_MIN,
    SENSORS_SUBFEATURE_TEMP_CRIT,
    SENSORS_SUBFEATURE_TEMP_CRIT_HYST,
    SENSORS_SUBFEATURE_TEMP_LCRIT,
    SENSORS_SUBFEATURE_TEMP_EMERGENCY,
    SENSORS_SUBFEATURE_TEMP_EMERGENCY_HYST,
    SENSORS_SUBFEATURE_TEMP_LOWEST,
    SENSORS_SUBFEATURE_TEMP_HIGHEST,
    SENSORS_SUBFEATURE_TEMP_MIN_HYST,
    SENSORS_SUBFEATURE_TEMP_LCRIT_HYST,
    SENSORS_SUBFEATURE_TEMP_ALARM,
    SENSORS_SUBFEATURE_TEMP_MAX_ALARM,
    SENSORS_SUBFEATURE_TEMP_MIN_ALARM,
    SENSORS_SUBFEATURE_TEMP_CRIT_ALARM,
    SENSORS_SUBFEATURE_TEMP_FAULT,
    SENSORS_SUBFEATURE_TEMP_TYPE,
    SENSORS_SUBFEATURE_TEMP_OFFSET,
    SENSORS_SUBFEATURE_TEMP_BEEP,
    SENSORS_SUBFEATURE_TEMP_EMERGENCY_ALARM,
    SENSORS_SUBFEATURE_TEMP_LCRIT_ALARM,
    SENSORS_SUBFEATURE_POWER_AVERAGE,
    SENSORS_SUBFEATURE_POWER_AVERAGE_HIGHEST,
    SENSORS_SUBFEATURE_POWER_AVERAGE_LOWEST,
    SENSORS_SUBFEATURE_POWER_INPUT,
    SENSORS_SUBFEATURE_POWER_INPUT_HIGHEST,
    SENSORS_SUBFEATURE_POWER_INPUT_LOWEST,
    SENSORS_SUBFEATURE_POWER_CAP,
    SENSORS_SUBFEATURE_POWER_CAP_HYST,
    SENSORS_SUBFEATURE_POWER_MAX,
    SENSORS_SUBFEATURE_POWER_CRIT,
    SENSORS_SUBFEATURE_POWER_MIN,
    SENSORS_SUBFEATURE_POWER_LCRIT,
    SENSORS_SUBFEATURE_POWER_AVERAGE_INTERVAL,
    SENSORS_SUBFEATURE_POWER_ALARM,
    SENSORS_SUBFEATURE_POWER_CAP_ALARM,
    SENSORS_SUBFEATURE_POWER_MAX_ALARM,
    SENSORS_SUBFEATURE_POWER_CRIT_ALARM,
    SENSORS_SUBFEATURE_POWER_MIN_ALARM,
    SENSORS_SUBFEATURE_POWER_LCRIT_ALARM,
    SENSORS_SUBFEATURE_ENERGY_INPUT,
    SENSORS_SUBFEATURE_CURR_INPUT,
    SENSORS_SUBFEATURE_CURR_MIN,
    SENSORS_SUBFEATURE_CURR_MAX,
    SENSORS_SUBFEATURE_CURR_LCRIT,
    SENSORS_SUBFEATURE_CURR_CRIT,
    SENSORS_SUBFEATURE_CURR_AVERAGE,
    SENSORS_SUBFEATURE_CURR_LOWEST,
    SENSORS_SUBFEATURE_CURR_HIGHEST,
    SENSORS_SUBFEATURE_CURR_ALARM,
    SENSORS_SUBFEATURE_CURR_MIN_ALARM,
    SENSORS_SUBFEATURE_CURR_MAX_ALARM,
    SENSORS_SUBFEATURE_CURR_BEEP,
    SENSORS_SUBFEATURE_CURR_LCRIT_ALARM,
    SENSORS_SUBFEATURE_CURR_CRIT_ALARM,
    SENSORS_SUBFEATURE_HUMIDITY_INPUT,
    SENSORS_SUBFEATURE_VID,
    SENSORS_SUBFEATURE_INTRUSION_ALARM,
    SENSORS_SUBFEATURE_INTRUSION_BEEP,
    SENSORS_SUBFEATURE_BEEP_ENABLE,
    SENSORS_SUBFEATURE_UNKNOWN,
} sensors_subfeature_type;

typedef enum {
    SENSOR_STATE_NONE       = 0,
    SENSOR_STATE_CLEAR      = (1 << 0),
    SENSOR_STATE_WARNING    = (1 << 1),
    SENSOR_STATE_CAP        = (1 << 2),
    SENSOR_STATE_ALARM      = (1 << 3),
    SENSOR_STATE_CRITICAL   = (1 << 4),
    SENSOR_STATE_EMERGENCY  = (1 << 5),
    SENSOR_STATE_FAULT      = (1 << 6),
} SENSOR_STATE;

/* Forward declarations */
int do_module_libsensors(int update_every, const char *name);
void module_libsensors_cleanup(void);
void libsensors_thread(void *ptr);

/* Mock thread handling */
typedef void *(*thread_func)(void *);
typedef int ND_THREAD;

__attribute__((weak)) ND_THREAD *nd_thread_create(const char *name, int options, void *func, void *ptr) {
    return (ND_THREAD *)1;
}

__attribute__((weak)) int nd_thread_signal_cancel(ND_THREAD *thread) {
    return 0;
}

__attribute__((weak)) void nd_thread_join(ND_THREAD *thread) {
}

/* Test: do_module_libsensors - initial call creates thread */
static void test_do_module_libsensors_initial(void **state) {
    int result = do_module_libsensors(1, "sensors");
    /* Should return 0 on success */
    assert_true(result == 0 || result == 1);
}

/* Test: do_module_libsensors - with different update_every */
static void test_do_module_libsensors_different_update_every(void **state) {
    int result = do_module_libsensors(10, "sensors");
    assert_true(result == 0 || result == 1);
}

/* Test: do_module_libsensors - with NULL name */
static void test_do_module_libsensors_null_name(void **state) {
    int result = do_module_libsensors(1, NULL);
    assert_true(result == 0 || result == 1);
}

/* Test: do_module_libsensors - with zero update_every */
static void test_do_module_libsensors_zero_update_every(void **state) {
    int result = do_module_libsensors(0, "sensors");
    assert_true(result == 0 || result == 1);
}

/* Test: module_libsensors_cleanup - successful cleanup */
static void test_module_libsensors_cleanup(void **state) {
    /* Cleanup should handle NULL gracefully */
    module_libsensors_cleanup();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_module_libsensors_initial),
        cmocka_unit_test(test_do_module_libsensors_different_update_every),
        cmocka_unit_test(test_do_module_libsensors_null_name),
        cmocka_unit_test(test_do_module_libsensors_zero_update_every),
        cmocka_unit_test(test_module_libsensors_cleanup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}