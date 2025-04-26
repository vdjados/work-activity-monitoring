#pragma once
#include <string>

struct ClientInfo {
    std::string domain;
    std::string machine;
    std::string user;
    unsigned long idleSeconds;
};

class ActivityMonitor {
public:
    static ClientInfo GetInfo();
};