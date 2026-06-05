#include "settings.h"
#include "../i18n.h"
#include <imgui.h>

bool render_settings_page(
    char* driver_buf, int driver_buf_size,
    int* tire_laps,
    float* grip_pct,
    float* delta,
    float* lateral_g,
    float mute_threshold,
    const char* gamepad_name,
    bool gamepad_connected)
{
    bool apply_clicked = false;

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

    // Title
    ImGui::TextColored(ImVec4(0.80f, 0.05f, 0.05f, 1.0f), "%s", L("SETTINGS", "设置"));
    ImGui::Separator();
    ImGui::Spacing();

    // ── Section 1: Language ──
    ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.13f, 1.0f), "%s", L("LANGUAGE", "语言设置"));
    ImGui::Separator();
    ImGui::Spacing();

    float btn_w = 120.0f;
    bool is_zh = (g_lang == Lang::ZH);

    // English button
    if (!is_zh) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.60f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.60f, 0.10f, 1.0f));
    }
    if (ImGui::Button("English", ImVec2(btn_w, 0))) {
        g_lang = Lang::EN;
    }
    if (!is_zh) {
        ImGui::PopStyleColor(2);
    }

    ImGui::SameLine();

    // Chinese button
    if (is_zh) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.60f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.60f, 0.10f, 1.0f));
    }
    if (ImGui::Button(L("中文 (Chinese)", "中文"), ImVec2(btn_w, 0))) {
        g_lang = Lang::ZH;
    }
    if (is_zh) {
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // ── Section 2: Data Debug ──
    ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.13f, 1.0f), "%s", L("DATA DEBUG", "数据调试"));
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::InputText(L("Driver", "车手"), driver_buf, driver_buf_size);
    ImGui::SliderInt(L("Tyre Laps", "轮胎圈数"), tire_laps, 0, 30);
    ImGui::SliderFloat(L("Grip %", "抓地力 %"), grip_pct, 0.0f, 100.0f, "%.0f");
    ImGui::SliderFloat(L("Delta (s)", "差距 (秒)"), delta, 0.0f, 10.0f, "%.1f");
    ImGui::SliderFloat(L("Lat G", "横向G力"), lateral_g, 0.0f, 3.0f, "%.2f");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.35f, 0.35f, 0.35f, 1.0f),
        "%s: %.1f G", L("Mute threshold", "静音阈值"), mute_threshold);

    if (gamepad_connected) {
        ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.13f, 1.0f),
            "CTRL: %s", gamepad_name);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Apply button
    if (ImGui::Button(L("Apply", "应用"), ImVec2(160, 0))) {
        apply_clicked = true;
    }

    ImGui::PopFont();
    return apply_clicked;
}
