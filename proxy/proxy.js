import express from 'express'
import { WebSocketServer } from 'ws'
import http from 'http'
import net from 'net'
import dotenv from 'dotenv'
import path from 'path'

dotenv.config()

const PORT = Number(process.env.PORT || 10000)
const TCP_HOST = process.env.TCP_HOST
const TCP_PORT = Number(process.env.TCP_PORT)

if (!TCP_HOST || !TCP_PORT) {
    console.error('TCP_HOST або TCP_PORT не задані')
    process.exit(1)
}

const app = express()

// Serve frontend static build
const distPath = path.resolve(process.cwd(), '../frontend/dist')
app.use(express.static(distPath))

// For SPA fallback
app.get('*', (req, res) => {
    res.sendFile(path.join(distPath, 'index.html'))
})

const server = http.createServer(app)

// WebSocket server on same HTTP server, path /ws
const wss = new WebSocketServer({ server, path: '/ws' })

wss.on('connection', (ws) => {
    console.log('[proxy] New WS connection — creating TCP connection to', TCP_HOST + ':' + TCP_PORT)
    const tcp = net.createConnection({ host: TCP_HOST, port: TCP_PORT })

    tcp.on('connect', () => {
        console.log('[proxy] TCP connected')
    })

    tcp.on('error', (err) => {
        console.error('[proxy] TCP error:', err.message)
        if (ws.readyState === ws.OPEN) {
            ws.send(`Помилка: ${err.message}`)
            ws.close()
        }
    })

    ws.on('message', (data) => {
        if (!tcp.destroyed) {
            tcp.write(Buffer.from(String(data), 'utf8'))
        }
    })

    ws.on('close', () => {
        if (!tcp.destroyed) tcp.end()
    })

    ws.on('error', (err) => {
        console.error('[proxy] WS error:', err && err.message)
        if (!tcp.destroyed) tcp.destroy()
    })

    tcp.on('data', (chunk) => {
        const txt = chunk.toString('utf8')
        if (ws.readyState === ws.OPEN) ws.send(txt)
    })

    tcp.on('close', () => {
        if (ws.readyState === ws.OPEN) ws.close()
    })
})

server.listen(PORT, '0.0.0.0', () => {
    console.log(`HTTP + WS server listening on ${PORT}`)
    console.log(`Підключення до TCP сервера: ${TCP_HOST}:${TCP_PORT}`)
})