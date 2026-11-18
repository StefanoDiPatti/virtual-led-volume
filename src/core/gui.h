/*
Gui class
- creation and destruction of the Dear ImGui context and of its GLFW / OpenGL 3 backends
- it opens the Gui frame (BeginFrame) and draws the Gui on top of the scene (Render)
*/

#pragma once


#include <glad/glad.h>
// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// Dear ImGui and its GLFW / OpenGL 3 backends
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// named constants of the application
#include <core/config.h>

#include <iostream>

/////////////////// Gui class ///////////////////////
class Gui
{
public:

    // constructor: ImGui context, dark style and backends for GLFW and OpenGL (GLSL 4.10)
    Gui(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        // we do not save the window layout to imgui.ini
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::StyleColorsDark();
        // true: ImGui installs its callbacks and chains the ones already set by Input
        bool res = ImGui_ImplGlfw_InitForOpenGL(window, true);

        if(!res)
            std::cerr << "Could not init ImGui for OpenGL" << std::endl;

        ImGui_ImplOpenGL3_Init(GLSL_VERSION);
    }

    // the instance owns OpenGL resources (font texture, shaders, buffers of ImGui): copies are not allowed
    Gui(const Gui& copy) = delete;
    Gui& operator=(const Gui&) = delete;

    // destructor: we shut down the backends and destroy the context, in reverse order of creation
    ~Gui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // a new Gui frame: the widgets of this frame can be declared after this call
    void BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // end of the Gui frame: we build the draw lists and render them with the OpenGL 3 backend
    void Render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

};