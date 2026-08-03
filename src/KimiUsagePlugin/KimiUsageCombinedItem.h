#pragma once

#include <string>
#include "PluginInterface.h"
#include "KimiConfig.h"

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
    virtual int GetItemWidth() const override { return 0; }
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual int IsDoubleLineExclusive() const override { return 1; }
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
    virtual bool DrawItemEx(IPluginDrawer* pDrawer, int x, int y, int w, int h, bool dark_mode) override;

private:
    void DoDraw(void* hDC, int x, int y, int w, int h, bool dark_mode);
    void DrawRow(void* hDC, int x, int y, int w, int h, bool dark_mode, bool is_5h);
    void DrawSingleLine(void* hDC, int x, int y, int w, int h, bool dark_mode);

    mutable std::wstring value_cache_;
    mutable bool last_draw_was_single_line_ = false;
};

} // namespace kimi_usage
