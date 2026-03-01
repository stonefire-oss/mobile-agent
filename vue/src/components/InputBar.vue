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
      发送
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
  padding: 12px;
  background: #f5f5f5;
  border-top: 1px solid var(--border-color);
}

.input-bar input {
  flex: 1;
  padding: 10px 14px;
  border: 1px solid var(--border-color);
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
  padding: 10px 20px;
  background: var(--primary-color);
  color: white;
  border: none;
  border-radius: 20px;
  font-size: 14px;
  cursor: pointer;
}

.send-btn:disabled {
  background: #ccc;
  cursor: not-allowed;
}

.send-btn:not(:disabled):hover {
  background: #06ad56;
}
</style>
