import { ArrowLeft } from 'lucide-react'

export default function ChatHeader({ roomName, onLeave, showLeaveButton }) {
    return (
        <div className="flex shrink-0 items-center justify-between bg-white px-4 py-3">
            <div>
                <div className="text-lg font-semibold">{roomName || 'Оберіть кімнату'}</div>
                <div className="text-sm text-slate-500">
                    {roomName ? 'Чат кімнати' : 'Список кімнат'}
                </div>
            </div>

            {showLeaveButton && (
                <button
                    onClick={onLeave}
                    className="flex items-center gap-2 rounded-xl border px-3 py-2 text-slate-700 hover:bg-slate-50"
                >
                    <ArrowLeft size={18} />
                    <span className="hidden sm:inline">Вийти</span>
                </button>
            )}
        </div>
    )
}