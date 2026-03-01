import axios from 'axios'
import { defaultConfig } from '../types/api'

const http = axios.create({
  baseURL: defaultConfig.baseUrl,
  timeout: defaultConfig.readTimeout,
  headers: {
    'Content-Type': 'application/json'
  }
})

http.interceptors.response.use(
  response => response,
  error => {
    console.error('API Error:', error)
    return Promise.reject(error)
  }
)

export default http
