#include "KimiUsagePlugin.h"

#include "KimiDataManager.h"
#include "OptionsDialog.h"

namespace kimi_usage {

namespace {

const wchar_t* kPluginName = L"KimiUsagePlugin";
const wchar_t* kPluginDescription = L"在任务栏显示 Kimi Code 5小时和7天限额";
const wchar_t* kPluginAuthor = L"KimiUsagePlugin";
const wchar_t* kPluginCopyright = L"Copyright (C) 2026";
const wchar_t* kPluginVersion = L"1.0.0";
const wchar_t* kPluginUrl = L"";

} // namespace

KimiUsagePlugin& KimiUsagePlugin::Instance() {
    static KimiUsagePlugin instance;
    return instance;
}

KimiUsagePlugin::KimiUsagePlugin() {
}

IPluginItem* KimiUsagePlugin::GetItem(int index) {
    if (index == 0) {
        return &item_;
    }
    return nullptr;
}

void KimiUsagePlugin::DataRequired() {
    KimiDataManager::Instance().DataRequired();
}

ITMPlugin::OptionReturn KimiUsagePlugin::ShowOptionsDialog(void* hParent) {
    return OptionsDialog::Show(reinterpret_cast<HWND>(hParent));
}

const wchar_t* KimiUsagePlugin::GetInfo(PluginInfoIndex index) {
    switch (index) {
        case TMI_NAME:
            return kPluginName;
        case TMI_DESCRIPTION:
            return kPluginDescription;
        case TMI_AUTHOR:
            return kPluginAuthor;
        case TMI_COPYRIGHT:
            return kPluginCopyright;
        case TMI_VERSION:
            return kPluginVersion;
        case TMI_URL:
            return kPluginUrl;
        default:
            return L"";
    }
}

const wchar_t* KimiUsagePlugin::GetTooltipInfo() {
    static std::wstring tooltip;
    tooltip = KimiDataManager::Instance().GetTooltipText();
    return tooltip.c_str();
}

void KimiUsagePlugin::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) {
    if (index == EI_CONFIG_DIR && data != nullptr && !initialized_) {
        KimiDataManager::Instance().Initialize(data);
        initialized_ = true;
    }
}

} // namespace kimi_usage
