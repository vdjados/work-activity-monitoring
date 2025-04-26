#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include "ActivityMonitor.h"

namespace Network {
    bool Send(const ClientInfo& info, const std::vector<BYTE>& screenshot);
}