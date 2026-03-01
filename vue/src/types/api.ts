export interface ChatRequest {
  message: string
  session_key: string
}

export interface ChatResponse {
  success: boolean
  response: string
  session_key: string
  error?: string
}

export interface NanobotConfig {
  baseUrl: string
  connectTimeout: number
  readTimeout: number
}

export const defaultConfig: NanobotConfig = {
  baseUrl: 'http://localhost:18791',
  connectTimeout: 30000,
  readTimeout: 60000
}
