#include "mock.h"
#include <cstdlib>
#include <cmath>
#include <ctime>

static bool seeded = false;

static double randf(double lo, double hi) {
    if (!seeded) { std::srand((unsigned)std::time(nullptr)); seeded = true; }
    return lo + (hi - lo) * (double)std::rand() / (double)RAND_MAX;
}

TelemetryFrame generate_frame(
    const std::string& driver,
    int tire_laps,
    double grip_pct,
    double delta,
    double lateral_g)
{
    if (tire_laps < 0) tire_laps = (int)randf(0, 25);
    if (grip_pct < 0.0) grip_pct = std::fmax(40.0, 100.0 - tire_laps * 2.4 + randf(-3, 3));
    if (delta < 0.0) delta = randf(0.1, 3.5);
    if (lateral_g < 0.0) lateral_g = randf(0.0, 3.0);

    TelemetryFrame f;
    f.driver = driver;
    f.tire_laps = tire_laps;
    f.grip_remaining_pct = grip_pct;
    f.delta_behind = delta;
    f.lateral_g_force = lateral_g;
    f.speed_kmh = 320.0 - lateral_g * 80.0 + randf(-10, 10);
    f.fuel_kg = randf(5, 110);
    f.track_section = (lateral_g > 0.8) ? "corner" : "straight";
    return f;
}

std::vector<TelemetryFrame> generate_stream(int frames, double /*interval*/) {
    std::vector<TelemetryFrame> v;
    v.reserve(frames);
    for (int i = 0; i < frames; ++i) v.push_back(generate_frame());
    return v;
}
