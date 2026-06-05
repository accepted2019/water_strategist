#pragma once
#include "../telemetry/frame.h"
#include <string>
#include <future>

struct StrategyResult {
    std::string text;
    bool success = false;
    std::string error_message;
    double latency_ms = 0.0;
};

// Returns a future that resolves with the strategy result.
// Caller should check future_status on main thread to avoid blocking UI.
std::future<StrategyResult> request_strategy_async(const TelemetryFrame& frame);

// Synchronous convenience wrapper (blocks calling thread).
StrategyResult request_strategy(const TelemetryFrame& frame);
