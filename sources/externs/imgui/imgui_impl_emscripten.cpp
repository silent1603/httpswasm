// dear imgui: Platform Binding for Emscripten
// This needs to be used along with a Renderer (Webgpu , webgl)

// Implemented features:
//  [X] Platform: Clipboard support 
//  [ ] Platform: Mouse cursor shape and visibility.
//  [X] Platform: Keyboard arrays indexed using
//  [X] Platform: Gamepad support. 

#include "imgui.h"
#ifndef IMGUI_DISABLE
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include "imgui_impl_emscripten.h"

#include <emscripten.h>
#include <string.h>
#include <emscripten/key_codes.h>
#include <emscripten/html5.h>
#include <math.h>

#include <iostream>

const float DEADZONE = 0.3f; // Adjust to taste
const int clipboardBufferSize = 1024;
char  clipboardBuffer[clipboardBufferSize] = "";

struct ImGui_ImplEmscripten_Data
{
    bool             MouseTracked;
    ImGuiMouseCursor MouseCursors[ImGuiMouseCursor_COUNT];
    const char*      CanvasSelectorId;
    double           Time;
    ImGui_ImplEmscripten_Data() { memset((void *)this, 0, sizeof(*this)); }
};

static ImGui_ImplEmscripten_Data* ImGui_ImplEmscripten_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplEmscripten_Data *)ImGui::GetIO().BackendPlatformUserData : NULL;
}

extern "C"
{
EM_JS(void, ImGui_ImplEmscripten_OpenURL, (char const* url),
{ 
    url = url ? UTF8ToString(url) : null; 
    if (url)
    {
        window.open(url, '_blank');
    } 
}
);

EM_JS(void, ImGui_ImplEmscripten_CopyToClipBoard, (ImGuiContext* ctx, const char* text),
{
    text = text ? UTF8ToString(text) : null; 
    if (text)
    {
        navigator.clipboard.writeText(text);
    }
})

EM_ASYNC_JS(void, ImGui_ImplEmscripten_GetFromClipBoard, (char* text, int maxSize), {
    try {
        const str = await navigator.clipboard.readText(); // Wait for async result
        const size = Math.min(lengthBytesUTF8(str), maxSize - 1);
        stringToUTF8(str, text, size + 1); // Copy text and null-terminate
    } catch (err) {
        console.error("Failed to read clipboard:", err);
        stringToUTF8("", text, maxSize); // Fallback to empty string
    }
});

const char* ImGui_ImplEmscripten_GetClipboardText(ImGuiContext* ctx)
{
    ImGui_ImplEmscripten_GetFromClipBoard(clipboardBuffer, clipboardBufferSize);
    return clipboardBuffer; 
}

static bool ImGui_ImplEmscripten_PlatformOpenInShell(ImGuiContext* context,const char* url)
{
    ImGui_ImplEmscripten_OpenURL(url); 
   return true;
}

static bool ImGui_ImplEmscripten_FilterInputKeyCode(const EmscriptenKeyboardEvent* keyEvent)
{
    // Filter by key codes for non-printable keys
    switch (keyEvent->keyCode)
    {
        case DOM_VK_RETURN: // Enter
        case DOM_VK_TAB:  // Tab
        case DOM_VK_SHIFT: // Shift
        case DOM_VK_CONTROL: // Control
        case DOM_VK_ALT: // Alt
        case DOM_VK_CAPS_LOCK: // CapsLock
        case DOM_VK_ESCAPE: // Escape
        case DOM_VK_LEFT: // ArrowLeft
        case DOM_VK_UP: // ArrowUp
        case DOM_VK_RIGHT: // ArrowRight
        case DOM_VK_DOWN: // ArrowDown
        case DOM_VK_BACK_SPACE:  // Backspace
        case DOM_VK_DELETE: // Delete
        case DOM_VK_HOME: // Home
        case DOM_VK_END: // End
        case DOM_VK_PAGE_UP: // PageUp
        case DOM_VK_PAGE_DOWN: // PageDown
        case DOM_VK_WIN: // Meta (Windows/Command key)
        case DOM_VK_CONTEXT_MENU: // ContextMenu
            return true; // Filter out these keys (do not treat them as text input)
    }
    return false; // Let other keys pass through
}

static ImGuiKey ImGui_ImplEmscripten_VirtualKeyToImGuiKey(uint32_t param)
{
    switch (param)
    {
    case DOM_VK_TAB:
        return ImGuiKey_Tab;
    case DOM_VK_LEFT:
        return ImGuiKey_LeftArrow;
    case DOM_VK_RIGHT:
        return ImGuiKey_RightArrow;
    case DOM_VK_UP:
        return ImGuiKey_UpArrow;
    case DOM_VK_DOWN:
        return ImGuiKey_DownArrow;
    case DOM_VK_PAGE_UP:
        return ImGuiKey_PageUp;
    case DOM_VK_PAGE_DOWN:
        return ImGuiKey_PageDown;
    case DOM_VK_HOME:
        return ImGuiKey_Home;
    case DOM_VK_END:
        return ImGuiKey_End;
    case DOM_VK_INSERT:
        return ImGuiKey_Insert;
    case DOM_VK_DELETE:
        return ImGuiKey_Delete;
    case DOM_VK_BACK_SPACE:
        return ImGuiKey_Backspace;
    case DOM_VK_SPACE:
        return ImGuiKey_Space;
    case DOM_VK_RETURN:
        return ImGuiKey_Enter;
    case DOM_VK_ESCAPE:
        return ImGuiKey_Escape;
    case DOM_VK_QUOTE:
        return ImGuiKey_Apostrophe;
    case DOM_VK_COMMA:
        return ImGuiKey_Comma;
    case DOM_VK_HYPHEN_MINUS:
        return ImGuiKey_Minus;
    case DOM_VK_PERIOD:
        return ImGuiKey_Period;
    case DOM_VK_SLASH:
        return ImGuiKey_Slash;
    case DOM_VK_SEMICOLON:
        return ImGuiKey_Semicolon;
    case DOM_VK_EQUALS:
        return ImGuiKey_Equal;
    case DOM_VK_OPEN_BRACKET:
        return ImGuiKey_LeftBracket;
    case DOM_VK_BACK_SLASH:
        return ImGuiKey_Backslash;
    case DOM_VK_CLOSE_BRACKET:
        return ImGuiKey_RightBracket;
    case DOM_VK_BACK_QUOTE:
        return ImGuiKey_GraveAccent;
    case DOM_VK_CAPS_LOCK:
        return ImGuiKey_CapsLock;
    case DOM_VK_SCROLL_LOCK:
        return ImGuiKey_ScrollLock;
    case DOM_VK_NUM_LOCK:
        return ImGuiKey_NumLock;
    case DOM_VK_PRINT:
        return ImGuiKey_PrintScreen;
    case DOM_VK_PAUSE:
        return ImGuiKey_Pause;
    case DOM_VK_NUMPAD0:
        return ImGuiKey_Keypad0;
    case DOM_VK_NUMPAD1:
        return ImGuiKey_Keypad1;
    case DOM_VK_NUMPAD2:
        return ImGuiKey_Keypad2;
    case DOM_VK_NUMPAD3:
        return ImGuiKey_Keypad3;
    case DOM_VK_NUMPAD4:
        return ImGuiKey_Keypad4;
    case DOM_VK_NUMPAD5:
        return ImGuiKey_Keypad5;
    case DOM_VK_NUMPAD6:
        return ImGuiKey_Keypad6;
    case DOM_VK_NUMPAD7:
        return ImGuiKey_Keypad7;
    case DOM_VK_NUMPAD8:
        return ImGuiKey_Keypad8;
    case DOM_VK_NUMPAD9:
        return ImGuiKey_Keypad9;
    case DOM_VK_DECIMAL:
        return ImGuiKey_KeypadDecimal;
    case DOM_VK_DIVIDE:
        return ImGuiKey_KeypadDivide;
    case DOM_VK_MULTIPLY:
        return ImGuiKey_KeypadMultiply;
    case DOM_VK_SUBTRACT:
        return ImGuiKey_KeypadSubtract;
    case DOM_VK_ADD:
        return ImGuiKey_KeypadAdd;
    case DOM_VK_ENTER:
        return ImGuiKey_KeypadEnter;
    case DOM_VK_SHIFT:
        return ImGuiKey_LeftShift;
    case DOM_VK_CONTROL:
        return ImGuiKey_LeftCtrl;
    case DOM_VK_ALT:
        return ImGuiKey_LeftAlt;
    case DOM_VK_WIN:
        return ImGuiKey_LeftSuper;
    case DOM_VK_0:
        return ImGuiKey_0;
    case DOM_VK_1:
        return ImGuiKey_1;
    case DOM_VK_2:
        return ImGuiKey_2;
    case DOM_VK_3:
        return ImGuiKey_3;
    case DOM_VK_4:
        return ImGuiKey_4;
    case DOM_VK_5:
        return ImGuiKey_5;
    case DOM_VK_6:
        return ImGuiKey_6;
    case DOM_VK_7:
        return ImGuiKey_7;
    case DOM_VK_8:
        return ImGuiKey_8;
    case DOM_VK_9:
        return ImGuiKey_9;
    case DOM_VK_A:
        return ImGuiKey_A;
    case DOM_VK_B:
        return ImGuiKey_B;
    case DOM_VK_C:
        return ImGuiKey_C;
    case DOM_VK_D:
        return ImGuiKey_D;
    case DOM_VK_E:
        return ImGuiKey_E;
    case DOM_VK_F:
        return ImGuiKey_F;
    case DOM_VK_G:
        return ImGuiKey_G;
    case DOM_VK_H:
        return ImGuiKey_H;
    case DOM_VK_I:
        return ImGuiKey_I;
    case DOM_VK_J:
        return ImGuiKey_J;
    case DOM_VK_K:
        return ImGuiKey_K;
    case DOM_VK_L:
        return ImGuiKey_L;
    case DOM_VK_M:
        return ImGuiKey_M;
    case DOM_VK_N:
        return ImGuiKey_N;
    case DOM_VK_O:
        return ImGuiKey_O;
    case DOM_VK_P:
        return ImGuiKey_P;
    case DOM_VK_Q:
        return ImGuiKey_Q;
    case DOM_VK_R:
        return ImGuiKey_R;
    case DOM_VK_S:
        return ImGuiKey_S;
    case DOM_VK_T:
        return ImGuiKey_T;
    case DOM_VK_U:
        return ImGuiKey_U;
    case DOM_VK_V:
        return ImGuiKey_V;
    case DOM_VK_W:
        return ImGuiKey_W;
    case DOM_VK_X:
        return ImGuiKey_X;
    case DOM_VK_Y:
        return ImGuiKey_Y;
    case DOM_VK_Z:
        return ImGuiKey_Z;
    case DOM_VK_F1:
        return ImGuiKey_F1;
    case DOM_VK_F2:
        return ImGuiKey_F2;
    case DOM_VK_F3:
        return ImGuiKey_F3;
    case DOM_VK_F4:
        return ImGuiKey_F4;
    case DOM_VK_F5:
        return ImGuiKey_F5;
    case DOM_VK_F6:
        return ImGuiKey_F6;
    case DOM_VK_F7:
        return ImGuiKey_F7;
    case DOM_VK_F8:
        return ImGuiKey_F8;
    case DOM_VK_F9:
        return ImGuiKey_F9;
    case DOM_VK_F10:
        return ImGuiKey_F10;
    case DOM_VK_F11:
        return ImGuiKey_F11;
    case DOM_VK_F12:
        return ImGuiKey_F12;
    default:
        return ImGuiKey_None;
    }
}

static void ImGui_ImplEmscripten_UpdateKeyModifiers(const EmscriptenKeyboardEvent *event)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, event->ctrlKey);
    io.AddKeyEvent(ImGuiMod_Shift, event->shiftKey);
    io.AddKeyEvent(ImGuiMod_Alt, event->altKey);
    io.AddKeyEvent(ImGuiMod_Super, event->metaKey);
}

static EM_BOOL ImGui_ImplEmscripten_KeyUpCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplEmscripten_UpdateKeyModifiers(keyEvent);
    int key = keyEvent->keyCode;
    const ImGuiKey imguiKey = ImGui_ImplEmscripten_VirtualKeyToImGuiKey(key);
    io.AddKeyEvent(imguiKey, false);
    return EM_TRUE; // Capture event
}

static EM_BOOL ImGui_ImplEmscripten_KeyDownCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplEmscripten_UpdateKeyModifiers(keyEvent);
    int key = keyEvent->keyCode;
    const ImGuiKey imguiKey = ImGui_ImplEmscripten_VirtualKeyToImGuiKey(key);
    io.AddKeyEvent(imguiKey, true);
    if (*keyEvent->key && !ImGui_ImplEmscripten_FilterInputKeyCode(keyEvent))
    {
        const char* utf8 = keyEvent->key;
        unsigned int codepoint = 0;
        unsigned char c = utf8[0];
        if (c < 0x80) // 1-byte sequence (ASCII)
        {
            codepoint = c;
        }
        else if ((c & 0xE0) == 0xC0) // 2-byte sequence
        {
            codepoint = ((c & 0x1F) << 6) | (utf8[1] & 0x3F);
        }
        else if ((c & 0xF0) == 0xE0) // 3-byte sequence
        {
            codepoint = ((c & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
        }
        else if ((c & 0xF8) == 0xF0) // 4-byte sequence
        {
            codepoint = ((c & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) |
                        ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
        }

        io.AddInputCharacter(codepoint);
    }
    return EM_TRUE; // Capture event
}

static EM_BOOL ImGui_ImplEmscripten_KeyPressCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
    return EM_TRUE; // Capture event
}

static EM_BOOL ImGui_ImplEmscripten_MouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplEmscripten_Data *bd = ImGui_ImplEmscripten_GetBackendData();
    
    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN || eventType == EMSCRIPTEN_EVENT_MOUSEUP)
    {
        ImGuiMouseButton button = (mouseEvent->button == 0) ? ImGuiMouseButton_Left : (mouseEvent->button == 2 ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
        io.AddMouseButtonEvent(button, eventType == EMSCRIPTEN_EVENT_MOUSEDOWN);
    }
    else if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE)
    {
        io.AddMousePosEvent( mouseEvent->targetX,mouseEvent->targetY);
    }

    return EM_TRUE;
}

static EM_BOOL ImGui_ImplEmscripten_WhellCallback(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData)
{
    ImGuiIO &io = ImGui::GetIO();
    float multiplier = 0.0f;
    if (wheelEvent->deltaMode == DOM_DELTA_PIXEL)       { multiplier = 1.0f / 100.0f; } // 100 pixels make up a step.
    else if (wheelEvent->deltaMode == DOM_DELTA_LINE)   { multiplier = 1.0f / 3.0f; }   // 3 lines make up a step.
    else if (wheelEvent->deltaMode == DOM_DELTA_PAGE)   { multiplier = 80.0f; }         // A page makes up 80 steps.
    float wheel_x = wheelEvent->deltaX * -multiplier;
    float wheel_y = wheelEvent->deltaY * -multiplier;
    io.AddMouseWheelEvent(wheel_x, wheel_y);
    return EM_TRUE;
}

inline float ApplyDeadzone(float value)
{
    if (fabsf(value) < DEADZONE)
        return 0.0f;
    return fminf(fmaxf(value, -1.0f), 1.0f); // Clamp to [-1, 1]
};

static EM_BOOL ImGui_ImplEmscripten_GamepadConnectedCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData)
{
    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    // Handle buttons
    for (int i = 0; i < gamepadEvent->numButtons; ++i)
    {
        bool pressed = gamepadEvent->digitalButton[i];
        float analogValue = gamepadEvent->analogButton[i];
        switch (i)
        {
        case 0:
            io.AddKeyEvent(ImGuiKey_GamepadFaceDown, pressed);
            break; // A / Cross
        case 1:
            io.AddKeyEvent(ImGuiKey_GamepadFaceRight, pressed);
            break; // B / Circle
        case 2:
            io.AddKeyEvent(ImGuiKey_GamepadFaceLeft, pressed);
            break; // X / Square
        case 3:
            io.AddKeyEvent(ImGuiKey_GamepadFaceUp, pressed);
            break; // Y / Triangle
        case 4:
            io.AddKeyEvent(ImGuiKey_GamepadL1, pressed);
            break; // LB / L1
        case 5:
            io.AddKeyEvent(ImGuiKey_GamepadR1, pressed);
            break; // RB / R1
        case 6:
            io.AddKeyAnalogEvent(ImGuiKey_GamepadL2, pressed, analogValue);
            break; // LT / L2
        case 7:
            io.AddKeyAnalogEvent(ImGuiKey_GamepadR2, pressed, analogValue);
            break; // RT / R2
        case 8:
            io.AddKeyEvent(ImGuiKey_GamepadBack, pressed);
            break; // Select / View / -
        case 9:
            io.AddKeyEvent(ImGuiKey_GamepadStart, pressed);
            break; // Start / Menu / +
        case 10:
            io.AddKeyEvent(ImGuiKey_GamepadL3, pressed);
            break; // Left stick click
        case 11:
            io.AddKeyEvent(ImGuiKey_GamepadR3, pressed);
            break; // Right stick click
        case 12:
            io.AddKeyEvent(ImGuiKey_GamepadDpadUp, pressed);
            break; // D-pad Up
        case 13:
            io.AddKeyEvent(ImGuiKey_GamepadDpadDown, pressed);
            break; // D-pad Down
        case 14:
            io.AddKeyEvent(ImGuiKey_GamepadDpadLeft, pressed);
            break; // D-pad Left
        case 15:
            io.AddKeyEvent(ImGuiKey_GamepadDpadRight, pressed);
            break; // D-pad Right
        }
    }

    if (gamepadEvent->numAxes >= 2)
    {
        float lx = ApplyDeadzone(gamepadEvent->axis[0]);
        float ly = ApplyDeadzone(gamepadEvent->axis[1]);
        io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickLeft, lx < -DEADZONE, lx);
        io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickRight, lx > DEADZONE, lx);
        io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickUp, ly < -DEADZONE, ly);
        io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickDown, ly > DEADZONE, ly);
    }

    if (gamepadEvent->numAxes >= 4)
    {
        float rx = ApplyDeadzone(gamepadEvent->axis[2]);
        float ry = ApplyDeadzone(gamepadEvent->axis[3]);
        io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickLeft, rx < -DEADZONE, rx);
        io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickRight, rx > DEADZONE, rx);
        io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickUp, ry < -DEADZONE, ry);
        io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickDown, ry > DEADZONE, ry);
    }

    return EM_TRUE;
}

static EM_BOOL ImGui_ImplEmscripten_GamepadDisconnectCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData)
{
    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags  &= ~ImGuiBackendFlags_HasGamepad;
    return EM_TRUE;
}

static EM_BOOL ImGui_ImplEmscripten_FocusInCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplEmscripten_Data *bd = ImGui_ImplEmscripten_GetBackendData();
    bd->MouseTracked = true;
    io.AddFocusEvent(true);
    return EM_TRUE;
}

static EM_BOOL ImGui_ImplEmscripten_FocusOutCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplEmscripten_Data *bd = ImGui_ImplEmscripten_GetBackendData();
    bd->MouseTracked = false;
    io.AddFocusEvent(false);
    return EM_TRUE;
}

static void ImGui_ImplEmscripten_UpdateMouseCursor()
{

}

static EM_BOOL ImGui_ImplEmscripten_FocusCallback(int eventType, const EmscriptenFocusEvent *e, void *userData)
{
    return EM_TRUE; 
}

static EM_BOOL ImGui_ImplEmscripten_TouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    ImGuiIO& io = ImGui::GetIO();

    for (int i = 0; i < touchEvent->numTouches; ++i)
    {
        const EmscriptenTouchPoint& touch = touchEvent->touches[i];
        if (!touch.isChanged) continue; 

        float x = touch.clientX;
        float y = touch.clientY;

        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART)
        {
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, true); 
        }
        else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND || eventType == EMSCRIPTEN_EVENT_TOUCHCANCEL)
        {
            io.AddMouseButtonEvent(0, false); 
        }
        else if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE)
        {
            io.AddMousePosEvent(x, y);
        }
    }
    return EM_TRUE; 
}

static void ImGui_ImplEmscripten_UpdateMouseData()
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplEmscripten_Data *bd = ImGui_ImplEmscripten_GetBackendData();
    if (bd->MouseTracked)
    {
        EmscriptenMouseEvent mouseEvent;
        EmscriptenPointerlockChangeEvent plEvent;

        if ((emscripten_get_mouse_status(&mouseEvent) == EMSCRIPTEN_RESULT_SUCCESS) ||
            (emscripten_get_pointerlock_status(&plEvent) == EMSCRIPTEN_RESULT_SUCCESS && plEvent.isActive))
        {
            io.AddMousePosEvent(mouseEvent.targetX, mouseEvent.targetY);
        }
    }
}

static void ImGui_ImplEmscripten_UpdateGamepads()
{
    ImGuiIO &io = ImGui::GetIO();
    EmscriptenGamepadEvent gamepadEvent;
    for (int i = 0; i < emscripten_get_num_gamepads(); ++i)
    {
        if (emscripten_get_gamepad_status(i, &gamepadEvent) == EMSCRIPTEN_RESULT_SUCCESS)
        {
            // Map buttons (adjust to match ImGui's expected gamepad keys)
            io.AddKeyEvent(ImGuiKey_GamepadFaceDown, gamepadEvent.analogButton[0] > 0.5f);
            io.AddKeyEvent(ImGuiKey_GamepadFaceUp, gamepadEvent.analogButton[1] > 0.5f);
            io.AddKeyEvent(ImGuiKey_GamepadFaceLeft, gamepadEvent.analogButton[2] > 0.5f);
            io.AddKeyEvent(ImGuiKey_GamepadFaceUp, gamepadEvent.analogButton[3] > 0.5f);
            io.AddKeyEvent(ImGuiKey_GamepadL1, gamepadEvent.analogButton[4] > 0.5f);
            io.AddKeyEvent(ImGuiKey_GamepadR1, gamepadEvent.analogButton[5] > 0.5f);
            io.AddKeyEvent(ImGuiKey_GamepadStart, gamepadEvent.analogButton[9] > 0.5f);

            // Axes (stick positions)
            io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickLeft, gamepadEvent.axis[0] < -0.3f, gamepadEvent.axis[0]);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickRight, gamepadEvent.axis[0] > 0.3f, gamepadEvent.axis[0]);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickUp, gamepadEvent.axis[1] < -0.3f, gamepadEvent.axis[1]);
            io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickDown, gamepadEvent.axis[1] > 0.3f, gamepadEvent.axis[1]);
        }
    }
}


}

IMGUI_IMPL_API bool ImGui_ImplEmscripten_Init(const char* CanvasSelectorId)
{
    ImGuiIO &io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplEmscripten_Data *bd = IM_NEW(ImGui_ImplEmscripten_Data)();
    io.BackendPlatformUserData = (void *)bd;
    io.BackendPlatformName = "imgui_impl_Emscripten";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGuiPlatformIO& platform = ImGui::GetPlatformIO();

    platform.Platform_GetClipboardTextFn = ImGui_ImplEmscripten_GetClipboardText;
    platform.Platform_SetClipboardTextFn = ImGui_ImplEmscripten_CopyToClipBoard;
    platform.Platform_OpenInShellFn = ImGui_ImplEmscripten_PlatformOpenInShell;

    //TODO :: init mouse course
    //
    bd->Time = 0;
    bd->CanvasSelectorId = CanvasSelectorId;
    EM_ASM({
        var canvas = Module.canvas;
        canvas.addEventListener('click', function() {
            if (document.activeElement !== canvas) {
                canvas.focus();
            }
        });
    });

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, ImGui_ImplEmscripten_KeyDownCallback);
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, ImGui_ImplEmscripten_KeyPressCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, ImGui_ImplEmscripten_KeyUpCallback);
    emscripten_set_mousedown_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, ImGui_ImplEmscripten_MouseCallback);
    emscripten_set_mouseup_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, ImGui_ImplEmscripten_MouseCallback);
    emscripten_set_mousemove_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, ImGui_ImplEmscripten_MouseCallback);
    emscripten_set_touchstart_callback(bd->CanvasSelectorId, 0, EM_TRUE, ImGui_ImplEmscripten_TouchCallback);
    emscripten_set_touchend_callback(bd->CanvasSelectorId, 0, EM_TRUE, ImGui_ImplEmscripten_TouchCallback);
    emscripten_set_touchmove_callback(bd->CanvasSelectorId, 0, EM_TRUE, ImGui_ImplEmscripten_TouchCallback);
    emscripten_set_touchcancel_callback(bd->CanvasSelectorId, 0, EM_TRUE, ImGui_ImplEmscripten_TouchCallback);
    emscripten_set_wheel_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, ImGui_ImplEmscripten_WhellCallback);
    emscripten_set_gamepadconnected_callback(nullptr, EM_TRUE, ImGui_ImplEmscripten_GamepadConnectedCallback);
    emscripten_set_gamepaddisconnected_callback(nullptr, EM_TRUE, ImGui_ImplEmscripten_GamepadDisconnectCallback);
    emscripten_set_focus_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, ImGui_ImplEmscripten_FocusCallback);
    emscripten_set_focusin_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, ImGui_ImplEmscripten_FocusInCallback);
    emscripten_set_focusout_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, ImGui_ImplEmscripten_FocusOutCallback);


    return true;
}

IMGUI_IMPL_API void ImGui_ImplEmscripten_Shutdown()
{
    ImGui_ImplEmscripten_Data *bd = ImGui_ImplEmscripten_GetBackendData();
    IM_ASSERT(bd != NULL && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos);
    io.GetClipboardTextFn = NULL;
    io.SetClipboardTextFn = NULL;
    io.BackendPlatformName = NULL;
    io.BackendPlatformUserData = NULL;
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_wheel_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, nullptr);
    emscripten_set_touchstart_callback(bd->CanvasSelectorId, 0, EM_TRUE, nullptr);
    emscripten_set_touchend_callback(bd->CanvasSelectorId, 0, EM_TRUE, nullptr);
    emscripten_set_touchmove_callback(bd->CanvasSelectorId, 0, EM_TRUE, nullptr);
    emscripten_set_touchcancel_callback(bd->CanvasSelectorId, 0, EM_TRUE, nullptr);
    emscripten_set_mousedown_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, nullptr);
    emscripten_set_mouseup_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, nullptr);
    emscripten_set_mousemove_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, nullptr);
    emscripten_set_gamepadconnected_callback(nullptr, EM_TRUE, nullptr);
    emscripten_set_gamepaddisconnected_callback(nullptr, EM_TRUE, nullptr);
    emscripten_set_focus_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, nullptr);
    emscripten_set_focusin_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, nullptr);
    emscripten_set_focusout_callback(bd->CanvasSelectorId, nullptr, EM_TRUE, nullptr);
    IM_DELETE(bd);
}

IMGUI_IMPL_API bool ImGui_ImplEmscripten_NewFrame()
{
    ImGui_ImplEmscripten_Data *bd = ImGui_ImplEmscripten_GetBackendData();
    ImGuiIO &io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! Missing call to renderer _NewFrame().");

    // Get canvas size
    double width, height;
    emscripten_get_element_css_size("#canvas", &width, &height);

    io.DisplaySize.x = static_cast<float>(width);
    io.DisplaySize.y = static_cast<float>(height);

    // Time management
  
    double current_time = emscripten_get_now() / 1000.0;
    if (current_time <= bd->Time)
    {
        current_time = bd->Time + 0.00001f;
    }
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    // Input updates
    ImGui_ImplEmscripten_UpdateMouseData();
    ImGui_ImplEmscripten_UpdateMouseCursor();
    ImGui_ImplEmscripten_UpdateGamepads();

    return true;
}
#endif