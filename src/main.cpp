#include <SDL.h>
#include <GL/gl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <future>
#include <chrono>

#include "config.h"
#include "telemetry/frame.h"
#include "telemetry/mock.h"
#include "ui/theme.h"
#include "ui/dashboard.h"
#include "ui/tr_button.h"
#include "strategy/engine.h"
#include "voice/tts.h"
#include "voice/audio.h"
#include "voice/mute_gate.h"
#include "input/gamepad.h"

// ── App state ──
static TelemetryFrame        g_telemetry;
static std::string           g_strategy_text;
static std::string           g_status_text = "Ready";
static std::string           g_api_status = "[checking...]";
static bool                  g_loading = false;
static std::future<StrategyResult> g_strategy_future;
static SapiTts               g_tts;
static GamepadManager        g_gamepad;
static GamepadState          g_gamepad_state;
static bool                  g_muted = false;
static std::string           g_queued_text;
static bool                  g_show_demo_controls = true;

// ── Sidebar edit buffers ──
static char  g_driver_buf[64] = "Lewis Hamilton";
static int   g_tire_laps = 12;
static float g_grip_pct = 68.0f;
static float g_delta = 1.8f;
static float g_lateral_g = 0.9f;

static void update_telemetry() {
    g_telemetry = generate_frame(g_driver_buf, g_tire_laps, g_grip_pct, g_delta, g_lateral_g);
}

static void on_tr_pressed() {
    update_telemetry();

    // Check mute gate
    if (should_mute(g_telemetry.lateral_g_force)) {
        g_muted = true;
        g_status_text = "MUTE GATE ACTIVE - Driver in high-G corner. Audio delayed.";
        // We still fire the strategy request, just queue the audio
    } else {
        g_muted = false;
        g_status_text = "Requesting strategy via Gemini...";
    }

    g_loading = true;
    g_strategy_text.clear();
    g_queued_text.clear();

    // Fire async Gemini request
    g_strategy_future = request_strategy_async(g_telemetry);
}

int main(int argc, char* argv[]) {
    // ── Init SDL ──
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    // OpenGL 3.0 + GLSL 130
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow(
        config::WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config::WINDOW_WIDTH, config::WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_ctx);
    SDL_GL_SetSwapInterval(1); // VSync

    // ── Init ImGui ──
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // Don't save imgui.ini

    apply_f1_theme();
    load_fonts();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_ctx);
    ImGui_ImplOpenGL3_Init("#version 130");

    // ── Init audio ──
    audio_init(config::AUDIO_SAMPLE_RATE);
    g_tts.init();

    // ── Init telemetry ──
    update_telemetry();

    // Check API key availability
    {
        const char* env = std::getenv("GEMINI_API_KEY");
        if (!env || !env[0]) env = std::getenv("GOOGLE_API_KEY");
        if (env && env[0]) {
            g_api_status = "API: OK";
        } else {
            // check config.ini
            FILE* fp = fopen("config.ini", "r");
            bool found = false;
            if (fp) {
                char line[512];
                while (fgets(line, sizeof(line), fp)) {
                    if (strncmp(line, "api_key", 7) == 0) {
                        const char* eq = strchr(line, '=');
                        if (eq && eq[1] && eq[1] != '\n' && eq[1] != '\r') found = true;
                    }
                }
                fclose(fp);
            }
            g_api_status = found ? "API: OK (config.ini)" : "API: DEMO MODE";
        }
    }

    // ── Main loop ──
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_CONTROLLERDEVICEADDED:
                g_gamepad.open(event.cdevice.which);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                g_gamepad.close();
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_SPACE && !g_loading) {
                    on_tr_pressed();
                }
                if (event.key.keysym.sym == SDLK_F11) {
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                        SDL_SetWindowFullscreen(window, 0);
                    else
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
                if (event.key.keysym.sym == SDLK_h) {
                    g_show_demo_controls = !g_show_demo_controls;
                }
                break;
            }
        }

        // Poll gamepad
        g_gamepad.poll(g_gamepad_state);
        if (g_gamepad.is_connected() && g_gamepad_state.tr_button && !g_loading) {
            on_tr_pressed();
        }

        // Check async strategy result
        if (g_loading && g_strategy_future.valid()) {
            auto status = g_strategy_future.wait_for(std::chrono::milliseconds(0));
            if (status == std::future_status::ready) {
                StrategyResult result = g_strategy_future.get();
                g_loading = false;
                g_strategy_text = result.text;

                if (!result.success) {
                    g_status_text = result.error_message;
                }

                // Handle mute gate / audio
                if (g_muted) {
                    g_queued_text = result.text;
                    g_status_text = "MUTE GATE ACTIVE - Strategy ready, waiting for G to drop.";
                } else {
                    g_status_text = "Strategy delivered. Speaking...";
                    // Synthesize and play on main thread (TTS blocks briefly)
                    std::vector<int16_t> pcm = g_tts.synthesize(result.text);
                    if (!pcm.empty()) {
                        audio_queue_push(pcm);
                    }
                }
            }
        }

        // Check mute gate release for queued audio
        if (g_muted && !g_queued_text.empty() && can_release_queued(g_telemetry.lateral_g_force)) {
            g_muted = false;
            g_status_text = "G-force cleared. Playing queued strategy...";
            std::vector<int16_t> pcm = g_tts.synthesize(g_queued_text);
            if (!pcm.empty()) {
                audio_queue_push(pcm);
            }
            g_queued_text.clear();
        }

        // ── ImGui frame ──
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Full-window UI
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)io.DisplaySize.x, (float)io.DisplaySize.y));
        ImGui::Begin("##MainWindow", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ── Title bar ──
        ImGui::TextColored(ImVec4(0.80f, 0.05f, 0.05f, 1.0f), "WATERSTRATEGIST");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.35f, 0.35f, 0.35f, 1.0f), "- AI Pit-Wall Engineer");
        ImGui::SameLine((float)io.DisplaySize.x - 180);
        ImGui::TextColored(g_api_status.find("DEMO") != std::string::npos
                           ? ImVec4(0.80f, 0.80f, 0.05f, 1.0f)
                           : ImVec4(0.00f, 0.70f, 0.10f, 1.0f),
                           "%s", g_api_status.c_str());

        ImGui::Separator();

        // ── Sidebar controls (collapsible) ──
        if (g_show_demo_controls) {
            ImGui::BeginChild("##Sidebar", ImVec2(220, 0), true);
            ImGui::Text("TELEMETRY CONTROLS");
            ImGui::Separator();
            ImGui::InputText("Driver", g_driver_buf, sizeof(g_driver_buf));
            ImGui::SliderInt("Tyre Laps", &g_tire_laps, 0, 30);
            ImGui::SliderFloat("Grip %%", &g_grip_pct, 0.0f, 100.0f, "%.0f");
            ImGui::SliderFloat("Delta (s)", &g_delta, 0.0f, 10.0f, "%.1f");
            ImGui::SliderFloat("Lat G", &g_lateral_g, 0.0f, 3.0f, "%.2f");
            ImGui::Separator();
            if (ImGui::Button("Apply", ImVec2(-1, 0))) {
                update_telemetry();
            }
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.35f, 0.35f, 0.35f, 1.0f), "Mute threshold: %.1f G", config::MUTE_G_THRESHOLD);
            ImGui::TextColored(ImVec4(0.35f, 0.35f, 0.35f, 1.0f), "Space = TR shortcut");
            ImGui::TextColored(ImVec4(0.35f, 0.35f, 0.35f, 1.0f), "F11 = Fullscreen");
            ImGui::TextColored(ImVec4(0.35f, 0.35f, 0.35f, 1.0f), "H = Hide controls");
            // gamepad status
            if (g_gamepad.is_connected()) {
                ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.13f, 1.0f), "CTRL: %s", g_gamepad.device_name().c_str());
            }
            ImGui::EndChild();
            ImGui::SameLine();
        }

        // ── Main content area ──
        ImGui::BeginChild("##Content", ImVec2(0, 0), true);
        render_dashboard(g_telemetry);

        ImGui::Spacing();
        ImGui::Separator();

        // Strategy output display
        if (!g_strategy_text.empty()) {
            ImGui::PushFont(io.Fonts->Fonts[0]);
            ImGui::TextColored(ImVec4(0.80f, 0.05f, 0.05f, 1.0f), "STRATEGIST: ");
            ImGui::SameLine();
            ImGui::TextWrapped("%s", g_strategy_text.c_str());
            ImGui::PopFont();
        }

        ImGui::Spacing();

        // Status bar
        ImGui::TextColored(
            g_muted ? ImVec4(0.80f, 0.05f, 0.05f, 1.0f) : ImVec4(0.35f, 0.35f, 0.35f, 1.0f),
            "%s", g_status_text.c_str());

        // Loading indicator
        if (g_loading) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.00f, 0.90f, 0.13f, 1.0f), " [processing...]");
        }

        // Audio playing indicator
        if (audio_is_playing()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.00f, 0.70f, 0.10f, 1.0f), " [speaking...]");
        }

        // ── TR Button ──
        bool tr_pressed = render_tr_button(g_loading);
        if (tr_pressed) {
            on_tr_pressed();
        }

        ImGui::EndChild();
        ImGui::End();

        // ── Render ──
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // ── Cleanup ──
    if (g_loading && g_strategy_future.valid()) {
        g_strategy_future.wait();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    audio_shutdown();
    g_gamepad.close();

    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
