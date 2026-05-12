import Modal from './Modal.jsx'

export default function ErrorModal({ message, onClose }) {
    if (!message) return null
    return (
        <Modal title="Помилка" onClose={onClose}>
            <div className="space-y-4">
                <div className="rounded-xl bg-red-50 px-4 py-3 text-red-600">{message}</div>
                <button onClick={onClose} className="w-full rounded-xl bg-slate-900 py-3 text-white">
                    ОК
                </button>
            </div>
        </Modal>
    )
}