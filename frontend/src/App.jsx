import { useEffect, useRef, useState, useCallback } from 'react'
import Sidebar from './components/Sidebar'
import ChatHeader from './components/ChatHeader'
import ChatWindow from './components/ChatWindow'
import NameModal from './components/Modals/NameModal'
import PasswordModal from './components/Modals/PasswordModal'
import CreateRoomModal from './components/Modals/CreateRoomModal'
import ErrorModal from './components/Modals/ErrorModal'
import { useSocket } from './hooks/useSocket'
import { parseRoomList } from './utils/protocol'

export default function App() {
    const [name, setName] = useState('')
    const [rooms, setRooms] = useState([])
    const [currentRoom, setCurrentRoom] = useState('')
    const [messages, setMessages] = useState([])
    const [pendingRoom, setPendingRoom] = useState(null)
    const [showCreate, setShowCreate] = useState(false)
    const [error, setError] = useState('')

    const currentRoomRef = useRef('')
    const connectedOnceRef = useRef(false)

    useEffect(() => {
        currentRoomRef.current = currentRoom
    }, [currentRoom])

    const { connect, send } = useSocket({
        onMessage: (data) => {
            const msg = String(data)

            if (msg.startsWith('Доступні кімнати:')) {
                setRooms(parseRoomList(msg))
                return
            }

            if (msg.startsWith('JOINED ')) {
                const room = msg.slice(7).trim()
                currentRoomRef.current = room
                setCurrentRoom(room)
                setMessages([])
                setError('')
                return
            }

            if (msg.startsWith('Помилка: ')) {
                setError(msg)
                return
            }

            if (msg.startsWith('Ви покинули кімнату ')) {
                currentRoomRef.current = ''
                setCurrentRoom('')
                setMessages([])
                setError('')
                return
            }
            if (msg.includes('приєднався до кімнати')) {
                setMessages(prev => [...prev, { text: msg, type: 'system-join' }])
                return
            }

            if (msg.includes('покинув кімнату')) {
                setMessages(prev => [...prev, { text: msg, type: 'system-leave' }])
                return
            }

            if (currentRoomRef.current) {
                setMessages(prev => [...prev, { text: msg, mine: false, type: 'user' }])
            }
        }
    })

    const refreshRooms = useCallback(() => {
        send('LIST')
    }, [send])

    useEffect(() => {
        if (name && !connectedOnceRef.current) {
            connectedOnceRef.current = true
            connect(name)
        }
    }, [name, connect])

    useEffect(() => {
        if (name) {
            const t = setTimeout(() => {
                refreshRooms()
            }, 300)
            return () => clearTimeout(t)
        }
    }, [name, refreshRooms])

    const handleSelectRoom = (room) => {
        if (currentRoomRef.current && currentRoomRef.current !== room.name) {
            send('LEAVE')
            currentRoomRef.current = ''
            setCurrentRoom('')
            setMessages([])
            setTimeout(refreshRooms, 300)
        }

        if (room.isPrivate) {
            setPendingRoom(room)
        } else {
            send(`JOIN ${room.name}`)
        }
    }

    const joinWithPassword = (password) => {
        if (!pendingRoom) return
        send(`JOIN ${pendingRoom.name} ${password}`)
        setPendingRoom(null)
    }

    const createRoom = ({ name: roomName, password, maxUsers }) => {
        send(`CREATE ${roomName} ${password || '-'} ${maxUsers}`)
        setShowCreate(false)
        setTimeout(refreshRooms, 300)
    }

    const handleLeaveRoom = () => {
        if (!currentRoomRef.current) return
        send('LEAVE')
        currentRoomRef.current = ''
        setCurrentRoom('')
        setMessages([])
        setTimeout(refreshRooms, 300)
    }

    const handleSendMessage = (text) => {
        if (!currentRoomRef.current) return
        send(text)
        setMessages(prev => [...prev, { text, mine: true }])
    }

    return (
        <div className="flex h-screen w-screen overflow-hidden bg-slate-100">
            {!name && <NameModal onSubmit={setName} />}
            {error && <ErrorModal message={error} onClose={() => setError('')} />}
            {pendingRoom && (
                <PasswordModal
                    roomName={pendingRoom.name}
                    onSubmit={joinWithPassword}
                    onCancel={() => setPendingRoom(null)}
                />
            )}
            {showCreate && (
                <CreateRoomModal onSubmit={createRoom} onCancel={() => setShowCreate(false)} />
            )}

            <div className="flex w-full flex-1 ">
                <div
                    className={`flex h-full w-full flex-col rounded-2xl bg-white shadow-sm md:w-[340px] md:shrink-0 md:flex-none min-h-0 ${
                        currentRoom ? 'hidden md:flex' : 'flex'
                    }`}
                >
                    <Sidebar
                        rooms={rooms}
                        currentRoom={currentRoom}
                        onSelectRoom={handleSelectRoom}
                        onCreate={() => setShowCreate(true)}
                        onRefresh={refreshRooms}
                    />
                </div>

                <div
                    className={`flex h-full w-full flex-1 min-w-0 min-h-0 flex-col overflow-hidden rounded-2xl bg-white shadow-sm ${
                        currentRoom ? 'flex' : 'hidden md:flex'
                    }`}
                >
                    <ChatHeader
                        roomName={currentRoom}
                        onLeave={handleLeaveRoom}
                        showLeaveButton={!!currentRoom}
                    />
                    <ChatWindow roomName={currentRoom} messages={messages} onSend={handleSendMessage} />
                </div>
            </div>
        </div>
    )
}