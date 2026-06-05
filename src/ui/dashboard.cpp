#include "dashboard.h"
#include <imgui.h>
#include <string>

static void big_metric(const char* label, const std::string& value, const char* unit = "") {
    ImGui::BeginGroup();
    ImGui::TextColored(ImVec4(0.35f, 0.35f, 0.35f, 1.0f), "%s", label);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
    ImGui::Text("%s", value.c_str());
    if (unit && unit[0]) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.00f, 0.50f, 0.06f, 1.0f), "%s", unit);
    }
    ImGui::PopStyleVar();
    ImGui::EndGroup();
}

void render_dashboard(const TelemetryFrame& f) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

    float win_w = ImGui::GetContentRegionAvail().x;
    float col_w = win_w / 6.0f;

    // Row 1 — 6 metrics
    ImGui::Columns(6, "##metrics1", false);
    ImGui::SetColumnWidth(0, col_w); ImGui::SetColumnWidth(1, col_w);
    ImGui::SetColumnWidth(2, col_w); ImGui::SetColumnWidth(3, col_w);
    ImGui::SetColumnWidth(4, col_w); ImGui::SetColumnWidth(5, col_w);

    big_metric("DRIVER", f.driver);
    ImGui::NextColumn();
    big_metric("TYRE LAPS", std::to_string(f.tire_laps));
    ImGui::NextColumn();
    big_metric("GRIP", std::to_string((int)f.grip_remaining_pct), "%");
    ImGui::NextColumn();
    big_metric("DELTA", std::to_string(f.delta_behind).substr(0, 4), "s");
    ImGui::NextColumn();
    big_metric("LAT G", std::to_string(f.lateral_g_force).substr(0, 3), "G");
    ImGui::NextColumn();
    big_metric("SPEED", std::to_string((int)f.speed_kmh), "km/h");
    ImGui::NextColumn();
    ImGui::Columns(1);

    ImGui::Spacing();

    // Row 2 — 3 metrics
    ImGui::Columns(3, "##metrics2", false);
    col_w = win_w / 3.0f;
    ImGui::SetColumnWidth(0, col_w); ImGui::SetColumnWidth(1, col_w);
    ImGui::SetColumnWidth(2, col_w);

    big_metric("FUEL", std::to_string((int)f.fuel_kg), "kg");
    ImGui::NextColumn();
    big_metric("SECTOR", f.track_section == "corner" ? "CORNER" : "STRAIGHT");
    ImGui::NextColumn();
    {
        bool muted = (f.lateral_g_force > 1.5);
        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.35f, 0.35f, 0.35f, 1.0f), "MUTE GATE");
        if (muted) {
            ImGui::TextColored(ImVec4(0.80f, 0.05f, 0.05f, 1.0f), "ACTIVE");
        } else {
            ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.13f, 1.0f), "OK");
        }
        ImGui::EndGroup();
    }
    ImGui::NextColumn();
    ImGui::Columns(1);

    ImGui::PopFont();
}
