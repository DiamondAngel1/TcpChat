import { Lock, Globe } from 'lucide-react'

export default function RoomItem({ room, active, onClick }) {
    return (
        <button
            onClick={onClick}
            className={`flex w-full items-center justify-between rounded-xl px-3 py-3 text-left transition ${
                active ? 'bg-sky-100' : 'hover:bg-slate-100'
            }`}
        >
            <div className="min-w-0">
                <div className="truncate font-medium">{room.name}</div>
                <div className="text-xs text-slate-500">{room.current}/{room.max}</div>
            </div>
            {room.isPrivate ? (
                <Lock size={16} className="shrink-0 text-slate-500" />
            ) : (
                <Globe size={16} className="shrink-0 text-slate-500" />
            )}
        </button>
    )
}