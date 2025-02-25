// dear imgui: Platform Binding for Emscripten 
// This needs to be used along with a Renderer (Webgpu , webgl)


// Implemented features:
//  [X] Platform: Clipboard support
//  [ ] Platform: Mouse cursor shape and visibility. Disable with XK_io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using
//  [X] Platform: Gamepad support. Enabled with XK_io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

#include "imgui.h"
#include "imgui_impl_emscripten.h"
#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/key_codes.h>
#include <emscripten/html5.h>


struct ImGui_ImplEmscripten_Data
{
    bool                 MouseTracked;
    ImGuiMouseCursor     LastMouseCursor;
    uint64_t             Time = 0;
    double TicksPerSecond = 1000000000.0; 
    ImGui_ImplEmscripten_Data()      { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
static ImGui_ImplEmscripten_Data* ImGui_ImplEmscripten_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplEmscripten_Data*)ImGui::GetIO().BackendPlatformUserData : NULL;
}



bool    ImGui_ImplEmscripten_Init(void *display, void *window, void* XQueryPointerFunction)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplEmscripten_Data* bd = IM_NEW(ImGui_ImplEmscripten_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_Emscripten";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.GetClipboardTextFn = NULL;
    io.SetClipboardTextFn = NULL;

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, ImGui_ImplEmscripten_KeyboardCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, ImGui_ImplEmscripten_KeyboardCallback);
    emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, ImGui_ImplEmscripten_MouseCallback);
    emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, ImGui_ImplEmscripten_MouseCallback);
    emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, ImGui_ImplEmscripten_MouseCallback);
    emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, ImGui_ImplEmscripten_MouseCallback);
    emscripten_set_gamepadconnected_callback(nullptr, EM_TRUE, ImGui_ImplEmscripten_GamepadCallback);
    emscripten_set_gamepaddisconnected_callback(nullptr, EM_TRUE, ImGui_ImplEmscripten_GamepadCallback);
    emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,EM_TRUE,ImGui_ImplEmscripten_FocusInCallback);
    emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,EM_TRUE,ImGui_ImplEmscripten_FocusOutCallback);

    bd->LastMouseCursor = ImGuiMouseCursor_COUNT;

    return true;
}

void    ImGui_ImplEmscripten_Shutdown()
{
    ImGui_ImplEmscripten_Data* bd = ImGui_ImplEmscripten_GetBackendData();
    IM_ASSERT(bd != NULL && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    io.GetClipboardTextFn = NULL;
    io.SetClipboardTextFn = NULL;
    io.BackendPlatformName = NULL;
    io.BackendPlatformUserData = NULL;
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, nullptr);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, nullptr);
    emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_gamepadconnected_callback(nullptr, EM_TRUE, nullptr);
    emscripten_set_gamepaddisconnected_callback(nullptr, EM_TRUE, nullptr);
    emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,EM_TRUE,nullptr);
    emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,EM_TRUE,nullptr);
    IM_DELETE(bd);
}

bool ImGui_ImplEmscripten_NewFrame()
{
    ImGui_ImplEmscripten_Data* bd = ImGui_ImplEmscripten_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! Missing call to renderer _NewFrame().");

    // Get canvas size
    double width, height;
    emscripten_get_element_css_size("#canvas", &width, &height);

    io.DisplaySize.x = static_cast<float>(width);
    io.DisplaySize.y = static_cast<float>(height);

    // Time management
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t current_time = static_cast<uint64_t>(ts.tv_nsec) + static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL;

    io.DeltaTime = bd->Time > 0 ? (float)(current_time - bd->Time) / bd->TicksPerSecond : (1.0f / 60.0f);
    bd->Time = current_time;

    // Input updates
    ImGui_ImplEmscripten_UpdateMouseData();
    ImGui_ImplEmscripten_UpdateGamepads();

    return true;
}


// Map XK_xxx to ImGuiKey_xxx.
static ImGuiKey ImGui_ImplEmscripten_VirtualKeyToImGuiKey(uint32_t param)
{
    switch (param)
    {
        case DOM_VK_TAB         :  return ImGuiKey_Tab;
        case DOM_VK_LEFT       : return ImGuiKey_LeftArrow;
        case DOM_VK_RIGHT   : return ImGuiKey_RightArrow;
        case DOM_VK_UP      : return ImGuiKey_UpArrow;
        case DOM_VK_DOWN    : return ImGuiKey_DownArrow;
        case DOM_VK_PAGE_UP  : return ImGuiKey_PageUp;
        case DOM_VK_PAGE_DOWN: return ImGuiKey_PageDown;
        case DOM_VK_HOME      : return ImGuiKey_Home;
        case DOM_VK_END     : return ImGuiKey_End;
        case DOM_VK_INSERT  : return ImGuiKey_Insert;
        case DOM_VK_DELETE  : return ImGuiKey_Delete;
        case DOM_VK_BACK_SPACE   : return ImGuiKey_Backspace;
        case DOM_VK_SPACE       : return ImGuiKey_Space;
        case DOM_VK_RETURN      : return ImGuiKey_Enter;
        case DOM_VK_ESCAPE      : return ImGuiKey_Escape;
        case DOM_VK_QUOTE   : return ImGuiKey_Apostrophe;
        case DOM_VK_COMMA: return ImGuiKey_Comma;
        case DOM_VK_HYPHEN_MINUS : return ImGuiKey_Minus;
        case DOM_VK_PERIOD: return ImGuiKey_Period;
        case DOM_VK_SLASH: return ImGuiKey_Slash;
        case DOM_VK_SEMICOLON: return ImGuiKey_Semicolon;
        case DOM_VK_EQUALS: return ImGuiKey_Equal;
        case DOM_VK_OPEN_BRACKET : return ImGuiKey_LeftBracket;
        case DOM_VK_BACK_SLASH: return ImGuiKey_Backslash;
        case DOM_VK_CLOSE_BRACKET: return ImGuiKey_RightBracket;
        case DOM_VK_BACK_QUOTE : return ImGuiKey_GraveAccent;
        case DOM_VK_CAPS_LOCK: return ImGuiKey_CapsLock;
        case DOM_VK_SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case DOM_VK_NUM_LOCK: return ImGuiKey_NumLock;
        case DOM_VK_PRINT: return ImGuiKey_PrintScreen;
        case DOM_VK_PAUSE: return ImGuiKey_Pause;
        case DOM_VK_NUMPAD0: return ImGuiKey_Keypad0;
        case DOM_VK_NUMPAD1: return ImGuiKey_Keypad1;
        case DOM_VK_NUMPAD2: return ImGuiKey_Keypad2;
        case DOM_VK_NUMPAD3: return ImGuiKey_Keypad3;
        case DOM_VK_NUMPAD4: return ImGuiKey_Keypad4;
        case DOM_VK_NUMPAD5: return ImGuiKey_Keypad5;
        case DOM_VK_NUMPAD6: return ImGuiKey_Keypad6;
        case DOM_VK_NUMPAD7: return ImGuiKey_Keypad7;
        case DOM_VK_NUMPAD8: return ImGuiKey_Keypad8;
        case DOM_VK_NUMPAD9: return ImGuiKey_Keypad9;
        case DOM_VK_DECIMAL : return ImGuiKey_KeypadDecimal;
        case DOM_VK_DIVIDE  : return ImGuiKey_KeypadDivide;
        case DOM_VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case DOM_VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case DOM_VK_ADD     : return ImGuiKey_KeypadAdd;
        case DOM_VK_ENTER   : return ImGuiKey_KeypadEnter;
        case DOM_VK_SHIFT    : return ImGuiKey_LeftShift;
        case DOM_VK_CONTROL  : return ImGuiKey_LeftCtrl;
        case DOM_VK_ALT      : return ImGuiKey_LeftAlt;
        case DOM_VK_WIN     : return ImGuiKey_LeftSuper;
        case DOM_VK_SHIFT    : return ImGuiKey_RightShift;
        case DOM_VK_CONTROL  : return ImGuiKey_RightCtrl;
        case DOM_VK_ALT      : return ImGuiKey_RightAlt;
        case DOM_VK_WIN    : return ImGuiKey_RightSuper;
        case DOM_VK_0: return ImGuiKey_0;
        case DOM_VK_1: return ImGuiKey_1;
        case DOM_VK_2: return ImGuiKey_2;
        case DOM_VK_3: return ImGuiKey_3;
        case DOM_VK_4: return ImGuiKey_4;
        case DOM_VK_5: return ImGuiKey_5;
        case DOM_VK_6: return ImGuiKey_6;
        case DOM_VK_7: return ImGuiKey_7;
        case DOM_VK_8: return ImGuiKey_8;
        case DOM_VK_9: return ImGuiKey_9;
        case DOM_VK_A: return ImGuiKey_A;
        case DOM_VK_B: return ImGuiKey_B;
        case DOM_VK_C: return ImGuiKey_C;
        case DOM_VK_D: return ImGuiKey_D;
        case DOM_VK_E: return ImGuiKey_E;
        case DOM_VK_F: return ImGuiKey_F;
        case DOM_VK_G: return ImGuiKey_G;
        case DOM_VK_H: return ImGuiKey_H;
        case DOM_VK_I: return ImGuiKey_I;
        case DOM_VK_J: return ImGuiKey_J;
        case DOM_VK_K: return ImGuiKey_K;
        case DOM_VK_L: return ImGuiKey_L;
        case DOM_VK_M: return ImGuiKey_M;
        case DOM_VK_N: return ImGuiKey_N;
        case DOM_VK_O: return ImGuiKey_O;
        case DOM_VK_P: return ImGuiKey_P;
        case DOM_VK_Q: return ImGuiKey_Q;
        case DOM_VK_R: return ImGuiKey_R;
        case DOM_VK_S: return ImGuiKey_S;
        case DOM_VK_T: return ImGuiKey_T;
        case DOM_VK_U: return ImGuiKey_U;
        case DOM_VK_V: return ImGuiKey_V;
        case DOM_VK_W: return ImGuiKey_W;
        case DOM_VK_X: return ImGuiKey_X;
        case DOM_VK_Y: return ImGuiKey_Y;
        case DOM_VK_Z: return ImGuiKey_Z;
        case DOM_VK_F1: return ImGuiKey_F1;
        case DOM_VK_F2: return ImGuiKey_F2;
        case DOM_VK_F3: return ImGuiKey_F3;
        case DOM_VK_F4: return ImGuiKey_F4;
        case DOM_VK_F5: return ImGuiKey_F5;
        case DOM_VK_F6: return ImGuiKey_F6;
        case DOM_VK_F7: return ImGuiKey_F7;
        case DOM_VK_F8: return ImGuiKey_F8;
        case DOM_VK_F9: return ImGuiKey_F9;
        case DOM_VK_F10: return ImGuiKey_F10;
        case DOM_VK_F11: return ImGuiKey_F11;
        case DOM_VK_F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
    }
}

static int ImGui_ImplEmscripten_HandleKeyEvent(const EmscriptenKeyboardEvent* event, bool is_key_down)
{
    ImGuiIO& io = ImGui::GetIO();
    int key = event->keyCode;
    const ImGuiKey imguiKey = ImGui_ImplEmscripten_VirtualKeyToImGuiKey(key);
    io.AddKeyEvent(imguiKey, is_key_down);
    if (is_key_down && event->charCode)
        io.AddInputCharacter(event->charCode);
    return 1;
}

static EM_BOOL ImGui_ImplEmscripten_KeyboardCallback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData)
{
    bool is_key_down = (eventType == EMSCRIPTEN_EVENT_KEYDOWN);
    ImGui_ImplEmscripten_HandleKeyEvent(keyEvent, is_key_down);
    return EM_TRUE; // Capture event
}

static EM_BOOL ImGui_ImplEmscripten_MouseCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
    ImGuiIO& io = ImGui::GetIO();
    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN || eventType == EMSCRIPTEN_EVENT_MOUSEUP)
    {
        ImGuiMouseButton button = (mouseEvent->button == 0) ? ImGuiMouseButton_Left : (mouseEvent->button == 2 ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
        io.AddMouseButtonEvent(button, eventType == EMSCRIPTEN_EVENT_MOUSEDOWN);
    }
    else if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE)
    {
        io.AddMousePosEvent(mouseEvent->clientX, mouseEvent->clientY);
    }
    else if (eventType == EMSCRIPTEN_EVENT_WHEEL)
    {
        io.AddMouseWheelEvent(mouseEvent->deltaX, mouseEvent->deltaY);
    }
    return EM_TRUE;
}

static EM_BOOL ImGui_ImplEmscripten_GamepadCallback(int eventType, const EmscriptenGamepadEvent* gamepadEvent, void* userData)
{
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < gamepadEvent->numButtons; ++i)
    {
        io.AddKeyEvent(ImGuiKey_GamepadStart + i, gamepadEvent->button[i].pressed);
    }
    for (int i = 0; i < gamepadEvent->numAxes; ++i)
    {
        io.AddAnalogInput(ImGuiKey_GamepadLStickX + i, gamepadEvent->axis[i]);
    }
    return EM_TRUE;
}

static EM_BOOL ImGui_ImplEmscripten_FocusInCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void* userData)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplEmscripten_Data* bd = ImGui_ImplEmscripten_Data();
    bd->MouseTracked = true;
    return EM_TRUE;
}

static EM_BOOL ImGui_ImplEmscripten_FocusOutCallback(int eventType,const EmscriptenFocusEvent *focusEvent, void* userData)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplEmscripten_Data* bd = ImGui_ImplEmscripten_Data();
    bd->MouseTracked = false;
    return EM_TRUE;
}


static void ImGui_ImplEmscripten_UpdateMouseData()
{
    ImGuiIO& io = ImGui::GetIO();
    EmscriptenPointerlockChangeEvent plEvent;
    ImGui_ImplEmscripten_Data* bd = ImGui_ImplEmscripten_Data();
    if(bd->MouseTracked)
    {
        if (emscripten_get_pointerlock_status(&plEvent) == EMSCRIPTEN_RESULT_SUCCESS && plEvent.isActive)
        {
            io.AddMousePosEvent((float)plEvent.clientX, (float)plEvent.clientY);
        }
        else
        {
            EmscriptenMouseEvent mouseEvent;
            if (emscripten_get_mouse_status(&mouseEvent) == EMSCRIPTEN_RESULT_SUCCESS)
            {
                io.AddMousePosEvent((float)mouseEvent.clientX, (float)mouseEvent.clientY);
            }
        }
    }
}

static void ImGui_ImplEmscripten_UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();
    EmscriptenGamepadEvent gamepadEvent;
    for (int i = 0; i < emscripten_get_num_gamepads(); ++i)
    {
        if (emscripten_get_gamepad_status(i, &gamepadEvent) == EMSCRIPTEN_RESULT_SUCCESS)
        {
            for (int j = 0; j < gamepadEvent.numButtons; ++j)
            {
                io.AddKeyEvent(ImGuiKey_GamepadStart + j, gamepadEvent.analogButton[j] > 0.5f);
            }
            for (int j = 0; j < gamepadEvent.numAxes; ++j)
            {
                io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickX + j, gamepadEvent.axis[j], gamepadEvent.axis[j]);
            }
        }
    }
}
