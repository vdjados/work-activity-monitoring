#include "Config.h"
#include "Network.h"
#include <windows.h>
#include <ws2tcpip.h>
#include <sstream>
#include <stdexcept>
#include <iostream>

static std::string ToUtf8(const std::string& ansi) {
    // 1) ANSI → wide
    int wlen = MultiByteToWideChar(CP_ACP, 0, ansi.data(), (int)ansi.size(), nullptr, 0);
    std::vector<wchar_t> wbuf(wlen);
    MultiByteToWideChar(CP_ACP, 0, ansi.data(), (int)ansi.size(), wbuf.data(), wlen);

    // 2) wide → UTF-8
    int ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wlen, nullptr, 0, nullptr, nullptr);
    std::vector<char> ubuf(ulen);
    WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wlen, ubuf.data(), ulen, nullptr, nullptr);

    return std::string(ubuf.data(), ubuf.data() + ulen);
}

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

        std::string hdrAnsi = os.str();              // ваш старый JSON в ANSI
        std::string hdr = ToUtf8(hdrAnsi);           // теперь корректный UTF-8
        uint32_t hdr_len = (uint32_t)hdr.size();
        uint32_t net_hdr_len = htonl(hdr_len);
        send(sock, reinterpret_cast<char*>(&net_hdr_len), sizeof(net_hdr_len), 0);
        send(sock, hdr.data(), hdr_len, 0);

        // Отправка изображения
        uint32_t scr_len = static_cast<uint32_t>(screenshot.size());
        uint32_t net_scr_len = htonl(scr_len); // <-- преобразуем в network byte order
        send(sock, reinterpret_cast<char*>(&net_scr_len), sizeof(net_scr_len), 0);
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
