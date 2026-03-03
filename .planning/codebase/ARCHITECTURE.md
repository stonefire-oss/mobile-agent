# Architecture

**Analysis Date:** 2026-03-03

## Pattern Overview

**Overall:** Hybrid MVP + Component-based Architecture

This codebase uses a dual architecture pattern:
- **Android (Java):** MVP (Model-View-Presenter) pattern
- **Frontend (Vue):** Component-based architecture with Pinia state management

**Key Characteristics:**
- Clean separation between UI and business logic via MVP Contract interfaces
- API abstraction allowing pluggable implementations (HTTP/Mock)
- Dual frontend: Native Android UI and WebView-hosted Vue frontend
- Session-based chat model with in-memory message storage

## Layers

### Android Layer (app module)

**View Layer:**
- Location: `app/src/main/java/com/hh/agent/`
- Contains: `MainActivity.java`, `VueActivity.java`, `MessageAdapter.java`
- Responsibilities: UI rendering, user input handling, lifecycle management
- Depends on: Presenter, Models
- Used by: Android framework

**Presenter Layer:**
- Location: `app/src/main/java/com/hh/agent/presenter/`
- Contains: `MainPresenter.java`
- Responsibilities: Business logic, API coordination, thread management
- Depends on: Contract interfaces, NanobotApi
- Used by: View (MainActivity)

**Contract Layer:**
- Location: `app/src/main/java/com/hh/agent/contract/`
- Contains: `MainContract.java`
- Responsibilities: Define View and Presenter interfaces for MVP communication
- Depends on: Models (Message)
- Used by: View and Presenter

### Library Layer (lib module)

**API Layer:**
- Location: `lib/src/main/java/com/hh/agent/lib/api/`
- Contains: `NanobotApi.java` (interface)
- Responsibilities: Define contract for chat operations
- Depends on: Models
- Used by: Presenters, HTTP/Mock implementations

**HTTP Implementation:**
- Location: `lib/src/main/java/com/hh/agent/lib/http/`
- Contains: `HttpNanobotApi.java`
- Responsibilities: Execute HTTP calls to Nanobot service
- Depends on: NanobotApi, Config, DTOs, OkHttp
- Used by: Presenters

**Mock Implementation:**
- Location: `lib/src/main/java/com/hh/agent/lib/impl/`
- Contains: `MockNanobotApi.java`
- Responsibilities: Provide mock responses for testing
- Depends on: NanobotApi interface
- Used by: Presenters (switchable via ApiType)

**Model Layer:**
- Location: `lib/src/main/java/com/hh/agent/lib/model/`
- Contains: `Message.java`, `Session.java`
- Responsibilities: Data entities for messages and sessions
- Used by: All layers

**DTO Layer:**
- Location: `lib/src/main/java/com/hh/agent/lib/dto/`
- Contains: `ChatRequest.java`, `ChatResponse.java`
- Responsibilities: Request/response serialization structures
- Used by: HTTP implementation

**Config Layer:**
- Location: `lib/src/main/java/com/hh/agent/lib/config/`
- Contains: `NanobotConfig.java`
- Responsibilities: HTTP endpoint and timeout configuration
- Used by: HTTP implementation

### Vue Frontend Layer

**Store Layer (Pinia):**
- Location: `vue/src/stores/`
- Contains: `chat.ts`
- Responsibilities: State management, business logic coordination
- Depends on: API layer
- Used by: Vue components

**API Layer:**
- Location: `vue/src/api/`
- Contains: `nanobot.ts`, `http.ts`
- Responsibilities: HTTP communication with Nanobot service
- Used by: Store

**Component Layer:**
- Location: `vue/src/components/`
- Contains: `MessageBubble.vue`, `InputBar.vue`, `ThinkingIndicator.vue`
- Responsibilities: Reusable UI components
- Depends on: Types, Stores

**View Layer:**
- Location: `vue/src/views/`
- Contains: `ChatView.vue`
- Responsibilities: Page-level component composition
- Depends on: Components, Stores

## Data Flow

**Message Send Flow:**

1. User enters message in UI (MainActivity/ChatView)
2. View calls Presenter.store method (sendMessage)
3. Presenter creates user message and notifies View
4. Presenter shows "thinking" indicator
5. Presenter executes API call in background thread
6. API implementation (HttpNanobotApi) sends HTTP POST to Nanobot
7. Response converted to Message model
8. Presenter updates View with assistant response
9. View renders message in RecyclerView/List

**State Management:**
- Android: Presenter holds reference to View, uses Handler for UI thread updates
- Vue: Pinia store manages reactive state, components subscribe via composables

## Key Abstractions

**NanobotApi Interface:**
- Purpose: Define chat operations contract
- Examples: `lib/src/main/java/com/hh/agent/lib/api/NanobotApi.java`
- Pattern: Strategy pattern - allows runtime selection of HTTP or Mock implementation

**MainContract Interface:**
- Purpose: Decouple View and Presenter
- Examples: `app/src/main/java/com/hh/agent/contract/MainContract.java`
- Pattern: Passive View - View interface only, no Presenter reference in View

**Session Management:**
- Purpose: Group messages per conversation
- Examples: `lib/src/main/java/com/hh/agent/lib/model/Session.java`
- Pattern: In-memory session store with key-based lookup

## Entry Points

**Android Main Entry:**
- Location: `app/src/main/java/com/hh/agent/MainActivity.java`
- Triggers: App launch from launcher
- Responsibilities: Initialize MVP, load messages, handle user input

**Android Vue Entry:**
- Location: `app/src/main/java/com/hh/agent/VueActivity.java`
- Triggers: Navigate from MainActivity or direct launch
- Responsibilities: Initialize WebView, load Vue assets

**Vue Entry:**
- Location: `vue/src/main.ts`
- Triggers: WebView loads index.html
- Responsibilities: Bootstrap Vue app, mount to DOM

**Android Application:**
- Location: `app/src/main/java/com/hh/agent/LauncherActivity.java`
- Triggers: Splash screen / app start
- Responsibilities: Route to MainActivity or VueActivity

## Error Handling

**Strategy:** Try-catch with fallback messaging

**Patterns:**
- Presenter catches exceptions and displays error via View.onError()
- HTTP implementation catches IOException and returns error Message
- Vue store catches errors and displays in UI
- Network errors show as user-friendly messages

## Cross-Cutting Concerns

**Logging:** Android Log.d/e for debugging WebView and HTTP operations

**Validation:** Input validation in Presenter (empty message check)

**Authentication:** Session-based via sessionKey parameter (no auth token)

---

*Architecture analysis: 2026-03-03*
