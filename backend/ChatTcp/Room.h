#pragma once
#include <string>
#include <vector>
#include <utility>
#include "socket_compat.h"

struct Room {
    std::string name;
    std::string password;
    int maxUsers;
    std::vector<std::pair<socket_t, std::string>> clients;
};

void addClient(Room& room, socket_t client, const std::string& name);
void removeClient(Room& room, socket_t client);
void broadcast(Room& room, const std::string& msg, socket_t sender = invalid_socket_v);