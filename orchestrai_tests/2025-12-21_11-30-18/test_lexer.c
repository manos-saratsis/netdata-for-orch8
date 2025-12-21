#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "munit.h"
#include "../eval-internal.h"
#include "parser_internal.h"

// Mock implementations of helper functions
extern void *ParseAlloc(void *(*mallocPtr)(size_t));
extern void Parse(void *parser, int token_type, YYSTYPE token_value, EVAL_NODE **result);
extern void ParseFree(void *parser, void (*freePtr)(void *));

// Mock evaluation node free function
void eval_node_free(EVAL_NODE *node) {
    if (node) free(node);
}

void *mock_mallocz(size_t size) {
    return calloc(1, size);
}

void mock_freez(void *ptr) {
    free(ptr);
}

static MunitResult test_scanner_init_null_input(const MunitParameter params[], void *fixture) {
    Scanner s;
    scanner_init(&s, NULL);
    
    munit_assert_string_equal(s.cursor, "");
    munit_assert_string_equal(s.marker, s.cursor);
    munit_assert_string_equal(s.token, s.cursor);
    munit_assert_string_equal(s.limit, s.cursor);
    munit_assert_int_equal(s.line, 1);
    munit_assert_int_equal(s.error, 1);
    
    return MUNIT_OK;
}

static MunitResult test_scanner_init_valid_input(const MunitParameter params[], void *fixture) {
    Scanner s;
    const char *input = "test input";
    scanner_init(&s, input);
    
    munit_assert_string_equal(s.cursor, input);
    munit_assert_string_equal(s.marker, s.cursor);
    munit_assert_string_equal(s.token, s.cursor);
    munit_assert_int_equal(s.line, 1);
    munit_assert_int_equal(s.error, 0);
    
    return MUNIT_OK;
}

static MunitResult test_parse_expression_empty_string(const MunitParameter params[], void *fixture) {
    const char *failed_at = NULL;
    int error = 0;
    
    EVAL_NODE *result = parse_expression_with_re2c_lemon("", &failed_at, &error);
    
    munit_assert_null(result);
    munit_assert_int_equal(error, EVAL_ERROR_SYNTAX);
    munit_assert_null(failed_at);
    
    return MUNIT_OK;
}

static MunitResult test_parse_expression_simple_numbers(const MunitParameter params[], void *fixture) {
    const char *inputs[] = {
        "42",
        "3.14",
        "-7",
        "0.001",
        "1e3",
        "-2.5e-2"
    };
    
    for (size_t i = 0; i < sizeof(inputs)/sizeof(inputs[0]); i++) {
        const char *failed_at = NULL;
        int error = 0;
        
        EVAL_NODE *result = parse_expression_with_re2c_lemon(inputs[i], &failed_at, &error);
        
        munit_assert_not_null(result);
        munit_assert_int_equal(error, EVAL_ERROR_OK);
        munit_assert_null(failed_at);
        
        // Optional: Add more specific assertions based on expected node structure
        eval_node_free(result);
    }
    
    return MUNIT_OK;
}

static MunitResult test_parse_expression_variables(const MunitParameter params[], void *fixture) {
    const char *inputs[] = {
        "$x",
        "${test_var}",
        "$long_variable_name"
    };
    
    for (size_t i = 0; i < sizeof(inputs)/sizeof(inputs[0]); i++) {
        const char *failed_at = NULL;
        int error = 0;
        
        EVAL_NODE *result = parse_expression_with_re2c_lemon(inputs[i], &failed_at, &error);
        
        munit_assert_not_null(result);
        munit_assert_int_equal(error, EVAL_ERROR_OK);
        munit_assert_null(failed_at);
        
        eval_node_free(result);
    }
    
    return MUNIT_OK;
}

static MunitResult test_parse_expression_operators(const MunitParameter params[], void *fixture) {
    const char *inputs[] = {
        "1 + 2",
        "5 * 3",
        "10 / 2",
        "7 % 3",
        "a && b",
        "x || y",
        "!z"
    };
    
    for (size_t i = 0; i < sizeof(inputs)/sizeof(inputs[0]); i++) {
        const char *failed_at = NULL;
        int error = 0;
        
        EVAL_NODE *result = parse_expression_with_re2c_lemon(inputs[i], &failed_at, &error);
        
        munit_assert_not_null(result);
        munit_assert_int_equal(error, EVAL_ERROR_OK);
        munit_assert_null(failed_at);
        
        eval_node_free(result);
    }
    
    return MUNIT_OK;
}

static MunitResult test_parse_expression_special_values(const MunitParameter params[], void *fixture) {
    const char *inputs[] = {
        "NaN",
        "Inf",
        "Infinity",
        "-Infinity"
    };
    
    for (size_t i = 0; i < sizeof(inputs)/sizeof(inputs[0]); i++) {
        const char *failed_at = NULL;
        int error = 0;
        
        EVAL_NODE *result = parse_expression_with_re2c_lemon(inputs[i], &failed_at, &error);
        
        munit_assert_not_null(result);
        munit_assert_int_equal(error, EVAL_ERROR_OK);
        munit_assert_null(failed_at);
        
        eval_node_free(result);
    }
    
    return MUNIT_OK;
}

static MunitResult test_parse_expression_invalid_tokens(const MunitParameter params[], void *fixture) {
    const char *inputs[] = {
        "#invalid",
        "1 @ 2",
        "${'bad_var}",
        "1.2.3"
    };
    
    for (size_t i = 0; i < sizeof(inputs)/sizeof(inputs[0]); i++) {
        const char *failed_at = NULL;
        int error = 0;
        
        EVAL_NODE *result = parse_expression_with_re2c_lemon(inputs[i], &failed_at, &error);
        
        munit_assert_null(result);
        munit_assert_int_not_equal(error, EVAL_ERROR_OK);
        munit_assert_not_null(failed_at);
    }
    
    return MUNIT_OK;
}

static MunitResult test_scan_token_extraction(const MunitParameter params[], void *fixture) {
    const char *input = "42 + $x";
    Scanner s;
    scanner_init(&s, input);
    YYSTYPE lval;
    
    int token = scan(&s, &lval);
    munit_assert_int_equal(token, TOK_NUMBER);
    munit_assert_float_equal(lval.dval, 42.0, 0.001);
    
    token = scan(&s, &lval);
    munit_assert_int_equal(token, TOK_PLUS);
    
    token = scan(&s, &lval);
    munit_assert_int_equal(token, TOK_VARIABLE);
    munit_assert_string_equal(lval.strval, "x");
    
    // Free allocated memory for variable
    free(lval.strval);
    
    return MUNIT_OK;
}

// Test suite definition
static MunitTest tests[] = {
    {
        "/scanner_init_null_input",
        test_scanner_init_null_input,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/scanner_init_valid_input",
        test_scanner_init_valid_input,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/parse_expression_empty_string",
        test_parse_expression_empty_string,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/parse_expression_simple_numbers",
        test_parse_expression_simple_numbers,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/parse_expression_variables",
        test_parse_expression_variables,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/parse_expression_operators",
        test_parse_expression_operators,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/parse_expression_special_values",
        test_parse_expression_special_values,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/parse_expression_invalid_tokens",
        test_parse_expression_invalid_tokens,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/scan_token_extraction",
        test_scan_token_extraction,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

// Test suite setup
static const MunitSuite test_suite = {
    "/lexer_tests",
    tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

// Main function to run tests
int main(int argc, char *argv[]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}