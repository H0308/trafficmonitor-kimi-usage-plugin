#include "KimiConfig.h"

#include <ShlObj.h>
#include <windows.h>

#pragma comment(lib, "shell32.lib")

namespace kimi_usage {

namespace {

constexpr int kMaxPath = 4096;
constexpr wchar_t kSectionGeneral[] = L"General";

std::wstring ReadIniString(
    const std::wstring& file_path,
    const std::wstring& section,
    const std::wstring& key,
    const std::wstring& default_value) {
    wchar_t buffer[kMaxPath] = {};
    GetPrivateProfileStringW(
        section.c_str(),
        key.c_str(),
        default_value.c_str(),
        buffer,
        kMaxPath,
        file_path.c_str());
    return std::wstring(buffer);
}

int ReadIniInt(
    const std::wstring& file_path,
    const std::wstring& section,
    const std::wstring& key,
    int default_value) {
    return GetPrivateProfileIntW(
        section.c_str(),
        key.c_str(),
        default_value,
        file_path.c_str());
}

void WriteIniString(
    const std::wstring& file_path,
    const std::wstring& section,
    const std::wstring& key,
    const std::wstring& value) {
    WritePrivateProfileStringW(
        section.c_str(),
        key.c_str(),
        value.c_str(),
        file_path.c_str());
}

} // namespace

KimiConfig& KimiConfig::Instance() {
    static KimiConfig instance;
    return instance;
}

void KimiConfig::Initialize(const std::wstring& config_dir) {
    config_dir_ = config_dir;
    if (!config_dir_.empty() && config_dir_.back() != L'\\' && config_dir_.back() != L'/') {
        config_dir_ += L'\\';
    }
    ini_path_ = config_dir_ + L"KimiUsagePlugin.ini";
    Load();
}

void KimiConfig::Load() {
    if (ini_path_.empty()) {
        return;
    }

    api_key = ReadIniString(ini_path_, kSectionGeneral, L"APIKey", L"");
    refresh_interval_seconds = ReadIniInt(ini_path_, kSectionGeneral, L"RefreshIntervalSeconds", 60);
    if (refresh_interval_seconds < 5) {
        refresh_interval_seconds = 5;
    }
    low_usage_threshold = ReadIniInt(ini_path_, kSectionGeneral, L"LowUsageThreshold", 80);
    if (low_usage_threshold < 0 || low_usage_threshold > 100) {
        low_usage_threshold = 80;
    }
    show_5h_reset_time = ReadIniInt(ini_path_, kSectionGeneral, L"Show5HResetTime", 0);
    if (show_5h_reset_time != 0 && show_5h_reset_time != 1) {
        show_5h_reset_time = 0;
    }
    show_7d_reset_time = ReadIniInt(ini_path_, kSectionGeneral, L"Show7DResetTime", 0);
    if (show_7d_reset_time != 0 && show_7d_reset_time != 1) {
        show_7d_reset_time = 0;
    }
    horizontal_layout = ReadIniInt(ini_path_, kSectionGeneral, L"HorizontalLayout", 0);
    if (horizontal_layout != 0 && horizontal_layout != 1) {
        horizontal_layout = 0;
    }
}

void KimiConfig::Save() const {
    if (ini_path_.empty()) {
        return;
    }

    WriteIniString(ini_path_, kSectionGeneral, L"APIKey", api_key);
    WriteIniString(ini_path_, kSectionGeneral, L"RefreshIntervalSeconds", std::to_wstring(refresh_interval_seconds));
    WriteIniString(ini_path_, kSectionGeneral, L"LowUsageThreshold", std::to_wstring(low_usage_threshold));
    WriteIniString(ini_path_, kSectionGeneral, L"Show5HResetTime", std::to_wstring(show_5h_reset_time));
    WriteIniString(ini_path_, kSectionGeneral, L"Show7DResetTime", std::to_wstring(show_7d_reset_time));
    WriteIniString(ini_path_, kSectionGeneral, L"HorizontalLayout", std::to_wstring(horizontal_layout));
}

} // namespace kimi_usage
