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

std::wstring ProcessGetter::PortToProcess(const u_short& port)
{
    MIB_TCPTABLE_OWNER_PID tcpTable;
    DWORD bufferSize = 0;
    if (GetTcpTable2(nullptr, &bufferSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
    {
        PMIB_TCPTABLE2 buffer = (PMIB_TCPTABLE2)new BYTE[bufferSize];
        if (GetTcpTable2(buffer, &bufferSize, TRUE) == NO_ERROR)
        {

            // Iterate through the TCP connections and find the one using the specified port
            for (DWORD i = 0; i < buffer->dwNumEntries; ++i)
            {
                auto tcpRow = buffer->table[i];
                auto test = ntohs(tcpRow.dwLocalPort);//delete this
                if (tcpRow.dwLocalPort == htons(port))//dwLocalPort is in network order
                {
                    delete[] buffer;
                    return IDToProcess(tcpRow.dwOwningPid);
                }
            }
        }
        delete[] buffer;
    }
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
    res = buffer + i + 1;
    return res;
}