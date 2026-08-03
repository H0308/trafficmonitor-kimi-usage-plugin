#pragma once

#include <string>

namespace kimi_usage {

class KimiConfig {
public:
    static KimiConfig& Instance();

    void Initialize(const std::wstring& config_dir);
    void Load();
    void Save() const;

    const std::wstring& GetIniPath() const { return ini_path_; }

    std::wstring api_key;
    int refresh_interval_seconds = 60;
    int low_usage_threshold = 80;
    int show_5h_reset_time = 0;
    int show_7d_reset_time = 0;

private:
    KimiConfig() = default;
    ~KimiConfig() = default;
    KimiConfig(const KimiConfig&) = delete;
    KimiConfig& operator=(const KimiConfig&) = delete;

    std::wstring config_dir_;
    std::wstring ini_path_;
};

} // namespace kimi_usage
