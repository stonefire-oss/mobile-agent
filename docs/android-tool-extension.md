# Android 工具扩展指南

只需 4 步添加新的 Android 工具和 Skills。

## 步骤 1: 创建 Tool 类

在 `app/src/main/java/com/hh/agent/tool/` 下创建新类：

```java
package com.hh.agent.tool;

import com.hh.agent.library.ToolExecutor;
import org.json.JSONObject;

public class MyTool implements ToolExecutor {

    @Override
    public String getName() {
        return "my_tool";
    }

    @Override
    public String execute(JSONObject args) {
        try {
            // 你的实现逻辑
            return "{\"success\": true, \"result\": \"done\"}";
        } catch (Exception e) {
            return "{\"success\": false, \"error\": \"failed\", \"message\": \"" + e.getMessage() + "\"}";
        }
    }

    @Override
    public String getDescription() {
        return "工具描述";
    }

    @Override
    public String getArgsDescription() {
        return "param1: 参数1说明, param2: 参数2说明";
    }

    @Override
    public String getArgsSchema() {
        return "{\"type\":\"object\",\"properties\":{\"param1\":{\"type\":\"string\",\"description\":\"参数1说明\"}},\"required\":[\"param1\"]}";
    }
}
```

**注意：** v2.2 之后，Tool 类放在 app 层而非 agent-android 层，这样可以支持运行时动态注册。

**返回格式约定：**
- 成功: `{"success": true, "result": "..."}`
- 失败: `{"success": false, "error": "error_code", "message": "..."}`

## 步骤 2: 注册 Tool

在 `app/src/main/java/com/hh/agent/LauncherActivity.java` 的 `initializeToolManager()` 方法中添加注册：

```java
private void initializeToolManager() {
    // 创建 AndroidToolManager 实例
    toolManager = new AndroidToolManager(this);

    // 注册自定义 Tool
    toolManager.registerTool(new MyTool());

    // 初始化 AndroidToolManager
    toolManager.initialize();
}
```

**注册方式说明：**
- 使用 `toolManager.registerTool(ToolExecutor)` 方法注册
- 支持运行时动态注册（在应用运行期间添加新 Tool）
- Tool 注册后会主动推送给 Agent，LLM 可以感知到新工具

## 步骤 3: 运行测试

```
./gradlew assembleDebug
adb install app/build/outputs/apk/debug/app-debug.apk
```

## 步骤 4: 添加 Skill（可选）

如果你想让 Agent 执行更复杂的工作流程，可以添加 Skill。

### Skill 放置路径

Skill 放在 `app/src/main/assets/workspace/skills/{skill_name}/` 目录下。

### SKILL.md 格式

每个 Skill 目录需要包含一个 `SKILL.md` 文件，格式如下：

```yaml
---
description: Skill 描述
emoji: "💬"
always: false
---

# Skill 名称

当用户请求xxx时，使用此 Skill 完成工作流程。

## 触发条件

- 用户要求"xxx"
- 用户要求"xxx"

## 工作流程

### 步骤 1: xxx

使用 `tool_name` 工具执行操作。

**输入参数：**
```json
{
  "param": "参数值"
}
```

### 步骤 2: 处理结果

根据结果进行相应处理。

### 步骤 3: 返回结果

向用户确认操作完成。

## 决策规则

- 情况A：处理方式
- 情况B：处理方式

## 示例对话

**用户：** "xxx"

**Agent：**
1. 执行第一步
2. 处理结果
3. 返回结果
```

### Skill 示例

参考 `app/src/main/assets/workspace/skills/im_sender/SKILL.md`，这是一个完整的 Skill 示例，包含：
- 前置元数据（description, emoji, always）
- 触发条件定义
- 工作流程步骤
- 决策规则
- 示例对话

### 自动加载

放置在 `workspace/skills/` 目录下的 Skill 会自动被 Agent 加载，无需额外配置。

## 现有工具参考

| 工具 | 文件 | 说明 |
|------|------|------|
| show_toast | ShowToastTool.java | 显示 Toast 通知 |
| display_notification | DisplayNotificationTool.java | 显示系统通知 |
| read_clipboard | ReadClipboardTool.java | 读取剪贴板内容 |
| take_screenshot | TakeScreenshotTool.java | 截取屏幕截图 |
| search_contacts | SearchContactsTool.java | 搜索联系人 |
| send_im_message | SendImMessageTool.java | 发送即时消息 |

**工具文件位置：** `app/src/main/java/com/hh/agent/tool/`
