#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#ifdef DEBUG
#include <emscripten/console.h> 
#endif
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <stdio.h>
#define IMGUI_IMPL_OPENGL_ES3
#include "externs/imgui/imgui.h"
#include "externs/imgui/imgui_impl_opengl3.h"
#include "externs/imgui/imgui_impl_emscripten.h"

#define STB_IMAGE_IMPLEMENTATION
#include "externs/stb/stb_image.h"

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
EmscriptenWebGLContextAttributes webgl_attributes;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context;
static bool show_demo_window = true;
static bool show_another_window = false;
static bool running = false;

const char *get_glsl_version()
{
#if defined(IMGUI_IMPL_OPENGL_ES2)
    return "#version 100"; // WebGL 1.0
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    return "#version 300 es"; // WebGL 2.0
#else
    return "#version 130"; // GL 3.0 + GLSL 130
#endif
}

bool initialize_webgl()
{
    emscripten_webgl_init_context_attributes(&webgl_attributes);
    webgl_attributes.alpha = 0;
    webgl_attributes.depth = 0;
    webgl_attributes.stencil = 0;
    webgl_attributes.antialias = 0;
    webgl_attributes.preserveDrawingBuffer = 0;
    webgl_attributes.failIfMajorPerformanceCaveat = 0;
    webgl_attributes.enableExtensionsByDefault = 1;
    webgl_attributes.premultipliedAlpha = 0;
    webgl_attributes.majorVersion = 2;
    webgl_attributes.minorVersion = 0;

    webgl_context = emscripten_webgl_create_context("#canvas", &webgl_attributes);
    if (webgl_context <= 0)
    {
        printf("Failed to create WebGL context\n");
        return false;
    }
    emscripten_webgl_make_context_current(webgl_context);
    return true;
}

bool app_init()
{
    if (!initialize_webgl())
        return false;

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Keyboard support
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Gamepad support
    io.IniFilename = "/data/imgui.ini";
    ImGui::StyleColorsDark();
    ImGui_ImplEmscripten_Init("#canvas");
    ImGui_ImplOpenGL3_Init(get_glsl_version());
    running = true;
    return true;
}

void render_frame()
{
    int display_w, display_h;
    emscripten_get_canvas_element_size("#canvas", &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    emscripten_webgl_commit_frame();
}

void app_update()
{
    if (!running)
    {
        emscripten_cancel_main_loop();
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplEmscripten_NewFrame();
    ImGui::NewFrame();
    // begin draw ui
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;
        ImGui::Begin("Hello, world!");                     // Create a window called "Hello, world!" and append into it.
        ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color
        if (ImGui::Button("Button"))                             // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
    // end draw ui
    ImGui::Render();
    render_frame();
}

void app_shutdown()
{
    emscripten_webgl_destroy_context(webgl_context);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplEmscripten_Shutdown();
    ImGui::DestroyContext();
}

int main(int argc, char **argv)
{
    if (!app_init())
        return 1;
    emscripten_set_main_loop(app_update, 0, true);
    app_shutdown();
    return 0;
}