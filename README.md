# TrafficMonitor显示Kimi Coding Plan限额插件

在 Windows 任务栏 [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 中显示 Kimi Code 的 **5 小时滚动窗口** 和 **7 天周期** 用量限额。

## 功能

- 同时显示 5 小时和 7 天两套限额的使用百分比
- 低限额时进度条变红提醒
- 支持通过插件选项对话框配置 API Key、刷新间隔和告警阈值

## 安装

1. 确保已安装 [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor/releases)（v1.82+）。
2. 从 [Releases](https://github.com/H0308/trafficmonitor-kimi-usage-plugin/releases/) 下载对应架构的 `KimiUsagePlugin.dll`：
   - 64 位 TrafficMonitor → `KimiUsagePlugin_x64.dll`
   - 32 位 TrafficMonitor → `KimiUsagePlugin_x86.dll`
3. 关闭 TrafficMonitor，将 DLL 重命名为 `KimiUsagePlugin.dll` 并复制到 TrafficMonitor 目录下的 `plugins` 文件夹：
   ```
   TrafficMonitor/
   ├── TrafficMonitor.exe
   └── plugins/
       └── KimiUsagePlugin.dll
   ```
4. 启动 TrafficMonitor，在任务栏窗口右键 → **显示设置** → 勾选 **Kimi Code 限额**。

## 配置

1. 任务栏窗口右键 → **插件管理** → 选中 **KimiUsagePlugin** → **选项**。
2. 在设置对话框中填写：
   - **API Key**：你的 Kimi Code API Key（`sk-...`）
   - **刷新间隔（秒）**：后台拉取数据的间隔，建议 `60`
   - **低限额阈值（%）**：进度条变红的百分比阈值，建议 `80`
3. 点击确定保存。

配置文件会保存在 TrafficMonitor 配置目录下的 `KimiUsagePlugin.ini` 中。

## 构建

使用 Visual Studio 2022/2026 打开 `KimiUsagePlugin.sln`，选择 `Release | x64` 或 `Release | x86` 构建即可。

构建输出：
```
build/
├── x64/
│   └── Release/
│       └── KimiUsagePlugin.dll
└── Win32/
    └── Release/
        └── KimiUsagePlugin.dll
```

## 项目结构

```
KimiUsagePlugin/
├── KimiUsagePlugin.sln
├── include/
│   └── PluginInterface.h          # TrafficMonitor 插件 SDK
├── src/
│   └── KimiUsagePlugin/
│       ├── KimiUsagePlugin.vcxproj
│       ├── HttpClient.h/.cpp      # WinHTTP HTTPS 请求
│       ├── KimiConfig.h/.cpp      # INI 配置读写
│       ├── KimiDataManager.h/.cpp # 后台数据拉取与缓存
│       ├── KimiUsageCombinedItem.h/.cpp # 双行自定义绘制显示项
│       ├── KimiUsagePlugin.h/.cpp # 插件入口
│       ├── OptionsDialog.h/.cpp   # 设置对话框
│       ├── dllmain.cpp            # DLL 导出
│       └── res/
│           ├── resource.h
│           └── resource.rc
└── third_party/
    └── nlohmann/
        └── json.hpp               # 单文件 JSON 解析库
```

## 致谢

- [zhongyang219/TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) - 优秀的 Windows 任务栏监控工具
- [nlohmann/json](https://github.com/nlohmann/json) - 单文件 JSON 库

## License

[MIT](LICENSE)
