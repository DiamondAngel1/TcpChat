import { useState } from 'react'
import Modal from "./Modal.jsx";

export default function NameModal({ onSubmit }) {
    const [name, setName] = useState('')

    return (
        <Modal title="Введіть ім'я" onClose={() => {}}>
            <form
                onSubmit={(e) => {
                    e.preventDefault()
                    if (!name.trim()) return
                    onSubmit(name.trim())
                }}
                className="space-y-4"
            >
                <input
                    className="w-full rounded-xl border px-4 py-3 outline-none focus:border-sky-500"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                    placeholder="Ваше ім'я"
                />
                <button className="w-full rounded-xl bg-sky-500 py-3 text-white hover:bg-sky-600">
                    Продовжити
                </button>
            </form>
        </Modal>
    )
}