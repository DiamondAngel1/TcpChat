import { RefreshCw, Plus } from 'lucide-react'
import RoomItem from './RoomItem'

export default function Sidebar({ rooms, currentRoom, onSelectRoom, onCreate, onRefresh }) {
    return (
        <div className="flex h-full min-h-0 w-full flex-col bg-white">
            <div className="shrink-0 flex items-center row p-4 space-y-3 ">
                <button
                    onClick={onCreate}
                    className="flex w-full items-center justify-center gap-2 m-1 rounded-xl bg-sky-500 px-4 py-3 font-medium text-white hover:bg-sky-600"
                >
                    <Plus size={18} />
                    Створити кімнату
                </button>

                <button
                    onClick={onRefresh}
                    className="flex h-min items-center justify-center gap-2 m-1 rounded-xl border px-4 py-3 font-medium text-slate-700 hover:bg-slate-50"
                >
                    <RefreshCw size={15} />
                </button>
            </div>

            <div className="flex-1 min-h-0 overflow-y-auto p-3 space-y-2">
                {rooms.map((room) => (
                    <RoomItem
                        key={room.name}
                        room={room}
                        active={currentRoom === room.name}
                        onClick={() => onSelectRoom(room)}
                    />
                ))}
            </div>
        </div>
    )
}