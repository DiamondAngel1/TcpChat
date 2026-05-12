import { useState } from 'react'
import Modal from './Modal.jsx'

export default function PasswordModal({ roomName, onSubmit, onCancel }) {
    const [password, setPassword] = useState('')

    return (
        <Modal title={`Пароль для ${roomName}`} onClose={onCancel}>
            <form
                onSubmit={(e) => {
                    e.preventDefault()
                    onSubmit(password)
                }}
                className="space-y-4"
            >
                <input
                    type="password"
                    className="w-full rounded-xl border px-4 py-3 outline-none focus:border-sky-500"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    placeholder="Введіть пароль"
                />
                <div className="flex gap-2">
                    <button type="button" onClick={onCancel} className="flex-1 rounded-xl border py-3">
                        Скасувати
                    </button>
                    <button className="flex-1 rounded-xl bg-sky-500 py-3 text-white hover:bg-sky-600">
                        Увійти
                    </button>
                </div>
            </form>
        </Modal>
    )
}