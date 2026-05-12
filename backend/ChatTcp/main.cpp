#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
#include <clocale>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "Room.h"

using namespace std;

vector<Room> rooms;
mutex rooms_mutex;

Room& getOrCreateRoom(const string& name, const string& password = "", int maxUsers = 100) {
    lock_guard<mutex> lock(rooms_mutex);
    for (auto& r : rooms) {
        if (r.name == name) return r;
    }
    rooms.push_back({ name, password, maxUsers, {} });
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

static void sendText(socket_t client, const string& text) {
    send(client, text.c_str(), (int)text.size(), 0);
}

void handle_client(socket_t client, string name) {
    char buffer[2048];
    Room* currentRoom = nullptr;

    while (true) {
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            if (currentRoom) {
                removeClient(*currentRoom, client);
                string leave_msg = name + " покинув кімнату " + currentRoom->name;
                broadcast(*currentRoom, leave_msg, client);
                currentRoom = nullptr;
                cleanupEmptyRooms();
            }
            socket_close(client);
            cout << "Клієнт " << name << " відключився" << endl;
            break;
        }

        buffer[bytes] = '\0';
        string cmd(buffer);
        while (!cmd.empty() && (cmd.back() == '\n' || cmd.back() == '\r')) cmd.pop_back();

        if (cmd.rfind("JOIN ", 0) == 0) {
            stringstream ss(cmd.substr(5));
            string roomName, password;
            ss >> roomName >> password;

            lock_guard<mutex> lock(rooms_mutex);
            auto it = find_if(rooms.begin(), rooms.end(),
                [&](Room& r) { return r.name == roomName; });

            if (it == rooms.end()) {
                sendText(client, "Помилка: Кімната " + roomName + " не існує");
                continue;
            }

            Room& room = *it;

            if (!room.password.empty() && room.password != password) {
                sendText(client, "Помилка: Невірний пароль для кімнати " + roomName);
                continue;
            }

            if ((int)room.clients.size() >= room.maxUsers) {
                sendText(client, "Помилка: Кімната " + roomName + " переповнена (" +
                    to_string(room.maxUsers) + " користувачів)");
                continue;
            }

            currentRoom = &room;
            addClient(room, client, name);

            sendText(client, "JOINED " + room.name);
            broadcast(room, name + " приєднався до кімнати " + room.name, client);
            cout << name << " зайшов у " << room.name << endl;
        }
        else if (cmd.rfind("CREATE ", 0) == 0) {
            string roomName, token2, token3;
            string password = "";
            int maxUsers = 0;

            stringstream ss(cmd.substr(7));
            ss >> roomName >> token2 >> token3;

            if (roomName.empty()) {
                sendText(client, "Помилка: треба вказати назву кімнати");
                continue;
            }

            if (token3.empty()) {
                if (token2.empty()) {
                    sendText(client, "Помилка: треба вказати кількість користувачів");
                    continue;
                }
                if (!all_of(token2.begin(), token2.end(), ::isdigit)) {
                    sendText(client, "Помилка: кількість користувачів має бути числом");
                    continue;
                }
                maxUsers = stoi(token2);
                password = "";
            }
            else {
                password = token2;
                if (password == "-") password = "";
                if (!all_of(token3.begin(), token3.end(), ::isdigit)) {
                    sendText(client, "Помилка: кількість користувачів має бути числом");
                    continue;
                }
                maxUsers = stoi(token3);
            }

            if (maxUsers < 1 || maxUsers > 100) {
                sendText(client, "Помилка: кількість користувачів має бути від 1 до 100");
                continue;
            }

            {
                lock_guard<mutex> lock(rooms_mutex);
                auto it = find_if(rooms.begin(), rooms.end(),
                    [&](Room& r) { return r.name == roomName; });

                if (it != rooms.end()) {
                    sendText(client, "Помилка: Кімната з назвою " + roomName + " вже існує");
                    continue;
                }
            }

            currentRoom = &getOrCreateRoom(roomName, password, maxUsers);
            addClient(*currentRoom, client, name);

            sendText(client, "JOINED " + roomName);

            cout << "[SERVER] Кімната " << roomName
                << (password.empty() ? " (публічна)" : " (приватна)")
                << " створена користувачем " << name << endl;
        }
        else if (cmd == "LIST") {
            lock_guard<mutex> lock(rooms_mutex);
            string list_msg = "Доступні кімнати:\n";
            for (auto& r : rooms) {
                list_msg += " - " + r.name;
                if (!r.password.empty()) list_msg += " (приватна)";
                list_msg += " (" + to_string(r.clients.size()) + "/" + to_string(r.maxUsers) + ")\n";
            }
            sendText(client, list_msg);
        }
        else if (cmd == "LEAVE") {
            if (currentRoom) {
                removeClient(*currentRoom, client);
                sendText(client, "Ви покинули кімнату " + currentRoom->name);
                broadcast(*currentRoom, name + " покинув кімнату " + currentRoom->name, client);
                cout << "[SERVER] " << name << " покинув " << currentRoom->name << endl;
                currentRoom = nullptr;
                cleanupEmptyRooms();
            }
            else {
                sendText(client, "Помилка: Ви не в кімнаті");
            }
        }
        else {
            if (currentRoom) {
                string msg = name + ": " + cmd;
                cout << "[SERVER][" << currentRoom->name << "] " << msg << endl;
                broadcast(*currentRoom, msg, client);
            }
            else {
                sendText(client, "Помилка: Ви не в кімнаті, перед тим як писати повідомлення зайдіть в кімнату");
            }
        }
    }
}

int main() {

#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U8TEXT);
    _setmode(_fileno(stdin), _O_U8TEXT);
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

#endif

    setlocale(LC_ALL, "");

    socket_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (!socket_valid(server)) {
        cerr << "Не вдалося створити socket\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;

    int port = getenv("PORT") ? atoi(getenv("PORT")) : 8080;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "bind failed\n";
        socket_close(server);
        return 1;
    }

    if (listen(server, 5) < 0) {
        cerr << "listen failed\n";
        socket_close(server);
        return 1;
    }

    {
        lock_guard<mutex> lock(rooms_mutex);
        rooms.push_back({ "general", "", 100, {} });
    }

    cout << "Сервер запущено на порту " << port << "...\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_size = sizeof(client_addr);

        socket_t client = accept(server, (sockaddr*)&client_addr, &client_size);
        if (!socket_valid(client)) continue;

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        string client_ip(ip_str);

        char name_buf[256];
        int name_len = recv(client, name_buf, sizeof(name_buf) - 1, 0);
        string name;
        if (name_len > 0) {
            name_buf[name_len] = '\0';
            name = string(name_buf);
            while (!name.empty() && (name.back() == '\n' || name.back() == '\r')) name.pop_back();
        }
        else {
            name = client_ip;
        }

        cout << "Новий клієнт: " << name << endl;
        thread(handle_client, client, name).detach();
    }

    socket_close(server);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}