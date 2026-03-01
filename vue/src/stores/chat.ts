import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import type { Message } from '../types/message'
import { nanobotApi } from '../api/nanobot'

export const useChatStore = defineStore('chat', () => {
  const currentSessionKey = ref('http:default')
  const messages = ref<Message[]>([])
  const isLoading = ref(false)
  const isThinking = ref(false)
  const error = ref<string | null>(null)

  const userMessages = computed(() =>
    messages.value.filter(m => m.role === 'user')
  )

  const assistantMessages = computed(() =>
    messages.value.filter(m => m.role === 'assistant')
  )

  async function loadMessages() {
    isLoading.value = true
    error.value = null

    try {
      const history = nanobotApi.getHistory(currentSessionKey.value, 50)
      messages.value = history
    } catch (e) {
      error.value = e instanceof Error ? e.message : 'Failed to load messages'
    } finally {
      isLoading.value = false
    }
  }

  async function sendMessage(content: string) {
    if (!content.trim()) return

    error.value = null

    const userMessage: Message = {
      id: crypto.randomUUID(),
      role: 'user',
      content: content.trim(),
      timestamp: Date.now()
    }
    messages.value.push(userMessage)

    isThinking.value = true
    isLoading.value = true

    try {
      const response = await nanobotApi.sendMessage(content, currentSessionKey.value)
      messages.value.push(response)
    } catch (e) {
      error.value = e instanceof Error ? e.message : 'Failed to send message'
    } finally {
      isThinking.value = false
      isLoading.value = false
    }
  }

  function clearMessages() {
    messages.value = []
    error.value = null
  }

  function setSessionKey(key: string) {
    currentSessionKey.value = key
    loadMessages()
  }

  return {
    currentSessionKey,
    messages,
    isLoading,
    isThinking,
    error,
    userMessages,
    assistantMessages,
    loadMessages,
    sendMessage,
    clearMessages,
    setSessionKey
  }
})
