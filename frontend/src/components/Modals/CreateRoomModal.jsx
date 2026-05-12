import { useState } from 'react'
import Modal from './Modal.jsx'

export default function CreateRoomModal({ onSubmit, onCancel }) {
    const [name, setName] = useState('')
    const [password, setPassword] = useState('')
    const [maxUsers, setMaxUsers] = useState('10')
    const [error, setError] = useState('')

    const validate = () => {
        if (!name.trim()) return 'Назва кімнати обовʼязкова'
        if (!/^\d+$/.test(maxUsers)) return 'Максимум користувачів має бути числом'
        const n = Number(maxUsers)
        if (n < 1 || n > 100) return 'Максимум користувачів має бути від 1 до 100'
        return ''
    }

    return (
        <Modal title="Створити кімнату" onClose={onCancel}>
            <form
                className="space-y-4"
                onSubmit={(e) => {
                    e.preventDefault()
                    const err = validate()
                    if (err) {
                        setError(err)
                        return
                    }
                    onSubmit({
                        name: name.trim(),
                        password: password.trim(),
                        maxUsers: Number(maxUsers)
                    })
                }}
            >
                <input
                    className="w-full rounded-xl border px-4 py-3 outline-none focus:border-sky-500"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                    placeholder="Назва кімнати"
                />
                <input
                    className="w-full rounded-xl border px-4 py-3 outline-none focus:border-sky-500"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    placeholder="Пароль (порожньо — публічна кімната)"
                />
                <input
                    className="w-full rounded-xl border px-4 py-3 outline-none focus:border-sky-500"
                    value={maxUsers}
                    onChange={(e) => setMaxUsers(e.target.value)}
                    placeholder="Максимум користувачів"
                />
                {error && <div className="rounded-xl bg-red-50 px-4 py-3 text-sm text-red-600">{error}</div>}
                <div className="flex gap-2">
                    <button type="button" onClick={onCancel} className="flex-1 rounded-xl border py-3">
                        Скасувати
                    </button>
                    <button className="flex-1 rounded-xl bg-sky-500 py-3 text-white hover:bg-sky-600">
                        Створити
                    </button>
                </div>
            </form>
        </Modal>
    )
}