#pragma once
#include <string>

struct TelemetryFrame {
    std::string driver = "Lewis Hamilton";
    int tire_laps = 0;
    double grip_remaining_pct = 100.0;
    double delta_behind = 0.0;
    double lateral_g_force = 0.0;
    double timestamp = 0.0;
    double speed_kmh = 0.0;
    double fuel_kg = 0.0;
    std::string track_section = "straight";
};
