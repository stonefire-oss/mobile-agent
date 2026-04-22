# Mobile Agent 模块关系图（PPT 版）

已提供可直接用于演示文稿的矢量图：

- `docs/architecture/mobile-agent-module-map.svg`

## 图中关系来源

主要依据 `settings.gradle` 与各模块 `build.gradle` 的 `implementation project(...)` 依赖：

- `app` -> `agent-core` / `agent-android` / `agent-screen-vision` / `benchmark-android`
- `benchmark-android` -> `agent-android`
- `agent-screen-vision` -> `agent-android`
- `agent-android` -> `agent-core`

## 在 PPT 中使用

1. 直接插入 SVG（推荐）
2. 按模块做逐步动画：
   - 第一步：`app`
   - 第二步：`agent-android`
   - 第三步：`agent-core` + `C++ Native`
   - 第四步：补充 `agent-screen-vision` 与 `benchmark-android`
