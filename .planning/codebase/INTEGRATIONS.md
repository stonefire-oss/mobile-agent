# External Integrations

**Analysis Date:** 2026-03-03

## APIs & External Services

**Nanobot Chat Service:**
- Primary external API for chat functionality
- SDK/Client: Custom `HttpNanobotApi` implementation using OkHttp 4.12.0
- Base URL: `http://localhost:18791` (configurable via `NanobotConfig`)
- Endpoints:
  - `POST /api/chat` - Send message and receive response
  - `GET /health` - Health check
- Connection: Configurable via `NanobotConfig.java`
  - Default: `http://localhost:18791`
  - Connect timeout: 30 seconds
  - Read timeout: 60 seconds

**API Request/Response Format:**
```json
// Request (ChatRequest)
{
  "message": "user message",
  "session_key": "channel:chatId"
}

// Response (ChatResponse)
{
  "success": true,
  "response": "assistant reply",
  "session_key": "channel:chatId",
  "error": null
}
```

## Data Storage

**In-Memory:**
- Session and message storage: In-memory `ConcurrentHashMap` in `HttpNanobotApi`
- No persistent database
- Session key format: `channel:chatId` (e.g., `http:default`)

**No External Database:**
- All data is ephemeral and lost on app restart
- Session history maintained per session key in memory

## Authentication & Identity

**None:**
- No authentication or identity provider integrated
- No user management system
- Session key used as lightweight session identifier

## Monitoring & Observability

**None:**
- No error tracking service (e.g., Crashlytics, Sentry)
- No analytics or monitoring
- Native Android logging only (Logcat)

**Logging Approach:**
- Android `Log` class for debug logging
- Native C++ logging via Android log library

## CI/CD & Deployment

**Build System:**
- Gradle 8.12.1 with AGP 8.3.2

**Hosting:**
- Not applicable (Android app distributed via APK)

**CI Pipeline:**
- None detected

## Environment Configuration

**Required Configuration:**
- Nanobot service must be running at configured base URL
- For emulator: `adb reverse tcp:18791 tcp:18791` to forward requests
- Network security config allows localhost cleartext traffic

**Optional Configuration:**
- Custom base URL via `NanobotConfig` constructor
- Custom timeouts via `NanobotConfig` constructor
- API type switching (MOCK vs HTTP) in `MainPresenter.ApiType`

## Webhooks & Callbacks

**Incoming:**
- None - App is the client, not a server

**Outgoing:**
- HTTP POST to Nanobot service for chat messages
- HTTP GET to Nanobot health endpoint

## Key Integration Points

**NanobotApi Interface:**
- `lib/src/main/java/com/hh/agent/lib/api/NanobotApi.java`
- Two implementations:
  - `HttpNanobotApi` - Real HTTP calls to Nanobot service
  - `MockNanobotApi` - Mock responses for testing/development

**Configuration:**
- `lib/src/main/java/com/hh/agent/lib/config/NanobotConfig.java`
- HTTP client: `lib/src/main/java/com/hh/agent/lib/http/HttpNanobotApi.java`

---

*Integration audit: 2026-03-03*
