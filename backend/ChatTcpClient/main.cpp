#include <string>
#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")

volatile bool inChat = false;
string currentRoom = "";

void receive_messages(SOCKET sock) {
    char buffer[1024];
    while (inChat) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            cout << "З'єднання закрито сервером\n";
            inChat = false;
            break;
        }
        buffer[bytes] = '\0';
        string msg(buffer);

        if (msg.rfind("JOINED ", 0) == 0) {
            currentRoom = msg.substr(7);
            system("cls");
            cout << "Ви приєднались до кімнати '" << currentRoom << "'. Напишіть 'exit room' для виходу\n\n";
        }
        else if (msg.rfind("Ви покинули кімнату ", 0) == 0) {
            inChat = false;
            currentRoom = "";
            system("cls");
            cout << msg << "\n\n";
        }
        else if (msg.rfind("Помилка: ", 0) == 0) {
            cout << msg << "\n";
        }
        else {
            cout << msg << "\n";
        }
    }
}

string get_server_response(SOCKET sock) {
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        cout << "З'єднання закрито сервером\n";
        return "";
    }
    buffer[bytes] = '\0';
    return string(buffer);
}

bool isValidNumber(const string& str, int& result) {
    try {
        size_t pos;
        result = stoi(str, &pos);
        return pos == str.length() && result >= 1 && result <= 100;
    }
    catch (...) {
        return false;
    }
}

void show_menu() {
    system("cls");
    cout << "===== ЧАТ СЕРВЕР =====\n\n";
    cout << "1. Приєднатись до кімнати (JOIN)\n";
    cout << "2. Створити кімнату (CREATE)\n";
    cout << "3. Список кімнат (LIST)\n";
    cout << "4. Вийти з програми\n\n";
    cout << "Ваш вибір (1-4): ";
}

void handle_join(SOCKET sock) {
    cout << "Введіть назву кімнати: ";
    string room;
    getline(cin, room);

    // Для приватних кімнат запитуємо пароль
    cout << "Пароль (Enter якщо публічна): ";
    string password;
    getline(cin, password);
    if (password.empty()) password = ""; // Порожній пароль для публічних

    string cmd = "JOIN " + room + " " + password;
    send(sock, cmd.c_str(), (int)cmd.size(), 0);

    string response = get_server_response(sock);
    if (response.rfind("JOINED ", 0) == 0) {
        inChat = true;
        thread(receive_messages, sock).detach();

        while (inChat) {
            string input;
            getline(cin, input);
            if (input == "exit room") {
                string leave_cmd = "LEAVE";
                send(sock, leave_cmd.c_str(), (int)leave_cmd.size(), 0);
                Sleep(100); // Невелика затримка для обробки LEAVE
                inChat = false;
                break;
            }
            send(sock, input.c_str(), (int)input.size(), 0);
        }
    }
    else {
        cout << response << "\n\nНатисніть Enter для продовження...";
        cin.get();
    }
}

void handle_create(SOCKET sock) {
    cout << "Введіть назву кімнати: ";
    string room;
    getline(cin, room);

    if (room.empty()) {
        cout << "Помилка: назва кімнати не може бути порожньою!\n\nНатисніть Enter...";
        cin.get();
        return;
    }

    cout << "Пароль (Enter для публічної або '-' для без пароля): ";
    string password;
    getline(cin, password);
    if (password == "-") password = "";

    // Перевірка введення максимальної кількості користувачів
    int maxUsers;
    string maxUsersStr;
    while (true) {
        cout << "Макс. користувачів (1-100): ";
        getline(cin, maxUsersStr);
        if (isValidNumber(maxUsersStr, maxUsers)) {
            break;
        }
        cout << "Помилка: введіть число від 1 до 100!\n";
    }

    string cmd = "CREATE " + room + " " + password + " " + maxUsersStr;
    send(sock, cmd.c_str(), (int)cmd.size(), 0);

    string response = get_server_response(sock);
    if (response.rfind("JOINED ", 0) == 0) {
        inChat = true;
        currentRoom = room;
        thread(receive_messages, sock).detach();

        while (inChat) {
            string input;
            getline(cin, input);
            if (input == "exit room") {
                string leave_cmd = "LEAVE";
                send(sock, leave_cmd.c_str(), (int)leave_cmd.size(), 0);
                Sleep(100);
                inChat = false;
                break;
            }
            send(sock, input.c_str(), (int)input.size(), 0);
        }
    }
    else {
        cout << response << "\n\nНатисніть Enter для продовження...";
        cin.get();
    }
}

void handle_list(SOCKET sock) {
    string cmd = "LIST";
    send(sock, cmd.c_str(), (int)cmd.size(), 0);

    string response = get_server_response(sock);
    system("cls");
    cout << "===== СПИСОК КІМНАТ =====\n\n";
    cout << response << "\n\n";
    cout << "Натисніть Enter для продовження...";
    cin.get();
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Не вдалося підключитися до сервера\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    cout << "Підключено до сервера\n";

    string name;
    cout << "Введіть своє ім'я: ";
    getline(cin, name);
    send(sock, name.c_str(), (int)name.size(), 0);

    while (true) {
        show_menu();
        string choice;
        getline(cin, choice);

        if (choice == "1") {
            handle_join(sock);
        }
        else if (choice == "2") {
            handle_create(sock);
        }
        else if (choice == "3") {
            handle_list(sock);
        }
        else if (choice == "4") {
            cout << "До побачення!\n";
            closesocket(sock);
            WSACleanup();
            return 0;
        }
        else {
            cout << "Невірний вибір. Спробуйте ще раз.\n\n";
            cout << "Натисніть Enter...";
            cin.get();
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}