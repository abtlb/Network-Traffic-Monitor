#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <TlHelp32.h>
#include <unordered_map>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

class ProcessGetter
{
public:
	std::wstring PortToProcess(const u_short& port);
	~ProcessGetter();
	ProcessGetter();
private:
	void InitializeWinsock();
	std::wstring IDToProcess(u_short id);
	PMIB_TCPTABLE2 buffer;
	int checkingInterval;
	int lastChecked;
	std::unordered_map<u_short, std::wstring> portToProcess;
};