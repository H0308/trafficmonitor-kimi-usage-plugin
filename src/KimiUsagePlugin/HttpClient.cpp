#include "HttpClient.h"

#include <windows.h>
#include <winhttp.h>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace kimi_usage {

HttpResponse HttpClient::Get(
    const std::wstring& host,
    const std::wstring& path,
    const std::wstring& bearer_token) {

    HttpResponse response{};

    HINTERNET hSession = WinHttpOpen(
        L"KimiUsagePlugin/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) {
        response.error = L"WinHttpOpen failed";
        return response;
    }

    HINTERNET hConnect = WinHttpConnect(
        hSession,
        host.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT,
        0);
    if (!hConnect) {
        response.error = L"WinHttpConnect failed";
        WinHttpCloseHandle(hSession);
        return response;
    }

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        response.error = L"WinHttpOpenRequest failed";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return response;
    }

    std::wstring headers;
    if (!bearer_token.empty()) {
        headers = L"Authorization: Bearer " + bearer_token + L"\r\n";
    }

    BOOL sent = WinHttpSendRequest(
        hRequest,
        headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
        static_cast<DWORD>(headers.length()),
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0);
    if (!sent) {
        response.error = L"WinHttpSendRequest failed";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return response;
    }

    BOOL received = WinHttpReceiveResponse(hRequest, nullptr);
    if (!received) {
        response.error = L"WinHttpReceiveResponse failed";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return response;
    }

    DWORD status_code = 0;
    DWORD status_code_size = sizeof(status_code);
    WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &status_code,
        &status_code_size,
        WINHTTP_NO_HEADER_INDEX);
    response.status_code = static_cast<long>(status_code);

    DWORD available = 0;
    std::vector<char> buffer(4096);
    while (WinHttpQueryDataAvailable(hRequest, &available) && available > 0) {
        if (available > buffer.size()) {
            buffer.resize(available);
        }
        DWORD read = 0;
        if (!WinHttpReadData(hRequest, buffer.data(), available, &read)) {
            break;
        }
        response.body.append(buffer.data(), read);
    }

    response.success = true;

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}

} // namespace kimi_usage
