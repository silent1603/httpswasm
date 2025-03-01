// dear imgui: Platform Binding for Emscripten 
// This needs to be used along with a Renderer (Webgpu , webgl)

// Implemented features:
//  [X] Platform: Clipboard support
//  [ ] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using XK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(XK_space).
//  [ ] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

#pragma once

#include "imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

IMGUI_IMPL_API bool     ImGui_ImplEmscripten_Init(const char* canvasSelectorId);
IMGUI_IMPL_API void     ImGui_ImplEmscripten_Shutdown();
IMGUI_IMPL_API bool     ImGui_ImplEmscripten_NewFrame();
#endif
