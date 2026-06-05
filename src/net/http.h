#pragma once
#include <string>

struct HttpResponse {
    long status_code;
    std::string body;
    std::string error;
};

// POST JSON to url, returns response. Uses WinHTTP internally.
HttpResponse http_post_json(const std::string& url,
                            const std::string& json_body,
                            const std::string& api_key,
                            int timeout_seconds);
