#include "theme.h"
#include <imgui.h>
#include <cstdio>

void apply_f1_theme() {
    ImGuiStyle& s = ImGui::GetStyle();
    ImVec4* c = s.Colors;

    // ── Backgrounds (carbon fiber dark) ──
    c[ImGuiCol_WindowBg]        = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    c[ImGuiCol_ChildBg]         = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    c[ImGuiCol_FrameBg]         = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    c[ImGuiCol_FrameBgHovered]  = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_FrameBgActive]   = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);

    // ── Text (F1 telemetry green) ──
    c[ImGuiCol_Text]            = ImVec4(0.00f, 0.90f, 0.13f, 1.00f);
    c[ImGuiCol_TextDisabled]    = ImVec4(0.00f, 0.50f, 0.06f, 1.00f);

    // ── Headers / Titles ──
    c[ImGuiCol_TitleBg]         = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    c[ImGuiCol_TitleBgActive]   = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed]= ImVec4(0.04f, 0.04f, 0.04f, 1.00f);

    // ── Buttons (dark gray, red on hover/active) ──
    c[ImGuiCol_Button]          = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    c[ImGuiCol_ButtonHovered]   = ImVec4(0.80f, 0.05f, 0.05f, 1.00f);
    c[ImGuiCol_ButtonActive]    = ImVec4(0.60f, 0.02f, 0.02f, 1.00f);

    // ── Borders / separators ──
    c[ImGuiCol_Border]          = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_Separator]       = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_SeparatorHovered]= ImVec4(0.80f, 0.05f, 0.05f, 1.00f);
    c[ImGuiCol_SeparatorActive] = ImVec4(0.80f, 0.05f, 0.05f, 1.00f);

    // ── Sliders ──
    c[ImGuiCol_SliderGrab]      = ImVec4(0.00f, 0.90f, 0.13f, 1.00f);
    c[ImGuiCol_SliderGrabActive]= ImVec4(0.00f, 1.00f, 0.20f, 1.00f);

    // ── Check marks ──
    c[ImGuiCol_CheckMark]       = ImVec4(0.00f, 0.90f, 0.13f, 1.00f);

    // ── Tab ──
    c[ImGuiCol_Tab]             = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    c[ImGuiCol_TabHovered]      = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_TabActive]       = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    // ── Scrollbar ──
    c[ImGuiCol_ScrollbarBg]     = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    c[ImGuiCol_ScrollbarGrab]   = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

    // ── Plot colors ──
    c[ImGuiCol_PlotLines]       = ImVec4(0.00f, 0.90f, 0.13f, 1.00f);
    c[ImGuiCol_PlotLinesHovered]= ImVec4(0.80f, 0.05f, 0.05f, 1.00f);
    c[ImGuiCol_PlotHistogram]   = ImVec4(0.00f, 0.70f, 0.10f, 1.00f);

    // ── Resize grip ──
    c[ImGuiCol_ResizeGrip]      = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_ResizeGripHovered]= ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    c[ImGuiCol_ResizeGripActive]= ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

    // ── Styling ──
    s.WindowRounding    = 4.0f;
    s.FrameRounding     = 3.0f;
    s.ScrollbarRounding = 2.0f;
    s.GrabRounding      = 2.0f;
    s.TabRounding       = 3.0f;
    s.WindowBorderSize  = 0.0f;
    s.FrameBorderSize   = 0.0f;
    s.WindowPadding     = ImVec2(12, 12);
    s.FramePadding      = ImVec2(8, 4);
    s.ItemSpacing       = ImVec2(10, 6);
    s.ItemInnerSpacing  = ImVec2(6, 4);
    s.ScrollbarSize     = 10.0f;
}

void load_fonts() {
    ImGuiIO& io = ImGui::GetIO();
    ImFontAtlas* atlas = io.Fonts;

    const char* font_paths[] = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simsun.ttc",
        nullptr
    };

    for (int i = 0; font_paths[i]; ++i) {
        FILE* f = fopen(font_paths[i], "rb");
        if (f) {
            fclose(f);
            // ~2500 common CJK chars + Latin, fits in default atlas
            atlas->AddFontFromFileTTF(font_paths[i], 16.0f, nullptr,
                                      atlas->GetGlyphRangesChineseSimplifiedCommon());
            return;
        }
    }

    atlas->AddFontDefault();
}
