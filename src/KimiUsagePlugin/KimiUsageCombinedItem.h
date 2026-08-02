#pragma once

#include <string>
#include "PluginInterface.h"

namespace kimi_usage {

class KimiUsageCombinedItem : public IPluginItem {
public:
    KimiUsageCombinedItem();

    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;

    virtual bool IsCustomDraw() const override { return true; }
    virtual int GetItemWidth() const override { return 110; }
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
    virtual bool DrawItemEx(IPluginDrawer* pDrawer, int x, int y, int w, int h, bool dark_mode) override;

private:
    void DoDraw(void* hDC, int x, int y, int w, int h, bool dark_mode);
    void DrawRow(void* hDC, int x, int y, int w, int h, bool dark_mode, bool is_5h);

    mutable std::wstring value_cache_;
};

} // namespace kimi_usage
