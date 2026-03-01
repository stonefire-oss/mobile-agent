<template>
  <div :class="['message-bubble', `message-${role}`]">
    <div class="message-content">
      <div class="markdown-content" v-html="renderedContent"></div>
    </div>
    <div class="message-time">{{ formattedTime }}</div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import MarkdownIt from 'markdown-it'
import hljs from 'highlight.js'
import container from 'markdown-it-container'
import type { Message } from '../types/message'

interface Props {
  message: Message
}

const props = defineProps<Props>()

const role = computed(() => props.message.role)
const content = computed(() => props.message.content)

// LLM 返回的文本统一当作 Markdown 渲染
const md = new MarkdownIt({
  html: false,
  linkify: true,
  typographer: true,
  breaks: true,
  highlight: (str: string, lang: string) => {
    if (lang && hljs.getLanguage(lang)) {
      try {
        return hljs.highlight(str, { language: lang }).value
      } catch {
        return str
      }
    }
    return str
  }
})

// 启用表格
md.renderer.rules.table_open = () => '<table class="md-table">'
md.renderer.rules.thead_open = () => '<thead class="md-thead">'
md.renderer.rules.tbody_open = () => '<tbody class="md-tbody">'
md.renderer.rules.tr_open = () => '<tr class="md-tr">'
md.renderer.rules.th_open = () => '<th class="md-th">'
md.renderer.rules.td_open = () => '<td class="md-td">'

const renderedContent = computed(() => {
  return md.render(content.value)
})

const formattedTime = computed(() => {
  const date = new Date(props.message.timestamp)
  return date.toLocaleTimeString('zh-CN', { hour: '2-digit', minute: '2-digit' })
})
</script>

<style scoped>
.message-bubble {
  max-width: 75%;
  padding: 10px 14px;
  border-radius: 8px;
  margin-bottom: 10px;
  word-break: break-word;
  box-shadow: 0 1px 2px rgba(0,0,0,0.1);
}

.message-user {
  background: var(--user-bubble);
  color: var(--text-primary);
  margin-left: auto;
  margin-right: 8px;
}

.message-assistant {
  background: var(--assistant-bubble);
  color: #333333;
  margin-right: auto;
  margin-left: 8px;
  border: 1px solid #e0e0e0;
}

.message-time {
  font-size: 10px;
  color: var(--text-secondary);
  margin-top: 4px;
  text-align: right;
}

.markdown-content :deep(pre) {
  background: #f5f5f5;
  padding: 8px;
  border-radius: 4px;
  overflow-x: auto;
  margin: 8px 0;
}

.markdown-content :deep(code) {
  font-family: monospace;
  background: #f5f5f5;
  padding: 2px 4px;
  border-radius: 2px;
  font-size: 13px;
}

.markdown-content :deep(p) {
  margin: 8px 0;
}

.markdown-content :deep(ul),
.markdown-content :deep(ol) {
  padding-left: 20px;
  margin: 8px 0;
}

.markdown-content :deep(table) {
  width: 100%;
  border-collapse: collapse;
  margin: 8px 0;
  font-size: 13px;
}

.markdown-content :deep(th),
.markdown-content :deep(td) {
  border: 1px solid var(--border-color);
  padding: 8px 10px;
  text-align: left;
}

.markdown-content :deep(th) {
  background: #f5f5f5;
  font-weight: 600;
}

.markdown-content :deep(tr:nth-child(even)) {
  background: #fafafa;
}

.markdown-content :deep(blockquote) {
  border-left: 3px solid var(--primary-color);
  margin: 8px 0;
  padding: 8px 12px;
  background: #f9f9f9;
  color: #666;
}

.markdown-content :deep(hr) {
  border: none;
  border-top: 1px solid var(--border-color);
  margin: 12px 0;
}

.markdown-content :deep(img) {
  max-width: 100%;
  height: auto;
  border-radius: 4px;
}

.markdown-content :deep(a) {
  color: var(--primary-color);
  text-decoration: none;
}
</style>
