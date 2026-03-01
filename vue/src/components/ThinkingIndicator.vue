<template>
  <div class="thinking-indicator">
    <span class="dot"></span>
    <span class="dot"></span>
    <span class="dot"></span>
  </div>
</template>

<style scoped>
.thinking-indicator {
  display: flex;
  gap: 4px;
  padding: 12px 16px;
  background: var(--assistant-bubble);
  border: 1px solid var(--border-color);
  border-radius: 12px;
  margin-left: 8px;
  margin-bottom: 12px;
  width: fit-content;
}

.dot {
  width: 8px;
  height: 8px;
  background: var(--text-secondary);
  border-radius: 50%;
  animation: bounce 1.4s infinite ease-in-out both;
}

.dot:nth-child(1) {
  animation-delay: -0.32s;
}

.dot:nth-child(2) {
  animation-delay: -0.16s;
}

@keyframes bounce {
  0%, 80%, 100% {
    transform: scale(0);
  }
  40% {
    transform: scale(1);
  }
}
</style>
