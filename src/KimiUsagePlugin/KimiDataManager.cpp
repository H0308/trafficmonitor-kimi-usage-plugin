#include "KimiDataManager.h"

#include "HttpClient.h"
#include "KimiConfig.h"

#include "../third_party/nlohmann/json.hpp"

#include <windows.h>
#include <vector>
#include <exception>
#include <string>
#include <sstream>
#include <iomanip>

namespace kimi_usage {

namespace {

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return std::wstring();
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.length()), nullptr, 0);
    if (size <= 0) {
        return std::wstring();
    }
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.length()), result.data(), size);
    return result;
}

std::wstring FormatSystemTime(const SYSTEMTIME& st) {
    wchar_t buffer[64] = {};
    swprintf_s(buffer, L"%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
               st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return std::wstring(buffer);
}

} // namespace

KimiDataManager& KimiDataManager::Instance() {
    static KimiDataManager instance;
    return instance;
}

KimiDataManager::~KimiDataManager() {
    Shutdown();
}

void KimiDataManager::Initialize(const std::wstring& config_dir) {
    config_dir_ = config_dir;
    KimiConfig::Instance().Initialize(config_dir);
    Shutdown();
    running_ = true;
    refresh_requested_ = true;
    worker_thread_ = std::thread(&KimiDataManager::WorkerLoop, this);
}

void KimiDataManager::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void KimiDataManager::Restart() {
    Initialize(config_dir_);
}

void KimiDataManager::DataRequired() {
    auto now = std::chrono::steady_clock::now();
    auto interval = std::chrono::seconds(KimiConfig::Instance().refresh_interval_seconds);

    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        return;
    }
    if (last_fetch_time_ == std::chrono::steady_clock::time_point() ||
        now - last_fetch_time_ >= interval) {
        refresh_requested_ = true;
        cv_.notify_one();
    }
}

std::wstring KimiDataManager::GetValueText(bool is_5h) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const UsageData& data = is_5h ? five_hour_ : seven_day_;

    if (!data.error.empty()) {
        return GetErrorDisplayText(data.error);
    }

    if (!data.valid || data.limit <= 0) {
        return L"N/A";
    }

    int pct = CalculatePercentage(data.used, data.limit);
    return is_5h ? L"5H: " + std::to_wstring(pct) + L"%"
                 : L"7D: " + std::to_wstring(pct) + L"%";
}

UsageInfo KimiDataManager::GetUsageInfo(bool is_5h) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const UsageData& data = is_5h ? five_hour_ : seven_day_;
    UsageInfo info{};
    info.valid = data.valid;
    info.used = data.used;
    info.limit = data.limit;
    info.error = data.error;
    info.reset_time = data.reset_time;
    if (data.valid && data.limit > 0) {
        info.percentage = CalculatePercentage(data.used, data.limit);
    }
    return info;
}

std::wstring KimiDataManager::GetTooltipText() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::wstring result;

    auto append_usage = [&](const wchar_t* title, const UsageData& data) {
        result += title;
        result += L"\n";
        if (!data.error.empty()) {
            result += L"状态: " + GetErrorDisplayText(data.error) + L"\n";
        } else if (!data.valid || data.limit <= 0) {
            result += L"状态: N/A\n";
        } else {
            result += L"已用: " + std::to_wstring(data.used) + L" / " + std::to_wstring(data.limit) + L"\n";
            result += L"比例: " + std::to_wstring(CalculatePercentage(data.used, data.limit)) + L"%\n";
            if (!data.reset_time.empty()) {
                result += L"重置: " + data.reset_time + L"\n";
            }
        }
        result += L"更新: " + FormatTimePoint(data.update_time) + L"\n\n";
    };

    append_usage(L"Kimi Code 5小时限额", five_hour_);
    append_usage(L"Kimi Code 7天限额", seven_day_);

    if (!result.empty() && result.back() == L'\n') {
        result.pop_back();
        if (!result.empty() && result.back() == L'\n') {
            result.pop_back();
        }
    }

    return result;
}

void KimiDataManager::WorkerLoop() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        auto interval = std::chrono::seconds(KimiConfig::Instance().refresh_interval_seconds);
        cv_.wait_for(lock, interval, [this]() {
            return !running_ || refresh_requested_.load();
        });

        if (!running_) {
            break;
        }

        refresh_requested_ = false;
        lock.unlock();

        FetchAndParse();
    }
}

void KimiDataManager::FetchAndParse() {
    const std::wstring& api_key = KimiConfig::Instance().api_key;
    if (api_key.empty()) {
        std::lock_guard<std::mutex> lock(mutex_);
        five_hour_.error = L"NO KEY";
        seven_day_.error = L"NO KEY";
        five_hour_.valid = false;
        seven_day_.valid = false;
        last_fetch_time_ = std::chrono::steady_clock::now();
        return;
    }

    HttpResponse response = HttpClient::Get(L"api.kimi.com", L"/coding/v1/usages", api_key);

    std::lock_guard<std::mutex> lock(mutex_);
    last_fetch_time_ = std::chrono::steady_clock::now();

    if (!response.success) {
        five_hour_.error = L"NET";
        seven_day_.error = L"NET";
        five_hour_.valid = false;
        seven_day_.valid = false;
        return;
    }

    if (response.status_code == 401) {
        five_hour_.error = L"401";
        seven_day_.error = L"401";
        five_hour_.valid = false;
        seven_day_.valid = false;
        return;
    }

    if (response.status_code != 200) {
        five_hour_.error = L"NET";
        seven_day_.error = L"NET";
        five_hour_.valid = false;
        seven_day_.valid = false;
        return;
    }

    five_hour_.error.clear();
    seven_day_.error.clear();
    ParseResponse(response.body);
}

void KimiDataManager::ParseResponse(const std::string& body) {
    try {
        auto root = nlohmann::json::parse(body);

        // 7 天限额：字段为字符串数字
        if (root.contains("usage") && root["usage"].is_object()) {
            const auto& usage = root["usage"];
            if (usage.contains("used") && usage["used"].is_string() &&
                usage.contains("limit") && usage["limit"].is_string()) {
                seven_day_.used = std::stoll(usage["used"].get<std::string>());
                seven_day_.limit = std::stoll(usage["limit"].get<std::string>());
                if (usage.contains("resetTime") && usage["resetTime"].is_string()) {
                    seven_day_.reset_time = Utf8ToWide(usage["resetTime"].get<std::string>());
                } else {
                    seven_day_.reset_time.clear();
                }
                seven_day_.valid = (seven_day_.limit > 0);
            } else {
                seven_day_.valid = false;
            }
        } else {
            seven_day_.valid = false;
        }
        seven_day_.update_time = std::chrono::steady_clock::now();

        // 5 小时限额：数据在 limits[].detail 中，limit/remaining 为字符串
        five_hour_.valid = false;
        if (root.contains("limits") && root["limits"].is_array()) {
            for (const auto& item : root["limits"]) {
                if (!item.is_object()) continue;
                if (!item.contains("window") || !item["window"].is_object()) continue;
                const auto& window = item["window"];

                std::string time_unit = window.value("timeUnit", "");
                int duration = window.value("duration", 0);

                if (time_unit == "TIME_UNIT_MINUTE" && duration == 300) {
                    if (item.contains("detail") && item["detail"].is_object()) {
                        const auto& detail = item["detail"];
                        if (detail.contains("limit") && detail["limit"].is_string() &&
                            detail.contains("remaining") && detail["remaining"].is_string()) {
                            long long limit = std::stoll(detail["limit"].get<std::string>());
                            long long remaining = std::stoll(detail["remaining"].get<std::string>());
                            five_hour_.limit = limit;
                            five_hour_.used = limit - remaining;
                            if (detail.contains("resetTime") && detail["resetTime"].is_string()) {
                                five_hour_.reset_time = Utf8ToWide(detail["resetTime"].get<std::string>());
                            } else {
                                five_hour_.reset_time.clear();
                            }
                            five_hour_.valid = (five_hour_.limit > 0);
                        }
                    }
                    break;
                }
            }
        }
        five_hour_.update_time = std::chrono::steady_clock::now();

    } catch (const std::exception&) {
        five_hour_.error = L"PARSE";
        seven_day_.error = L"PARSE";
        five_hour_.valid = false;
        seven_day_.valid = false;
    }
}

std::wstring KimiDataManager::GetErrorDisplayText(const std::wstring& error) const {
    return error;
}

int KimiDataManager::CalculatePercentage(long long used, long long limit) const {
    if (limit <= 0) return 0;
    long long pct = (used * 100) / limit;
    if (pct < 0) pct = 0;
    if (pct > 999) pct = 999;
    return static_cast<int>(pct);
}

std::wstring KimiDataManager::FormatTimePoint(const std::chrono::steady_clock::time_point& tp) const {
    if (tp == std::chrono::steady_clock::time_point()) {
        return L"--";
    }
    auto now_system = std::chrono::system_clock::now();
    auto steady_now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(tp - steady_now);
    auto target = now_system + diff;

    time_t tt = std::chrono::system_clock::to_time_t(target);
    tm local_time{};
    localtime_s(&local_time, &tt);

    SYSTEMTIME st{};
    st.wYear = static_cast<WORD>(local_time.tm_year + 1900);
    st.wMonth = static_cast<WORD>(local_time.tm_mon + 1);
    st.wDay = static_cast<WORD>(local_time.tm_mday);
    st.wHour = static_cast<WORD>(local_time.tm_hour);
    st.wMinute = static_cast<WORD>(local_time.tm_min);
    st.wSecond = static_cast<WORD>(local_time.tm_sec);

    return FormatSystemTime(st);
}

} // namespace kimi_usage
