import { useState } from 'react'
import MessageBubble from './MessageBubble'

export default function ChatWindow({ roomName, messages, onSend }) {
    const [text, setText] = useState('')

    const submit = (e) => {
        e.preventDefault()
        if (!text.trim()) return
        onSend(text.trim())
        setText('')
    }

    return (
        <div className="flex h-full w-full flex-1 flex-col min-h-0 bg-[url('/chat-bg.png')] bg-cover bg-center">
            <div className="flex-1 min-h-0 overflow-y-auto p-4 space-y-3 bg-emerald-50/50">
                {messages.map((m, idx) => (
                    <MessageBubble key={idx} message={m} />
                ))}
            </div>

            <form onSubmit={submit} className="shrink-0 bg-white p-4">
                <div className="flex gap-3">
                    <input
                        className="flex-1 rounded-xl border px-4 py-3 outline-none focus:border-sky-500"
                        value={text}
                        onChange={(e) => setText(e.target.value)}
                        placeholder={roomName ? 'Напишіть повідомлення...' : 'Спочатку виберіть кімнату'}
                        disabled={!roomName}
                    />
                    <button
                        disabled={!roomName}
                        className="rounded-xl bg-sky-500 px-5 py-3 text-white disabled:opacity-50"
                    >
                        Надіслати
                    </button>
                </div>
            </form>
        </div>
    )
}