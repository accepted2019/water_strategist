#include "mute_gate.h"
#include "../config.h"

bool should_mute(double lateral_g) {
    return lateral_g > config::MUTE_G_THRESHOLD;
}

bool can_release_queued(double lateral_g) {
    return lateral_g <= config::MUTE_G_THRESHOLD;
}
