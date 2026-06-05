#include "audio.h"
#include <SDL.h>
#include <vector>
#include <atomic>
#include <mutex>

static std::vector<int16_t> g_audio_buffer;
static std::mutex g_mutex;
static std::atomic<size_t> g_write_pos{0};
static SDL_AudioDeviceID g_device = 0;
static int g_sample_rate = 22050;

static void SDLCALL audio_callback(void* /*userdata*/, Uint8* stream, int len) {
    int samples_needed = len / (int)sizeof(int16_t);
    std::lock_guard<std::mutex> lock(g_mutex);

    int16_t* out = (int16_t*)stream;
    size_t avail = g_audio_buffer.size() - g_write_pos;
    int to_copy = samples_needed < (int)avail ? samples_needed : (int)avail;

    if (to_copy > 0) {
        SDL_memcpy(out, g_audio_buffer.data() + g_write_pos, to_copy * sizeof(int16_t));
        g_write_pos += to_copy;
    }
    // Fill remainder with silence
    if (to_copy < samples_needed) {
        SDL_memset(out + to_copy, 0, (samples_needed - to_copy) * sizeof(int16_t));
    }
}

void audio_init(int sample_rate) {
    g_sample_rate = sample_rate;
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;

    g_device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (g_device) SDL_PauseAudioDevice(g_device, 0);
}

void audio_shutdown() {
    if (g_device) {
        SDL_CloseAudioDevice(g_device);
        g_device = 0;
    }
}

void audio_queue_push(const std::vector<int16_t>& samples) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_audio_buffer.clear();
    g_write_pos = 0;
    g_audio_buffer = samples;
}

bool audio_is_playing() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_write_pos < g_audio_buffer.size();
}
