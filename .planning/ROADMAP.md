# Roadmap: Mobile Agent

## Milestones

- 📋 **v1.5 LLM → Android 调用管道** — Phases 5-9 (in progress)
- ✅ **v1.4 Android Tools 通道** — Phases 1-4 (shipped 2026-03-05)

## v1.5 Roadmap

| Phase | Goal | Requirements | Success Criteria |
|-------|------|--------------|------------------|
| 5 | tools.json 迁移到 inputSchema | PIPE-01 | tools.json 使用 inputSchema 格式 |
| 6 | 通用 call_android_tool | PIPE-02, MODE-01 | C++ 可调用 Java 层任意工具 |
| 7 | 验证工具框架 | PIPE-03 | show_toast 通过新管道调用成功 |
| 8 | Skills 加载机制 | SKILL-01 | Android Skills 从 workspace 加载 |
| 9 | Skill 编排示例 | SKILL-02, SKILL-03 | 示例 Skill 触发工具调用 |

---

## Progress

| Phase | Milestone | Status |
|-------|-----------|--------|
| 1-4 | v1.4 | ✓ Complete |
| 5-9 | v1.5 | ○ Not started |

---

*Roadmap updated: 2026-03-05*
