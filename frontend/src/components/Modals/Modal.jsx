export default function Modal({ title, children, onClose }) {
    return (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/50 p-4">
            <div className="w-full max-w-md rounded-2xl bg-white shadow-xl">
                <div className="flex items-center justify-between border-b px-5 py-4">
                    <h2 className="text-lg font-semibold">{title}</h2>
                    <button className="text-slate-500 hover:text-slate-800" onClick={onClose}>✕</button>
                </div>
                <div className="p-5">{children}</div>
            </div>
        </div>
    )
}