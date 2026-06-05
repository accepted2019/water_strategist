// Voice output — uses SDL audio for notification sound + on-screen text display.
// Full Windows SAPI TTS reserved for v1.1 when proper sapi.h headers are available.
// The public API (tts.h) remains unchanged so callers need no modifications.

#include "tts.h"
#include <windows.h>
#include <cstdio>
#include <vector>
#include <string>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SapiTts::SapiTts() : m_voice(nullptr), m_rate(0), m_volume(100), m_initialized(false) {}
SapiTts::~SapiTts() {}

bool SapiTts::init() {
    m_initialized = true;
    return true;
}

void SapiTts::setRate(int rate) { m_rate = rate; }
void SapiTts::setVolume(int vol) { m_volume = vol; }

// Generate a short radio "beep" tone as PCM samples.
// This serves as an audible cue that a strategy message is ready.
static std::vector<int16_t> generate_beep(int volume, int sample_rate = 22050,
                                           double freq = 800.0,
                                           double duration_ms = 200.0) {
    int total_samples = (int)(sample_rate * duration_ms / 1000.0);
    std::vector<int16_t> samples(total_samples);

    // Simple sine wave with exponential decay envelope
    for (int i = 0; i < total_samples; ++i) {
        double t = (double)i / (double)sample_rate;
        double envelope = exp(-t * 8.0);  // fast decay
        double amplitude = envelope * (volume / 100.0) * 16000.0;
        samples[i] = (int16_t)(amplitude * sin(2.0 * M_PI * freq * t));
    }
    return samples;
}

std::vector<int16_t> SapiTts::synthesize(const std::string& text) {
    // For v1.0: return a notification beep. Text is displayed on screen in the UI.
    // v1.1: will use Windows SAPI ISpVoice to synthesize actual speech.
    if (text.empty()) return {};

    m_initialized = true;

    // Print to console for debugging
    std::printf("[TTS] Strategy: %s\n", text.c_str());

    // Return a short beep as audio notification
    return generate_beep(m_volume);
}
