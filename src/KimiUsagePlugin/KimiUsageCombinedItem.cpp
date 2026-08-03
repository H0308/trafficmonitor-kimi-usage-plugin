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
    return L"5H:100% 7d1h23m 7D:100% 7d1h23m";
}

int KimiUsageCombinedItem::GetItemWidthEx(void* hDC_ptr) const {
    HDC hDC = reinterpret_cast<HDC>(hDC_ptr);
    if (!hDC) {
        return 110;
    }

    bool show_5h = KimiConfig::Instance().show_5h_reset_time != 0;
    bool show_7d = KimiConfig::Instance().show_7d_reset_time != 0;
    if (!show_5h && !show_7d) {
        return 110;
    }

    UsageInfo info_5h = KimiDataManager::Instance().GetUsageInfo(true);
    UsageInfo info_7d = KimiDataManager::Instance().GetUsageInfo(false);

    auto make_pct_text = [](const UsageInfo& info, bool is_5h, bool show_reset) -> std::wstring {
        if (!info.error.empty()) {
            return info.error;
        }
        if (!info.valid) {
            return L"N/A";
        }
        std::wstring result = std::to_wstring(info.percentage) + L"%";
        if (show_reset) {
            std::wstring countdown = KimiDataManager::Instance().GetResetCountdownText(is_5h);
            if (!countdown.empty()) {
                result += L" " + countdown;
            }
        }
        return result;
    };

    std::wstring pct_5h = make_pct_text(info_5h, true, show_5h);
    std::wstring pct_7d = make_pct_text(info_7d, false, show_7d);

    SIZE size_5h{}, size_7d{};
    GetTextExtentPoint32W(hDC, pct_5h.c_str(), static_cast<int>(pct_5h.length()), &size_5h);
    GetTextExtentPoint32W(hDC, pct_7d.c_str(), static_cast<int>(pct_7d.length()), &size_7d);

    if (last_draw_was_single_line_) {
        // 横向排列：单行并排 "5H:...  7D:..."
        SIZE size_label_5h{}, size_label_7d{};
        GetTextExtentPoint32W(hDC, L"5H:", 3, &size_label_5h);
        GetTextExtentPoint32W(hDC, L"7D:", 3, &size_label_7d);

        constexpr int kSingleLineGap = 10;
        constexpr int kSingleLinePadding = 4;
        int total_w = size_label_5h.cx + size_5h.cx + size_label_7d.cx + size_7d.cx +
                      kSingleLineGap + kSingleLinePadding;
        return total_w > 110 ? total_w : 110;
    }

    // 正常排列：双行，左侧固定占用 + 当前较长文本 + 少量右侧留白
    constexpr int kLeftFixed = 24 + 6 + 44 + 4; // label + gap + bar + margin
    int text_w = size_5h.cx > size_7d.cx ? size_5h.cx : size_7d.cx;
    constexpr int kRightPadding = 2;
    int total_w = kLeftFixed + text_w + kRightPadding;
    return total_w > 110 ? total_w : 110;
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
    last_draw_was_single_line_ = (h < kDoubleLineMinHeight);
    if (last_draw_was_single_line_) {
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
    if (KimiConfig::Instance().show_5h_reset_time) {
        std::wstring countdown = KimiDataManager::Instance().GetResetCountdownText(true);
        if (!countdown.empty()) {
            text_5h += L" " + countdown;
        }
    }
    std::wstring text_7d = make_text(info_7d, L"7D:");
    if (KimiConfig::Instance().show_7d_reset_time) {
        std::wstring countdown = KimiDataManager::Instance().GetResetCountdownText(false);
        if (!countdown.empty()) {
            text_7d += L" " + countdown;
        }
    }

    int old_bk_mode = SetBkMode(hDC, TRANSPARENT);
    COLORREF old_text_color = SetTextColor(hDC, text_color);

    // 按两段文本的实际宽度分配单行空间，避免 7D 倒计时较长时被截断。
    // 空间充足时按文本宽度+内边距分配；空间不足时按比例分配。
    // 5H 右对齐、7D 左对齐，确保中间始终有间隙，不会互相覆盖。
    SIZE size_5h{}, size_7d{};
    GetTextExtentPoint32W(hDC, text_5h.c_str(), static_cast<int>(text_5h.length()), &size_5h);
    GetTextExtentPoint32W(hDC, text_7d.c_str(), static_cast<int>(text_7d.length()), &size_7d);

    constexpr int kMinGap = 10;
    constexpr int kTextPadding = 4;

    int needed_5h = size_5h.cx + kTextPadding * 2;
    int needed_7d = size_7d.cx + kTextPadding * 2;
    int total_needed = needed_5h + needed_7d + kMinGap;

    int w_5h = 0;
    int w_7d = 0;
    if (total_needed <= w) {
        int extra = w - total_needed;
        w_5h = needed_5h + extra / 2;
        w_7d = needed_7d + extra / 2;
    } else {
        int avail_for_text = w - kMinGap;
        if (avail_for_text < 0) avail_for_text = 0;
        int total_text_w = size_5h.cx + size_7d.cx;
        if (total_text_w > 0) {
            w_5h = (avail_for_text * size_5h.cx) / total_text_w;
            w_7d = avail_for_text - w_5h;
        } else {
            w_5h = avail_for_text / 2;
            w_7d = avail_for_text - w_5h;
        }
    }

    RECT rc_5h{ x, y, x + w_5h, y + h };
    DrawTextW(hDC, text_5h.c_str(), -1, &rc_5h, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    RECT rc_7d{ x + w_5h + kMinGap, y, x + w, y + h };
    DrawTextW(hDC, text_7d.c_str(), -1, &rc_7d, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

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

    // 百分比/倒计时文本显示在进度条右侧。
    // 显示重置时间时左对齐，使 5H 和 7D 的百分比/倒计时在同一竖线对齐；
    // 不显示重置时间时，在进度条右侧一个固定最大宽度的区域内居中。
    bool show_reset = is_5h ? KimiConfig::Instance().show_5h_reset_time
                            : KimiConfig::Instance().show_7d_reset_time;

    std::wstring pct_text;
    if (!info.error.empty()) {
        pct_text = info.error;
    } else if (!info.valid) {
        pct_text = L"N/A";
    } else {
        pct_text = std::to_wstring(info.percentage) + L"%";
        if (show_reset) {
            std::wstring countdown = KimiDataManager::Instance().GetResetCountdownText(is_5h);
            if (!countdown.empty()) {
                pct_text += L" " + countdown;
            }
        }
    }

    SIZE text_size{};
    GetTextExtentPoint32W(hDC, pct_text.c_str(), static_cast<int>(pct_text.length()), &text_size);
    int text_w = text_size.cx;

    constexpr int kPctAreaMargin = 4;
    constexpr int kMaxPctAreaWidth = 100;

    int pct_base_x = bar_x + kBarWidth + kPctAreaMargin;
    int pct_x = pct_base_x;

    if (!show_reset) {
        int pct_area_available = x + w - pct_base_x;
        int pct_area_w = pct_area_available < kMaxPctAreaWidth ? pct_area_available : kMaxPctAreaWidth;
        if (pct_area_w < 0) {
            pct_area_w = 0;
        }
        pct_x = pct_base_x + (pct_area_w - text_w) / 2;
        if (pct_x < pct_base_x) {
            pct_x = pct_base_x;
        }
    }

    RECT pct_rc{ pct_x, y, x + w, y + h };
    DrawTextW(hDC, pct_text.c_str(), -1, &pct_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    SetBkMode(hDC, old_bk_mode);
    SetTextColor(hDC, old_text_color);
}

} // namespace kimi_usage
