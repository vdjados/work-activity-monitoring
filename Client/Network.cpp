#include "Config.h"
#include "Network.h"
#include <windows.h>
#include <ws2tcpip.h>
#include <sstream>
#include <stdexcept>
#include <iostream>

bool Network::Send(const ClientInfo& info, const std::vector<BYTE>& screenshot) {
    SOCKET sock = INVALID_SOCKET;
    try {
        WSAData wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            throw std::runtime_error("WSAStartup failed");

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
            throw std::runtime_error("socket() failed");

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_HOST, &addr.sin_addr);

        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
            throw std::runtime_error("connect() failed");

        // Формируем JSON-заголовок
        std::ostringstream os;
        os << "{"
            << "\"domain\":\"" << info.domain << "\","
            << "\"machine\":\"" << info.machine << "\","
            << "\"user\":\"" << info.user << "\","
            << "\"idle\":" << info.idleSeconds
            << "}";
        std::string hdr = os.str();

        // Отправка заголовка
        int hdr_len = static_cast<int>(hdr.size());
        send(sock, reinterpret_cast<char*>(&hdr_len), sizeof(hdr_len), 0);
        send(sock, hdr.c_str(), hdr_len, 0);

        // Отправка изображения
        int scr_len = static_cast<int>(screenshot.size());
        send(sock, reinterpret_cast<char*>(&scr_len), sizeof(scr_len), 0);
        send(sock, reinterpret_cast<const char*>(screenshot.data()), scr_len, 0);

        closesocket(sock);
        WSACleanup();
        return true;
    }
    catch (const std::exception& ex) {
        std::ostringstream log;
        log << "[Network] " << ex.what() << "\n";
        OutputDebugStringA(log.str().c_str());
        if (sock != INVALID_SOCKET) closesocket(sock);
        WSACleanup();
        return false;
    }
}
