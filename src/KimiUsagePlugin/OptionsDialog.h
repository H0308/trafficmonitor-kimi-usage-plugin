#pragma once

#include <windows.h>
#include "PluginInterface.h"

namespace kimi_usage {

class OptionsDialog {
public:
    static ITMPlugin::OptionReturn Show(HWND hParent);

private:
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static void OnInitDialog(HWND hDlg);
    static bool OnOK(HWND hDlg);
};

} // namespace kimi_usage
