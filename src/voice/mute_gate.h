#pragma once
#include <string>

struct MuteState {
    bool is_muted = false;
    std::string queued_text;  // text waiting for G to drop
};

// Returns true if audio should be muted at this G level
bool should_mute(double lateral_g);

// Check if previously queued text should be released
bool can_release_queued(double lateral_g);
