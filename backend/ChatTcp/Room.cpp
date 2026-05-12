#include "Room.h"
#include <algorithm>

void addClient(Room& room, socket_t client, const std::string& name) {
    room.clients.push_back({ client, name });
}

void removeClient(Room& room, socket_t client) {
    room.clients.erase(
        std::remove_if(room.clients.begin(), room.clients.end(),
            [client](const auto& p) { return p.first == client; }),
        room.clients.end()
    );
}

void broadcast(Room& room, const std::string& msg, socket_t sender) {
    for (const auto& p : room.clients) {
        socket_t c = p.first;
        if (c != sender) {
            send(c, msg.c_str(), (int)msg.size(), 0);
        }
    }
}