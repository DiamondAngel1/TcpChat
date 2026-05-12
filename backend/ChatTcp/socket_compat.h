#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using socket_t = SOCKET;
static const socket_t invalid_socket_v = INVALID_SOCKET;

inline int socket_close(socket_t s) {
    return closesocket(s);
}

inline bool socket_valid(socket_t s) {
    return s != INVALID_SOCKET;
}

#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using socket_t = int;
static const socket_t invalid_socket_v = -1;

inline int socket_close(socket_t s) {
    return close(s);
}

inline bool socket_valid(socket_t s) {
    return s >= 0;
}
#endif