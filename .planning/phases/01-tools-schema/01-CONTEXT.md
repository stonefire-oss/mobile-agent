# Phase 1: tools.json 迁移到 inputSchema - Context

**Gathered:** 2026-03-05
**Status:** Ready for planning

<domain>
## Phase Boundary

将工具定义从自定义 params 格式迁移到 OpenAI 风格的 inputSchema (JSON Schema) 格式，确保 LLM 可以正确理解工具参数结构。

</domain>

<decisions>
## Implementation Decisions

### 格式风格
- 使用 OpenAI 格式: `name`, `description`, `parameters` (JSON Schema)
- 顶级结构: `{ name, description, parameters }`

### 参数 required 标记
- 使用 JSON Schema 标准方式: `parameters.required` 数组列出必填参数
- 不在每个参数内部单独标记 required

### 参数默认值
- 支持 `default` 字段
- 在 `properties` 中每个参数使用 `"default": value` 指定默认值

### 参数类型
- 使用 JSON Schema 标准类型: `string`, `integer`, `number`, `boolean`, `object`, `array`, `null`
- 不扩展额外类型

### 迁移后格式示例
```json
{
  "tools": [
    {
      "name": "show_toast",
      "description": "显示 Toast 消息到屏幕",
      "parameters": {
        "type": "object",
        "properties": {
          "message": {
            "type": "string",
            "description": "要显示的消息内容"
          },
          "duration": {
            "type": "integer",
            "description": "显示时长(毫秒)",
            "default": 2000
          }
        },
        "required": ["message"]
      }
    }
  ]
}
```

### Claude's Discretion
- C++ 解析实现的细节
- 迁移策略（一次性替换 vs 渐进式）
- 向后兼容性处理

</decisions>

<specifics>
## Specific Ideas

- 保持与 OpenAI Chat Completions API 的 tool calling 格式兼容
- 便于 LLM 理解工具参数结构

</specifics>

<deferred>
## Deferred Ideas

- 迁移策略讨论
- C++ 解析实现细节
- 向后兼容性

</deferred>

---

*Phase: 01-tools-schema*
*Context gathered: 2026-03-05*
