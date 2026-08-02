#pragma once

#include "PluginInterface.h"

#include "KimiUsageCombinedItem.h"

namespace kimi_usage {

class KimiUsagePlugin : public ITMPlugin {
public:
    static KimiUsagePlugin& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual void DataRequired() override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;

private:
    KimiUsagePlugin();
    ~KimiUsagePlugin() = default;
    KimiUsagePlugin(const KimiUsagePlugin&) = delete;
    KimiUsagePlugin& operator=(const KimiUsagePlugin&) = delete;

    KimiUsageCombinedItem item_;
    bool initialized_ = false;
};

} // namespace kimi_usage
