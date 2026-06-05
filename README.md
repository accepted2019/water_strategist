# WaterStrategist
WaterStrategist

# WaterStrategist — C++ 开发报告

**日期**: 2026-06-05  
**版本**: blueprint 1.0
**状态**: 构建成功，可运行

---

## 1. 项目概述

将 Python/Streamlit 原型重写为纯 C++ 原生 Windows 桌面应用，为 Steam 分发做准备。  
核心功能：遥测数据 → Gemini AI 策略分析 → 语音播报。  
UI 采用 F1 专业风格（碳纤维黑 + 遥测绿 + 警告红），TR 按钮 40x40px。

**原型（Python）**: `F:\water_strategist_demo`  
**C++ 项目**: `F:\water_strategist`  
**构建输出**: `F:\water_strategist\build\water_strategist.exe`

---

## 2. 技术栈

| 层 | 选择 | 版本 |
|---|------|------|
| 窗口/输入/音频 | SDL2 (预编译 MinGW 二进制) | 2.30.0 |
| UI 渲染 | Dear ImGui (SDL2 + OpenGL 3.0) | v1.91.0 |
| HTTP 客户端 | WinHTTP (Windows 原生) | — |
| JSON | nlohmann/json (header-only) | v3.11.3 |
| TTS | 蜂鸣音 (正弦波 800Hz 200ms) | — |
| 字体 | 微软雅黑 (C:/Windows/Fonts/msyh.ttc) | — |
| 构建 | CMake + MinGW Makefiles | cmake 4.3.2 |
| 编译器 | g++ (Dev-Cpp MinGW64) | 4.9.2 |

**仅 3 个第三方依赖**，无外部运行时。

---

## 3. 目录结构

```
F:\water_strategist\
├── CMakeLists.txt              # CMake 构建脚本 (FetchContent + SDL2)
├── config.ini                  # API key, TTS 设置, 手柄映射
├── sdl2_mingw/                 # SDL2 预编译 MinGW 二进制文件
│   └── SDL2-2.30.0/
├── src/
│   ├── main.cpp                # SDL 初始化, 主循环, 事件分发 (344 行)
│   ├── config.h                # 常量: 窗口/API/音频/UI 配置
│   ├── ui/
│   │   ├── theme.h / theme.cpp       # F1 深色 ImGui 主题 + 字体加载
│   │   ├── dashboard.h / dashboard.cpp # 遥测仪表盘 (3 行布局)
│   │   └── tr_button.h / tr_button.cpp # "TR" 圆形按钮 (40x40px)
│   ├── telemetry/
│   │   ├── frame.h              # TelemetryFrame 数据结构
│   │   └── mock.h / mock.cpp    # 模拟遥测数据生成器
│   ├── strategy/
│   │   ├── engine.h / engine.cpp   # Gemini API (WinHTTP, std::async)
│   │   └── prompt.h / prompt.cpp   # 中文策略工程师 System Prompt
│   ├── voice/
│   │   ├── tts.h / tts.cpp         # 音频通知蜂鸣音生成器
│   │   ├── mute_gate.h / mute_gate.cpp # G 力 >1.5 静音闸
│   │   └── audio.h / audio.cpp     # SDL 音频环形缓冲播放
│   ├── input/
│   │   └── gamepad.h / gamepad.cpp # SDL GameController 接口 (手柄预留)
│   └── net/
│       └── http.h / http.cpp       # WinHTTP RAII 封装
└── build/
    ├── water_strategist.exe         # 3.5 MB
    ├── SDL2.dll                     # 2.5 MB
    └── config.ini
```

**代码统计**: 12 个 .cpp + 12 个 .h + 1 main.cpp = 1180 行

---

## 4. 模块说明

### 4.1 main.cpp — 应用入口

- SDL2 初始化 (`SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER`)
- OpenGL 3.0 Core Profile, VSync 开启
- Dear ImGui 初始化 + F1 主题 + 字体加载
- 主循环: 事件轮询 → 异步策略检查 → Mute Gate → ImGui 渲染
- 快捷键: Space = TR, F11 = 全屏, H = 隐藏侧边栏
- 游戏手柄事件: `SDL_CONTROLLERDEVICEADDED/REMOVED` 钩子
- 状态管理: g_loading, g_muted, g_queued_text, g_show_demo_controls

### 4.2 UI 模块

**theme.cpp — F1 深色主题**
- 碳纤维黑背景 (#0F0F0F), F1 遥测绿文字 (#00E620), 警告红按钮 (#CC0000)
- 字体: 微软雅黑 16px, 覆盖拉丁 + 2500 常用简体中文
- 回退链: msyh.ttc → simsun.ttc → ImGui 内置 ProggyClean

**dashboard.cpp — 遥测仪表盘**
- Row 1 (6 列): DRIVER, TYRE LAPS, GRIP%, DELTA, LAT G, SPEED
- Row 2 (3 列): FUEL, SECTOR, MUTE GATE 状态

**tr_button.cpp — TR 按钮**
- 40x40px 圆形按钮, 左下角固定位置
- 加载态: 旋转绿色圆环 + 动态点数动画
- 悬浮态: 红色高亮, tooltip "Team Radio — Request strategy via Gemini AI"

### 4.3 遥测模块

**TelemetryFrame** 数据结构:
| 字段 | 类型 | 说明 |
|------|------|------|
| driver | string | 车手名 |
| tire_laps | int | 轮胎已跑圈数 |
| grip_remaining_pct | double | 剩余抓地力 % |
| delta_behind | double | 后车差距 (秒) |
| lateral_g_force | double | 横向 G 力 |
| speed_kmh | double | 当前速度 |
| fuel_kg | double | 剩余燃油 |
| track_section | string | 路段 (straight/corner) |

**mock.cpp**: 随机生成合理范围数据, 侧边栏可手动调节参数

### 4.4 策略引擎

**engine.cpp — Gemini API**
- API Key 读取优先级: `GEMINI_API_KEY` 环境变量 → `GOOGLE_API_KEY` → `config.ini`
- 模型: `gemini-1.5-flash`, 超时 5s, 最大 50 tokens, 温度 0.3
- 异步: `std::async(std::launch::async, request_strategy, frame)`
- 离线降级: 无 API key 时自动回退 DEMO MODE 预设消息
- JSON 解析: 从 Gemini 响应提取 `candidates[0].content.parts[0].text`

**prompt.cpp — System Prompt**
- 中文 F1/ACC 围场策略工程师人设
- 硬性约束: 每次输出 ≤35 汉字
- 用户提示: 将全部遥测字段打包为结构化文本

**http.cpp — WinHTTP**
- RAII 封装 (Session → Connect → Request 三级句柄)
- HTTPS 支持, UTF-8 ↔ UTF-16 转换
- 流式读取响应体, 4KB 缓冲区
- 超时统一设置 (解析/连接/发送/接收)

### 4.5 语音模块

**tts.cpp — 蜂鸣音生成**
- 当前版本: 800Hz 正弦波, 200ms, 指数衰减包络
- PCM 格式: mono, 16-bit signed, 22050Hz
- 策略文本同时输出到控制台 `[TTS] Strategy: ...`
- SAPI 完整 TTS 预留 v1.1 (需 `<sapi.h>`, MinGW 4.9.2 不内置)

**audio.cpp — SDL 音频播放**
- 环形缓冲 + `std::mutex` 线程安全
- SDL 音频回调: `AUDIO_S16SYS`, mono, 1024 samples/buffer
- `audio_queue_push()` / `audio_is_playing()` 接口

**mute_gate.cpp — G 力静音闸**
- `should_mute(g)`: lateral_g > 1.5 → 静音
- `can_release_queued(g)`: lateral_g ≤ 1.5 → 释放排队音频
- UI 显示: "MUTE GATE ACTIVE" 红色状态

### 4.6 输入模块

**gamepad.cpp — 手柄接口**
- SDL_GameController API 封装
- TR 映射: `SDL_CONTROLLER_BUTTON_A`
- 设备名显示于侧边栏
- D-Pad / 扳机轴读取预留

---

## 5. 数据流

```
[TR] 按下 (Space / 手柄A / 屏幕按钮)
  → update_telemetry() 快照 TelemetryFrame
  → std::async: WinHTTP POST → Gemini API (< 5s)
  → StrategyResult { text, success, latency_ms }
  → MuteGate::should_mute(lateral_g)
  → if safe: tts.synthesize(text) → PCM 蜂鸣音 → audio_queue_push() → SDL 播放
  → if muted: 排队 g_queued_text, 等待 G 力回落 → can_release_queued() → 播放
  → UI 渲染: 策略文本 + 状态栏 + 加载动画
```

---

## 6. API 配置

**config.ini** 格式:
```ini
[gemini]
api_key = <your-key>
model = gemini-1.5-flash
timeout_seconds = 5

[voice]
rate = 0
volume = 100
mute_g_threshold = 1.5

[ui]
fullscreen = false

[gamepad]
tr_button = a
deadzone = 0.1
```

---

## 7. 已知问题 & 待完成

| 问题 | 状态 | 计划 |
|------|------|------|
| TTS 仅为蜂鸣音 | 已知 | v1.1: 集成 Windows SAPI 完整语音合成 |
| 字体仅 2500 CJK | 已修复 | 使用 `GetGlyphRangesChineseSimplifiedCommon()` |
| F1Regular.ttf 缺失 | 已修复 | 改用系统微软雅黑 |
| 无单元测试 | 待做 | Phase 7: 添加 Google Test |
| Steam Overlay 兼容性 | 待验证 | 需 Steam SDK 集成测试 |
| 手柄 D-Pad/扳机未使用 | 预留 | 接口已定义, 功能待实现 |

---

## 8. 构建指令

```bash
# 配置 (使用 Dev-Cpp MinGW64)
cmake -B build -G "MinGW Makefiles"

# 编译
cmake --build build

# 运行
cd build && ./water_strategist.exe

# 发布版 (单文件夹)
# 需要文件: water_strategist.exe + SDL2.dll + config.ini
```

---

## 9. 编译兼容性

- **编译器**: g++ 4.9.2 (Dev-Cpp 内置 MinGW64) — C++14
- **已知限制**: `<cstring>` / `<cstdio>` 中 C 函数仅在全局命名空间, 不在 `std::`
  - 解决方案: 使用 `<string.h>` / `<stdio.h>`, 调用时不加 `std::` 前缀
- **OpenGL**: 需要 `<GL/gl.h>` 显式包含 (glViewport, glClear 等)
- **SAPI**: `<sapi.h>` / `<sphelper.h>` 不可用 — TTS 用蜂鸣音替代
- **链接库**: SDL2, OpenGL32, WinHTTP, ole32, imm32, gdi32, winmm, setupapi, mingw32

---

## 10. 文件清单

### 源文件 (1180 行总计)

| 文件 | 行数 | 职责 |
|------|------|------|
| src/main.cpp | 344 | 应用入口, 主循环, 事件分发 |
| src/config.h | 31 | 全局常量 |
| src/telemetry/frame.h | 14 | 遥测数据结构 |
| src/telemetry/mock.cpp | 42 | 模拟遥测生成 |
| src/net/http.cpp | 117 | WinHTTP POST 封装 |
| src/strategy/engine.cpp | 99 | Gemini API 调用 + JSON 解析 |
| src/strategy/prompt.cpp | 22 | AI System Prompt |
| src/voice/tts.cpp | 57 | 蜂鸣音 PCM 生成 |
| src/voice/audio.cpp | 62 | SDL 音频队列播放 |
| src/voice/mute_gate.cpp | 10 | G 力静音闸 |
| src/ui/theme.cpp | 101 | F1 主题 + 字体加载 |
| src/ui/dashboard.cpp | 71 | 遥测仪表盘 |
| src/ui/tr_button.cpp | 61 | TR 圆形按钮 |
| src/input/gamepad.cpp | 40 | 手柄 GameController |
