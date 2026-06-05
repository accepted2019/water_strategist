#pragma once

// Returns true if "Apply" was clicked (caller should update telemetry).
bool render_settings_page(
    char* driver_buf, int driver_buf_size,
    int* tire_laps,
    float* grip_pct,
    float* delta,
    float* lateral_g,
    float mute_threshold,
    const char* gamepad_name,
    bool gamepad_connected);
