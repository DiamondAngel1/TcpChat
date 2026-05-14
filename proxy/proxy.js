import { WebSocketServer } from 'ws'
import net from 'net'
import dotenv from 'dotenv'

dotenv.config()

const WS_PORT = Number(process.env.PORT || 8081)
const TCP_HOST = process.env.TCP_HOST
const TCP_PORT = Number(process.env.TCP_PORT)

if (!TCP_HOST || !TCP_PORT) {
    console.error('TCP_HOST або TCP_PORT не задані')
    process.exit(1)
}

const wss = new WebSocketServer({ port: WS_PORT, host: '0.0.0.0' })

wss.on('connection', (ws) => {
    const tcp = net.createConnection({ host: TCP_HOST, port: TCP_PORT })
    let buffered = ''

    ws.on('message', (data) => {
        if (!tcp.destroyed) {
            tcp.write(Buffer.from(String(data), 'utf8'))
        }
    })

    ws.on('close', () => {
        if (!tcp.destroyed) tcp.end()
    })

    ws.on('error', () => {
        if (!tcp.destroyed) tcp.destroy()
    })

    tcp.on('data', (chunk) => {
        buffered += chunk.toString('utf8')
        if (ws.readyState === ws.OPEN) {
            ws.send(buffered)
        }
        buffered = ''
    })

    tcp.on('close', () => {
        if (ws.readyState === ws.OPEN) ws.close()
    })

    tcp.on('error', (err) => {
        if (ws.readyState === ws.OPEN) {
            ws.send(`Помилка: ${err.message}`)
            ws.close()
        }
    })
})

console.log(`WebSocket proxy запущено на port ${WS_PORT}`)
console.log(`Підключення до TCP сервера: ${TCP_HOST}:${TCP_PORT}`)