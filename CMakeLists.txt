cmake_minimum_required(VERSION 3.16)
project(WaterStrategist VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Pre-built SDL2 (MinGW) ────────────────────────────────────────
set(SDL2_ROOT "${CMAKE_SOURCE_DIR}/sdl2_mingw/SDL2-2.30.0/x86_64-w64-mingw32")
set(SDL2_INCLUDE_DIR "${SDL2_ROOT}/include/SDL2")
set(SDL2_LIBRARY "${SDL2_ROOT}/lib/libSDL2.dll.a")
set(SDL2_MAIN_LIBRARY "${SDL2_ROOT}/lib/libSDL2main.a")

add_library(SDL2::SDL2 STATIC IMPORTED)
set_target_properties(SDL2::SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
)
add_library(SDL2::SDL2main STATIC IMPORTED)
set_target_properties(SDL2::SDL2main PROPERTIES
    IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}"
)

# Copy SDL2.dll to build output
file(COPY "${SDL2_ROOT}/bin/SDL2.dll" DESTINATION "${CMAKE_BINARY_DIR}")

# ── FetchContent for header-only deps ──────────────────────────────
include(FetchContent)

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# nlohmann/json (header-only)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(json)

# ── Source files ───────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/telemetry/mock.cpp
    src/strategy/engine.cpp
    src/strategy/prompt.cpp
    src/net/http.cpp
    src/voice/tts.cpp
    src/voice/audio.cpp
    src/voice/mute_gate.cpp
    src/ui/theme.cpp
    src/ui/dashboard.cpp
    src/ui/settings.cpp
    src/ui/tr_button.cpp
    src/input/gamepad.cpp
)

# ImGui core + SDL2/OpenGL3 backends
list(APPEND SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ── Executable ─────────────────────────────────────────────────────
add_executable(water_strategist WIN32 ${SOURCES})

target_include_directories(water_strategist PRIVATE
    src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${json_SOURCE_DIR}/include
)

target_link_libraries(water_strategist PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    opengl32
    ole32
    oleaut32
    winhttp
    comdlg32
    version
    imm32
    gdi32
    winmm
    setupapi
    mingw32
)

target_compile_definitions(water_strategist PRIVATE SDL_MAIN_HANDLED)

# Copy config to build dir
add_custom_command(TARGET water_strategist POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.ini"
        "$<TARGET_FILE_DIR:water_strategist>/config.ini"
)
