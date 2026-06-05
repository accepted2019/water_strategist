#include "tr_button.h"
#include "../config.h"
#include <imgui.h>
#include <cmath>
#include <string>

bool render_tr_button(bool is_loading) {
    float win_w = ImGui::GetWindowWidth();
    float win_h = ImGui::GetWindowHeight();
    float btn_sz = config::TR_BUTTON_SIZE;

    // Position: bottom-left corner
    ImGui::SetCursorPos(ImVec2(16, win_h - btn_sz - 16));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, btn_sz * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    // TR button colors
    ImVec4 tr_bg       = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    ImVec4 tr_hover    = ImVec4(0.85f, 0.05f, 0.05f, 1.0f);
    ImVec4 tr_active   = ImVec4(0.60f, 0.02f, 0.02f, 1.0f);
    ImVec4 tr_text     = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, tr_bg);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, tr_hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, tr_active);
    ImGui::PushStyleColor(ImGuiCol_Text, tr_text);

    bool pressed = false;

    if (is_loading) {
        // Draw animated spinner dots
        double t = ImGui::GetTime();
        int dots = ((int)(t * 4.0)) % 3 + 1;
        std::string label(dots, '.');
        ImGui::Button(label.c_str(), ImVec2(btn_sz, btn_sz));
        // Also draw a rotating ring
        ImVec2 center = ImGui::GetItemRectMin();
        center.x += btn_sz * 0.5f;
        center.y += btn_sz * 0.5f;
        float r = btn_sz * 0.38f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddCircle(center, r, IM_COL32(0, 230, 20, 180), 0, 3.0f);
        float a0 = (float)(t * 3.0f);
        dl->PathArcTo(center, r, a0, a0 + 2.5f, 12);
        dl->PathStroke(IM_COL32(255, 255, 255, 220), false, 3.0f);
    } else {
        if (ImGui::Button("TR", ImVec2(btn_sz, btn_sz))) {
            pressed = true;
        }
    }

    if (ImGui::IsItemHovered() && !is_loading) {
        ImGui::SetTooltip("Team Radio — Request strategy via Gemini AI");
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    return pressed;
}
