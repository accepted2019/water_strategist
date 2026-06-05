#pragma once
#include "../telemetry/frame.h"
#include <string>

const char* get_system_prompt();
std::string build_user_prompt(const TelemetryFrame& f);
