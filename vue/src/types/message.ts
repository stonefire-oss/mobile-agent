export interface Message {
  id: string
  role: 'user' | 'assistant' | 'system' | 'thinking'
  content: string
  timestamp: number
}

export interface Session {
  key: string
  messages: Message[]
  createdAt: number
  updatedAt: number
}
