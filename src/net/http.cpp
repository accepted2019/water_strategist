#include "http.h"
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <string.h>

#pragma comment(lib, "winhttp.lib")

static std::wstring to_w(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring ws(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &ws[0], len);
    return ws;
}

static std::string to_utf8(const wchar_t* ws, int len = -1) {
    if (!ws) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, ws, len, nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws, len, &s[0], n, nullptr, nullptr);
    return s;
}

static bool parse_url(const std::string& url,
                      std::wstring& host, std::wstring& path, int& port, bool& secure)
{
    const char* p = url.c_str();
    secure = (strncmp(p, "https://", 8) == 0);
    p = secure ? p + 8 : (strncmp(p, "http://", 7) == 0 ? p + 7 : p);

    const char* slash = strchr(p, '/');
    std::string host_part(slash ? std::string(p, slash) : std::string(p));
    path = slash ? to_w(slash) : L"/";

    const char* colon = strchr(host_part.c_str(), ':');
    if (colon) {
        host = to_w(std::string(host_part.c_str(), colon));
        port = std::atoi(colon + 1);
    } else {
        host = to_w(host_part);
        port = secure ? 443 : 80;
    }
    return !host.empty();
}

HttpResponse http_post_json(const std::string& url,
                            const std::string& json_body,
                            const std::string& api_key,
                            int timeout_seconds)
{
    HttpResponse resp;

    std::wstring host, path;
    int port; bool secure;
    if (!parse_url(url, host, path, port, secure)) {
        resp.error = "Failed to parse URL";
        return resp;
    }

    HINTERNET hSession = WinHttpOpen(
        L"WaterStrategist/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { resp.error = "WinHttpOpen failed"; return resp; }

    WinHttpSetTimeouts(hSession, timeout_seconds * 1000,
                       timeout_seconds * 1000,
                       timeout_seconds * 1000,
                       timeout_seconds * 1000);

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), (INTERNET_PORT)port, 0);
    if (!hConnect) { resp.error = "WinHttpConnect failed"; WinHttpCloseHandle(hSession); return resp; }

    DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"POST", path.c_str(), nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) { resp.error = "WinHttpOpenRequest failed"; WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return resp; }

    std::string headers = "Content-Type: application/json\r\n";
    if (!api_key.empty()) {
        headers += "x-goog-api-key: " + api_key + "\r\n";
    }

    bool ok = WinHttpSendRequest(
        hRequest,
        to_w(headers).c_str(), (DWORD)headers.size(),
        (LPVOID)json_body.c_str(), (DWORD)json_body.size(),
        (DWORD)json_body.size(), 0) != 0;

    if (ok) ok = WinHttpReceiveResponse(hRequest, nullptr) != 0;

    if (ok) {
        DWORD status = 0, size = sizeof(status);
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
        resp.status_code = status;

        DWORD avail = 0, read = 0;
        char buf[4096];
        while (WinHttpQueryDataAvailable(hRequest, &avail) && avail > 0) {
            DWORD to_read = avail < sizeof(buf) ? avail : sizeof(buf);
            if (WinHttpReadData(hRequest, buf, to_read, &read))
                resp.body.append(buf, read);
        }
    } else {
        resp.error = "WinHttpSendRequest/ReceiveResponse failed (" + std::to_string(GetLastError()) + ")";
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return resp;
}
