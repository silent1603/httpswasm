#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <stdio.h>
#include "ui/imgui.h"
#include "ui/imgui_impl_opengl3.h"
#include "ui/imgui_impl_emscripten.h"
#include <stdio.h>
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
extern "C"
{
    // Function used by c++ to get the size of the html canvas
    EM_JS(int, canvas_get_width, (), {
        return Module.canvas.width;
    });

    // Function used by c++ to get the size of the html canvas
    EM_JS(int, canvas_get_height, (), {
        return Module.canvas.height;
    });

    // Function called by javascript
    EM_JS(void, resizeCanvas, (), {
        js_resizeCanvas();
    });

    void app_init()
    {
#if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100 (WebGL 1.0)
        const char *glsl_version = "#version 100";
#elif defined(IMGUI_IMPL_OPENGL_ES3)
        // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
        const char *glsl_version = "#version 300 es";
#elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        const char *glsl_version = "#version 150"; // Required on Mac
#else
        // GL 3.0 + GLSL 130
        const char *glsl_version = "#version 130";
#endif
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGui_ImplEmscripten_Init();
        ImGui_ImplOpenGL3_Init(glsl_version);
        io.IniFilename = nullptr;
    }

    void app_update()
    {
        bool done = false;
        while(!done)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplEmscripten_NewFrame();
            ImGui::NewFrame();
            // Begin  Rendering
            // End Rendering
            ImGui::Render();
            int display_w, display_h;
            int display_w = canvas_get_width();
            int display_h = canvas_get_height();
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        }
    }

    void app_shutdown()
    {
        	// Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplEmscripten_Shutdown();
        ImGui::DestroyContext();
    }
}
int main(int argc, char **argv)
{
    app_init();
    emscripten_set_main_loop(app_update, 0, true);
    app_shutdown();
    return 0;
}
