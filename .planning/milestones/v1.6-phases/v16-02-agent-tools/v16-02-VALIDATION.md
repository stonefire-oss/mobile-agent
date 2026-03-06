---
phase: v16-02
slug: agent-tools
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-03-06
---

# Phase v16-02 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Catch2 3.x (C++) + Android JUnit (Java) |
| **Config file** | cxxplatform/tests/CMakeLists.txt |
| **Quick run command** | `ctest -R tool_registry -V` |
| **Full suite command** | `ctest --output-on-failure` |
| **Estimated runtime** | ~60 seconds |

---

## Sampling Rate

- **After every task commit:** Manual test im_sender skill flow
- **After every plan wave:** Full C++ test suite (`ctest --output-on-failure`)
- **Before `/gsd:verify-work:** Full C++ test suite must be green
- **Max feedback latency:** 60 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|-----------|-------------------|-------------|--------|
| v16-02-01-01 | 01 | 1 | CALL-01 | unit | `ctest -R tool_registry -V` | ✅ | ⬜ pending |
| v16-02-01-02 | 01 | 1 | CALL-01 | integration | Manual test via chat | No - new | ⬜ pending |
| v16-02-02-01 | 02 | 1 | CALL-02 | integration | Manual test with im_sender | No - new | ⬜ pending |
| v16-02-02-02 | 02 | 2 | CALL-03 | unit | Check tool result format | Yes - existing | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `agent/src/main/java/com/hh/agent/library/tools/SearchContactsTool.java` — Mock contact search tool
- [ ] `agent/src/main/java/com/hh/agent/library/tools/SendImMessageTool.java` — Mock IM send tool
- [ ] `agent/src/main/assets/workspace/skills/im_sender/SKILL.md` — Test skill definition
- [ ] `app/src/main/assets/tools.json` — Add search_contacts and send_im_message to enum

*If none: "Existing infrastructure covers all phase requirements."*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Skill triggers Android Tool | CALL-01 | Requires chat UI interaction | 1. Build APK 2. Send "给张三发消息说我到了" 3. Verify search and send flow |
| Multi-step tool chain | CALL-02 | Full skill workflow | 1. Load im_sender skill 2. Send message request 3. Verify multi-step chain executes |
| Tool result returned to Agent | CALL-03 | End-to-end validation | 1. Complete tool call 2. Check LLM receives result |

*If none: "All phase behaviors have automated verification."*

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 60s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
