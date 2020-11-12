// ImGui ZPL binding with OpenGL3

// Implemented features:
//  [X] User texture binding. Cast 'GLuint' OpenGL texture identifier as void*/ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

struct zpl_platform;

IMGUI_API bool        ImGui_ImplZPLGL3_Init(zpl_platform* platform, const char* glsl_version = NULL);
IMGUI_API void        ImGui_ImplZPLGL3_Shutdown();
IMGUI_API void        ImGui_ImplZPLGL3_NewFrame(zpl_platform* platform);
IMGUI_API void        ImGui_ImplZPLGL3_RenderDrawData(ImDrawData* draw_data);
IMGUI_API bool        ImGui_ImplZPLGL3_ProcessUpdate(zpl_platform* platform);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void        ImGui_ImplZPLGL3_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_ImplZPLGL3_CreateDeviceObjects();