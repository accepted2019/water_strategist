#pragma once
#include <vector>
#include <cstdint>

// Push PCM audio samples for SDL playback
void audio_queue_push(const std::vector<int16_t>& samples);
void audio_init(int sample_rate = 22050);
void audio_shutdown();
bool audio_is_playing();
