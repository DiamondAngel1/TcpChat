import { WebSocketServer } from 'ws'
import net from 'net'

const WS_PORT = 8081
const TCP_HOST = '127.0.0.1'
const TCP_PORT = 8080

const wss = new WebSocketServer({ port: WS_PORT })

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

        // Важливо: якщо сервер шле кілька повідомлень підряд,
        // тут просто відправляємо весь текст далі як є.
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

console.log(`WebSocket proxy запущено на ws://127.0.0.1:${WS_PORT}`)
console.log(`Підключення до TCP сервера: ${TCP_HOST}:${TCP_PORT}`)