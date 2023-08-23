#include "ProcessGetter.h"

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

wchar_t* ProcessGetter::PortToProcess(const u_short& port)
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

wchar_t* ProcessGetter::IDToProcess(u_short pid)
{
    std::string processName;
    auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        std::cerr << "err";
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (entry.th32ProcessID == pid)
            {
                //std::wstring wStr = (std::wstring)(entry.szExeFile);
                //wStr.c
                //std::string str(wStr.begin(), wStr.end());
                return entry.szExeFile;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return nullptr;
}