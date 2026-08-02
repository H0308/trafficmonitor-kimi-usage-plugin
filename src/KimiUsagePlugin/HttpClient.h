#pragma once

#include <string>

namespace kimi_usage {

struct HttpResponse {
    long status_code = 0;
    std::string body;
    std::wstring error;
    bool success = false;
};

class HttpClient {
public:
    static HttpResponse Get(
        const std::wstring& host,
        const std::wstring& path,
        const std::wstring& bearer_token = L"");
};

} // namespace kimi_usage
