/*
Gui class
- creation and destruction of the Dear ImGui context and of its GLFW / OpenGL 3 backends
- it opens the Gui frame (BeginFrame) and draws the Gui on top of the scene (Render)
- parameter panel (DrawPanel): the widgets read the state with the getters and change it with the setters
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

#include <scene/cine_camera.h>

#include <core/input.h>

#include <render/line_renderer.h>
// lighting parameters and shadow technique
#include <render/lighting.h>
// LED wall (content shown on the wall)
#include <scene/led_wall.h>

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

    // in camera mode ImGui ignores the mouse (the cursor is captured and the panel cannot be clicked by mistake)
    void SetMouseEnabled(bool enabled)
    {
        ImGuiIO& io = ImGui::GetIO();

        // if enabled, clear the bit. Otherwise set it.
        if(enabled)
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        else
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;                            
    }

    // parameter panel, top left: cine camera settings, view selection, debug lines, lighting keys and frame rate
    void DrawPanel (CineCamera& camera, Input& input, LineToggles& toggles, Lighting& lighting, Lighting& environmentLighting, LedWall& wall)
    {
        // initial position only: then the window can be dragged
        ImGui::SetNextWindowPos(ImVec2(GUI_POSITION_X, GUI_POSITION_Y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Virtual Production", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if(ImGui::CollapsingHeader("Cine Camera Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // the widgets work on local copies: the setters apply the limits of the camera
            float f = camera.GetFocalLength();
            if(ImGui::SliderFloat("Focal Length", &f, MIN_FOCAL_LENGTH, MAX_FOCAL_LENGTH, "%.1f mm"))
                camera.SetFocalLength(f);

            int item = (int) camera.GetSensorPreset();

            if(ImGui::Combo("Sensor", &item, "Full Frame 36x24\0Super 35 24.89x18.66\0\0"))
                camera.SetSensorPreset((SENSOR_PRESET)item);

            // read-only values
            ImGui::Text("Vertical FOV %.1f deg", glm::degrees(camera.GetVerticalFov()));
            ImGui::Text("Position X %.1f", camera.GetPosition().x);
            ImGui::Text("Position Y %.1f", camera.GetPosition().y);
            ImGui::Text("Position Z %.1f", camera.GetPosition().z);
            ImGui::Text("Pan %.1f", camera.GetPan());
            ImGui::Text("Tilt %.1f", camera.GetTilt());
        }

        // current state of the input, shown by the radio buttons
        CONTROL_TARGET target = input.GetControlTarget();
        VIEW_MODE mode = input.GetViewMode();

        if(ImGui::CollapsingHeader("View", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // same effect as the keys 1..5 and Tab
            if(ImGui::RadioButton("Split", mode == VIEW_MODE::SPLIT))
                input.SetViewMode(VIEW_MODE::SPLIT);
            ImGui::SameLine();
            if(ImGui::RadioButton("Cine-Camera", mode == VIEW_MODE::CINE_CAMERA))
                input.SetViewMode(VIEW_MODE::CINE_CAMERA);
            ImGui::SameLine();
            if(ImGui::RadioButton("Stage", mode == VIEW_MODE::STAGE))
                input.SetViewMode(VIEW_MODE::STAGE);
            ImGui::SameLine();
            if(ImGui::RadioButton("Ground Truth", mode == VIEW_MODE::GROUND_TRUTH))
                input.SetViewMode(VIEW_MODE::GROUND_TRUTH);
            ImGui::SameLine();
            if(ImGui::RadioButton("Difference", mode == VIEW_MODE::DIFFERENCE))
                input.SetViewMode(VIEW_MODE::DIFFERENCE);

            ImGui::Text("Controls:");
            ImGui::SameLine();

            if(ImGui::RadioButton("Observer", target == CONTROL_TARGET::OBSERVER))
                input.SetControlTarget(CONTROL_TARGET::OBSERVER);
            ImGui::SameLine();
            if(ImGui::RadioButton("Camera", target == CONTROL_TARGET::CAMERA))
                input.SetControlTarget(CONTROL_TARGET::CAMERA);

            // content of the wall: inner frustum (cine camera) or outer frustum (fixed eye)
            int wallItem = (int)wall.GetContent();
            if(ImGui::Combo("Wall Content", &wallItem, "Inner\0Outer\0Inner + Outer\0\0"))
                wall.SetContent((WALL_CONTENT)wallItem);
        }

        // set light, environment light and shadows
        if(ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat3("Light Position", &lighting.lightPosition.x, LIGHT_POSITION_MIN, LIGHT_POSITION_MAX, "%.1f m");
            ImGui::SliderFloat3("Environment Light", &environmentLighting.lightPosition.x, LIGHT_POSITION_MIN, LIGHT_POSITION_MAX, "%.1f m");

            int shadowItem = (int) lighting.shadowMode;
            if(ImGui::Combo("Shadows", &shadowItem, "Off\0No Bias\0Bias\0PCF 3x3\0PCF Hardware\0\0"))
                lighting.shadowMode = (SHADOW_MODE)shadowItem;
        }

        // groups of debug lines of the observer views: the checkboxes write directly in the toggles
        if(ImGui::CollapsingHeader("Debug Lines", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("World axes", &toggles.worldAxes);
            ImGui::Checkbox("Wall frame", &toggles.wallFrame);
            ImGui::Checkbox("Cine frustum", &toggles.cineFrustum);
            ImGui::Checkbox("Camera gizmo", &toggles.cameraGizmo);
            ImGui::Checkbox("Off-axis lines", &toggles.offAxisLines);
        }

        // table of the commands (closed at start)
        if(ImGui::CollapsingHeader("Keys"))
        {
            static const char* KEYS[][2] = { {"G", "UI mode on / off"},
                                             {"Tab", "observer / cine camera"},
                                             {"W A S D E Q", "move (dolly, truck, pedestal)"},
                                             {"Mouse", "look (pan / tilt)"},
                                             {"Wheel", "zoom (cine camera)"},
                                             {"Right click", "look at the subject"},
                                             {"1 - 5", "views"} };

            // EndTable only if the table has been opened
            if(ImGui::BeginTable("keys", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                for(size_t i = 0; i < sizeof(KEYS) / sizeof(KEYS[0]); i++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(KEYS[i][0]);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(KEYS[i][1]);
                }

                ImGui::EndTable();
            }
        }

        // frame rate averaged by ImGui, and frame time (ms) = 1000 / FPS
        ImGui::Separator();
        ImGui::Text("%.0f FPS - %.2f ms/frame", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

        ImGui::End();
    }

};