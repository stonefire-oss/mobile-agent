<template>
  <div class="input-bar">
    <input
      v-model="inputText"
      type="text"
      placeholder="发送消息..."
      :disabled="disabled"
      @keyup.enter="handleSend"
    />
    <button
      class="send-btn"
      :disabled="disabled || !inputText.trim()"
      @click="handleSend"
    >
      <svg viewBox="0 0 24 24" width="24" height="24" fill="currentColor">
        <path d="M2.01 21L23 12 2.01 3 2 10l15 2-15 2z"/>
      </svg>
    </button>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'

interface Props {
  disabled?: boolean
}

interface Emits {
  (e: 'send', content: string): void
}

const props = withDefaults(defineProps<Props>(), {
  disabled: false
})

const emit = defineEmits<Emits>()

const inputText = ref('')

function handleSend() {
  const text = inputText.value.trim()
  if (!text || props.disabled) return

  emit('send', text)
  inputText.value = ''
}
</script>

<style scoped>
.input-bar {
  display: flex;
  gap: 8px;
  padding: 10px 12px;
  background: #f7f7f7;
  border-top: 1px solid #d9d9d9;
}

.input-bar input {
  flex: 1;
  padding: 10px 14px;
  border: 1px solid #d9d9d9;
  border-radius: 20px;
  font-size: 14px;
  outline: none;
  background: white;
}

.input-bar input:focus {
  border-color: var(--primary-color);
}

.input-bar input:disabled {
  background: #f5f5f5;
}

.send-btn {
  width: 48px;
  height: 48px;
  padding: 0;
  background: var(--primary-color);
  color: white;
  border: none;
  border-radius: 50%;
  font-size: 14px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
}

.send-btn:disabled {
  background: #b0b0b0;
  cursor: not-allowed;
}

.send-btn:not(:disabled):hover {
  background: var(--primary-dark);
}
</style>
