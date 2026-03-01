import http from './http'
import type { Message, Session } from '../types/message'
import type { ChatRequest, ChatResponse } from '../types/api'

const sessions = new Map<string, Session>()

export const nanobotApi = {
  createSession(channel: string, chatId: string): Session {
    const key = `${channel}:${chatId}`
    const session: Session = {
      key,
      messages: [],
      createdAt: Date.now(),
      updatedAt: Date.now()
    }
    sessions.set(key, session)
    return session
  },

  getSession(sessionKey: string): Session | null {
    return sessions.get(sessionKey) || null
  },

  async sendMessage(content: string, sessionKey: string): Promise<Message> {
    let session = sessions.get(sessionKey)
    if (!session) {
      session = nanobotApi.createSession('http', sessionKey)
    }

    const userMessage: Message = {
      id: crypto.randomUUID(),
      role: 'user',
      content,
      timestamp: Date.now()
    }
    session.messages.push(userMessage)

    try {
      const request: ChatRequest = {
        message: content,
        session_key: sessionKey
      }

      const response = await http.post<ChatResponse>('/api/chat', request)

      if (!response.data.success) {
        throw new Error(response.data.error || 'Unknown error')
      }

      const assistantMessage: Message = {
        id: crypto.randomUUID(),
        role: 'assistant',
        content: response.data.response,
        timestamp: Date.now()
      }
      session.messages.push(assistantMessage)

      return assistantMessage
    } catch (error) {
      const errorMessage: Message = {
        id: crypto.randomUUID(),
        role: 'assistant',
        content: `Error: ${error instanceof Error ? error.message : 'Unknown error'}`,
        timestamp: Date.now()
      }
      session.messages.push(errorMessage)
      return errorMessage
    }
  },

  getHistory(sessionKey: string, maxMessages: number = 50): Message[] {
    const session = sessions.get(sessionKey)
    if (!session) return []

    const messages = session.messages
    if (messages.length <= maxMessages) return messages

    return messages.slice(-maxMessages)
  },

  async isAvailable(): Promise<boolean> {
    try {
      const response = await http.get('/health')
      return response.status === 200
    } catch {
      return false
    }
  }
}
