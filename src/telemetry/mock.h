#pragma once
#include <vector>
#include "frame.h"

TelemetryFrame generate_frame(
    const std::string& driver = "Lewis Hamilton",
    int tire_laps = -1,
    double grip_pct = -1.0,
    double delta = -1.0,
    double lateral_g = -1.0);

std::vector<TelemetryFrame> generate_stream(int frames = 120, double interval_sec = 1.0 / 60.0);
