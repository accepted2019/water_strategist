#include "engine.h"
#include "prompt.h"
#include "../net/http.h"
#include "../config.h"
#include <chrono>
#include <future>
#include <cstdlib>
#include <string.h>

static std::string get_api_key() {
    const char* env = std::getenv("GEMINI_API_KEY");
    if (env && env[0]) return env;
    env = std::getenv("GOOGLE_API_KEY");
    if (env && env[0]) return env;

    // try to read config.ini next to exe
    FILE* fp = fopen("config.ini", "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "api_key", 7) == 0) {
                const char* eq = strchr(line, '=');
                if (eq) {
                    std::string val(eq + 1);
                    // trim whitespace
                    while (!val.empty() && (val.back() == '\n' || val.back() == '\r' || val.back() == ' '))
                        val.pop_back();
                    while (!val.empty() && (val.front() == ' '))
                        val.erase(0, 1);
                    fclose(fp);
                    return val;
                }
            }
        }
        fclose(fp);
    }
    return "";
}

StrategyResult request_strategy(const TelemetryFrame& frame) {
    StrategyResult result;
    auto t0 = std::chrono::steady_clock::now();

    std::string api_key = get_api_key();
    if (api_key.empty()) {
        result.text = config::DEMO_MSG_1;
        result.success = true;
        result.error_message = "[DEMO MODE] No API key configured";
        return result;
    }

    std::string url = std::string(config::GEMINI_ENDPOINT) + config::GEMINI_MODEL + ":generateContent";

    std::string body =
        "{\"system_instruction\":{\"parts\":[{\"text\":\"" + std::string(get_system_prompt()) + "\"}]},"
        "\"contents\":[{\"parts\":[{\"text\":\"" + build_user_prompt(frame) + "\"}]}],"
        "\"generationConfig\":{\"maxOutputTokens\":" + std::to_string(config::GEMINI_MAX_TOKENS) + ","
        "\"temperature\":" + std::to_string(config::GEMINI_TEMPERATURE) + "}}";

    HttpResponse http_resp = http_post_json(url, body, api_key, config::GEMINI_TIMEOUT_SEC);

    auto t1 = std::chrono::steady_clock::now();
    result.latency_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!http_resp.error.empty()) {
        result.text = config::DEMO_MSG_3;
        result.success = false;
        result.error_message = http_resp.error;
        return result;
    }

    if (http_resp.status_code != 200) {
        result.text = config::DEMO_MSG_3;
        result.success = false;
        result.error_message = "HTTP " + std::to_string(http_resp.status_code) + ": " + http_resp.body.substr(0, 100);
        return result;
    }

    // extract text from Gemini JSON response: {"candidates":[{"content":{"parts":[{"text":"..."}]}}]}
    const char* key = "\"text\": \"";
    size_t pos = http_resp.body.find(key);
    if (pos == std::string::npos) {
        result.text = config::DEMO_MSG_2;
        result.success = true;
        result.error_message = "Could not parse Gemini response";
        return result;
    }
    pos += strlen(key);
    size_t end = http_resp.body.find("\"", pos);
    std::string text = http_resp.body.substr(pos, end - pos);

    result.text = text.empty() ? config::DEMO_MSG_2 : text;
    result.success = true;
    return result;
}

std::future<StrategyResult> request_strategy_async(const TelemetryFrame& frame) {
    return std::async(std::launch::async, request_strategy, frame);
}
