#include "Room.h"
#include <algorithm>

void addClient(Room& room, SOCKET client, const std::string& name) {
    room.clients.push_back({ client, name });
}

void removeClient(Room& room, SOCKET client) {
    room.clients.erase(
        std::remove_if(room.clients.begin(), room.clients.end(),
            [client](auto& p) { return p.first == client; }),
        room.clients.end()
    );
}

void broadcast(Room& room, const std::string& msg, SOCKET sender) {
    for (size_t i = 0; i < room.clients.size(); ++i) {
        SOCKET c = room.clients[i].first;
        if (c != sender) {
            send(c, msg.c_str(), (int)msg.size(), 0);
        }
    }
}
