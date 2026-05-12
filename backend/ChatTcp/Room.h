#pragma once
#include <string>
#include <vector>
#include <winsock2.h>
using namespace std;
struct Room {
    string name;
    string password;
    int maxUsers;
    vector<pair<SOCKET, string>> clients;
};

void addClient(Room& room, SOCKET client, const string& name);
void removeClient(Room& room, SOCKET client);
void broadcast(Room& room, const string& msg, SOCKET sender = INVALID_SOCKET);
