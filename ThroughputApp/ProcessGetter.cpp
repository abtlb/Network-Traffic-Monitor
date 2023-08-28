#include "ProcessGetter.h"
#include <psapi.h>

void ProcessGetter::InitializeWinsock()
{
    // Initialize Winsock
    WSADATA wsaData;//contains information about the winsock implementation
    WORD version = MAKEWORD(2, 2);//v2.2
    int success = WSAStartup(version, &wsaData);
    if (success != 0)
    {
        throw new std::exception();
    }
}

ProcessGetter::ProcessGetter()
{
    InitializeWinsock();
    lastChecked = 0;
    checkingInterval = 1000;//every 1000ms
}

ProcessGetter::~ProcessGetter()
{
    delete[] buffer;
}

std::wstring ProcessGetter::PortToProcess(const u_short& port)
{
    int currTime = clock();
    if (currTime - lastChecked >= checkingInterval)
    {
        DWORD bufferSize = 0;
        GetTcpTable2(nullptr, &bufferSize, TRUE);//will fail and set bufferSize to the proper buffer size
        buffer = (PMIB_TCPTABLE2)new BYTE[bufferSize];
        GetTcpTable2(buffer, &bufferSize, TRUE);//get the tcp table
        lastChecked = currTime;
    }

    if (buffer == NULL)
    {
        return L"a7a";
    }

    // Iterate through the TCP connections and find the one using the specified port
    for (DWORD i = 0; i < buffer->dwNumEntries; ++i)
    {
        auto tcpRow = buffer->table[i];
        if (tcpRow.dwLocalPort == htons(port))//dwLocalPort is in network order
        {
            return IDToProcess(tcpRow.dwOwningPid);
        }
    }
    return L"a7a";//pid not found
}

std::wstring ProcessGetter::IDToProcess(u_short pid)
{
    std::wstring res;
    if (pid == 0)
    {
        res = L"System";
        return res;
    }

    HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameEx(handle, NULL, buffer, MAX_PATH);
    int i;
    for (i = 259; i > 0; i--)
    {
        if (buffer[i] == '\\')
        {
            break;
        }
    }

    //only include the process name
    wchar_t* man;
    man = _wcsdup(buffer + i + 1);
    return man;
}