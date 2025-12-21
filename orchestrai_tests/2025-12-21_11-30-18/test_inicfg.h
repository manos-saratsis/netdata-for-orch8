#ifndef TEST_INICFG_H
#define TEST_INICFG_H

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/libnetdata/inicfg/inicfg.h"

// Macro to create test function prototype
#define TEST(name) static void name(void **state)

// Helper function declarations
struct config* setup_test_config(void);
void teardown_test_config(struct config* cfg);

// Test function prototypes
TEST(test_inicfg_load);
TEST(test_inicfg_foreach_value_in_section);
TEST(test_inicfg_set_default_raw_value);
TEST(test_inicfg_exists);
TEST(test_inicfg_move);
TEST(test_inicfg_move_everywhere);
TEST(test_inicfg_generate);
TEST(test_inicfg_section_compare);
TEST(test_inicfg_test_boolean_value);
TEST(test_add_connector_instance);
TEST(test_stream_conf_needs_dbengine);
TEST(test_stream_conf_has_api_enabled);
TEST(test_inicfg_free);
TEST(test_inicfg_get);
TEST(test_inicfg_set);
TEST(test_inicfg_get_number);
TEST(test_inicfg_get_number_range);
TEST(test_inicfg_set_number);
TEST(test_inicfg_get_double);
TEST(test_inicfg_set_double);
TEST(test_inicfg_get_boolean);
TEST(test_inicfg_get_boolean_ondemand);
TEST(test_inicfg_set_boolean);
TEST(test_inicfg_get_size_bytes);
TEST(test_inicfg_set_size_bytes);
TEST(test_inicfg_get_size_mb);
TEST(test_inicfg_set_size_mb);
TEST(test_inicfg_get_duration_ms);
TEST(test_inicfg_set_duration_ms);
TEST(test_inicfg_get_duration_seconds);
TEST(test_inicfg_set_duration_seconds);
TEST(test_inicfg_get_duration_days_to_seconds);

#endif // TEST_INICFG_H