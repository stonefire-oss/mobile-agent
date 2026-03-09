# Mobile Agent

让用户通过自然对话，指挥手机自动完成日常任务的 Android 自动化框架。

## 架构概览

Mobile Agent 采用三层模块化架构：

```
┌─────────────────────────────────────────────┐
│  app (演示壳)                                │
│  - MainActivity (UI 入口)                   │
│  - 配置加载与初始化绑定                       │
└─────────────────┬───────────────────────────┘
                  ↓ 依赖
┌─────────────────────────────────────────────┐
│  agent-android (Android 适配层)              │
│  - AndroidToolManager (工具管理)             │
│  - WorkspaceManager (工作空间)               │
│  - Android Tools (Toast, 通知, 剪贴板等)      │
│  - NativeMobileAgentApiAdapter              │
└─────────────────┬───────────────────────────┘
                  ↓ 依赖
┌─────────────────────────────────────────────┐
│  agent-core (核心库)                          │
│  - C++ JNI 绑定                              │
│  - ToolExecutor 接口                        │
│  - Agent 逻辑                               │
└─────────────────────────────────────────────┘
```

### 模块说明

| 模块 | 职责 |
|------|------|
| **app** | 演示壳，包含 Activity 和简单绑定，不包含业务逻辑 |
| **agent-core** | 核心库，提供 C++ JNI 绑定和 ToolExecutor 接口 |
| **agent-android** | Android 适配层，包含所有 Android 工具实现和工作空间管理 |

### 依赖关系

```
app → agent-android → agent-core
```

## 快速开始

### 前置要求

- Android Studio Arctic Fox+
- Gradle 8.0+
- Android SDK 34+

### 构建步骤

1. **克隆项目**
   ```bash
   git clone <repository-url>
   cd mobile-agent
   ```

2. **配置 API Key**

   复制配置模板文件并填写你的 API Key：
   ```bash
   cp config.json.template config.json
   ```

   编辑 `config.json`：
   ```json
   {
     "provider": {
       "apiKey": "your-api-key-here",
       "baseUrl": "https://api.minimaxi.com/v1"
     },
     "agent": {
       "model": "MiniMax-M2.5-highspeed"
     }
   }
   ```

3. **构建项目**
   ```bash
   ./gradlew assembleDebug
   ```

4. **安装运行**
   ```bash
   adb install app/build/outputs/apk/debug/app-debug.apk
   ```

## 项目结构

```
mobile-agent/
├── app/                          # 演示壳模块
│   ├── build.gradle
│   └── src/main/
│       ├── java/.../
│       │   └── MainActivity.kt
│       ├── assets/
│       │   ├── config.json       # 运行时配置
│       │   └── tools.json        # 工具定义
│       └── AndroidManifest.xml
│
├── agent-android/                # Android 适配层
│   ├── build.gradle
│   └── src/main/java/.../
│       ├── AndroidToolManager.java
│       ├── WorkspaceManager.java
│       └── tools/                # Android 工具实现
│           ├── ShowToastTool.java
│           ├── DisplayNotificationTool.java
│           └── ...
│
├── agent-core/                   # 核心库
│   ├── build.gradle
│   ├── src/main/
│   │   ├── cpp/                 # C++ JNI 代码
│   │   └── java/                # Java 接口
│   └── src/main/jni/
│
├── config.json.template          # 配置模板
├── build.gradle                  # 根构建文件
└── settings.gradle
```

## 扩展指南

### 添加新的 Android 工具

参考 [Android 工具扩展指南](docs/android-tool-extension.md)

### 配置说明

`config.json` 配置项：

| 字段 | 类型 | 说明 |
|------|------|------|
| provider.apiKey | string | LLM API Key |
| provider.baseUrl | string | API 基础 URL |
| agent.model | string | 使用的模型名称 |

### 工具列表

内置工具：

| 工具 | 功能 |
|------|------|
| show_toast | 显示 Toast 提示 |
| display_notification | 显示通知 |
| read_clipboard | 读取剪贴板 |
| take_screenshot | 截图 |
| search_contacts | 搜索联系人 |
| send_im_message | 发送即时消息 |

## 开发指南

### 模块职责边界

- **app**: 只负责 UI 展示和初始化，不包含任何业务逻辑
- **agent-android**: 包含所有 Android 平台相关代码，不包含 C++
- **agent-core**: 纯逻辑层，无 Android 依赖

### 代码风格

- Java 代码使用驼峰命名
- 包名: `com.hh.agent.{module}`
- 遵循 Android 编码规范

## 许可证

MIT License
