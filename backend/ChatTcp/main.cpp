#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Room.h"
using namespace std;
#pragma comment(lib, "ws2_32.lib")

vector<Room> rooms;
mutex rooms_mutex;

Room& getOrCreateRoom(const string& name, const string& password = "", int maxUsers = 100) {
    lock_guard<mutex> lock(rooms_mutex);
    for (auto& r : rooms) {
        if (r.name == name)
            return r;
    }
    rooms.push_back({ name, password, maxUsers,{} });
    return rooms.back();
}

void cleanupEmptyRooms() {
    lock_guard<mutex> lock(rooms_mutex);
    rooms.erase(
        remove_if(rooms.begin(), rooms.end(),
            [](const Room& r) {
                return r.name != "general" && r.clients.empty();
            }),
        rooms.end()
    );
}

void handle_client(SOCKET client, string name) {
    char buffer[1024];
    Room* currentRoom = nullptr;

    while (true) {
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            if (currentRoom) {
                removeClient(*currentRoom, client);
                string leave_msg = name + " покинув кімнату " + currentRoom->name;
                broadcast(*currentRoom, leave_msg);
                currentRoom = nullptr;
                cleanupEmptyRooms();
            }
            closesocket(client);
            cout << "Клієнт " << name << " відключився\n";
            break;
        }
        buffer[bytes] = '\0';
        string cmd(buffer);

        // ===== JOIN =====
        if (cmd.rfind("JOIN ", 0) == 0) {
            stringstream ss(cmd.substr(5));
            string roomName, password;
            ss >> roomName >> password;

            lock_guard<mutex> lock(rooms_mutex);
            auto it = find_if(rooms.begin(), rooms.end(),
                [&](Room& r) { return r.name == roomName; });

            if (it == rooms.end()) {
                string err = "Помилка: Кімната " + roomName + " не існує";
                send(client, err.c_str(), (int)err.size(), 0);
                continue;
            }

            Room& room = *it;

            if (!room.password.empty() && room.password != password) {
                string err = "Помилка: Невірний пароль для кімнати " + roomName;
                send(client, err.c_str(), (int)err.size(), 0);
                continue;
            }

            if ((int)room.clients.size() >= room.maxUsers) {
                string err = "Помилка: Кімната " + roomName + " переповнена (" +
                    to_string(room.maxUsers) + " користувачів)";
                send(client, err.c_str(), (int)err.size(), 0);
                continue;
            }

            currentRoom = &room;
            addClient(room, client, name);

            string join_msg = "JOINED " + room.name;
            send(client, join_msg.c_str(), (int)join_msg.size(), 0);
            broadcast(room, name + " приєднався до кімнати " + room.name, client);
            cout << name << " зайшов у " << room.name << endl;
        }

        // ===== CREATE =====
        else if (cmd.rfind("CREATE ", 0) == 0) {
            string roomName, token2, token3;
            string password = "";
            int maxUsers = 0;

            stringstream ss(cmd.substr(7));
            ss >> roomName >> token2 >> token3;

            if (roomName.empty()) {
                string err = "Помилка: треба вказати назву кімнати";
                send(client, err.c_str(), (int)err.size(), 0);
                continue;
            }

            if (token3.empty()) {
                if (token2.empty()) {
                    string err = "Помилка: треба вказати кількість користувачів";
                    send(client, err.c_str(), (int)err.size(), 0);
                    continue;
                }

                if (!all_of(token2.begin(), token2.end(), ::isdigit)) {
                    string err = "Помилка: кількість користувачів має бути числом";
                    send(client, err.c_str(), (int)err.size(), 0);
                    continue;
                }

                maxUsers = stoi(token2);
                password = "";
            }
            else {
                password = token2;
                if (password == "-") password = "";

                if (!all_of(token3.begin(), token3.end(), ::isdigit)) {
                    string err = "Помилка: кількість користувачів має бути числом";
                    send(client, err.c_str(), (int)err.size(), 0);
                    continue;
                }

                maxUsers = stoi(token3);
            }

            if (maxUsers < 1 || maxUsers > 100) {
                string err = "Помилка: кількість користувачів має бути від 1 до 100";
                send(client, err.c_str(), (int)err.size(), 0);
                continue;
            }

            {
                lock_guard<mutex> lock(rooms_mutex);
                auto it = find_if(rooms.begin(), rooms.end(),
                    [&](Room& r) { return r.name == roomName; });

                if (it != rooms.end()) {
                    string err = "Помилка: Кімната з назвою " + roomName + " вже існує";
                    send(client, err.c_str(), (int)err.size(), 0);
                    continue;
                }
            }

            currentRoom = &getOrCreateRoom(roomName, password, maxUsers);
            addClient(*currentRoom, client, name);

            string join_msg = "JOINED " + roomName;
            send(client, join_msg.c_str(), (int)join_msg.size(), 0);

            cout << "[SERVER] Кімната " << roomName
                << (password.empty() ? " (публічна)" : " (приватна)")
                << " створена користувачем " << name << endl;
        }


        // ===== LIST =====
        else if (cmd == "LIST") {
            lock_guard<mutex> lock(rooms_mutex);
            string list_msg = "Доступні кімнати:\n";
            for (auto& r : rooms) {
                list_msg += " - " + r.name;
                if (!r.password.empty()) list_msg += " (приватна)";
                list_msg += " (" + to_string(r.clients.size()) + "/" + to_string(r.maxUsers) + ")\n";
            }
            send(client, list_msg.c_str(), (int)list_msg.size(), 0);
        }

        // ===== LEAVE =====
        else if (cmd == "LEAVE") {
            if (currentRoom) {
                removeClient(*currentRoom, client);
                string leave_msg = "Ви покинули кімнату " + currentRoom->name;
                send(client, leave_msg.c_str(), (int)leave_msg.size(), 0);
                broadcast(*currentRoom, name + " покинув кімнату " + currentRoom->name);
                cout << "[SERVER] " << name << " покинув " << currentRoom->name << endl;
                currentRoom = nullptr;
                cleanupEmptyRooms();
            }
            else {
                string err = "Помилка: Ви не в кімнаті";
                send(client, err.c_str(), (int)err.size(), 0);
            }
        }

        // ===== MESSAGE =====
        else {
            if (currentRoom) {
                string msg = name + ": " + cmd;
                cout << "[SERVER][" << currentRoom->name << "] " << msg << endl;
                broadcast(*currentRoom, msg, client);
            }
            else {
                string warn = "Помилка: Ви не в кімнаті, перед тим як писати повідомлення зайдіть в кімнату";
                send(client, warn.c_str(), (int)warn.size(), 0);
            }
        }
    }
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (sockaddr*)&addr, sizeof(addr));
    listen(server, 5);
    {
        lock_guard<mutex> lock(rooms_mutex);
        rooms.push_back({ "general","",100,{} });
    }
    cout << "Сервер запущено на порту 8080...\n";

    while (true) {
        sockaddr_in client_addr{};
        int client_size = sizeof(client_addr);
        SOCKET client = accept(server, (sockaddr*)&client_addr, &client_size);

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        string client_ip(ip_str);

        char name_buf[256];
        int name_len = recv(client, name_buf, sizeof(name_buf) - 1, 0);
        string name;
        if (name_len > 0) {
            name_buf[name_len] = '\0';
            name = string(name_buf);
        }
        else {
            name = client_ip;
        }

        cout << "Новий клієнт: " << name << endl;
        thread(handle_client, client, name).detach();
    }

    closesocket(server);
    WSACleanup();
}
