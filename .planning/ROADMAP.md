# Roadmap: 手机上的 AI Agent

## Milestones

- ✅ **v2.0 接入真实项目** — Phases v20-01 to v20-04 (shipped 2026-03-09)
- ✅ **v1.6 自定义 Skills 验证** — Phases v16-01 to v16-02 (shipped 2026-03-06)
- ✅ **v1.5 LLM → Android 调用管道** — Phases v15-01 to v15-03 (shipped 2026-03-05)
- ✅ **v1.4 Android Tools 通道** — Phases 1-4 (shipped 2026-03-05)

---

## v2.0 (Shipped)

<details>
<summary>✅ v2.0 接入真实项目 (Phases v20-01 to v20-04) — SHIPPED 2026-03-09</summary>

- [x] Phase v20-01: 代码清理 (done as part of v20-02)
- [x] Phase v20-02: 重命名 (1/1 plans) — completed 2026-03-06
- [x] Phase v20-03: 代码迁移 (3/3 plans) — completed 2026-03-09
- [x] Phase v20-04: 验证 (1/1 plans) — completed 2026-03-09

**Key Deliverables:**
- Nanobot → MobileAgent 重命名完成
- 平台工具从 agent 迁移到 app 模块
- agent 模块实现纯 Java AAR（无 Android 依赖）
- Agent API 改为接收 JSON 字符串
- APK 编译成功，功能验证通过

**Known Gaps (Tech Debt):**
- CLEAN-01~CLEAN-05: 代码清理未正式验证（实际已完成）

</details>

---

## v1.6 (Shipped)

<details>
<summary>✅ v1.6 自定义 Skills 验证 (Phases v16-01 to v16-02) — SHIPPED 2026-03-06</summary>

- [x] Phase v16-01: 自定义 Skills 机制 (1/1 plans) — completed 2026-03-06
- [x] Phase v16-02: Agent 调用 Tools (1/1 plans) — completed 2026-03-06

**Key Deliverables:**
- SKILL.md 格式定义和 YAML frontmatter 解析
- C++ 层 SkillLoader 加载机制
- search_contacts / send_im_message Android Tools
- im_sender 测试 Skill

</details>

---

## Progress

| Milestone | Phase Range | Status | Completed |
|-----------|-------------|--------|-----------|
| v2.0 | v20-01 to v20-04 | ✓ Complete | 2026-03-09 |
| v1.6 | v16-01 to v16-02 | ✓ Complete | 2026-03-06 |
| v1.5 | v15-01 to v15-03 | ✓ Complete | 2026-03-05 |
| v1.4 | 1-4 | ✓ Complete | 2026-03-05 |

---

*Roadmap updated: 2026-03-09*
