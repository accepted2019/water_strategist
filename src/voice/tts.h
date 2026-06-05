#pragma once
#include <string>
#include <vector>
#include <cstdint>

// Windows SAPI text-to-speech wrapper.
// Synthesizes text to in-memory PCM (mono, configurable sample rate).
class SapiTts {
public:
    SapiTts();
    ~SapiTts();
    SapiTts(const SapiTts&) = delete;
    SapiTts& operator=(const SapiTts&) = delete;

    bool init();
    std::vector<int16_t> synthesize(const std::string& text);
    void setRate(int rate);    // -10 to 10, 0 = normal
    void setVolume(int vol);   // 0-100

private:
    void* m_voice;  // ISpVoice*
    int m_rate;
    int m_volume;
    bool m_initialized;
};
