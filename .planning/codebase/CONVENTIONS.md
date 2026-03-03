# Coding Conventions

**Analysis Date:** 2026-03-03

## Naming Patterns

**Files:**
- Java source files: PascalCase - `MainActivity.java`, `MessageAdapter.java`, `NanobotApi.java`
- Layout XML files: snake_case - `activity_main.xml`, `item_message.xml`
- Drawable files: snake_case - `bg_send_button.xml`, `ic_launcher.xml`

**Functions:**
- Methods: camelCase - `initViews()`, `loadMessages()`, `sendMessage()`, `onCreate()`
- Boolean getters: `is` prefix - `isAvailable()`, `isSuccess()`
- Private methods: camelCase with descriptive names

**Variables:**
- Fields: camelCase - `rvMessages`, `etMessage`, `btnSend`
- Local variables: camelCase - `content`, `messages`, `sessionKey`
- Constants: UPPER_SNAKE_CASE - `VIEW_TYPE_USER`, `VIEW_TYPE_ASSISTANT`

**Types:**
- Classes: PascalCase - `MainPresenter`, `MessageAdapter`, `NanobotConfig`
- Interfaces: PascalCase - `NanobotApi`, `MainContract.View`
- Enums: PascalCase - `ApiType.MOCK`, `ApiType.HTTP`
- Packages: lowercase - `com.hh.agent.lib.api`, `com.hh.agent.contract`

## Code Style

**Formatting:**
- Standard Java code conventions
- 4-space indentation
- Opening brace on same line: `public class MainActivity {`
- Line length: Not strictly enforced, but reasonable (< 120 chars)

**Linting:**
- No explicit linting tool configured
- Relies on Android Studio/Gradle built-in checks

**Java Version:**
- Java 21 - specified in `app/build.gradle`
- `sourceCompatibility` and `targetCompatibility` set to `JavaVersion.VERSION_21`

## Import Organization

**Order:**
1. Android framework imports - `android.os.*`, `android.widget.*`
2. AndroidX imports - `androidx.appcompat.*`, `androidx.recyclerview.*`
3. Project internal imports - `com.hh.agent.*`
4. Third-party imports - `com.google.gson.*`, `io.noties.markwon.*`
5. Java standard library - `java.util.*`, `java.io.*`

**Example:**
```java
import android.os.Bundle;
import android.widget.EditText;
import androidx.appcompat.app.AppCompatActivity;
import com.hh.agent.contract.MainContract;
import com.hh.agent.lib.model.Message;
import com.google.gson.Gson;
import java.util.List;
```

## Error Handling

**Patterns:**
- Try-catch blocks for network operations in `HttpNanobotApi.sendMessage()`
- Catch blocks return error messages instead of throwing:
  ```java
  } catch (IOException e) {
      Message errorMsg = new Message();
      errorMsg.setRole("assistant");
      errorMsg.setContent("Error: " + e.getMessage());
      return errorMsg;
  }
  ```
- Presenter uses callbacks to notify View of errors:
  ```java
  mainHandler.post(() -> view.onError("加载消息失败: " + e.getMessage()));
  ```

## Logging

**Framework:** Android Log (android.util.Log)
- Not heavily used in current codebase
- Console output via Toast for UI feedback

**Patterns:**
- UI errors: `Toast.makeText(this, error, Toast.LENGTH_SHORT).show()`
- No systematic logging throughout business logic

## Comments

**When to Comment:**
- Javadoc for public APIs and interfaces:
  ```java
  /**
   * 发送消息（同步）
   *
   * @param content    消息内容
   * @param sessionKey 会话密钥
   * @return 机器人回复消息
   */
  Message sendMessage(String content, String sessionKey);
  ```
- Chinese comments for business logic explanations
- Inline comments for complex logic

**JSDoc/TSDoc:**
- Javadoc style (`/** ... */`) for public methods
- Parameter descriptions with `@param`
- Return value descriptions with `@return`

## Function Design

**Size:** Generally small, single-responsibility methods
- Example: `initViews()` (~20 lines), `sendMessage()` (~50 lines)

**Parameters:**
- Clear, descriptive parameter names
- Type-specific: `String content`, `String sessionKey`, `int maxMessages`
- Maximum 3-4 parameters in most cases

**Return Values:**
- Specific return types: `Message`, `List<Message>`, `Session`, `void`
- Collections return empty instead of null: `Collections.emptyList()`

## Module Design

**Exports:**
- Public classes: Activities, Presenters, Adapters, API interfaces
- Package-private classes: ViewHolders (nested in adapters)

**Barrel Files:**
- Not used - imports use full package paths

**MVP Pattern Implementation:**
- Contract interface in separate package: `com.hh.agent.contract.MainContract`
- View interface: UI update callbacks
- Presenter interface: Business logic methods
- Concrete implementations: `MainPresenter`, Activity implements View

## Model/Entity Patterns

**POJO Structure:**
- Private fields
- Getters and setters for all fields
- Constructors (default + parameterized)
- Timestamps: `System.currentTimeMillis()`

**Example from `Message.java`:**
```java
public class Message {
    private String id;
    private String role;
    private String content;
    private long timestamp;

    public Message() {
        this.timestamp = System.currentTimeMillis();
    }

    public String getRole() { return role; }
    public void setRole(String role) { this.role = role; }
    // ... other getters/setters
}
```

## DTO Patterns

**DTOs (Data Transfer Objects):**
- Use `@SerializedName` for Gson JSON mapping
- Default constructor required for Gson
- Example from `ChatRequest.java`:
  ```java
  public class ChatRequest {
      @SerializedName("message")
      private String message;

      @SerializedName("session_key")
      private String sessionKey;
  }
  ```

---

*Convention analysis: 2026-03-03*
