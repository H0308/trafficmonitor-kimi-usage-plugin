#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>
#include <atomic>

namespace kimi_usage {

struct UsageData {
    bool valid = false;
    long long used = -1;
    long long limit = -1;
    std::wstring reset_time;
    std::wstring error;
    std::chrono::steady_clock::time_point update_time;
};

struct UsageInfo {
    bool valid = false;
    long long used = -1;
    long long limit = -1;
    int percentage = 0;
    std::wstring error;
    std::wstring reset_time;
};

class KimiDataManager {
public:
    static KimiDataManager& Instance();

    void Initialize(const std::wstring& config_dir);
    void Shutdown();
    void Restart();

    void DataRequired();

    UsageInfo GetUsageInfo(bool is_5h) const;
    std::wstring GetValueText(bool is_5h) const;
    std::wstring GetTooltipText() const;

private:
    KimiDataManager() = default;
    ~KimiDataManager();
    KimiDataManager(const KimiDataManager&) = delete;
    KimiDataManager& operator=(const KimiDataManager&) = delete;

    void WorkerLoop();
    void FetchAndParse();
    void ParseResponse(const std::string& body);

    std::wstring GetErrorDisplayText(const std::wstring& error) const;
    int CalculatePercentage(long long used, long long limit) const;
    std::wstring FormatTimePoint(const std::chrono::steady_clock::time_point& tp) const;

    mutable std::mutex mutex_;
    std::wstring config_dir_;
    std::thread worker_thread_;
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::atomic<bool> refresh_requested_{false};

    UsageData five_hour_;
    UsageData seven_day_;
    std::chrono::steady_clock::time_point last_fetch_time_;
};

} // namespace kimi_usage
