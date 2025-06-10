#include "imgui.h"
#include "Style.h"

void menuBar() {
    if (ImGui::BeginMainMenuBar())               // ← starts the main menu bar
    {
        if (ImGui::BeginMenu("File"))            // ← a “File” dropdown
        {
            if (ImGui::MenuItem("New", "Ctrl+N")) { /* New action */ }
            if (ImGui::MenuItem("Open...", "Ctrl+O")) { /* Open action */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { /* Exit action */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))            // ← an “Edit” dropdown
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* Undo */ }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) { /* Redo */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Style"))            
        {
            if (ImGui::MenuItem("Volcano")) { setStyle(StyleType::Volcano); }
            if (ImGui::MenuItem("Ocean")) { setStyle(StyleType::Ocean); }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();                 // ← ends the main menu bar
    }
}