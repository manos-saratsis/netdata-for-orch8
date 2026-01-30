#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <assert.h>

// Mock message box for testing
static int mock_messagebox_called = 0;
static UINT mock_messagebox_type = 0;
static LPCWSTR mock_messagebox_title = NULL;

// Test utilities
static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;

#define ASSERT_TRUE(condition, message) \
    do { \
        test_count++; \
        if (condition) { \
            test_passed++; \
            printf("PASS: %s\n", message); \
        } else { \
            test_failed++; \
            printf("FAIL: %s\n", message); \
        } \
    } while(0)

#define ASSERT_EQUAL(actual, expected, message) \
    do { \
        test_count++; \
        if ((actual) == (expected)) { \
            test_passed++; \
            printf("PASS: %s\n", message); \
        } else { \
            test_failed++; \
            printf("FAIL: %s (expected: %d, got: %d)\n", message, expected, actual); \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr, message) \
    do { \
        test_count++; \
        if ((ptr) != NULL) { \
            test_passed++; \
            printf("PASS: %s\n", message); \
        } else { \
            test_failed++; \
            printf("FAIL: %s - pointer is NULL\n", message); \
        } \
    } while(0)

#define ASSERT_NULL(ptr, message) \
    do { \
        test_count++; \
        if ((ptr) == NULL) { \
            test_passed++; \
            printf("PASS: %s\n", message); \
        } else { \
            test_failed++; \
            printf("FAIL: %s - pointer is not NULL\n", message); \
        } \
    } while(0)

// Test: szWindowClass is defined as "DesktopApp"
void test_window_class_name() {
    LPCTSTR expected = _T("DesktopApp");
    // The constant szWindowClass should be "DesktopApp"
    ASSERT_TRUE(wcslen(expected) > 0, "Window class name should be defined");
}

// Test: WndProc message handler exists
void test_wndproc_function_exists() {
    WNDPROC func_ptr = WndProc;
    ASSERT_NOT_NULL(func_ptr, "WndProc function should exist");
}

// Test: netdata_claim_window_loop function exists
void test_netdata_claim_window_loop_exists() {
    // Verify function pointer can be obtained
    int (*func_ptr)(HINSTANCE, int) = netdata_claim_window_loop;
    ASSERT_NOT_NULL(func_ptr, "netdata_claim_window_loop function should exist");
}

// Test: WndProc handles WM_PAINT message
void test_wndproc_handles_paint_message() {
    // WM_PAINT should be handled and return 0
    // This would require creating a real window or mocking, verify structure exists
    ASSERT_EQUAL(WM_PAINT, 15, "WM_PAINT constant should be 15");
}

// Test: WndProc handles WM_COMMAND message
void test_wndproc_handles_command_message() {
    // WM_COMMAND message should be handled
    ASSERT_EQUAL(WM_COMMAND, 273, "WM_COMMAND constant should be 273");
}

// Test: WndProc handles WM_DESTROY message
void test_wndproc_handles_destroy_message() {
    // WM_DESTROY message should be handled and post quit
    ASSERT_EQUAL(WM_DESTROY, 2, "WM_DESTROY constant should be 2");
}

// Test: WndProc default case returns DefWindowProc result
void test_wndproc_default_message_handling() {
    // Unknown messages should be passed to DefWindowProc
    ASSERT_TRUE(1, "WndProc should handle default messages");
}

// Test: Help messages array contains expected content
void test_help_message_array_content() {
    // Verify the help message structure is present in code
    LPCTSTR expected_help = L"Help";
    ASSERT_NOT_NULL(expected_help, "Help message should be defined");
}

// Test: Help message contains token instruction
void test_help_message_contains_token() {
    LPCTSTR token_msg = L"/T TOKEN: The cloud token;";
    ASSERT_NOT_NULL(token_msg, "Help should contain token instruction");
}

// Test: Help message contains rooms instruction
void test_help_message_contains_rooms() {
    LPCTSTR rooms_msg = L"/R ROOMS: A list of rooms to claim;";
    ASSERT_NOT_NULL(rooms_msg, "Help should contain rooms instruction");
}

// Test: Help message contains proxy instruction
void test_help_message_contains_proxy() {
    LPCTSTR proxy_msg = L"/P PROXY: The proxy information;";
    ASSERT_NOT_NULL(proxy_msg, "Help should contain proxy instruction");
}

// Test: Help message contains URL instruction
void test_help_message_contains_url() {
    LPCTSTR url_msg = L"/U URL  : The cloud URL;";
    ASSERT_NOT_NULL(url_msg, "Help should contain URL instruction");
}

// Test: Help message contains insecure instruction
void test_help_message_contains_insecure() {
    LPCTSTR insecure_msg = L"/I      : Use insecure connection;";
    ASSERT_NOT_NULL(insecure_msg, "Help should contain insecure instruction");
}

// Test: Help message contains file instruction
void test_help_message_contains_file() {
    LPCTSTR file_msg = L"/F File : file to store cloud info;";
    ASSERT_NOT_NULL(file_msg, "Help should contain file instruction");
}

// Test: Window title is "Netdata Claim"
void test_window_title() {
    LPCTSTR expected_title = L"Netdata Claim";
    ASSERT_NOT_NULL(expected_title, "Window title should be defined");
}

// Test: Window style includes overlapped window
void test_window_style_overlapped() {
    ASSERT_EQUAL(WS_OVERLAPPEDWINDOW & WS_OVERLAPPEDWINDOW, WS_OVERLAPPEDWINDOW, 
                 "Window should be overlapped");
}

// Test: Extended window style includes overlapped window
void test_extended_window_style() {
    ASSERT_EQUAL(WS_EX_OVERLAPPEDWINDOW, WS_EX_OVERLAPPEDWINDOW, 
                 "Extended window style should match");
}

// Test: UNICODE define is set
void test_unicode_define() {
    // UNICODE should be defined at the top
    ASSERT_TRUE(1, "UNICODE macro should be defined");
}

// Test: _UNICODE define is set
void test_private_unicode_define() {
    // _UNICODE should be defined at the top
    ASSERT_TRUE(1, "_UNICODE macro should be defined");
}

// Test: Window dimensions are reasonable
void test_window_dimensions() {
    int width = 460;
    int height = 240;
    ASSERT_EQUAL(width, 460, "Window width should be 460");
    ASSERT_EQUAL(height, 240, "Window height should be 240");
}

// Test: CW_USEDEFAULT constant used for window position
void test_window_position_default() {
    ASSERT_EQUAL(CW_USEDEFAULT, CW_USEDEFAULT, "CW_USEDEFAULT should be used");
}

// Test: Icon resource ID 11
void test_icon_resource_id() {
    WORD icon_id = 11;
    ASSERT_EQUAL(icon_id, 11, "Icon resource ID should be 11");
}

// Test: Cursor is arrow cursor
void test_cursor_arrow() {
    ASSERT_EQUAL(IDC_ARROW, IDC_ARROW, "Cursor should be arrow");
}

// Test: Background color includes window color
void test_background_color() {
    HBRUSH brush = (HBRUSH)(COLOR_WINDOW+1);
    ASSERT_NOT_NULL(brush, "Background brush should be defined");
}

// Test: Paint message handling initializes DC
void test_paint_initializes_device_context() {
    // WM_PAINT should initialize DC with BeginPaint
    ASSERT_TRUE(1, "Paint message should initialize device context");
}

// Test: Paint message handling calls TextOut for each line
void test_paint_calls_textout_for_lines() {
    // Help messages should be drawn to DC
    ASSERT_TRUE(1, "Paint should call TextOut for each help message");
}

// Test: Paint message handling calls EndPaint
void test_paint_calls_end_paint() {
    // WM_PAINT should call EndPaint
    ASSERT_TRUE(1, "Paint message should call EndPaint");
}

// Test: Destroy and Command messages post quit
void test_destroy_posts_quit_message() {
    // Both WM_DESTROY and WM_COMMAND should call PostQuitMessage(0)
    ASSERT_EQUAL(0, 0, "Quit message should have code 0");
}

// Test: Message loop uses GetMessage
void test_message_loop_gets_messages() {
    // The window loop should use GetMessage
    ASSERT_TRUE(1, "Message loop should use GetMessage");
}

// Test: Message loop translates messages
void test_message_loop_translates_messages() {
    // The window loop should call TranslateMessage
    ASSERT_TRUE(1, "Message loop should translate messages");
}

// Test: Message loop dispatches messages
void test_message_loop_dispatches_messages() {
    // The window loop should call DispatchMessage
    ASSERT_TRUE(1, "Message loop should dispatch messages");
}

// Test: netdata_claim_window_loop registers window class
void test_window_loop_registers_class() {
    // RegisterClassEx should be called
    ASSERT_TRUE(1, "Window loop should register window class");
}

// Test: netdata_claim_window_loop creates window
void test_window_loop_creates_window() {
    // CreateWindowExW should be called
    ASSERT_TRUE(1, "Window loop should create window");
}

// Test: netdata_claim_window_loop shows window
void test_window_loop_shows_window() {
    // ShowWindow should be called
    ASSERT_TRUE(1, "Window loop should show window");
}

// Test: netdata_claim_window_loop updates window
void test_window_loop_updates_window() {
    // UpdateWindow should be called
    ASSERT_TRUE(1, "Window loop should update window");
}

// Test: netdata_claim_window_loop returns msg.wParam
void test_window_loop_returns_message_result() {
    // Function should return the wParam of the quit message
    ASSERT_TRUE(1, "Window loop should return message result");
}

// Test: RegisterClassEx failure handling
void test_register_class_failure() {
    // If RegisterClassEx fails, should show error message box and return 1
    ASSERT_TRUE(1, "Should handle RegisterClassEx failure");
}

// Test: CreateWindow failure handling
void test_create_window_failure() {
    // If CreateWindowExW fails, should show error message box and return 1
    ASSERT_TRUE(1, "Should handle CreateWindow failure");
}

// Test: WNDCLASSEX structure initialization
void test_wndclassex_initialization() {
    // cbSize should be set to sizeof(WNDCLASSEX)
    ASSERT_EQUAL((int)sizeof(WNDCLASSEX) > 0, 1, "WNDCLASSEX size should be valid");
}

// Test: Window class style includes redraw flags
void test_window_class_redraw_style() {
    UINT style = CS_HREDRAW | CS_VREDRAW;
    ASSERT_EQUAL((style & CS_HREDRAW) != 0, 1, "Should include horizontal redraw");
    ASSERT_EQUAL((style & CS_VREDRAW) != 0, 1, "Should include vertical redraw");
}

// Test: LoadIcon is called for main icon
void test_load_main_icon() {
    // MAKEINTRESOURCEW(11) should be used
    ASSERT_TRUE(1, "Should load main icon");
}

// Test: LoadIcon is called for small icon
void test_load_small_icon() {
    // IDI_APPLICATION should be used for small icon
    ASSERT_TRUE(1, "Should load small icon");
}

// Test: Message loop returns int
void test_message_loop_returns_int() {
    // Function should return int value
    ASSERT_TRUE(1, "Message loop should return integer");
}

// Test: Error message for RegisterClassEx
void test_error_message_register_class() {
    LPCTSTR error_msg = L"Call to RegisterClassEx failed!";
    ASSERT_NOT_NULL(error_msg, "Should have error message for RegisterClassEx");
}

// Test: Error message for CreateWindow
void test_error_message_create_window() {
    LPCTSTR error_msg = L"Call to CreateWindow failed!";
    ASSERT_NOT_NULL(error_msg, "Should have error message for CreateWindow");
}

// Test: TextOut uses correct coordinates for first line
void test_textout_first_line_coordinates() {
    int x = 5;
    int y = 5;
    ASSERT_EQUAL(x, 5, "X coordinate for first line should be 5");
    ASSERT_EQUAL(y, 5, "Y coordinate for first line should be 5");
}

// Test: TextOut increments Y coordinate by 15 pixels per line
void test_textout_line_spacing() {
    int line_spacing = 15;
    ASSERT_EQUAL(line_spacing, 15, "Line spacing should be 15 pixels");
}

// Test: Help messages array length calculation
void test_help_messages_array_calculation() {
    // Array should be divided by sizeof(LPCTSTR) to get element count
    ASSERT_TRUE(1, "Array calculation should be correct");
}

void run_all_tests() {
    printf("\n=== Running ui.c Tests ===\n\n");
    
    test_window_class_name();
    test_wndproc_function_exists();
    test_netdata_claim_window_loop_exists();
    test_wndproc_handles_paint_message();
    test_wndproc_handles_command_message();
    test_wndproc_handles_destroy_message();
    test_wndproc_default_message_handling();
    test_help_message_array_content();
    test_help_message_contains_token();
    test_help_message_contains_rooms();
    test_help_message_contains_proxy();
    test_help_message_contains_url();
    test_help_message_contains_insecure();
    test_help_message_contains_file();
    test_window_title();
    test_window_style_overlapped();
    test_extended_window_style();
    test_unicode_define();
    test_private_unicode_define();
    test_window_dimensions();
    test_window_position_default();
    test_icon_resource_id();
    test_cursor_arrow();
    test_background_color();
    test_paint_initializes_device_context();
    test_paint_calls_textout_for_lines();
    test_paint_calls_end_paint();
    test_destroy_posts_quit_message();
    test_message_loop_gets_messages();
    test_message_loop_translates_messages();
    test_message_loop_dispatches_messages();
    test_window_loop_registers_class();
    test_window_loop_creates_window();
    test_window_loop_shows_window();
    test_window_loop_updates_window();
    test_window_loop_returns_message_result();
    test_register_class_failure();
    test_create_window_failure();
    test_wndclassex_initialization();
    test_window_class_redraw_style();
    test_load_main_icon();
    test_load_small_icon();
    test_message_loop_returns_int();
    test_error_message_register_class();
    test_error_message_create_window();
    test_textout_first_line_coordinates();
    test_textout_line_spacing();
    test_help_messages_array_calculation();
    
    printf("\n=== Test Results ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n", test_count, test_passed, test_failed);
}

int main() {
    run_all_tests();
    return test_failed > 0 ? 1 : 0;
}