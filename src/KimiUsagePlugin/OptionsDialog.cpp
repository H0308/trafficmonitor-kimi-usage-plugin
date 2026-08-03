#include "OptionsDialog.h"

#include "PluginInterface.h"
#include "KimiConfig.h"
#include "KimiDataManager.h"

#include "resource.h"

extern HINSTANCE g_hInstance;

namespace kimi_usage {

ITMPlugin::OptionReturn OptionsDialog::Show(HWND hParent) {
    INT_PTR result = DialogBoxParam(
        g_hInstance,
        MAKEINTRESOURCE(IDD_OPTIONS_DIALOG),
        hParent,
        DialogProc,
        0);

    if (result == IDOK) {
        return ITMPlugin::OR_OPTION_CHANGED;
    }
    return ITMPlugin::OR_OPTION_UNCHANGED;
}

INT_PTR CALLBACK OptionsDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
            OnInitDialog(hDlg);
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                if (OnOK(hDlg)) {
                    EndDialog(hDlg, IDOK);
                }
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

void OptionsDialog::OnInitDialog(HWND hDlg) {
    KimiConfig& config = KimiConfig::Instance();

    SetDlgItemText(hDlg, IDC_EDIT_API_KEY, config.api_key.c_str());
    SetDlgItemInt(hDlg, IDC_EDIT_INTERVAL, config.refresh_interval_seconds, FALSE);
    SetDlgItemInt(hDlg, IDC_EDIT_THRESHOLD, config.low_usage_threshold, FALSE);
    CheckDlgButton(hDlg, IDC_CHECK_SHOW_5H_RESET, config.show_5h_reset_time ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_SHOW_7D_RESET, config.show_7d_reset_time ? BST_CHECKED : BST_UNCHECKED);
}

bool OptionsDialog::OnOK(HWND hDlg) {
    KimiConfig& config = KimiConfig::Instance();

    wchar_t api_key[1024] = {};
    GetDlgItemText(hDlg, IDC_EDIT_API_KEY, api_key, 1024);
    config.api_key = api_key;

    BOOL translated = FALSE;
    int interval = static_cast<int>(GetDlgItemInt(hDlg, IDC_EDIT_INTERVAL, &translated, FALSE));
    if (translated && interval >= 5) {
        config.refresh_interval_seconds = interval;
    }

    translated = FALSE;
    int threshold = static_cast<int>(GetDlgItemInt(hDlg, IDC_EDIT_THRESHOLD, &translated, FALSE));
    if (translated && threshold >= 0 && threshold <= 100) {
        config.low_usage_threshold = threshold;
    }

    config.show_5h_reset_time = (IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_5H_RESET) == BST_CHECKED) ? 1 : 0;
    config.show_7d_reset_time = (IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_7D_RESET) == BST_CHECKED) ? 1 : 0;

    config.Save();
    KimiDataManager::Instance().Restart();

    return true;
}

} // namespace kimi_usage
