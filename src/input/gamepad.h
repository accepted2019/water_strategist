#pragma once
#include <string>
#include <SDL.h>

// Reserved interface for external device (steering wheel / gamepad) recognition.
// Hooks are in place in the main SDL event loop.
// Full wheel button-box parsing (CAN bus, SimHub Shared Memory) scoped for v1.1.

struct GamepadState {
    bool tr_button = false;       // Mapped to SDL_CONTROLLER_BUTTON_A by default
    bool dpad_up = false;
    bool dpad_down = false;
    bool dpad_left = false;
    bool dpad_right = false;
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;
};

class GamepadManager {
public:
    void open(int joystick_index);
    void close();
    bool is_connected() const;
    std::string device_name() const;
    void poll(GamepadState& state);

private:
    SDL_GameController* m_controller = nullptr;
    std::string m_name;
};
