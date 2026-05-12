export default function MessageBubble({ message }) {
    if (message.type === 'system-join') {
        return (
            <div className="flex justify-center">
                <div className="max-w-[75%] rounded-2xl bg-sky-300/30 px-4 py-2 text-center text-sm text-sky-900 backdrop-blur-sm">
                    {message.text}
                </div>
            </div>
        )
    }

    if (message.type === 'system-leave') {
        return (
            <div className="flex justify-center">
                <div className="max-w-[75%] rounded-2xl bg-red-300/30 px-4 py-2 text-center text-sm text-red-900 backdrop-blur-sm">
                    {message.text}
                </div>
            </div>
        )
    }

    return (
        <div className={`flex ${message.mine ? 'justify-end' : 'justify-start'}`}>
            <div
                className={`max-w-[70%] rounded-2xl px-4 py-2 text-sm shadow ${
                    message.mine ? 'bg-sky-500 text-white' : 'bg-white text-slate-800'
                }`}
            >
                {message.text}
            </div>
        </div>
    )
}