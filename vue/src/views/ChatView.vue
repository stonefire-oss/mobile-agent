<template>
  <div class="chat-view">
    <header class="chat-header">
      <h1>Nanobot</h1>
      <button class="clear-btn" @click="handleClear">清空</button>
    </header>

    <div class="messages-container" ref="messagesContainer">
      <div v-if="chatStore.isLoading && !chatStore.messages.length" class="loading">
        加载中...
      </div>

      <template v-else>
        <MessageBubble
          v-for="msg in chatStore.messages"
          :key="msg.id"
          :message="msg"
        />
      </template>

      <ThinkingIndicator v-if="chatStore.isThinking" />
    </div>

    <div v-if="chatStore.error" class="error-toast">
      {{ chatStore.error }}
    </div>

    <InputBar
      :disabled="chatStore.isLoading"
      @send="handleSend"
    />
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref, nextTick, watch } from 'vue'
import { useChatStore } from '../stores/chat'
import MessageBubble from '../components/MessageBubble.vue'
import ThinkingIndicator from '../components/ThinkingIndicator.vue'
import InputBar from '../components/InputBar.vue'

const chatStore = useChatStore()
const messagesContainer = ref<HTMLElement | null>(null)

onMounted(() => {
  chatStore.loadMessages()
})

async function handleSend(content: string) {
  await chatStore.sendMessage(content)
  await nextTick()
  scrollToBottom()
}

function handleClear() {
  chatStore.clearMessages()
}

function scrollToBottom() {
  if (messagesContainer.value) {
    messagesContainer.value.scrollTop = messagesContainer.value.scrollHeight
  }
}

watch(
  () => chatStore.messages.length,
  () => nextTick(scrollToBottom)
)
</script>

<style scoped>
.chat-view {
  display: flex;
  flex-direction: column;
  height: 100vh;
  background: var(--bg-color);
}

.chat-header {
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 12px 16px;
  background: var(--primary-color);
  color: white;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  position: relative;
}

.chat-header h1 {
  font-size: 20px;
  margin: 0;
  font-weight: 500;
}

.clear-btn {
  position: absolute;
  right: 16px;
  padding: 6px 12px;
  background: rgba(255, 255, 255, 0.2);
  color: white;
  border: none;
  border-radius: 4px;
  font-size: 14px;
  cursor: pointer;
}

.clear-btn:active {
  background: rgba(255, 255, 255, 0.3);
}

.messages-container {
  flex: 1;
  overflow-y: auto;
  padding: 16px;
  -webkit-overflow-scrolling: touch;
}

.loading {
  text-align: center;
  color: var(--text-secondary);
  padding: 20px;
}

.error-toast {
  padding: 8px 16px;
  background: #ff4d4f;
  color: white;
  text-align: center;
  font-size: 14px;
}
</style>
