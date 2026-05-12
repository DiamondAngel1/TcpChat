export function parseRoomList(text) {
    const lines = text.split('\n').map(s => s.trim()).filter(Boolean)
    const rooms = []
    for (const line of lines) {
        if (!line.startsWith('- ')) continue
        const nameMatch = line.match(/-\s(.+?)(\s\(приватна\))?\s\((\d+)\/(\d+)\)$/)
        if (!nameMatch) continue
        rooms.push({
            name: nameMatch[1].replace(/\s\(приватна\)/, '').trim(),
            isPrivate: line.includes('(приватна)'),
            current: Number(nameMatch[3]),
            max: Number(nameMatch[4])
        })
    }
    return rooms
}