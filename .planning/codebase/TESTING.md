# Testing Patterns

**Analysis Date:** 2026-03-03

## Test Framework

**Runner:**
- JUnit 4.13.2 - configured in `app/build.gradle`
- Android unit tests (local JVM tests)

**Assertion Library:**
- JUnit built-in assertions: `assertEquals()`, `assertNotNull()`, `assertTrue()`

**Run Commands:**
```bash
./gradlew test                  # Run all unit tests
./gradlew testDebugUnitTest    # Run debug variant unit tests
./gradlew testReleaseUnitTest  # Run release variant unit tests
```

## Test File Organization

**Location:**
- Co-located with source in `app/src/test/java/` directory
- Mirrors source package structure

**Naming:**
- Source file: `MessageAdapter.java`
- Test file: `MessageAdapterTest.java`
- Pattern: `<ClassName>Test.java`

**Structure:**
```
app/src/test/java/com/hh/agent/
├── contract/
│   └── MainContractTest.java
├── presenter/
│   └── MainPresenterTest.java
└── ui/
    └── MessageAdapterTest.java
```

## Test Structure

**Suite Organization:**
```java
package com.hh.agent.ui;

import com.hh.agent.lib.model.Message;
import org.junit.Test;
import java.util.ArrayList;
import java.util.List;
import static org.junit.Assert.*;

/**
 * MessageAdapter 的单元测试
 */
public class MessageAdapterTest {

    @Test
    public void testSetMessages() {
        // Test implementation
    }

    @Test
    public void testMessageRoles() {
        // Test implementation
    }
}
```

**Patterns:**
- `@Test` annotation for each test method
- Descriptive test method names: `test<WhatIsBeingTested>`
- No explicit setup/teardown methods used
- Chinese comments for test descriptions

## Mocking

**Framework:** Manual mocking (no mocking library)

**Patterns:**
- Anonymous inner class for View interface:
```java
MainContract.View mockView = new MainContract.View() {
    @Override
    public void onMessagesLoaded(List<Message> messages) {
        receivedMessages.addAll(messages);
    }

    @Override
    public void onMessageReceived(Message message) {
        receivedMessages.add(message);
    }

    @Override
    public void onError(String error) {
    }

    @Override
    public void showLoading() {
    }

    @Override
    public void hideLoading() {
    }
};
```

**What to Mock:**
- View interfaces (MVP pattern)
- API interfaces (`NanobotApi`)
- External dependencies

**What NOT to Mock:**
- Model objects (Message, Session) - tested directly
- DTOs - tested with real data

## Fixtures and Factories

**Test Data:**
```java
List<Message> messages = new ArrayList<>();
Message msg1 = new Message();
msg1.setId("1");
msg1.setRole("user");
msg1.setContent("Hello");
messages.add(msg1);
```

**Location:**
- Inline fixtures within test methods
- No separate fixture files

**Helper Methods:**
- None currently - tests create data inline

## Coverage

**Requirements:** None enforced

**View Coverage:**
```bash
./gradlew testDebugUnitTest --info 2>&1 | grep "tests completed"
```

**Note:** No JaCoCo or code coverage plugin configured.

## Test Types

**Unit Tests:**
- Test model objects: Message, Session
- Test MVP contract interfaces
- Test presenter creation and API switching
- Test adapter data manipulation (setMessages, addMessage)

**Integration Tests:**
- Not present in current codebase

**E2E Tests:**
- Not present - would require Android instrumentation

## Test Limitations

**Android-Specific Challenges:**
- Presenter tests limited because `Handler` and `Looper` require Android environment
- From `MainPresenterTest.java`:
  ```java
  @Test
  public void testPresenterCreation() {
      // 注意：在非 Android 环境下，Handler/Looper 不可用，所以这里只验证类结构
      assertNotNull(MainPresenter.class);
  }
  ```
- RecyclerView adapter tests limited - cannot test full ViewHolder binding

**Current Test Coverage:**
- `MessageAdapterTest.java`: 3 tests (setMessages, messageRoles, messageContent)
- `MainContractTest.java`: 3 tests (viewInterface, presenterInterface, mockView)
- `MainPresenterTest.java`: 4 tests (apiTypeEnum, presenterCreation, mockApiCreation, mockView)

## Common Patterns

**Async Testing:**
- Not tested - async operations in Presenter use Handler to post to main thread
- Tests verify synchronous aspects only

**Error Testing:**
- Tests verify error handling in mock implementations
- Example: Network errors caught and converted to error messages in `HttpNanobotApi`

## Test Best Practices Observed

**Good Practices:**
- Tests use JUnit 4 standard annotations
- Assertions are clear and descriptive
- Test method names describe expected behavior

**Areas for Improvement:**
- No mocking library (Mockito) - manual mocking verbose
- No parameterized tests
- No test coverage enforcement
- Limited to unit tests only
- Tests in `app/src/androidTest/` (instrumented tests) not present

---

*Testing analysis: 2026-03-03*
