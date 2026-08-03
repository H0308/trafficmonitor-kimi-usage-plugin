#include "KimiUsageCombinedItem.h"

#include <windows.h>

#include "KimiConfig.h"
#include "KimiDataManager.h"

namespace kimi_usage {

namespace {

constexpr int kLabelWidth = 24;
constexpr int kGapLabelBar = 6;
constexpr int kBarWidth = 44;
constexpr int kBarHeight = 6;
constexpr int kTotalWidth = kLabelWidth + kGapLabelBar + kBarWidth;

} // namespace

KimiUsageCombinedItem::KimiUsageCombinedItem() {
}

const wchar_t* KimiUsageCombinedItem::GetItemName() const {
    return L"Kimi Code 限额";
}

const wchar_t* KimiUsageCombinedItem::GetItemId() const {
    return L"KimiUsage";
}

const wchar_t* KimiUsageCombinedItem::GetItemLableText() const {
    return L"";
}

const wchar_t* KimiUsageCombinedItem::GetItemValueText() const {
    UsageInfo info_5h = KimiDataManager::Instance().GetUsageInfo(true);
    UsageInfo info_7d = KimiDataManager::Instance().GetUsageInfo(false);
    value_cache_ = L"5H:";
    if (!info_5h.error.empty()) {
        value_cache_ += info_5h.error;
    } else if (!info_5h.valid) {
        value_cache_ += L"N/A";
    } else {
        value_cache_ += std::to_wstring(info_5h.percentage) + L"%";
    }
    value_cache_ += L" 7D:";
    if (!info_7d.error.empty()) {
        value_cache_ += info_7d.error;
    } else if (!info_7d.valid) {
        value_cache_ += L"N/A";
    } else {
        value_cache_ += std::to_wstring(info_7d.percentage) + L"%";
    }
    return value_cache_.c_str();
}

const wchar_t* KimiUsageCombinedItem::GetItemValueSampleText() const {
    return L"5H:100% 7D:100%";
}

void KimiUsageCombinedItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) {
    DoDraw(reinterpret_cast<HDC>(hDC), x, y, w, h, dark_mode);
}

bool KimiUsageCombinedItem::DrawItemEx(IPluginDrawer* pDrawer, int x, int y, int w, int h, bool dark_mode) {
    if (!pDrawer) {
        return false;
    }
    HDC hDC = reinterpret_cast<HDC>(pDrawer->GetHDC());
    if (!hDC) {
        return false;
    }
    DoDraw(hDC, x, y, w, h, dark_mode);
    return true;
}

void KimiUsageCombinedItem::DoDraw(void* hDC_ptr, int x, int y, int w, int h, bool dark_mode) {
    HDC hDC = reinterpret_cast<HDC>(hDC_ptr);
    if (!hDC) {
        return;
    }

    // 当高度不足以容纳两行进度条时，改为单行并排显示文本。
    // IsDoubleLineExclusive 返回 1 会让插件在垂直布局下尽量独占双行高度；
    // 横向排列时高度受限，自动回落到单行数字。
    constexpr int kDoubleLineMinHeight = 32;
    if (h < kDoubleLineMinHeight) {
        DrawSingleLine(hDC, x, y, w, h, dark_mode);
    } else {
        int row_h = h / 2;
        DrawRow(hDC, x, y, w, row_h, dark_mode, true);
        DrawRow(hDC, x, y + row_h, w, row_h, dark_mode, false);
    }
}

void KimiUsageCombinedItem::DrawSingleLine(void* hDC_ptr, int x, int y, int w, int h, bool dark_mode) {
    HDC hDC = reinterpret_cast<HDC>(hDC_ptr);
    if (!hDC) {
        return;
    }

    UsageInfo info_5h = KimiDataManager::Instance().GetUsageInfo(true);
    UsageInfo info_7d = KimiDataManager::Instance().GetUsageInfo(false);

    COLORREF text_color = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);

    auto make_text = [](const UsageInfo& info, const wchar_t* prefix) -> std::wstring {
        std::wstring result = prefix;
        if (!info.error.empty()) {
            result += info.error;
        } else if (!info.valid) {
            result += L"N/A";
        } else {
            result += std::to_wstring(info.percentage) + L"%";
        }
        return result;
    };

    std::wstring text_5h = make_text(info_5h, L"5H:");
    std::wstring text_7d = make_text(info_7d, L"7D:");

    int old_bk_mode = SetBkMode(hDC, TRANSPARENT);
    COLORREF old_text_color = SetTextColor(hDC, text_color);

    int half_w = w / 2;

    RECT rc_5h{ x, y, x + half_w, y + h };
    DrawTextW(hDC, text_5h.c_str(), -1, &rc_5h, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    RECT rc_7d{ x + half_w, y, x + w, y + h };
    DrawTextW(hDC, text_7d.c_str(), -1, &rc_7d, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    SetBkMode(hDC, old_bk_mode);
    SetTextColor(hDC, old_text_color);
}

void KimiUsageCombinedItem::DrawRow(void* hDC_ptr, int x, int y, int w, int h, bool dark_mode, bool is_5h) {
    HDC hDC = reinterpret_cast<HDC>(hDC_ptr);
    if (!hDC) {
        return;
    }

    UsageInfo info = KimiDataManager::Instance().GetUsageInfo(is_5h);

    COLORREF text_color = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);
    COLORREF bar_bg = dark_mode ? RGB(68, 68, 68) : RGB(221, 221, 221);
    COLORREF bar_fill = RGB(0, 136, 204); // 蓝色

    if (info.valid && info.percentage >= KimiConfig::Instance().low_usage_threshold) {
        bar_fill = RGB(255, 0, 0); // 红色
    }

    const wchar_t* label = is_5h ? L"5H" : L"7D";

    int old_bk_mode = SetBkMode(hDC, TRANSPARENT);
    COLORREF old_text_color = SetTextColor(hDC, text_color);

    // 标签
    RECT label_rc{ x, y, x + kLabelWidth, y + h };
    DrawTextW(hDC, label, -1, &label_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // 进度条背景
    int bar_x = x + kLabelWidth + kGapLabelBar;
    int bar_y = y + (h - kBarHeight) / 2;
    RECT bar_rc{ bar_x, bar_y, bar_x + kBarWidth, bar_y + kBarHeight };
    HBRUSH bg_brush = CreateSolidBrush(bar_bg);
    FillRect(hDC, &bar_rc, bg_brush);
    DeleteObject(bg_brush);

    // 进度条填充
    if (info.valid && info.limit > 0) {
        int fill_w = (kBarWidth * info.percentage) / 100;
        if (fill_w > kBarWidth) fill_w = kBarWidth;
        if (fill_w < 0) fill_w = 0;
        if (fill_w > 0) {
            RECT fill_rc{ bar_x, bar_y, bar_x + fill_w, bar_y + kBarHeight };
            HBRUSH fill_brush = CreateSolidBrush(bar_fill);
            FillRect(hDC, &fill_rc, fill_brush);
            DeleteObject(fill_brush);
        }
    }

    // 百分比：在进度条右侧到绘制区域右边界之间居中
    std::wstring pct_text;
    if (!info.error.empty()) {
        pct_text = info.error;
    } else if (!info.valid) {
        pct_text = L"N/A";
    } else {
        pct_text = std::to_wstring(info.percentage) + L"%";
    }

    SIZE text_size{};
    GetTextExtentPoint32W(hDC, pct_text.c_str(), static_cast<int>(pct_text.length()), &text_size);
    int text_w = text_size.cx;

    int right_space_start = bar_x + kBarWidth;
    int right_space_end = x + w;
    int right_space_w = right_space_end - right_space_start;

    int pct_x = right_space_start + (right_space_w - text_w) / 2;
    if (pct_x < right_space_start) {
        pct_x = right_space_start;
    }

    RECT pct_rc{ pct_x, y, pct_x + text_w, y + h };
    DrawTextW(hDC, pct_text.c_str(), -1, &pct_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    SetBkMode(hDC, old_bk_mode);
    SetTextColor(hDC, old_text_color);
}

} // namespace kimi_usage
