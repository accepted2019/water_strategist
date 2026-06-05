#include "gamepad.h"
#include <stdio.h>

void GamepadManager::open(int joystick_index) {
    close();
    m_controller = SDL_GameControllerOpen(joystick_index);
    if (m_controller) {
        m_name = SDL_GameControllerName(m_controller) ? SDL_GameControllerName(m_controller) : "Unknown";
        printf("[INPUT] Controller connected: %s\n", m_name.c_str());
    }
}

void GamepadManager::close() {
    if (m_controller) {
        printf("[INPUT] Controller disconnected: %s\n", m_name.c_str());
        SDL_GameControllerClose(m_controller);
        m_controller = nullptr;
        m_name.clear();
    }
}

bool GamepadManager::is_connected() const {
    return m_controller != nullptr;
}

std::string GamepadManager::device_name() const {
    return m_name;
}

void GamepadManager::poll(GamepadState& state) {
    state = GamepadState{};
    if (!m_controller) return;
    state.tr_button = SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_A) != 0;
    state.dpad_up   = SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
    state.dpad_down = SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
    state.dpad_left = SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
    state.dpad_right= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
    state.left_trigger  = (float)SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
    state.right_trigger = (float)SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
}
