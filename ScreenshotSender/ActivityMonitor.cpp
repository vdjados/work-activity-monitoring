#include "ActivityMonitor.h"
#include <windows.h>
#include <Lmcons.h>

ClientInfo ActivityMonitor::GetInfo() {
    ClientInfo ci;
    char buf[MAX_COMPUTERNAME_LENGTH + 1]; DWORD size = sizeof(buf);
    GetComputerNameA(buf, &size);
    ci.machine = buf;

    char user[UNLEN + 1]; size = sizeof(user);
    GetUserNameA(user, &size);
    ci.user = user;

    // Домен можно получить через GetEnvironmentVariable
    char domain[256]; GetEnvironmentVariableA("USERDOMAIN", domain, 256);
    ci.domain = domain;

    LASTINPUTINFO li{ sizeof(li) };
    GetLastInputInfo(&li);
    ci.idleSeconds = (GetTickCount() - li.dwTime) / 1000;

    return ci;
}