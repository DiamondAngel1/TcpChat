import { useCallback, useEffect, useRef, useState } from 'react'

export function useSocket({ onMessage, onOpen, onClose } = {}) {
    const wsRef = useRef(null)
    const connectingRef = useRef(false)
    const [connected, setConnected] = useState(false)

    const connect = useCallback((name) => {
        if (!name || connectingRef.current || wsRef.current) return

        connectingRef.current = true
        const ws = new WebSocket(import.meta.env.VITE_WS_URL)
        wsRef.current = ws

        ws.onopen = () => {
            connectingRef.current = false
            setConnected(true)
            ws.send(name)
            onOpen?.()
        }

        ws.onmessage = (event) => {
            onMessage?.(String(event.data))
        }

        ws.onclose = () => {
            connectingRef.current = false
            setConnected(false)
            wsRef.current = null
            onClose?.()
        }

        ws.onerror = () => {
            connectingRef.current = false
            setConnected(false)
        }
    }, [onMessage, onOpen, onClose])

    const send = useCallback((data) => {
        const ws = wsRef.current
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(data)
            return true
        }
        return false
    }, [])

    const close = useCallback(() => {
        const ws = wsRef.current
        if (!ws) return
        wsRef.current = null
        connectingRef.current = false
        if (ws.readyState === WebSocket.CONNECTING || ws.readyState === WebSocket.OPEN) {
            ws.close()
        }
    }, [])

    useEffect(() => {
        return () => {
            close()
        }
    }, [close])

    return { connect, send, close, connected }
}