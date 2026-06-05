#pragma once

namespace config {

    // ── Window ──
    constexpr const char* WINDOW_TITLE = "WaterStrategist";
    constexpr int WINDOW_WIDTH = 1100;
    constexpr int WINDOW_HEIGHT = 700;

    // ── Gemini API ──
    constexpr const char* GEMINI_MODEL = "gemini-1.5-flash";
    constexpr const char* GEMINI_ENDPOINT =
        "https://generativelanguage.googleapis.com/v1beta/models/";
    constexpr int GEMINI_TIMEOUT_SEC = 5;
    constexpr int GEMINI_MAX_TOKENS = 50;
    constexpr double GEMINI_TEMPERATURE = 0.3;

    // ── Mute Gate ──
    constexpr double MUTE_G_THRESHOLD = 1.5;

    // ── Demo fallbacks ──
    constexpr const char* DEMO_MSG_1 = "Hamilton, Box this lap. Hard tyre. Push now.";
    constexpr const char* DEMO_MSG_2 = "轮胎衰退临界，立即进站。";
    constexpr const char* DEMO_MSG_3 = "Box, Box. Undercut window open.";

    // ── Audio ──
    constexpr int AUDIO_SAMPLE_RATE = 22050;

    // ── UI ──
    constexpr float TR_BUTTON_SIZE = 40.0f;
}
